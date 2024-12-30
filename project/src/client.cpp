#include "client.h"
#include "coordinator.grpc.pb.h"

#include <asio.hpp>
#include <thread>
namespace ECProject
{
  std::string Client::sayHelloToCoordinatorByGrpc(std::string hello)
  {
    coordinator_proto::RequestToCoordinator request;
    request.set_name(hello);
    coordinator_proto::ReplyFromCoordinator reply;
    grpc::ClientContext context;
    grpc::Status status = m_coordinator_ptr->sayHelloToCoordinator(&context, request, &reply);
    if (status.ok())
    {
      return reply.message();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }
  // grpc, set the parameters stored in the variable of m_encode_parameters in coordinator
  bool Client::SetParameterByGrpc(ECSchema input_ecschema)
  {
    int k = input_ecschema.k_datablock;
    int l = input_ecschema.l_localparityblock;
    int g_m = input_ecschema.g_m_globalparityblock;
    int b = input_ecschema.b_datapergroup;
    EncodeType encodetype = input_ecschema.encodetype;
    int m = b % (g_m + 1);
    if (b != k / l || (m != 0 && encodetype == Azure_LRC && g_m % m != 0))
    {
      std::cout << "Set parameters failed! Illegal parameters!" << std::endl;
      exit(0);
    }
    coordinator_proto::Parameter parameter;
    parameter.set_partial_decoding((int)input_ecschema.partial_decoding);
    parameter.set_encodetype(encodetype);
    parameter.set_s_stripe_placementtype(input_ecschema.s_stripe_placementtype);
    parameter.set_m_stripe_placementtype(input_ecschema.m_stripe_placementtype);
    parameter.set_k_datablock(k);
    parameter.set_l_localparityblock(l);
    parameter.set_g_m_globalparityblock(g_m);
    parameter.set_b_datapergroup(b);
    parameter.set_x_stripepermergegroup(input_ecschema.x_stripepermergegroup);
    grpc::ClientContext context;
    coordinator_proto::RepIfSetParaSuccess reply;
    grpc::Status status = m_coordinator_ptr->setParameter(&context, parameter, &reply);
    if (status.ok())
    {
      return reply.ifsetparameter();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return false;
    }
  }
  /*
    Function: set
    1. send the set request including the information of key and valuesize to the coordinator
    2. get the address of proxy
    3. send the value to the proxy by socket
  */
  bool Client::set(std::string key, std::string value)
  {
    grpc::ClientContext get_proxy_ip_port;
    coordinator_proto::RequestProxyIPPort request;
    coordinator_proto::ReplyProxyIPPort reply;
    request.set_key(key);
    request.set_valuesizebytes(value.size());
    grpc::Status status = m_coordinator_ptr->uploadOriginKeyValue(&get_proxy_ip_port, request, &reply);
    if (!status.ok())
    {
      std::cout << "[SET] upload data failed!" << std::endl;
      return false;
    }
    else
    {
      std::string proxy_ip = reply.proxyip();
      int proxy_port = reply.proxyport();
      std::cout << "[SET] Send " << key << " to proxy_address:" << proxy_ip << ":" << proxy_port << std::endl;
      // read to send the value
      asio::io_context io_context;
      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(proxy_ip, std::to_string(proxy_port));
      asio::ip::tcp::socket sock_data(io_context);
      asio::connect(sock_data, endpoints);

      // std::cout << "[SET] key_size:" << key.size() << ", value_size:" << value.size();
      // std::cout << ", proxy_address:" << proxy_ip << ":" << proxy_port << std::endl;
      asio::write(sock_data, asio::buffer(key, key.size()), error);
      asio::write(sock_data, asio::buffer(value, value.size()), error);
      asio::error_code ignore_ec;
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      sock_data.close(ignore_ec);

      // check if metadata is saved successfully
      grpc::ClientContext check_commit;
      coordinator_proto::AskIfSuccess request;
      request.set_key(key);
      OpperateType opp = SET;
      request.set_opp(opp);
      coordinator_proto::RepIfSuccess reply;
      grpc::Status status;
      status = m_coordinator_ptr->checkCommitAbort(&check_commit, request, &reply);
      if (status.ok())
      {
        if (reply.ifcommit())
        {
          return true;
        }
        else
        {
          std::cout << "[SET] " << key << " not commit!!!!!" << std::endl;
        }
      }
      else
      {
        std::cout << "[SET] " << key << " Fail to check!!!!!" << std::endl;
      }
    }
    return false;
  }

  void Client::encode()
  {
  }

  bool Client::append(int append_size)
  {
    int tmp_append_size = append_size;
    // align to aligned size
    tmp_append_size = (tmp_append_size + m_sys_config->AlignedSize - 1) / m_sys_config->AlignedSize * m_sys_config->AlignedSize;

    while (tmp_append_size > 0)
    {
      int sub_append_size = std::min(tmp_append_size, m_sys_config->BlockSize * m_sys_config->k - m_append_logical_offset);
      bool if_append_success = sub_append(sub_append_size);
      if (!if_append_success)
      {
        std::cout << "[APPEND148] Sub append failed with sub append size " << sub_append_size << "!" << std::endl;
        return false;
      }
      tmp_append_size -= sub_append_size;
    }

    return true;
  }

  /*
    Function: append within a block stripe
    1. send the append request including the information of the value to the coordinator
    2. get the address of proxy
    3. send the value to the proxy by socket
  */
  bool Client::sub_append(int append_size)
  {
    grpc::ClientContext get_proxy_ip_port;
    coordinator_proto::RequestProxyIPPort request;
    coordinator_proto::ReplyProxyIPsPorts reply;
    request.set_key(m_clientID);
    request.set_valuesizebytes(append_size);
    grpc::Status status = m_coordinator_ptr->uploadAppendValue(&get_proxy_ip_port, request, &reply);

    auto async_append_to_proxies = [this](char *cluster_slice_data, std::string append_key, int cluster_slice_size, std::string proxy_ip, int proxy_port, int index, bool *if_commit_arr)
    {
      std::cout << "[Append174] Appending size " << cluster_slice_size << " to proxy_address:" << proxy_ip << ":" << proxy_port << std::endl;
      asio::io_context io_context;
      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(proxy_ip, std::to_string(proxy_port));
      asio::ip::tcp::socket sock_data(io_context);
      asio::connect(sock_data, endpoints);

      asio::write(sock_data, asio::buffer(cluster_slice_data, cluster_slice_size), error);
      asio::error_code ignore_ec;
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      sock_data.close(ignore_ec);

      // check if metadata is saved successfully
      grpc::ClientContext check_commit;
      coordinator_proto::AskIfSuccess request;
      request.set_key(append_key);
      OpperateType opp = APPEND;
      request.set_opp(opp);
      coordinator_proto::RepIfSuccess reply;
      grpc::Status status;
      status = m_coordinator_ptr->checkCommitAbort(&check_commit, request, &reply);
      if (status.ok())
      {
        if (reply.ifcommit())
        {
          if_commit_arr[index] = true;
        }
        else
        {
          std::cout << "[APPEND205] " << append_key << " not commit!!!!!" << " cluster_slice_size: " << cluster_slice_size << " proxy_ip: " << proxy_ip << " proxy_port: " << proxy_port << std::endl;
        }
      }
      else
      {
        std::cout << "[APPEND210] " << append_key << " Fail to check!!!!!" << " cluster_slice_size: " << cluster_slice_size << " proxy_ip: " << proxy_ip << " proxy_port: " << proxy_port << std::endl;
      }
    };

    if (!status.ok())
    {
      std::cout << "[APPEND216] upload data failed!" << std::endl;
      return false;
    }
    else
    {
      std::vector<std::thread> threads;
      std::vector<char *> cluster_slice_data = m_toolbox->splitCharPointer(m_pre_allocated_buffer, &reply);
      std::unique_ptr<bool[]> if_commit_arr(new bool[reply.append_keys_size()]);
      std::fill_n(if_commit_arr.get(), reply.append_keys_size(), false);

      // TODO: add encode interface
      encode();

      for (int i = 0; i < reply.append_keys_size(); i++)
      {
        threads.push_back(std::thread(async_append_to_proxies, cluster_slice_data[i], reply.append_keys(i), reply.cluster_slice_sizes(i), reply.proxyips(i), reply.proxyports(i), i, if_commit_arr.get()));
      }
      for (auto &thread : threads)
      {
        thread.join();
      }

      // check if all appends are successful
      bool all_true = std::all_of(if_commit_arr.get(), if_commit_arr.get() + reply.append_keys_size(), [](bool val)
                                  { return val == true; });

      if (all_true)
      {
        std::cout << "[APPEND244] Client " << m_clientID << " append " << append_size << " bytes successfully!" << std::endl;
        m_append_logical_offset = (m_append_logical_offset + append_size) % (m_sys_config->BlockSize * m_sys_config->k);
        return true;
      }
      else
      {
        return false;
      }
    }

    return false;
  }
  /*
    Function: get
    1. send the get request including the information of key and clientipport to the coordinator
    2. accept the value transferred from the proxy
  */
  bool Client::get(std::string key, std::string &value)
  {
    grpc::ClientContext context;
    coordinator_proto::KeyAndClientIP request;
    request.set_key(key);
    request.set_clientip(m_clientIPForGet);
    request.set_clientport(m_clientPortForGet);
    // request
    coordinator_proto::RepIfGetSuccess reply;
    grpc::Status status = m_coordinator_ptr->getValue(&context, request, &reply);
    asio::ip::tcp::socket socket_data(io_context);
    int value_size = reply.valuesizebytes();
    acceptor.accept(socket_data);
    asio::error_code error;
    std::vector<char> buf_key(key.size());
    std::vector<char> buf(value_size);
    // read from socket
    size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
    int flag = 1;
    for (int i = 0; i < int(key.size()); i++)
    {
      if (key[i] != buf_key[i])
      {
        flag = 0;
      }
    }
    if (flag)
    {
      len = asio::read(socket_data, asio::buffer(buf, value_size), error);
    }
    else
    {
      std::cout << "[GET] key not matches!" << std::endl;
    }
    asio::error_code ignore_ec;
    socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
    socket_data.close(ignore_ec);
    if (flag)
    {
      std::cout << "[GET] get key: " << buf_key.data() << " ,valuesize: " << len << std::endl;
    }
    value = std::string(buf.data(), buf.size());
    return true;
  }

  /*
    Function: delete
    1. send the get request including the information of key to the coordinator
  */
  bool Client::delete_key(std::string key)
  {
    grpc::ClientContext context;
    coordinator_proto::KeyFromClient request;
    request.set_key(key);
    coordinator_proto::RepIfDeling reply;
    grpc::Status status = m_coordinator_ptr->delByKey(&context, request, &reply);
    if (status.ok())
    {
      if (reply.ifdeling())
      {
        std::cout << "[DEL] deleting " << key << std::endl;
      }
      else
      {
        std::cout << "[DEL] delete failed!" << std::endl;
      }
    }
    // check if metadata is saved successfully
    grpc::ClientContext check_commit;
    coordinator_proto::AskIfSuccess req;
    req.set_key(key);
    ECProject::OpperateType opp = DEL;
    req.set_opp(opp);
    req.set_stripe_id(-1);
    coordinator_proto::RepIfSuccess rep;
    grpc::Status stat;
    stat = m_coordinator_ptr->checkCommitAbort(&check_commit, req, &rep);
    if (stat.ok())
    {
      if (rep.ifcommit())
      {
        return true;
      }
      else
      {
        std::cout << "[DEL]" << key << " not delete!!!!!";
      }
    }
    else
    {
      std::cout << "[DEL]" << key << " Fail to check!!!!!";
    }
    return false;
  }

  bool Client::delete_stripe(int stripe_id)
  {
    grpc::ClientContext context;
    coordinator_proto::StripeIdFromClient request;
    request.set_stripe_id(stripe_id);
    coordinator_proto::RepIfDeling reply;
    grpc::Status status = m_coordinator_ptr->delByStripe(&context, request, &reply);
    if (status.ok())
    {
      if (reply.ifdeling())
      {
        std::cout << "[DEL] deleting Stripe " << stripe_id << std::endl;
      }
      else
      {
        std::cout << "[DEL] delete failed!" << std::endl;
      }
    }
    // check if metadata is saved successfully
    grpc::ClientContext check_commit;
    coordinator_proto::AskIfSuccess req;
    req.set_key("");
    ECProject::OpperateType opp = DEL;
    req.set_opp(opp);
    req.set_stripe_id(stripe_id);
    coordinator_proto::RepIfSuccess rep;
    grpc::Status stat;
    stat = m_coordinator_ptr->checkCommitAbort(&check_commit, req, &rep);
    if (stat.ok())
    {
      if (rep.ifcommit())
      {
        return true;
      }
      else
      {
        std::cout << "[DEL] Stripe" << stripe_id << " not delete!!!!!";
      }
    }
    else
    {
      std::cout << "[DEL] Stripe" << stripe_id << " Fail to check!!!!!";
    }
    return false;
  }

  bool Client::delete_all_stripes()
  {
    grpc::ClientContext context;
    coordinator_proto::RepStripeIds rep;
    coordinator_proto::RequestToCoordinator req;
    grpc::Status status = m_coordinator_ptr->listStripes(&context, req, &rep);
    if (status.ok())
    {
      std::cout << "Deleting all stripes!" << std::endl;
      for (int i = 0; i < int(rep.stripe_ids_size()); i++)
      {
        delete_stripe(rep.stripe_ids(i));
      }
      return true;
    }
    return false;
  }
} // namespace ECProject