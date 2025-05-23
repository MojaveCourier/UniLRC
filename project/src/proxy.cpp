#include "proxy.h"
#include "jerasure.h"
#include "reed_sol.h"
#include "tinyxml2.h"
#include "toolbox.h"
#include "lrc.h"
#include <thread>
#include <cassert>
#include <string>
#include <fstream>
#include <sys/mman.h>
#include "unilrc_encoder.h"
#include <chrono>
template <typename T>
inline T ceil(T const &A, T const &B)
{
  return T((A + B - 1) / B);
};
namespace ECProject
{
  bool ProxyImpl::init_coordinator()
  {
    m_coordinator_ptr = coordinator_proto::coordinatorService::NewStub(grpc::CreateChannel(m_coordinator_address, grpc::InsecureChannelCredentials()));
    // coordinator_proto::RequestToCoordinator req;
    // coordinator_proto::ReplyFromCoordinator rep;
    // grpc::ClientContext context;
    // std::string proxy_info = "Proxy [" + proxy_ip_port + "]";
    // req.set_name(proxy_info);
    // grpc::Status status;
    // status = m_coordinator_ptr->checkalive(&context, req, &rep);
    // if (status.ok())
    // {
    //   std::cout << "[Coordinator Check] ok from " << m_coordinator_address << std::endl;
    // }
    // else
    // {
    //   std::cout << "[Coordinator Check] failed to connect " << m_coordinator_address << std::endl;
    // }
    return true;
  }

  bool ProxyImpl::init_datanodes(std::string m_datanodeinfo_path)
  {
    tinyxml2::XMLDocument xml;
    xml.LoadFile(m_datanodeinfo_path.c_str());
    tinyxml2::XMLElement *root = xml.RootElement();
    for (tinyxml2::XMLElement *cluster = root->FirstChildElement(); cluster != nullptr; cluster = cluster->NextSiblingElement())
    {
      std::string cluster_id(cluster->Attribute("id"));
      std::string proxy(cluster->Attribute("proxy"));
      if (proxy == proxy_ip_port)
      {
        m_self_cluster_id = std::stoi(cluster_id);
      }
      for (tinyxml2::XMLElement *node = cluster->FirstChildElement()->FirstChildElement(); node != nullptr; node = node->NextSiblingElement())
      {
        std::string node_uri(node->Attribute("uri"));
        auto _stub = datanode_proto::datanodeService::NewStub(grpc::CreateChannel(node_uri, grpc::InsecureChannelCredentials()));
        // datanode_proto::CheckaliveCMD cmd;
        // datanode_proto::RequestResult result;
        // grpc::ClientContext context;
        // std::string proxy_info = "Proxy [" + proxy_ip_port + "]";
        // cmd.set_name(proxy_info);
        // grpc::Status status;
        // status = _stub->checkalive(&context, cmd, &result);
        // if (status.ok())
        // {
        //   // std::cout << "[Datanode Check] ok from " << node_uri << std::endl;
        // }
        // else
        // {
        //   std::cout << "[Datanode Check] failed to connect " << node_uri << std::endl;
        // }
        m_datanode_ptrs.insert(std::make_pair(node_uri, std::move(_stub)));
      }
    }
    return true;
  }

  grpc::Status ProxyImpl::checkalive(grpc::ServerContext *context,
                                     const proxy_proto::CheckaliveCMD *request,
                                     proxy_proto::RequestResult *response)
  {

    std::cout << "[Proxy] checkalive" << request->name() << std::endl;
    response->set_message(false);
    init_coordinator();
    return grpc::Status::OK;
  }

  bool ProxyImpl::MergeParityOnDatanode(const char *block_key, int block_id, const char *ip, int port, const std::string &append_mode)
  {
    try
    {
      grpc::ClientContext context;
      datanode_proto::MergeParityInfo merge_parity_info;
      datanode_proto::RequestResult result;
      merge_parity_info.set_block_key(std::string(block_key));
      merge_parity_info.set_block_id(block_id);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      // REP_MODE; UNILRC_MODE; CACHED_MODE
      if (append_mode == "UNILRC_MODE" || append_mode == "CACHED_MODE")
      {
        grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleMergeParity(&context, merge_parity_info, &result);
      }
      /*else if (append_mode == "REP_MODE")
      {
        grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleMergeParityWithRep(&context, merge_parity_info, &result);
      }*/
      else
      {
        throw std::runtime_error("Invalid append mode: " + append_mode);
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  // slice_offset is the physical offset of the data block
  bool ProxyImpl::AppendToDatanode(const char *block_key, int block_id, size_t slice_size, const char *slice_buf, int slice_offset, const char *ip, int port, bool is_serialized)
  {
    try
    {
      grpc::ClientContext context;
      datanode_proto::AppendInfo append_info;
      datanode_proto::RequestResult result;
      append_info.set_block_key(std::string(block_key));
      append_info.set_block_id(block_id);
      append_info.set_append_size(slice_size);
      append_info.set_append_offset(slice_offset);
      append_info.set_is_serialized(is_serialized);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleAppend(&context, append_info, &result);

      asio::error_code error;
      asio::io_context io_context;
      asio::ip::tcp::socket socket(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::error_code con_error;
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}), con_error);
      if (!con_error && IF_DEBUG)
      {
        std::cout << "Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " success! block_key: " << block_key << " block_id: " << block_id << " slice_size: " << slice_size << " slice_offset: " << slice_offset << " is_serialized: " << is_serialized << std::endl;
      }
      else if (IF_DEBUG)
      {
        std::cout << "Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " failed! block_key: " << block_key << " block_id: " << block_id << " slice_size: " << slice_size << " slice_offset: " << slice_offset << " is_serialized: " << is_serialized << std::endl;
        exit(-1);
      }
      asio::write(socket, asio::buffer(slice_buf, slice_size), error);
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Append139]"
                  << "Append to " << block_key << " with length of " << slice_size << std::endl;
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  bool ProxyImpl::RecoveryToDatanode(const char *block_key, int block_id, const char *buf, const char *ip, int port)
  {
    try
    {
      grpc::ClientContext context;
      datanode_proto::MergeParityInfo recovery_info;
      datanode_proto::RequestResult result;
      recovery_info.set_block_key(std::string(block_key));
      recovery_info.set_block_id(block_id);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      std::thread notify_datanode_thread([this, &context, &recovery_info, &result, &node_ip_port, &block_key, &block_id]()
      {
        grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleRecovery(&context, recovery_info, &result);
        if (!stat.ok())
        {
          std::cout << "[RecoveryToDatanode] notify datanode failed! block_key: " << block_key << " block_id: " << block_id << std::endl;
          exit(-1);
        }
      });
      //grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleRecovery(&context, recovery_info, &result);

      asio::error_code error;
      asio::io_context io_context;
      asio::ip::tcp::socket socket(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::error_code con_error;
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}), con_error);
      if (!con_error)
      {
        std::cout << "[RecoveryToDatanode] Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " success! block_key: " << block_key << " block_id: " << block_id << std::endl;
      }
      else
      {
        std::cout << "[RecoveryToDatanode] Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " failed! block_key: " << block_key << " block_id: " << block_id << std::endl;
        exit(-1);
      }
      asio::write(socket, asio::buffer(buf, m_sys_config->BlockSize), error);
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      notify_datanode_thread.join();
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  bool ProxyImpl::RecoveryToDatanodeBreakdown(const char *block_key, int block_id, const char *buf, const char *ip, int port, double *network_time, double *disk_io_time)
  {
    try
    {
      grpc::ClientContext context;
      datanode_proto::MergeParityInfo recovery_info;
      datanode_proto::RequestResult result;
      recovery_info.set_block_key(std::string(block_key));
      recovery_info.set_block_id(block_id);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      std::chrono::high_resolution_clock::time_point grpc_notify_time;
      std::thread notify_datanode_thread([this, &context, &recovery_info, &result, &grpc_notify_time, &node_ip_port, &block_key, &block_id]()
      {
        grpc_notify_time = std::chrono::high_resolution_clock::now();
        grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleRecoveryBreakdown(&context, recovery_info, &result);
        if (!stat.ok())
        {
          std::cout << "[RecoveryToDatanode] notify datanode failed! block_key: " << block_key << " block_id: " << block_id << std::endl;
          exit(-1);
        }
      });

      asio::error_code error;
      asio::io_context io_context;
      asio::ip::tcp::socket socket(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::error_code con_error;
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}), con_error);
      std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now(); // start time for network
      if (!con_error)
      {
        std::cout << "[RecoveryToDatanode] Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " success! block_key: " << block_key << " block_id: " << block_id << std::endl;
      }
      else
      {
        std::cout << "[RecoveryToDatanode] Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " failed! block_key: " << block_key << " block_id: " << block_id << std::endl;
        exit(-1);
      }
      asio::write(socket, asio::buffer(buf, m_sys_config->BlockSize), error);
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now(); // end time for network
      *network_time = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
      notify_datanode_thread.join();
      *disk_io_time = result.disk_io_end_time() - result.disk_io_start_time();
      *network_time += result.grpc_start_time() - std::chrono::duration_cast<std::chrono::duration<double>>(grpc_notify_time.time_since_epoch()).count();
  
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  bool ProxyImpl::SetToDatanode(const char *key, size_t key_length, const char *value, size_t value_length, const char *ip, int port, int offset)
  {
    try
    {
      grpc::ClientContext context;
      datanode_proto::SetInfo set_info;
      datanode_proto::RequestResult result;
      set_info.set_block_key(std::string(key));
      set_info.set_block_size(value_length);
      set_info.set_proxy_ip(m_ip);
      set_info.set_proxy_port(m_port + offset);
      set_info.set_ispull(false);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleSet(&context, set_info, &result);

      asio::error_code error;
      asio::io_context io_context;
      asio::ip::tcp::socket socket(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::error_code con_error;
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}), con_error);
      if (!con_error && IF_DEBUG)
      {
        std::cout << "Connect to " << ip << ":" << port + ECProject::DATANODE_PORT_SHIFT << " success!" << std::endl;
      }

      asio::write(socket, asio::buffer(value, value_length), error);

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                  << "Write " << key << " to socket finish! With length of " << strlen(value) << std::endl;
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }
  bool ProxyImpl::GetFromDatanode(const std::string &key, char *value, const size_t value_length, const char *ip, const int port, 
    double *disk_io_start_time, double *disk_io_end_time, double *network_start_time, double *network_end_time, double* grpc_notify_time, double *grpc_start_time)
  {
    try
    {

      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << " Ready to recieve data from datanode " << std::endl;

      grpc::ClientContext context;
      datanode_proto::GetInfo get_info;
      datanode_proto::RequestResult result;
      get_info.set_block_key(key);
      get_info.set_block_size(value_length);
      // set proxy ip and port is useless, however, to competitive with the original code, we still need to set it
      get_info.set_proxy_ip(m_ip);
      get_info.set_proxy_port(m_port);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      std::chrono::high_resolution_clock::time_point grpc_notify = std::chrono::high_resolution_clock::now();
      grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleGetBreakdown(&context, get_info, &result);
      if (stat.ok() && IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Call datanode to handle get " << key << std::endl;
      }
      else if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Call datanode to handle get " << key << " failed!" << std::endl;
        return false;
      }
      *disk_io_start_time = result.disk_io_start_time();
      *disk_io_end_time = result.disk_io_end_time();
      *grpc_notify_time = std::chrono::duration_cast<std::chrono::duration<double>>(grpc_notify.time_since_epoch()).count();
      *grpc_start_time = result.grpc_start_time();
      

      asio::io_context io_context;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::socket socket(io_context);
      std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now(); // start time for network
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}));
      asio::error_code ec;
      asio::read(socket, asio::buffer(value, value_length), ec);
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now(); // end time for network
      *network_start_time = std::chrono::duration_cast<std::chrono::duration<double>>(begin.time_since_epoch()).count();
      *network_end_time = std::chrono::duration_cast<std::chrono::duration<double>>(end.time_since_epoch()).count();
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Read data from socket with length of " << value_length << std::endl;
      }
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
      << " Read data from socket with length of " << value_length << std::endl;
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }
  bool ProxyImpl::GetFromDatanode(const char *key, size_t key_length, char *value, size_t value_length, const char *ip, int port, int offset)
  {
    try
    {
      // ready to recieve
      char *buf = new char[value_length];
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Ready to recieve data from datanode " << std::endl;
      }

      grpc::ClientContext context;
      datanode_proto::GetInfo get_info;
      datanode_proto::RequestResult result;
      get_info.set_block_key(std::string(key));
      get_info.set_block_size(value_length);
      get_info.set_proxy_ip(m_ip);
      get_info.set_proxy_port(m_port + offset);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleGet(&context, get_info, &result);
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Call datanode to handle get " << key << std::endl;
      }

      asio::io_context io_context;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}));
      asio::error_code ec;
      asio::read(socket, asio::buffer(buf, value_length), ec);
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Read data from socket with length of " << value_length << std::endl;
      }
      memcpy(value, buf, value_length);
      delete buf;
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  bool ProxyImpl::GetFromDatanode(const std::string &key, char *value, const size_t value_length, const char *ip, const int port)
  {
    try
    {

      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << " Ready to recieve data from datanode " << std::endl;

      grpc::ClientContext context;
      datanode_proto::GetInfo get_info;
      datanode_proto::RequestResult result;
      get_info.set_block_key(key);
      get_info.set_block_size(value_length);
      // set proxy ip and port is useless, however, to competitive with the original code, we still need to set it
      get_info.set_proxy_ip(m_ip);
      get_info.set_proxy_port(m_port);
      std::string node_ip_port = std::string(ip) + ":" + std::to_string(port);
      grpc::Status stat = m_datanode_ptrs[node_ip_port]->handleGet(&context, get_info, &result);
      if (stat.ok() && IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Call datanode to handle get " << key << std::endl;
      }
      else if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Call datanode to handle get " << key << " failed!" << std::endl;
        return false;
      }

      asio::io_context io_context;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, resolver.resolve({std::string(ip), std::to_string(port + ECProject::DATANODE_PORT_SHIFT)}));
      asio::error_code ec;
      asio::read(socket, asio::buffer(value, value_length), ec);
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << " Read data from socket with length of " << value_length << std::endl;
      }
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
      << " Read data from socket with length of " << value_length << std::endl;
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  bool ProxyImpl::DelInDatanode(std::string key, std::string node_ip_port)
  {
    try
    {
      grpc::ClientContext context;
      datanode_proto::DelInfo delinfo;
      datanode_proto::RequestResult response;
      delinfo.set_block_key(key);
      grpc::Status status = m_datanode_ptrs[node_ip_port]->handleDelete(&context, delinfo, &response);
      if (status.ok() && IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][DEL] delete block " << key << " success!" << std::endl;
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  void ProxyImpl::printAppendStripeDataPlacement(const proxy_proto::AppendStripeDataPlacement *append_stripe_data_placement)
  {
    // Print basic info
    std::cout << "=== AppendStripeDataPlacement Info ===" << std::endl;
    std::cout << "Key: " << append_stripe_data_placement->key() << std::endl;
    std::cout << "Stripe ID: " << append_stripe_data_placement->stripe_id() << std::endl;
    std::cout << "Cluster ID: " << append_stripe_data_placement->cluster_id() << std::endl;
    std::cout << "Total Append Size: " << append_stripe_data_placement->append_size() << std::endl;
    std::cout << "Is Merge Parity: " << (append_stripe_data_placement->is_merge_parity() ? "true" : "false") << std::endl;
    std::cout << "Append Mode: " << append_stripe_data_placement->append_mode() << std::endl;
    std::cout << "Is Serialized: " << (append_stripe_data_placement->is_serialized() ? "true" : "false") << std::endl;

    // Print datanode info
    std::cout << "\n=== Datanode Info ===" << std::endl;
    for (int i = 0; i < append_stripe_data_placement->datanodeip_size(); i++)
    {
      std::cout << "Datanode " << i << ":" << std::endl;
      std::cout << "  IP: " << append_stripe_data_placement->datanodeip(i) << std::endl;
      std::cout << "  Port: " << append_stripe_data_placement->datanodeport(i) << std::endl;
    }

    // Print block info
    std::cout << "\n=== Block Info ===" << std::endl;
    for (int i = 0; i < append_stripe_data_placement->blockkeys_size(); i++)
    {
      std::cout << "Block " << i << ":" << std::endl;
      std::cout << "  Key: " << append_stripe_data_placement->blockkeys(i) << std::endl;
      std::cout << "  ID: " << append_stripe_data_placement->blockids(i) << std::endl;
      std::cout << "  Offset: " << append_stripe_data_placement->offsets(i) << std::endl;
      std::cout << "  Size: " << append_stripe_data_placement->sizes(i) << std::endl;
    }
    std::cout << "===================================" << std::endl;
  }

  grpc::Status ProxyImpl::scheduleAppend2Datanode(
      grpc::ServerContext *context,
      const proxy_proto::AppendStripeDataPlacement *append_stripe_data_placement,
      proxy_proto::SetReply *response)
  {
    // printAppendStripeDataPlacement(append_stripe_data_placement);

    int stripe_id = append_stripe_data_placement->stripe_id();
    // sum of all append slices allocated to this proxy
    size_t cluster_append_size = append_stripe_data_placement->append_size();
    // number of slices allocated to this proxy
    int slice_num = append_stripe_data_placement->blockkeys_size();
    bool is_serialized = append_stripe_data_placement->is_serialized();

    auto placement_copy = std::make_shared<proxy_proto::AppendStripeDataPlacement>(*append_stripe_data_placement);

    auto append_and_save = [this, stripe_id, cluster_append_size, slice_num, placement_copy, is_serialized]() mutable
    {
      try
      {
        asio::ip::tcp::socket socket_data(io_context);
        acceptor.accept(socket_data);
        asio::error_code error;

        // assert(m_pre_allocated_buffer_queue.size() > 0 && "Pre-allocated buffer queue is empty");
        // std::shared_ptr<char[]> append_buf = m_pre_allocated_buffer_queue.front();
        // m_pre_allocated_buffer_queue.pop();
        // char *append_buf = new char[cluster_append_size];
        // memset(append_buf, 0, cluster_append_size);
        // std::shared_ptr<char> append_buf_ptr(append_buf, [](char* p) { delete[] p; }); // 使用智能指针管理内存
        std::vector<char> append_buf(cluster_append_size, 0);
        asio::read(socket_data, asio::buffer(append_buf.data(), cluster_append_size), error);
        if (error == asio::error::eof)
        {
          std::cout << "error == asio::error::eof" << std::endl;
        }
        else if (error)
        {
          throw asio::system_error(error);
        }

        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Append339]"
                    << "Append to Stripe " << stripe_id << " with length of " << cluster_append_size << std::endl;
        }

        asio::error_code ignore_ec;
        socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
        socket_data.close(ignore_ec);

        std::vector<char *> slices = m_toolbox->splitCharPointer(append_buf.data(), placement_copy);

        auto append_to_datanode = [this](const char *block_key, int block_id, size_t slice_size, const char *slice_buf, int slice_offset, const char *ip, int port, bool is_serialized)
        {
          if (IF_DEBUG)
          {
            std::cout << "[Proxy" << m_self_cluster_id << "][Append353]"
                      << "Append to Block " << block_key << " of block_id " << block_id << " at the offset of " << slice_offset << " with length of " << slice_size << std::endl;
          }
          AppendToDatanode(block_key, block_id, slice_size, slice_buf, slice_offset, ip, port, is_serialized);
        };

        std::vector<std::thread> senders;
        for (int j = 0; j < slice_num; j++)
        {
          senders.push_back(std::thread(append_to_datanode, placement_copy->blockkeys(j).c_str(), placement_copy->blockids(j), placement_copy->sizes(j), slices[j], placement_copy->offsets(j), placement_copy->datanodeip(j).c_str(), placement_copy->datanodeport(j), is_serialized));
        }
        for (int j = 0; j < int(senders.size()); j++)
        {
          senders[j].join();
        }

        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Append371]"
                    << "Finish appending to Stripe " << stripe_id << std::endl;
        }

        if (placement_copy->is_merge_parity())
        {
          for (int j = 0; j < slice_num; j++)
          {
            if (placement_copy->blockids(j) >= m_sys_config->k)
            {
              MergeParityOnDatanode(placement_copy->blockkeys(j).c_str(), placement_copy->blockids(j), placement_copy->datanodeip(j).c_str(), placement_copy->datanodeport(j), placement_copy->append_mode());
            }
          }

          if (IF_DEBUG)
          {
            std::cout << "[Proxy" << m_self_cluster_id << "][Append387]"
                      << "Async merging parities of Stripe " << stripe_id << std::endl;
          }
        }

        // report to coordinator
        coordinator_proto::CommitAbortKey commit_abort_key;
        coordinator_proto::ReplyFromCoordinator result;
        grpc::ClientContext context;
        ECProject::OpperateType opp = APPEND;
        commit_abort_key.set_opp(opp);
        commit_abort_key.set_key(placement_copy->key());
        commit_abort_key.set_stripe_id(stripe_id);
        commit_abort_key.set_ifcommitmetadata(true);
        grpc::Status status;
        status = m_coordinator_ptr->reportCommitAbort(&context, commit_abort_key, &result);
        if (status.ok() && IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][APPEND405]"
                    << " report to coordinator success" << std::endl;
        }
        else if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][APPEND410]"
                    << " report to coordinator fail!" << std::endl;
        }
      }
      catch (std::exception &e)
      {
        std::cout << "exception in append_and_save" << std::endl;
        std::cout << e.what() << std::endl;
      }
    };
    try
    {
      if (IF_DEBUG)
      {
        std::cout << "[Proxy][APPEND424] Handle append_and_save" << std::endl;
      }
      std::thread my_thread(append_and_save);
      my_thread.detach();
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::encodeAndSetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::SetReply *response)
  {
    std::string key = object_and_placement->key();
    int value_size_bytes = object_and_placement->valuesizebyte();
    int k = object_and_placement->k();
    int g_m = object_and_placement->g_m();
    int l = object_and_placement->l();
    // int stripe_id = object_and_placement->stripe_id();
    int block_size = object_and_placement->block_size();
    ECProject::EncodeType encode_type = (ECProject::EncodeType)object_and_placement->encode_type();
    std::vector<std::pair<std::string, std::pair<std::string, int>>> keys_nodes;
    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      keys_nodes.push_back(std::make_pair(object_and_placement->blockkeys(i), std::make_pair(object_and_placement->datanodeip(i), object_and_placement->datanodeport(i))));
    }
    auto encode_and_save = [this, key, value_size_bytes, k, g_m, l, block_size, keys_nodes, encode_type]() mutable
    {
      try
      {
        // read the key and value in the socket sent by client
        // initialize the socket of reading key and value
        asio::ip::tcp::socket socket_data(io_context);
        acceptor.accept(socket_data);
        asio::error_code error;

        int extend_value_size_byte = block_size * k;
        std::vector<char> buf_key(key.size());
        std::vector<char> v_buf(extend_value_size_byte);
        for (int i = value_size_bytes; i < extend_value_size_byte; i++)
        {
          v_buf[i] = '0';
        }

        // read the key
        asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
        if (error == asio::error::eof)
        {
          std::cout << "error == asio::error::eof" << std::endl;
        }
        else if (error)
        {
          throw asio::system_error(error);
        }
        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                    << "Check key " << buf_key.data() << std::endl;
        }

        // check the key
        bool flag = true;
        for (int i = 0; i < int(key.size()); i++)
        {
          if (key[i] != buf_key[i])
          {
            flag = false;
          }
        }
        if (flag)
        {
          if (IF_DEBUG)
          {
            std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                      << "Read value of " << buf_key.data() << std::endl;
          }
          // read the value
          asio::read(socket_data, asio::buffer(v_buf.data(), value_size_bytes), error);
        }
        asio::error_code ignore_ec;
        socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
        socket_data.close(ignore_ec);

        // set the blocks to the datanode
        char *buf = v_buf.data();
        // define a lambda function to send to datanode
        auto send_to_datanode = [this](int j, int k, std::string block_key, char **data, char **coding, int block_size, std::pair<std::string, int> ip_and_port)
        {
          if (IF_DEBUG)
          {
            std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                      << "Thread " << j << " send " << block_key << " to Datanode" << ip_and_port.second << std::endl;
          }
          if (j < k)
          {
            // send to data node
            SetToDatanode(block_key.c_str(), block_key.size(), data[j], block_size, ip_and_port.first.c_str(), ip_and_port.second, j + 2);
          }
          else
          {
            // send to parity node
            SetToDatanode(block_key.c_str(), block_key.size(), coding[j - k], block_size, ip_and_port.first.c_str(), ip_and_port.second, j + 2);
          }
        };

        // calculate parity blocks
        // initialize the area of parity blocks
        std::vector<char *> v_data(k);
        std::vector<char *> v_coding(g_m + l + 1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();

        std::vector<std::vector<char>> v_coding_area(g_m + l + 1, std::vector<char>(block_size));
        for (int j = 0; j < k; j++)
        {
          data[j] = &buf[j * block_size];
        }
        for (int j = 0; j < g_m + l + 1; j++)
        {
          coding[j] = v_coding_area[j].data();
        }
        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                    << "Encode value with size of " << v_buf.size() << std::endl;
        }
        int send_num;
        if (encode_type == Azure_LRC || encode_type == Optimal_Cauchy_LRC)
        {
          encode(k, g_m, l, data, coding, block_size, encode_type);
          send_num = k + g_m + l;
        }
        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                    << "Distribute blocks to datanodes" << std::endl;
        }

        // call the lambda function send_to_datanode to send data and parity blocks
        std::vector<std::thread> senders;
        for (int j = 0; j < send_num; j++)
        {
          std::string block_key = keys_nodes[j].first;
          std::pair<std::string, int> &ip_and_port = keys_nodes[j].second;
          senders.push_back(std::thread(send_to_datanode, j, k, block_key, data, coding, block_size, ip_and_port));
        }
        for (int j = 0; j < int(senders.size()); j++)
        {
          senders[j].join();
        }
        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                    << "Finish distributing blocks!" << std::endl;
        }

        // report to coordinator
        coordinator_proto::CommitAbortKey commit_abort_key;
        coordinator_proto::ReplyFromCoordinator result;
        grpc::ClientContext context;
        ECProject::OpperateType opp = SET;
        commit_abort_key.set_opp(opp);
        commit_abort_key.set_key(key);
        commit_abort_key.set_ifcommitmetadata(true);
        grpc::Status status;
        status = m_coordinator_ptr->reportCommitAbort(&context, commit_abort_key, &result);
        if (status.ok() && IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                    << "[SET] report to coordinator success" << std::endl;
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][SET]"
                    << " report to coordinator fail!" << std::endl;
        }
      }
      catch (std::exception &e)
      {
        std::cout << "exception in encode_and_save" << std::endl;
        std::cout << e.what() << std::endl;
      }
    };
    try
    {
      if (IF_DEBUG)
      {
        std::cout << "[Proxy][SET] Handle encode and set" << std::endl;
      }
      std::thread my_thread(encode_and_save);
      my_thread.detach();
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::decodeAndGetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::GetReply *response)
  {
    ECProject::EncodeType encode_type = (ECProject::EncodeType)object_and_placement->encode_type();
    std::string key = object_and_placement->key();
    int k = object_and_placement->k();
    int g_m = object_and_placement->g_m();
    int l = object_and_placement->l();
    // int block_size = object_and_placement->block_size();
    int value_size_bytes = object_and_placement->valuesizebyte();
    int block_size = ceil(value_size_bytes, k);
    std::string clientip = object_and_placement->clientip();
    int clientport = object_and_placement->clientport();
    int stripe_id = object_and_placement->stripe_id();

    std::vector<std::pair<std::string, std::pair<std::string, int>>> keys_nodes;
    std::vector<int> block_idxs;
    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      block_idxs.push_back(object_and_placement->blockids(i));
      keys_nodes.push_back(std::make_pair(object_and_placement->blockkeys(i), std::make_pair(object_and_placement->datanodeip(i), object_and_placement->datanodeport(i))));
    }

    auto decode_and_get = [this, key, k, g_m, l, block_size, value_size_bytes, stripe_id,
                           clientip, clientport, keys_nodes, block_idxs, encode_type]() mutable
    {
      int expect_block_number = (encode_type == Azure_LRC) ? (k + l) : k;
      int all_expect_blocks = (encode_type == Azure_LRC) ? (k + g_m + l) : (k + g_m);

      auto blocks_ptr = std::make_shared<std::vector<std::vector<char>>>();
      auto blocks_key_ptr = std::make_shared<std::vector<std::string>>();
      auto blocks_idx_ptr = std::make_shared<std::vector<int>>();
      auto myLock_ptr = std::make_shared<std::mutex>();
      auto cv_ptr = std::make_shared<std::condition_variable>();

      std::vector<char *> v_data(k);
      std::vector<char *> v_coding(all_expect_blocks - k);
      char **data = v_data.data();
      char **coding = v_coding.data();

      auto getFromNode = [this, k, blocks_ptr, blocks_key_ptr, blocks_idx_ptr, myLock_ptr, cv_ptr](int expect_block_number, int block_idx, std::string block_key, int block_size, std::string ip, int port)
      {
        if (IF_DEBUG)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                    << "Block " << block_idx << " with key " << block_key << " from Datanode" << ip << ":" << port << std::endl;
        }

        std::vector<char> temp(block_size);
        bool ret = GetFromDatanode(block_key.c_str(), block_key.size(), temp.data(), block_size, ip.c_str(), port, block_idx + 2);

        if (!ret)
        {
          std::cout << "getFromNode !ret" << std::endl;
          return;
        }
        myLock_ptr->lock();
        // get any k blocks and decode
        if (!check_received_block(k, expect_block_number, blocks_idx_ptr, blocks_ptr->size()))
        {
          blocks_ptr->push_back(temp);
          blocks_key_ptr->push_back(block_key);
          blocks_idx_ptr->push_back(block_idx);
          if (check_received_block(k, expect_block_number, blocks_idx_ptr, blocks_ptr->size()))
          {
            cv_ptr->notify_all();
          }
        }
        // get all the blocks
        // blocks_ptr->push_back(temp);
        // blocks_key_ptr->push_back(block_key);
        // blocks_idx_ptr->push_back(block_idx);
        myLock_ptr->unlock();
      };

      std::vector<std::vector<char>> v_data_area(k, std::vector<char>(block_size));
      std::vector<std::vector<char>> v_coding_area(all_expect_blocks - k, std::vector<char>(block_size));
      for (int j = 0; j < k; j++)
      {
        data[j] = v_data_area[j].data();
      }
      for (int j = 0; j < all_expect_blocks - k; j++)
      {
        coding[j] = v_coding_area[j].data();
      }
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "ready to get blocks from datanodes!" << std::endl;
      }
      std::vector<std::thread> read_treads;
      // for (int j = 0; j < k; j++)
      for (int j = 0; j < all_expect_blocks; j++)
      {
        int block_idx = block_idxs[j];
        std::string block_key = keys_nodes[j].first;
        std::pair<std::string, int> &ip_and_port = keys_nodes[j].second;
        // std::vector<char> temp(block_size);
        // GetFromDatanode(block_key.c_str(), block_key.size(), temp.data(), block_size, ip_and_port.first.c_str(), ip_and_port.second, j + 2);
        // blocks_ptr->push_back(temp);
        // blocks_key_ptr->push_back(block_key);
        // blocks_idx_ptr->push_back(j);
        read_treads.push_back(std::thread(getFromNode, expect_block_number, block_idx, block_key, block_size, ip_and_port.first, ip_and_port.second));
      }
      for (int j = 0; j < all_expect_blocks; j++)
      {
        read_treads[j].detach();
        // read_treads[j].join();
      }

      std::unique_lock<std::mutex> lck(*myLock_ptr);
      while (!check_received_block(k, expect_block_number, blocks_idx_ptr, blocks_ptr->size()))
      {
        cv_ptr->wait(lck);
      }
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "ready to decode!" << std::endl;
      }
      for (int j = 0; j < int(blocks_idx_ptr->size()); j++)
      {
        int idx = (*blocks_idx_ptr)[j];
        if (idx < k)
        {
          memcpy(data[idx], (*blocks_ptr)[j].data(), block_size);
        }
        else
        {
          memcpy(coding[idx - k], (*blocks_ptr)[j].data(), block_size);
        }
      }

      auto erasures = std::make_shared<std::vector<int>>();
      for (int j = 0; j < all_expect_blocks; j++)
      {
        if (std::find(blocks_idx_ptr->begin(), blocks_idx_ptr->end(), j) == blocks_idx_ptr->end())
        {
          erasures->push_back(j);
        }
      }
      erasures->push_back(-1);
      if (encode_type == Azure_LRC)
      {
        if (!decode(k, g_m, l, data, coding, erasures, block_size, encode_type))
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][GET] proxy cannot decode!" << std::endl;
        }
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET] proxy decode error!" << std::endl;
      }
      std::string value;
      for (int j = 0; j < k; j++)
      {
        value += std::string(data[j]);
      }

      if (IF_DEBUG)
      {
        std::cout << "\033[1;31m[Proxy" << m_self_cluster_id << "][GET]"
                  << "send " << key << " to client with length of " << value.size() << "\033[0m" << std::endl;
      }

      // send to the client
      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(clientip, std::to_string(clientport));
      asio::ip::tcp::socket sock_data(io_context);
      asio::connect(sock_data, endpoints);

      asio::write(sock_data, asio::buffer(key, key.size()), error);
      if(error)
      {
        std::cout << "error in write key" << std::endl;
      }
      asio::write(sock_data, asio::buffer(value, value_size_bytes), error);
      if(error)
      {
        std::cout << "error in write value" << std::endl;
      }
      asio::error_code ignore_ec;
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      sock_data.close(ignore_ec);
    };
    try
    {
      // std::cerr << "decode_and_get_thread start" << std::endl;
      if (IF_DEBUG)
      {
        std::cout << "[Proxy] Handle get and decode" << std::endl;
      }
      std::thread my_thread(decode_and_get);
      my_thread.detach();
      // std::cerr << "decode_and_get_thread detach" << std::endl;
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  std::vector<unsigned char *> ProxyImpl::convertToUnsignedCharArray(std::vector<char*> &input)
  {
    std::vector<unsigned char *> output;

    for (auto &row : input)
    {
      output.push_back(reinterpret_cast<unsigned char *>(row));
    }

    return output;
  }

  void ProxyImpl::get_from_node(const std::string &block_key, char *block_value, const size_t block_size, const char *datanode_ip, const int datanode_port, bool *status, int index)
  {
    if (IF_DEBUG)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "Block key " << block_key << " from Datanode" << datanode_ip << ":" << datanode_port << std::endl;
    }
    status[index] = GetFromDatanode(block_key, block_value, block_size, datanode_ip, datanode_port);
  }

  void ProxyImpl::get_from_node_breakdown(const std::string &block_key, char *block_value, const size_t block_size, const char *datanode_ip, const int datanode_port, bool *status, int index, 
    double *disk_io_start_time, double *disk_io_end_time, double *network_start_time, double *network_end_time, double *grpc_notify_time, double *grpc_start_time)
  {
    if (IF_DEBUG)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "Block key " << block_key << " from Datanode" << datanode_ip << ":" << datanode_port << std::endl;
    }
    status[index] = GetFromDatanode(block_key, block_value, block_size, datanode_ip, datanode_port, disk_io_start_time, disk_io_end_time, network_start_time, network_end_time, grpc_notify_time, grpc_start_time);
  }
  // degraded read
  grpc::Status ProxyImpl::degradedRead(
      grpc::ServerContext *context,
      const proxy_proto::DegradedReadRequest *degraded_read_request,
      proxy_proto::DegradedReadReply *response)
  {
    auto request_copy = std::make_shared<proxy_proto::DegradedReadRequest>(*degraded_read_request);

    auto degraded_read = [this, request_copy, &response]() mutable
    {
      std::string code_type = m_sys_config->CodeType;
      // auto status = std::make_shared<std::vector<bool>>(request_copy->datanodeip_size(), false);
      std::unique_ptr<bool[]> status(new bool[request_copy->datanodeip_size()]);
      std::fill_n(status.get(), request_copy->datanodeip_size(), false);

      //std::vector<std::vector<char>> get_bufs(request_copy->datanodeip_size(), std::vector<char>(m_sys_config->BlockSize, 0));
      std::vector<char*> get_bufs(request_copy->datanodeip_size());

      for(int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      }

      //std::vector<char> res_buf(m_sys_config->BlockSize, 0);
      char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      std::vector<std::thread> get_threads;
      //std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        get_threads.push_back(std::thread(&ProxyImpl::get_from_node, this,
                                          request_copy->blockkeys(i), // block key
                                          get_bufs[i],               // buffer to store the block value
                                          m_sys_config->BlockSize,   // block size
                                          request_copy->datanodeip(i).c_str(), // datanode IP
                                          request_copy->datanodeport(i),       // datanode port
                                          status.get(),              // status array to track success/failure
                                          i                          // index for status array
        ));
      }
      for (int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        get_threads[i].join();
      }
      bool all_true = std::all_of(status.get(), status.get() + request_copy->datanodeip_size(), [](bool val)
                                  { return val == true; });
      if (!all_true)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes failed!" << std::endl;
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes success!" << std::endl;

        std::vector<int> block_idxs;
        for (int i = 0; i < request_copy->datanodeip_size(); i++)
        {
          block_idxs.push_back(request_copy->blockids(i));
        }
        std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);
        if (code_type == "UniLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_unilrc" << std::endl;
          decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
        }
        else if (code_type == "AzureLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_azure_lrc" << std::endl;
          decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_azure_lrc success!" << std::endl;
        }
        else if (code_type == "OptimalLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_optimal_lrc" << std::endl;
          decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_optimal_lrc success!" << std::endl;
        }
        else if (code_type == "UniformLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_uniform_lrc" << " failed block id: " << request_copy->failed_block_id() << std::endl;
          decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_uniform_lrc success!" << std::endl;
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
          exit(1);
        }
        std::string client_ip = request_copy->clientip();
        int client_port = request_copy->clientport();

        // send to the client
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(client_ip, std::to_string(client_port));
        //asio::io_context io_context;
        asio::ip::tcp::socket socket_data(io_context);
        asio::connect(socket_data, endpoints);
        //acceptor.accept(socket_data);
        if (error)
        {
          std::cout << "error in connect" << std::endl;
        }
        asio::write(socket_data, asio::buffer(res_buf, m_sys_config->BlockSize), error);
        if (error)
        {
          std::cout << "error in write" << std::endl;
        }
        asio::error_code ignore_ec;
        socket_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        socket_data.close(ignore_ec);
        delete res_buf;
        for(int i = 0; i < request_copy->datanodeip_size(); i++)
        {
          delete get_bufs[i];
        }
        //std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] send to the client done" << std::endl;
      }
    };

    try
    {
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] Handle degraded read" << std::endl;
      }

      std::thread my_thread(degraded_read);
      my_thread.join();
    }
    catch (const std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::degradedReadBreakdown(
      grpc::ServerContext *context,
      const proxy_proto::DegradedReadRequest *degraded_read_request,
      proxy_proto::DegradedReadReply *response)
  {
    std::chrono::high_resolution_clock::time_point START = std::chrono::high_resolution_clock::now();
    response->set_grpc_start_time(std::chrono::duration_cast<std::chrono::duration<double>>(START.time_since_epoch()).count());

    auto request_copy = std::make_shared<proxy_proto::DegradedReadRequest>(*degraded_read_request);

    auto degraded_read = [this, request_copy, &response]() mutable
    {
      std::string code_type = m_sys_config->CodeType;
      // auto status = std::make_shared<std::vector<bool>>(request_copy->datanodeip_size(), false);
      std::unique_ptr<bool[]> status(new bool[request_copy->datanodeip_size()]);
      std::fill_n(status.get(), request_copy->datanodeip_size(), false);

      //std::vector<std::vector<char>> get_bufs(request_copy->datanodeip_size(), std::vector<char>(m_sys_config->BlockSize, 0));
      std::vector<char*> get_bufs(request_copy->datanodeip_size());
      std::vector<double> data_node_disk_io_start_time(request_copy->datanodeip_size(), 0.0);
      std::vector<double> data_node_disk_io_end_time(request_copy->datanodeip_size(), 0.0);
      std::vector<double> data_node_network_start_time(request_copy->datanodeip_size(), 0.0);
      std::vector<double> data_node_network_end_time(request_copy->datanodeip_size(), 0.0);
      std::vector<double> data_node_grpc_notify_time(request_copy->datanodeip_size(), 0.0);
      std::vector<double> data_node_grpc_start_time(request_copy->datanodeip_size(), 0.0);
      for(int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      }

      //std::vector<char> res_buf(m_sys_config->BlockSize, 0);
      char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      std::vector<std::thread> get_threads;
      //std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        get_threads.push_back(std::thread(&ProxyImpl::get_from_node_breakdown, this,
                                          request_copy->blockkeys(i), // block key
                                          get_bufs[i],               // buffer to store the block value
                                          m_sys_config->BlockSize,   // block size
                                          request_copy->datanodeip(i).c_str(), // datanode IP
                                          request_copy->datanodeport(i),       // datanode port
                                          status.get(),              // status array to track success/failure
                                          i,                         // index for status array
                                          &data_node_disk_io_start_time[i],         // pointer to store disk IO time
                                          &data_node_disk_io_end_time[i],           // pointer to store disk IO time
                                          &data_node_network_start_time[i],          // pointer to store network time
                                          &data_node_network_end_time[i],            // pointer to store network time
                                          &data_node_grpc_notify_time[i],            // pointer to store grpc notify time
                                          &data_node_grpc_start_time[i]              // pointer to store grpc start time
        ));
      }
      for (int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        get_threads[i].join();
      }
      //std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      //std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
      bool all_true = std::all_of(status.get(), status.get() + request_copy->datanodeip_size(), [](bool val)
                                  { return val == true; });
      if (!all_true)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes failed!" << std::endl;
      }
      else
      {
        //response->set_disk_io_time(*std::max_element(data_node_disk_io_end_time.begin(), data_node_disk_io_end_time.end()) - *std::min_element(data_node_disk_io_start_time.begin(), data_node_disk_io_start_time.end()));
        //response->set_network_time(*std::max_element(data_node_network_end_time.begin(), data_node_network_end_time.end()) - *std::min_element(data_node_network_start_time.begin(), data_node_network_start_time.end()));
        response->set_disk_io_start_time(*std::min_element(data_node_disk_io_start_time.begin(), data_node_disk_io_start_time.end()));
        response->set_disk_io_end_time(*std::max_element(data_node_disk_io_end_time.begin(), data_node_disk_io_end_time.end()));
        response->set_network_start_time(*std::min_element(data_node_network_start_time.begin(), data_node_network_start_time.end()));
        response->set_network_end_time(*std::max_element(data_node_network_end_time.begin(), data_node_network_end_time.end()));
        response->set_data_node_grpc_notify_time(*std::min_element(data_node_grpc_notify_time.begin(), data_node_grpc_notify_time.end()));
        response->set_data_node_grpc_start_time(*std::max_element(data_node_grpc_start_time.begin(), data_node_grpc_start_time.end()));
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes success!" << std::endl;

        std::vector<int> block_idxs;
        for (int i = 0; i < request_copy->datanodeip_size(); i++)
        {
          block_idxs.push_back(request_copy->blockids(i));
        }
        std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        if (code_type == "UniLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_unilrc" << std::endl;
          decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
        }
        else if (code_type == "AzureLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_azure_lrc" << std::endl;
          decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_azure_lrc success!" << std::endl;
        }
        else if (code_type == "OptimalLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_optimal_lrc" << std::endl;
          decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_optimal_lrc success!" << std::endl;
        }
        else if (code_type == "UniformLRC")
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_uniform_lrc" << " failed block id: " << request_copy->failed_block_id() << std::endl;
          decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_uniform_lrc success!" << std::endl;
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
          exit(1);
        }
        std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
        response->set_decode_start_time(std::chrono::duration_cast<std::chrono::duration<double>>(t2.time_since_epoch()).count());
        response->set_decode_end_time(std::chrono::duration_cast<std::chrono::duration<double>>(t3.time_since_epoch()).count());
        std::string client_ip = request_copy->clientip();
        int client_port = request_copy->clientport();

        // send to the client
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(client_ip, std::to_string(client_port));
        asio::ip::tcp::socket sock_data(io_context);
        //asio::connect(sock_data, endpoints);
        sock_data.connect(*endpoints, error);
        if (error)
        {
          std::cout << "error in connect" << std::endl;
        }
        asio::write(sock_data, asio::buffer(res_buf, m_sys_config->BlockSize), error);
        if (error)
        {
          std::cout << "error in write" << std::endl;
        }
        asio::error_code ignore_ec;
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        sock_data.close(ignore_ec);
        delete res_buf;
        for(int i = 0; i < request_copy->datanodeip_size(); i++)
        {
          delete get_bufs[i];
        }
        //std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] send to the client done" << std::endl;
      }
    };

    try
    {
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] Handle degraded read" << std::endl;
      }

      std::thread my_thread(degraded_read);
      my_thread.join();
    }
    catch (const std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::degradedReadWithBlockStripeID(
    grpc::ServerContext *context,
    const proxy_proto::DegradedReadRequest *degraded_read_request,
    proxy_proto::GetReply *response)
{
  auto request_copy = std::make_shared<proxy_proto::DegradedReadRequest>(*degraded_read_request);

  auto degraded_read = [this, request_copy]() mutable
  {
    int stripe_id = request_copy->failed_block_stripe_id();
    std::string code_type = m_sys_config->CodeType;
    // auto status = std::make_shared<std::vector<bool>>(request_copy->datanodeip_size(), false);
    std::unique_ptr<bool[]> status(new bool[request_copy->datanodeip_size()]);
    std::fill_n(status.get(), request_copy->datanodeip_size(), false);

    //std::vector<std::vector<char>> get_bufs(request_copy->datanodeip_size(), std::vector<char>(m_sys_config->BlockSize, 0));
    std::vector<char*> get_bufs(request_copy->datanodeip_size());
    for(int i = 0; i < request_copy->datanodeip_size(); i++)
    {
      get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    }

    //std::vector<char> res_buf(m_sys_config->BlockSize, 0);
    char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    std::vector<std::thread> get_threads;
    for (int i = 0; i < request_copy->datanodeip_size(); i++)
    {
      get_threads.push_back(std::thread(&ProxyImpl::get_from_node, this, request_copy->blockkeys(i), get_bufs[i], m_sys_config->BlockSize, request_copy->datanodeip(i).c_str(), request_copy->datanodeport(i), status.get(), i));
    }
    for (int i = 0; i < request_copy->datanodeip_size(); i++)
    {
      get_threads[i].join();
    }

    bool all_true = std::all_of(status.get(), status.get() + request_copy->datanodeip_size(), [](bool val)
                                { return val == true; });
    if (!all_true)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "read from datanodes failed!" << std::endl;
    }
    else
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "read from datanodes success!" << std::endl;

      std::vector<int> block_idxs;
      for (int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        block_idxs.push_back(request_copy->blockids(i));
      }
      std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);

      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      if (code_type == "UniLRC")
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_unilrc" << std::endl;
        decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
      }
      else if (code_type == "AzureLRC")
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_azure_lrc" << std::endl;
        decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_azure_lrc success!" << std::endl;
      }
      else if (code_type == "OptimalLRC")
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_optimal_lrc" << std::endl;
        decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_optimal_lrc success!" << std::endl;
      }
      else if (code_type == "UniformLRC")
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_uniform_lrc" << " failed block id: " << request_copy->failed_block_id() << std::endl;
        decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, request_copy->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, request_copy->failed_block_id());
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode_uniform_lrc success!" << std::endl;
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
        exit(1);
      }

      std::string client_ip = request_copy->clientip();
      int client_port = request_copy->clientport();

      // send to the client
      asio::error_code error;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(client_ip, std::to_string(client_port));
      asio::ip::tcp::socket sock_data(io_context);
      //asio::connect(sock_data, endpoints);
      sock_data.connect(*endpoints, error);
      if (error)
      {
        std::cout << "error in connect" << std::endl;
      }
      asio::write(sock_data, asio::buffer(&stripe_id, sizeof(int)), error);
      asio::write(sock_data, asio::buffer(res_buf, m_sys_config->BlockSize), error);
      if (error)
      {
        std::cout << "error in write" << std::endl;
      }
      asio::error_code ignore_ec;
      sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      sock_data.close(ignore_ec);
      delete res_buf;
      for(int i = 0; i < request_copy->datanodeip_size(); i++)
      {
        delete get_bufs[i];
      }
      //std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] send to the client done" << std::endl;
    }
  };

  try
  {
    if (IF_DEBUG)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] Handle degraded read" << std::endl;
    }

    std::thread my_thread(degraded_read);
    my_thread.detach();
  }
  catch (const std::exception &e)
  {
    std::cout << "exception" << std::endl;
    std::cerr << e.what() << '\n';
  }

  return grpc::Status::OK;
}

  grpc::Status ProxyImpl::degradedRead2Client(
    grpc::ServerContext *context,
    const proxy_proto::RecoveryRequest *recovery_request,
    proxy_proto::DegradedReadReply *response)
{
  try
  {
    if (IF_DEBUG)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] Handle Degraded Read" << std::endl;
    }

    std::string code_type = m_sys_config->CodeType;
    int cross_rack_num = recovery_request->cross_rack_num();
    std::unique_ptr<bool[]> status(new bool[recovery_request->datanodeip_size()]);
    std::fill_n(status.get(), recovery_request->datanodeip_size(), false);
    std::vector<char*> get_bufs(recovery_request->datanodeip_size());
    for(int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    }

    char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    char *real_res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    std::vector<std::thread> get_threads;
    //std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      get_threads.push_back(std::thread(&ProxyImpl::get_from_node, this, recovery_request->blockkeys(i), get_bufs[i], m_sys_config->BlockSize, 
      recovery_request->datanodeip(i).c_str(), recovery_request->datanodeport(i), status.get(), i));
    }
    for (int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      get_threads[i].join();
    }

    bool all_true = std::all_of(status.get(), status.get() + recovery_request->datanodeip_size(), [](bool val)
                                { return val == true; });
    if (!all_true)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "read from datanodes failed!" << std::endl;
    }

    else
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "read from datanodes success!" << std::endl;
      std::vector<int> block_idxs;
      for (int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        block_idxs.push_back(recovery_request->blockids(i));
      }
      std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);

      std::string failed_block_key = recovery_request->failed_block_key();
      int failed_block_id = recovery_request->failed_block_id();
      std::string replaced_node_ip = recovery_request->replaced_node_ip();
      int replaced_node_port = recovery_request->replaced_node_port();

      if (code_type == "UniLRC")
      {
        decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
      }
      else if (code_type == "AzureLRC")
      {
        decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
      }
      else if (code_type == "OptimalLRC")
      {
        decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
      }
      else if (code_type == "UniformLRC")
      {
        decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
        exit(1);
      }
      if(cross_rack_num){
        std::cout << "start to recover cross rack" << std::endl;
        char **cross_rack_bufs = new char*[cross_rack_num];
        for(int i = 0; i < cross_rack_num; i++)
        {
          cross_rack_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
        }
        std::vector<std::thread> get_from_proxies_threads;
        std::vector<double> accept_start_time(cross_rack_num, 0.0);
        for(int i = 0; i < cross_rack_num; i++)
        {
          get_from_proxies_threads.push_back(std::thread([i, this, &cross_rack_bufs]()mutable{
            //asio::io_context io_context;
            asio::ip::tcp::socket socket(this->io_context);
            //asio::ip::tcp::resolver resolver(io_context);
            this->acceptor.accept(socket);
            std::cout << "connected to porxy" << std::endl;
            asio::error_code error;
            size_t len = asio::read(socket, asio::buffer(cross_rack_bufs[i], this->m_sys_config->BlockSize), error);
            if(len != this->m_sys_config->BlockSize)
            {
              std::cout << "error in read" << std::endl;
            }
            asio::error_code ignore_ec;
            socket.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
            socket.close(ignore_ec);
          }));
        }
        for(int i = 0; i < cross_rack_num; i++)
        {
          get_from_proxies_threads[i].join();
        }
        std::cout << "start to xor" << std::endl;
        char **buf_ptrs = new char*[cross_rack_num + 2];
        for(int i = 0; i < cross_rack_num; i++)
        {
          buf_ptrs[i] = cross_rack_bufs[i];
        }
        buf_ptrs[cross_rack_num] = res_buf;
        buf_ptrs[cross_rack_num + 1] = real_res_buf;
        xor_avx(cross_rack_num + 2, m_sys_config->BlockSize, (void**)buf_ptrs);
        for(int i = 0; i < cross_rack_num; i++)
        {
          delete cross_rack_bufs[i];
        }
        delete cross_rack_bufs;
        delete buf_ptrs;
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode success!" << std::endl;
      }
    }
  
    std::string replaced_node_ip = recovery_request->replaced_node_ip();
    int replaced_node_port = recovery_request->replaced_node_port();
    std::cout << "[Proxy" << m_self_cluster_id << "][Degraded] send to the client" << replaced_node_ip << ":" << replaced_node_port << std::endl;
    asio::io_context io_context;
    asio::ip::tcp::socket socket(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(replaced_node_ip, std::to_string(replaced_node_port));
    asio::connect(socket, endpoints);
    if(recovery_request->is_to_send_block_id()){
      int32_t block_id_to_send = recovery_request->block_id_to_send();
      asio::write(socket, asio::buffer(&block_id_to_send, sizeof(int32_t)));
    }
    if(cross_rack_num){
      asio::write(socket, asio::buffer(real_res_buf, m_sys_config->BlockSize));
    }
    else{
      asio::write(socket, asio::buffer(res_buf, m_sys_config->BlockSize));
    }
    asio::error_code ignore_ec;
    socket.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
    socket.close(ignore_ec);
    std::cout << "[Proxy" << m_self_cluster_id << "][Degraded Read] send to the client done" << std::endl;
    delete res_buf;
    delete real_res_buf;
    for(int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      delete get_bufs[i];
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "exception" << std::endl;
    std::cerr << e.what() << '\n';
  }
  return grpc::Status::OK;
}


  grpc::Status ProxyImpl::degradedRead2ClientBreakdown(
    grpc::ServerContext *context,
    const proxy_proto::RecoveryRequest *recovery_request,
    proxy_proto::DegradedReadReply *response)
{
  std::chrono::high_resolution_clock::time_point START = std::chrono::high_resolution_clock::now();
  response->set_grpc_start_time(std::chrono::duration_cast<std::chrono::duration<double>>(START.time_since_epoch()).count());
  try
  {
    if (IF_DEBUG)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] Handle Degraded Read" << std::endl;
    }

    std::string code_type = m_sys_config->CodeType;
    int cross_rack_num = recovery_request->cross_rack_num();
    std::unique_ptr<bool[]> status(new bool[recovery_request->datanodeip_size()]);
    std::fill_n(status.get(), recovery_request->datanodeip_size(), false);


    std::vector<double> data_node_disk_io_start_time(recovery_request->datanodeip_size(), 0.0);
    std::vector<double> data_node_disk_io_end_time(recovery_request->datanodeip_size(), 0.0);
    std::vector<double> data_node_network_start_time(recovery_request->datanodeip_size(), 0.0);
    std::vector<double> data_node_network_end_time(recovery_request->datanodeip_size(), 0.0);
    std::vector<double> data_node_grpc_notify_time(recovery_request->datanodeip_size(), 0.0);
    std::vector<double> data_node_grpc_start_time(recovery_request->datanodeip_size(), 0.0);

    std::vector<char*> get_bufs(recovery_request->datanodeip_size());
    for(int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    }

    char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    char *real_res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
    std::vector<std::thread> get_threads;
    //std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      get_threads.push_back(std::thread(&ProxyImpl::get_from_node_breakdown, this, recovery_request->blockkeys(i), get_bufs[i], m_sys_config->BlockSize, recovery_request->datanodeip(i).c_str(), recovery_request->datanodeport(i), status.get(), i, 
        &data_node_disk_io_start_time[i], &data_node_disk_io_end_time[i], &data_node_network_start_time[i], &data_node_network_end_time[i], &data_node_grpc_notify_time[i], &data_node_grpc_start_time[i]));
    }
    for (int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      get_threads[i].join();
    }

    bool all_true = std::all_of(status.get(), status.get() + recovery_request->datanodeip_size(), [](bool val)
                                { return val == true; });
    if (!all_true)
    {
      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "read from datanodes failed!" << std::endl;
    }

    else
    {
      //response->set_disk_io_time(*std::max_element(data_node_disk_io_end_time.begin(), data_node_disk_io_end_time.end()) - *std::min_element(data_node_disk_io_start_time.begin(), data_node_disk_io_start_time.end()));
      //response->set_network_time(*std::max_element(data_node_network_end_time.begin(), data_node_network_end_time.end()) - *std::min_element(data_node_network_start_time.begin(), data_node_network_start_time.end()));
      response->set_disk_io_start_time(*std::min_element(data_node_disk_io_start_time.begin(), data_node_disk_io_start_time.end()));
      response->set_disk_io_end_time(*std::max_element(data_node_disk_io_end_time.begin(), data_node_disk_io_end_time.end()));
      response->set_network_start_time(*std::min_element(data_node_network_start_time.begin(), data_node_network_start_time.end()));
      response->set_network_end_time(*std::max_element(data_node_network_end_time.begin(), data_node_network_end_time.end()));
      response->set_data_node_grpc_notify_time(*std::min_element(data_node_grpc_notify_time.begin(), data_node_grpc_notify_time.end()));
      response->set_data_node_grpc_start_time(*std::max_element(data_node_grpc_start_time.begin(), data_node_grpc_start_time.end()));

      std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                << "read from datanodes success!" << std::endl;
      std::chrono::high_resolution_clock::time_point decode_start_time = std::chrono::high_resolution_clock::now();
      std::vector<int> block_idxs;
      for (int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        block_idxs.push_back(recovery_request->blockids(i));
      }
      std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);

      std::string failed_block_key = recovery_request->failed_block_key();
      int failed_block_id = recovery_request->failed_block_id();
      std::string replaced_node_ip = recovery_request->replaced_node_ip();
      int replaced_node_port = recovery_request->replaced_node_port();

      if (code_type == "UniLRC")
      {
        decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
      }
      else if (code_type == "AzureLRC")
      {
        decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
      }
      else if (code_type == "OptimalLRC")
      {
        decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
      }
      else if (code_type == "UniformLRC")
      {
        decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
        exit(1);
      }
      std::chrono::high_resolution_clock::time_point decode_end_time = std::chrono::high_resolution_clock::now();
      response->set_decode_start_time(std::chrono::duration_cast<std::chrono::duration<double>>(decode_start_time.time_since_epoch()).count());
      response->set_decode_end_time(std::chrono::duration_cast<std::chrono::duration<double>>(decode_end_time.time_since_epoch()).count());

      if(cross_rack_num){
        std::cout << "start to recover cross rack" << std::endl;
        char **cross_rack_bufs = new char*[cross_rack_num];
        for(int i = 0; i < cross_rack_num; i++)
        {
          cross_rack_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
        }
        std::vector<std::thread> get_from_proxies_threads;
        std::vector<double> accept_start_time(cross_rack_num, 0.0);
        for(int i = 0; i < cross_rack_num; i++)
        {
          get_from_proxies_threads.push_back(std::thread([i, this, &cross_rack_bufs, &accept_start_time]()mutable{
            //asio::io_context io_context;
            asio::ip::tcp::socket socket(this->io_context);
            //asio::ip::tcp::resolver resolver(io_context);
            this->acceptor.accept(socket);
            std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
            accept_start_time[i] = std::chrono::duration_cast<std::chrono::duration<double>>(start.time_since_epoch()).count();
            std::cout << "connected to porxy" << std::endl;
            asio::error_code error;
            size_t len = asio::read(socket, asio::buffer(cross_rack_bufs[i], this->m_sys_config->BlockSize), error);
            if(len != this->m_sys_config->BlockSize)
            {
              std::cout << "error in read" << std::endl;
            }
            asio::error_code ignore_ec;
            socket.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
            socket.close(ignore_ec);
          }));
        }
        for(int i = 0; i < cross_rack_num; i++)
        {
          get_from_proxies_threads[i].join();
        }
        double min_accept_start_time = *std::min_element(accept_start_time.begin(), accept_start_time.end());
        std::chrono::high_resolution_clock::time_point accept_end_time = std::chrono::high_resolution_clock::now();
        double time_span3 = std::chrono::duration_cast<std::chrono::duration<double>>(accept_end_time.time_since_epoch()).count() - min_accept_start_time;
        response->set_cross_rack_time(time_span3);
        std::cout << "start to xor" << std::endl;
        char **buf_ptrs = new char*[cross_rack_num + 2];
        for(int i = 0; i < cross_rack_num; i++)
        {
          buf_ptrs[i] = cross_rack_bufs[i];
        }
        buf_ptrs[cross_rack_num] = res_buf;
        buf_ptrs[cross_rack_num + 1] = real_res_buf;
        std::chrono::high_resolution_clock::time_point cross_rack_xor_start_time = std::chrono::high_resolution_clock::now();
        xor_avx(cross_rack_num + 2, m_sys_config->BlockSize, (void**)buf_ptrs);
        std::chrono::high_resolution_clock::time_point cross_rack_xor_end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> xor_time = std::chrono::duration_cast<std::chrono::duration<double>>(cross_rack_xor_end_time - cross_rack_xor_start_time);
        response->set_cross_rack_xor_time(xor_time.count());
        for(int i = 0; i < cross_rack_num; i++)
        {
          delete cross_rack_bufs[i];
        }
        delete cross_rack_bufs;
        delete buf_ptrs;
      }
      else
      {
        response->set_cross_rack_time(0);
        response->set_cross_rack_xor_time(0);
        std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode success!" << std::endl;
      }
    }
  
    std::string replaced_node_ip = recovery_request->replaced_node_ip();
    int replaced_node_port = recovery_request->replaced_node_port();
    std::cout << "[Proxy" << m_self_cluster_id << "][Degraded] send to the client" << replaced_node_ip << ":" << replaced_node_port << std::endl;
    asio::io_context io_context;
    asio::ip::tcp::socket socket(io_context);
    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(replaced_node_ip, std::to_string(replaced_node_port));
    asio::connect(socket, endpoints);
    if(cross_rack_num){
      asio::write(socket, asio::buffer(real_res_buf, m_sys_config->BlockSize));
    }
    else{
      asio::write(socket, asio::buffer(res_buf, m_sys_config->BlockSize));
    }
    asio::error_code ignore_ec;
    socket.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
    socket.close(ignore_ec);
    std::cout << "[Proxy" << m_self_cluster_id << "][Degraded Read] send to the client done" << std::endl;
    delete res_buf;
    delete real_res_buf;
    for(int i = 0; i < recovery_request->datanodeip_size(); i++)
    {
      delete get_bufs[i];
    }
  }
  catch (const std::exception &e)
  {
    std::cout << "exception" << std::endl;
    std::cerr << e.what() << '\n';
  }
  return grpc::Status::OK;
}


  // recovery
  grpc::Status ProxyImpl::recovery(
    grpc::ServerContext *context,
    const proxy_proto::RecoveryRequest *recovery_request,
    proxy_proto::RecoveryReply *response)
  {
    try
    {
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] Handle recovery" << std::endl;
      }

      std::string code_type = m_sys_config->CodeType;
      int cross_rack_num = recovery_request->cross_rack_num();
      // auto status = std::make_shared<std::vector<bool>>(recovery_request->datanodeip_size(), false);
      std::unique_ptr<bool[]> status(new bool[recovery_request->datanodeip_size()]);
      std::fill_n(status.get(), recovery_request->datanodeip_size(), false);

      //std::vector<std::vector<char>> get_bufs(recovery_request->datanodeip_size(), std::vector<char>(m_sys_config->BlockSize, 0));
      std::vector<char*> get_bufs(recovery_request->datanodeip_size());
      for(int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      }
      //std::vector<char> res_buf(m_sys_config->BlockSize, 0);
      char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      char *real_res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));

      std::vector<std::thread> get_threads;
      for (int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        get_threads.push_back(std::thread(&ProxyImpl::get_from_node, this, 
          recovery_request->blockkeys(i), get_bufs[i], m_sys_config->BlockSize, recovery_request->datanodeip(i).c_str(), 
          recovery_request->datanodeport(i), status.get(), i));
      }
      for (int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        get_threads[i].join();
      }
      
      bool all_true = std::all_of(status.get(), status.get() + recovery_request->datanodeip_size(), [](bool val)
                                  { return val == true; });
      if (!all_true)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes failed!" << std::endl;
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes success!" << std::endl;

        std::vector<int> block_idxs;
        for (int i = 0; i < recovery_request->datanodeip_size(); i++)
        {
          block_idxs.push_back(recovery_request->blockids(i));
        }
        std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);

        std::string failed_block_key = recovery_request->failed_block_key();
        int failed_block_id = recovery_request->failed_block_id();
        std::string replaced_node_ip = recovery_request->replaced_node_ip();
        int replaced_node_port = recovery_request->replaced_node_port();

        if (code_type == "UniLRC")
        {
          decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
        }
        else if (code_type == "AzureLRC")
        {
          decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
        }
        else if (code_type == "OptimalLRC")
        {
          decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
        }
        else if (code_type == "UniformLRC")
        {
          decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
          exit(1);
        }

        if(cross_rack_num){
          std::cout << "start to recover cross rack" << std::endl;
          char **cross_rack_bufs = new char*[cross_rack_num];
          for(int i = 0; i < cross_rack_num; i++)
          {
            cross_rack_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
          }
          std::vector<std::thread> get_from_proxies_threads;
          std::vector<std::string> cross_rack_ips;
          //std::vector<int> cross_rack_ports;
          //for(int i = 0; i < cross_rack_num; i++)
          //{
          //  cross_rack_ips.push_back(recovery_request->proxyip(i));
          //  cross_rack_ports.push_back(recovery_request->proxyport(i));
          //}
          //std::lock_guard<std::mutex> lock(m_mutex);
          for(int i = 0; i < cross_rack_num; i++)
          {
            get_from_proxies_threads.push_back(std::thread([i, this, &cross_rack_bufs]()mutable{
              asio::ip::tcp::socket socket(this->io_context);
              std::cout << "connecting to proxy" << std::endl;
              this->acceptor.accept(socket);
              std::cout << "connected to porxy" << std::endl;
              asio::error_code error;
              asio::read(socket, asio::buffer(cross_rack_bufs[i], this->m_sys_config->BlockSize), error);
              std::cout << "read from proxy"  << std::endl;
              if(error)
              {
                std::cout << "error in read" << std::endl;
              }
              asio::error_code ignore_ec;
              socket.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
              socket.close(ignore_ec);
            }));
          }
          for(int i = 0; i < cross_rack_num; i++)
          {
            get_from_proxies_threads[i].join();
          }

          std::cout << "start to xor" << std::endl;
          char **buf_ptrs = new char*[cross_rack_num + 2];
          for(int i = 0; i < cross_rack_num; i++)
          {
            buf_ptrs[i] = cross_rack_bufs[i];
          }
          buf_ptrs[cross_rack_num] = res_buf;
          buf_ptrs[cross_rack_num + 1] = real_res_buf;
          xor_avx(cross_rack_num + 2, m_sys_config->BlockSize, (void**)buf_ptrs);
          for(int i = 0; i < cross_rack_num; i++)
          {
            delete cross_rack_bufs[i];
          }
          delete cross_rack_bufs;
          delete buf_ptrs;
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode success!" << std::endl;
        }
        std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] send to the replaced node" << std::endl;
        // send to the replaced node
        if(cross_rack_num){
          RecoveryToDatanode(failed_block_key.c_str(), failed_block_id, real_res_buf, replaced_node_ip.c_str(), replaced_node_port);
        }
        else{
          RecoveryToDatanode(failed_block_key.c_str(), failed_block_id, res_buf, replaced_node_ip.c_str(), replaced_node_port);
        }
      }
      delete res_buf;
      delete real_res_buf;
      for(int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        delete get_bufs[i];
      }
    }
    catch (const std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cerr << e.what() << '\n';
    }
    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::recoveryBreakdown(
    grpc::ServerContext *context,
    const proxy_proto::RecoveryRequest *recovery_request,
    proxy_proto::RecoveryReply *response)
  {
    std::chrono::high_resolution_clock::time_point START = std::chrono::high_resolution_clock::now();
    response->set_grpc_start_time(std::chrono::duration_cast<std::chrono::duration<double>>(START.time_since_epoch()).count());
    try
    {
      if (IF_DEBUG)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] Handle recovery" << std::endl;
      }

      std::string code_type = m_sys_config->CodeType;
      int cross_rack_num = recovery_request->cross_rack_num();
      // auto status = std::make_shared<std::vector<bool>>(recovery_request->datanodeip_size(), false);
      std::unique_ptr<bool[]> status(new bool[recovery_request->datanodeip_size()]);
      std::fill_n(status.get(), recovery_request->datanodeip_size(), false);

      //std::vector<std::vector<char>> get_bufs(recovery_request->datanodeip_size(), std::vector<char>(m_sys_config->BlockSize, 0));
      std::vector<char*> get_bufs(recovery_request->datanodeip_size());
      for(int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        get_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      }
      //std::vector<char> res_buf(m_sys_config->BlockSize, 0);
      char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      char *real_res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));

      std::vector<double> data_node_disk_io_start_time(recovery_request->datanodeip_size(), 0.0);
      std::vector<double> data_node_disk_io_end_time(recovery_request->datanodeip_size(), 0.0);
      std::vector<double> data_node_network_start_time(recovery_request->datanodeip_size(), 0.0);
      std::vector<double> data_node_network_end_time(recovery_request->datanodeip_size(), 0.0);
      std::vector<double> data_node_grpc_notify_time(recovery_request->datanodeip_size(), 0.0);
      std::vector<double> data_node_grpc_start_time(recovery_request->datanodeip_size(), 0.0);

      std::vector<std::thread> get_threads;
      for (int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        get_threads.push_back(std::thread(&ProxyImpl::get_from_node_breakdown, this, 
          recovery_request->blockkeys(i), get_bufs[i], m_sys_config->BlockSize, recovery_request->datanodeip(i).c_str(), 
          recovery_request->datanodeport(i), status.get(), i, &data_node_disk_io_start_time[i], &data_node_disk_io_end_time[i],
          &data_node_network_start_time[i], &data_node_network_end_time[i], &data_node_grpc_notify_time[i], &data_node_grpc_start_time[i]));
      }
      for (int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        get_threads[i].join();
      }

      response->set_disk_io_start_time(*std::min_element(data_node_disk_io_start_time.begin(), data_node_disk_io_start_time.end()));
      response->set_disk_io_end_time(*std::max_element(data_node_disk_io_end_time.begin(), data_node_disk_io_end_time.end()));
      response->set_network_start_time(*std::min_element(data_node_network_start_time.begin(), data_node_network_start_time.end()));
      response->set_network_end_time(*std::max_element(data_node_network_end_time.begin(), data_node_network_end_time.end()));
      response->set_data_node_grpc_notify_time(*std::min_element(data_node_grpc_notify_time.begin(), data_node_grpc_notify_time.end()));
      response->set_data_node_grpc_start_time(*std::max_element(data_node_grpc_start_time.begin(), data_node_grpc_start_time.end()));
      
      bool all_true = std::all_of(status.get(), status.get() + recovery_request->datanodeip_size(), [](bool val)
                                  { return val == true; });
      if (!all_true)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes failed!" << std::endl;
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes success!" << std::endl;

        std::vector<int> block_idxs;
        for (int i = 0; i < recovery_request->datanodeip_size(); i++)
        {
          block_idxs.push_back(recovery_request->blockids(i));
        }
        std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs);

        std::string failed_block_key = recovery_request->failed_block_key();
        int failed_block_id = recovery_request->failed_block_id();
        std::string replaced_node_ip = recovery_request->replaced_node_ip();
        int replaced_node_port = recovery_request->replaced_node_port();

        std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
        if (code_type == "UniLRC")
        {
          decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize);
        }
        else if (code_type == "AzureLRC")
        {
          decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
        }
        else if (code_type == "OptimalLRC")
        {
          decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
        }
        else if (code_type == "UniformLRC")
        {
          decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, recovery_request->datanodeip_size(), &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_buf), m_sys_config->BlockSize, failed_block_id);
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
          exit(1);
        }
        std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
        response->set_decode_start_time(std::chrono::duration_cast<std::chrono::duration<double>>(t3.time_since_epoch()).count());
        response->set_decode_end_time(std::chrono::duration_cast<std::chrono::duration<double>>(t4.time_since_epoch()).count());

        if(cross_rack_num){
          std::cout << "start to recover cross rack" << std::endl;
          char **cross_rack_bufs = new char*[cross_rack_num];
          for(int i = 0; i < cross_rack_num; i++)
          {
            cross_rack_bufs[i] = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
          }
          std::vector<std::thread> get_from_proxies_threads;
          std::vector<double> accept_start_time(cross_rack_num, 0.0);
          for(int i = 0; i < cross_rack_num; i++)
          {
            get_from_proxies_threads.push_back(std::thread([i, this, &cross_rack_bufs, &accept_start_time]()mutable{
              //asio::io_context io_context;
              asio::ip::tcp::socket socket(this->io_context);
              //asio::ip::tcp::resolver resolver(io_context);
              std::cout << "connecting to proxy" << std::endl;
              this->acceptor.accept(socket);
              std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
              accept_start_time[i] = std::chrono::duration_cast<std::chrono::duration<double>>(start.time_since_epoch()).count();
              std::cout << "connected to porxy" << std::endl;
              asio::error_code error;
              asio::read(socket, asio::buffer(cross_rack_bufs[i], this->m_sys_config->BlockSize), error);
              std::cout << "read from proxy"  << std::endl;
              if(error)
              {
                std::cout << "error in read" << std::endl;
              }
              asio::error_code ignore_ec;
              socket.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
              socket.close(ignore_ec);
            }));
          }
          for(int i = 0; i < cross_rack_num; i++)
          {
            get_from_proxies_threads[i].join();
          }
          double min_accept_start_time = *std::min_element(accept_start_time.begin(), accept_start_time.end());
          std::chrono::high_resolution_clock::time_point accept_end_time = std::chrono::high_resolution_clock::now();
          double time_span3 = std::chrono::duration_cast<std::chrono::duration<double>>(accept_end_time.time_since_epoch()).count() - min_accept_start_time;
          response->set_cross_rack_time(time_span3);
          std::cout << "start to xor" << std::endl;
          char **buf_ptrs = new char*[cross_rack_num + 2];
          for(int i = 0; i < cross_rack_num; i++)
          {
            buf_ptrs[i] = cross_rack_bufs[i];
          }
          buf_ptrs[cross_rack_num] = res_buf;
          buf_ptrs[cross_rack_num + 1] = real_res_buf;
          std::chrono::high_resolution_clock::time_point t7 = std::chrono::high_resolution_clock::now();
          xor_avx(cross_rack_num + 2, m_sys_config->BlockSize, (void**)buf_ptrs);
          std::chrono::high_resolution_clock::time_point t8 = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double> time_span4 = std::chrono::duration_cast<std::chrono::duration<double>>(t8 - t7);
          response->set_cross_rack_xor_time(time_span4.count());
          for(int i = 0; i < cross_rack_num; i++)
          {
            delete cross_rack_bufs[i];
          }
          delete cross_rack_bufs;
          delete buf_ptrs;
        }
        else
        {
          response->set_cross_rack_time(0);
          response->set_cross_rack_xor_time(0);
          std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] decode success!" << std::endl;
        }
        std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] send to the replaced node" << std::endl;
        // send to the replaced node
        double dest_data_node_network_time, dest_data_node_disk_io_time;
        if(cross_rack_num){
          RecoveryToDatanodeBreakdown(failed_block_key.c_str(), failed_block_id, real_res_buf, replaced_node_ip.c_str(), replaced_node_port, 
            &dest_data_node_network_time, &dest_data_node_disk_io_time);
        }
        else{
          RecoveryToDatanodeBreakdown(failed_block_key.c_str(), failed_block_id, res_buf, replaced_node_ip.c_str(), replaced_node_port, 
            &dest_data_node_network_time, &dest_data_node_disk_io_time);
        }
        response->set_dest_data_node_network_time(dest_data_node_network_time);
        response->set_dest_data_node_disk_io_time(dest_data_node_disk_io_time);
      }
      delete res_buf;
      delete real_res_buf;
      for(int i = 0; i < recovery_request->datanodeip_size(); i++)
      {
        delete get_bufs[i];
      }
    }
    catch (const std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cerr << e.what() << '\n';
    }
    return grpc::Status::OK;
  }


  grpc::Status ProxyImpl::multipleRecovery(
    grpc::ServerContext *context,
    const proxy_proto::MultipleRecoveryRequest *multiple_recovery_request,
    proxy_proto::GetReply *response)
  {
    std::string code_type = m_sys_config->CodeType;
    int failed_block_num = multiple_recovery_request->failed_block_num();
    std::vector<int> failed_block_ids;
    std::vector<int> failed_block_stripe_ids;
    std::vector<std::string> replaced_node_ips;
    std::vector<int> replaced_node_ports;
    std::vector<int> datanode_num;
    std::vector<int> cross_rack_nums;
    std::vector<std::string> failed_block_keys;
    for(int i = 0; i < failed_block_num; i++)
    {
      failed_block_ids.push_back(multiple_recovery_request->failed_block_id(i));
      failed_block_stripe_ids.push_back(multiple_recovery_request->failed_block_stripe_id(i));
      replaced_node_ips.push_back(multiple_recovery_request->replaced_node_ip(i));
      replaced_node_ports.push_back(multiple_recovery_request->replaced_node_port(i));
      datanode_num.push_back(multiple_recovery_request->datanode_num(i));
      cross_rack_nums.push_back(multiple_recovery_request->cross_rack_num(i));
      failed_block_keys.push_back(multiple_recovery_request->failed_block_key(i));
    }

    std::unordered_map<int, int> stripe_id_to_idx;
    for(int i = 0; i < failed_block_num; i++)
    {
      stripe_id_to_idx[failed_block_stripe_ids[i]] = i;
    }

    std::vector<std::vector<std::string>> block_keys;
    std::vector<std::vector<int>> block_ids;
    std::vector<std::vector<std::string>> datanode_ips;
    std::vector<std::vector<int>> datanode_ports;

    int total_datanode_num = 0;
    for(int i = 0; i < failed_block_num; i++)
    {
      std::vector<std::string> block_keys_temp;
      std::vector<int> block_ids_temp;
      std::vector<std::string> datanode_ips_temp;
      std::vector<int> datanode_ports_temp;
      for(int j = 0; j < datanode_num[i]; j++)
      {
        block_keys_temp.push_back(multiple_recovery_request->blockkeys(j + total_datanode_num));
        block_ids_temp.push_back(multiple_recovery_request->blockids(j + total_datanode_num));
        datanode_ips_temp.push_back(multiple_recovery_request->datanodeip(j + total_datanode_num)); 
        datanode_ports_temp.push_back(multiple_recovery_request->datanodeport(j + total_datanode_num));
      }
      total_datanode_num += datanode_num[i];
      block_ids.push_back(block_ids_temp);
      block_keys.push_back(block_keys_temp);
      datanode_ips.push_back(datanode_ips_temp);
      datanode_ports.push_back(datanode_ports_temp);
    }
    std::vector<std::vector<char *>> get_bufs;
    for(int i = 0; i < failed_block_num; i++)
    {
      std::vector<char *> get_bufs_temp;
      for(int j = 0; j < datanode_num[i]; j++)
      {
        char *get_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
        memset(get_buf, 0, m_sys_config->BlockSize);
        get_bufs_temp.push_back(get_buf);
      }
      get_bufs.push_back(get_bufs_temp);
    }
    std::vector<char *> res_bufs;
    std::vector<char *> real_res_bufs;
    for(int i = 0; i < failed_block_num; i++)
    {
      char *res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      memset(res_buf, 0, m_sys_config->BlockSize);
      res_bufs.push_back(res_buf);
      char *real_res_buf = static_cast<char*>(std::aligned_alloc(32, m_sys_config->BlockSize));
      memset(real_res_buf, 0, m_sys_config->BlockSize);
      real_res_bufs.push_back(real_res_buf);
    }
    std::vector<std::unique_ptr<bool[]>> status;
    for(int i = 0; i < failed_block_num; i++)
    {
      std::unique_ptr<bool[]> status_temp(new bool[datanode_num[i]]);
      std::fill_n(status_temp.get(), datanode_num[i], false);
      status.push_back(std::move(status_temp));
    }

    std::vector<std::thread> get_threads;
    for(int i = 0; i < failed_block_num; i++)
    {
      for(int j = 0; j < datanode_num[i]; j++)
      {
        get_threads.push_back(std::thread(&ProxyImpl::get_from_node, this, block_keys[i][j], get_bufs[i][j], m_sys_config->BlockSize, datanode_ips[i][j].c_str(), datanode_ports[i][j], status[i].get(), j));
      }
    }

    for(int i = 0; i < failed_block_num; i++)
    {
      for(int j = 0; j < datanode_num[i]; j++)
      {
        get_threads[i * datanode_num[i] + j].join();
      }
    }
    for(int i = 0; i < failed_block_num; i++)
    {
      bool all_true = std::all_of(status[i].get(), status[i].get() + datanode_num[i], [](bool val)
                                  { return val == true; });
      if (!all_true)
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes failed!" << std::endl;
      }
      else
      {
        std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                  << "read from datanodes success!" << std::endl;
        for(int i = 0; i < failed_block_num; i++)
        {
          std::vector<int> block_idxs;
          for (int j = 0; j < datanode_num[i]; j++)
          {
            block_idxs.push_back(block_ids[i][j]);
          }
          std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs[i]);
          if(code_type == "UniLRC")
          {
            decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize);
          }
          else if(code_type == "AzureLRC")
          {
            decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize, failed_block_ids[i]);
          }
          else if(code_type == "OptimalLRC")
          {
            decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize, failed_block_ids[i]);
          }
          else if(code_type == "UniformLRC")
          {
            decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize, failed_block_ids[i]);
          }
          else
          {
            std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
            exit(1);
          }
        }
      }
    }
    std::cout << "Partial Decode Done" << std::endl;

    /*std::vector<std::thread> get_and_decode_threads;
    for(int i = 0; i < failed_block_num; i++)
    {
      get_and_decode_threads.push_back(std::thread([&](){
        for(int j = 0; j < datanode_num[i]; j++)
        {
          get_from_node(block_keys[i][j], get_bufs[i][j], m_sys_config->BlockSize, datanode_ips[i][j].c_str(), datanode_ports[i][j], status[i].get(), j);
        }
        bool all_true = std::all_of(status[i].get(), status[i].get() + datanode_num[i], [](bool val)
                                  { return val == true; });
        if (!all_true)
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                    << "read from datanodes failed!" << std::endl;
        }
        else
        {
          std::cout << "[Proxy" << m_self_cluster_id << "][GET]"
                    << "read from datanodes success!" << std::endl;
          std::vector<int> block_idxs;
          for (int j = 0; j < datanode_num[i]; j++)
          {
            block_idxs.push_back(block_ids[i][j]);
          }
          std::vector<unsigned char *> block_ptrs = convertToUnsignedCharArray(get_bufs[i]);
          if(code_type == "UniLRC")
          {
            decode_unilrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize);
          }
          else if(code_type == "AzureLRC")
          {
            decode_azure_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize, failed_block_ids[i]);
          }
          else if(code_type == "OptimalLRC")
          {
            decode_optimal_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize, failed_block_ids[i]);
          }
          else if(code_type == "UniformLRC")
          {
            decode_uniform_lrc(m_sys_config->k, m_sys_config->r, m_sys_config->z, datanode_num[i], &block_idxs, block_ptrs.data(), reinterpret_cast<unsigned char *>(res_bufs[i]), m_sys_config->BlockSize, failed_block_ids[i]);
          }
          else
          {
            std::cout << "[Proxy" << m_self_cluster_id << "][Degrade read] code type error!" << std::endl;
            exit(1);
          }
        }
      }));
    }
    for(int i = 0; i < failed_block_num; i++)
    {
      get_and_decode_threads[i].join();
    }
    std::cout << "Partial Decode Done" << std::endl;*/

    std::vector<std::vector<char *>> cross_rack_bufs;
    for(int i = 0; i < failed_block_num; i++)
    {
      std::vector<char *> cross_rack_bufs_temp;
      cross_rack_bufs.push_back(cross_rack_bufs_temp);
    }
    int total_cross_rack_num = 0;
    for(int i = 0; i < failed_block_num; i++)
    {
      total_cross_rack_num += cross_rack_nums[i];
    }
    std::vector<std::thread> get_from_proxies_threads;
    for(int i = 0; i < total_cross_rack_num; i++)
    {
      get_from_proxies_threads.push_back(std::thread([&]()mutable{
        asio::ip::tcp::socket socket(this->io_context);
        this->acceptor.accept(socket);
        asio::error_code error;
        int stripe_id;
        asio::read(socket, asio::buffer(&stripe_id, sizeof(int)), error);
        char *read_buf = static_cast<char*>(std::aligned_alloc(32, this->m_sys_config->BlockSize));
        size_t len = asio::read(socket, asio::buffer(read_buf, this->m_sys_config->BlockSize), error);
        if(len != this->m_sys_config->BlockSize)
        {
          std::cout << "error in read" << std::endl;
        }
        asio::error_code ignore_ec;
        socket.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
        socket.close(ignore_ec);
        int idx = stripe_id_to_idx[stripe_id];
        cross_rack_bufs[idx].push_back(read_buf);
      }));
    }
    for(int i = 0; i < total_cross_rack_num; i++)
    {
      get_from_proxies_threads[i].join();
    }
    std::cout << "Cross Rack Read Done" << std::endl;
    std::vector<std::vector<char *>> buf_ptrs;
    for(int i = 0; i < failed_block_num; i++)
    {
      std::vector<char *> buf_ptrs_temp;
      for(int j = 0; j < cross_rack_bufs[i].size(); j++)
      {
        buf_ptrs_temp.push_back(cross_rack_bufs[i][j]);
      }
      buf_ptrs_temp.push_back(res_bufs[i]);
      buf_ptrs_temp.push_back(real_res_bufs[i]);
      buf_ptrs.push_back(buf_ptrs_temp);
    }
    std::vector<std::thread> xor_threads;
    for(int i = 0; i < failed_block_num; i++)
    {
      if(cross_rack_nums[i])
      {
        xor_threads.push_back(std::thread(&xor_avx, cross_rack_bufs[i].size() + 2, m_sys_config->BlockSize, (void**)buf_ptrs[i].data()));
      }
    }
    for(auto &t : xor_threads)
    {
      t.join();
    }
    std::cout << "XOR Done" << std::endl;
    for(int i = 0; i < failed_block_num; i++)
    {
      std::string replaced_node_ip = replaced_node_ips[i];
      int replaced_node_port = replaced_node_ports[i];
      std::cout << "[Proxy" << m_self_cluster_id << "][Recovery] send to the replaced node" << replaced_node_ip << ":" << replaced_node_port << std::endl;
      // send to the replaced node
      if(!cross_rack_nums[i])
      {
        RecoveryToDatanode(failed_block_keys[i].c_str(), failed_block_ids[i], real_res_bufs[i], replaced_node_ip.c_str(), replaced_node_port);
      }
      else
      {
        RecoveryToDatanode(failed_block_keys[i].c_str(), failed_block_ids[i], res_bufs[i], replaced_node_ip.c_str(), replaced_node_port);
      }
      //RecoveryToDatanode(failed_block_keys[i].c_str(), failed_block_ids[i], real_res_bufs[i], replaced_node_ip.c_str(), replaced_node_port);
    }
    for(int i = 0; i < failed_block_num; i++)
    {
      delete res_bufs[i];
      delete real_res_bufs[i];
      for(int j = 0; j < datanode_num[i]; j++)
      {
        delete get_bufs[i][j];
      }
    }
    for(int i = 0; i < failed_block_num; i++)
    {
      for(int j = 0; j < cross_rack_bufs[i].size(); j++)
      {
        delete cross_rack_bufs[i][j];
      }
    }
    return grpc::Status::OK;
  }
  // delete
  grpc::Status ProxyImpl::deleteBlock(
      grpc::ServerContext *context,
      const proxy_proto::NodeAndBlock *node_and_block,
      proxy_proto::DelReply *response)
  {
    std::vector<std::string> blocks_id;
    std::vector<std::string> nodes_ip_port;
    std::string key = node_and_block->key();
    int stripe_id = node_and_block->stripe_id();
    for (int i = 0; i < node_and_block->blockkeys_size(); i++)
    {
      blocks_id.push_back(node_and_block->blockkeys(i));
      std::string ip_port = node_and_block->datanodeip(i) + ":" + std::to_string(node_and_block->datanodeport(i));
      nodes_ip_port.push_back(ip_port);
    }
    auto delete_blocks = [this, key, blocks_id, stripe_id, nodes_ip_port]() mutable
    {
      auto request_and_delete = [this](std::string block_key, std::string node_ip_port)
      {
        bool ret = DelInDatanode(block_key, node_ip_port);
        if (!ret)
        {
          std::cout << "Delete value no return!" << std::endl;
          return;
        }
      };
      try
      {
        std::vector<std::thread> senders;
        for (int j = 0; j < int(blocks_id.size()); j++)
        {
          senders.push_back(std::thread(request_and_delete, blocks_id[j], nodes_ip_port[j]));
        }

        for (int j = 0; j < int(senders.size()); j++)
        {
          senders[j].join();
        }

        if (stripe_id != -1 || key != "")
        {
          grpc::ClientContext c_context;
          coordinator_proto::CommitAbortKey commit_abort_key;
          coordinator_proto::ReplyFromCoordinator rep;
          ECProject::OpperateType opp = DEL;
          commit_abort_key.set_opp(opp);
          commit_abort_key.set_key(key);
          commit_abort_key.set_ifcommitmetadata(true);
          commit_abort_key.set_stripe_id(stripe_id);
          grpc::Status stat;
          stat = m_coordinator_ptr->reportCommitAbort(&c_context, commit_abort_key, &rep);
        }
      }
      catch (const std::exception &e)
      {
        std::cout << "exception" << std::endl;
        std::cerr << e.what() << '\n';
      }
    };
    try
    {
      std::thread my_thread(delete_blocks);
      my_thread.detach();
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::getBlocks(grpc::ServerContext *context,
    const proxy_proto::StripeAndBlockIDs *request, proxy_proto::GetReply *response)
  {
    std::cout << "getting blocks" << "[" << request->block_ids(0) << "]" << "to" << "[" << request->block_ids(request->block_ids_size() - 1) << "]" << std::endl;
    int BlockSize = m_sys_config->BlockSize;
    size_t total_size = static_cast<size_t> (BlockSize) * request->block_ids_size();
    char *blocks = new char[total_size];
    uint32_t group_id = request->group_id();

    std::vector<std::thread> get_threads;
    for(int i = 0; i < request->block_ids_size(); i++)
    {
      get_threads.push_back(std::thread([this, i, &blocks, &request, BlockSize]() {
        this->GetFromDatanode(
            request->block_keys(i), 
            blocks + i * BlockSize,
            static_cast<size_t>(m_sys_config->BlockSize), 
            request->datanodeips(i).c_str(), 
            static_cast<int>(request->datanodeports(i))
        );    
      }));
      asio::error_code error;
      asio::io_context io_context;
      asio::ip::tcp::socket socket_data(io_context);
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(request->clientip(), std::to_string(request->clientport()));;
      socket_data.connect(*endpoints, error);
      if (error)
      {
        std::cout << "error in connect" << std::endl;
      }
      std::cout << "connected to client" << std::endl;
      u_int32_t block_id = request->block_ids(i);
      asio::write(socket_data, asio::buffer(&block_id, sizeof(u_int32_t)));
      asio::write(socket_data, asio::buffer(blocks + i * static_cast<size_t>(BlockSize), BlockSize));
      asio::error_code ignore_ec;
      socket_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      socket_data.close(ignore_ec);
    }

    for(int i = 0; i < request->block_ids_size(); i++)
    {
      get_threads[i].join();
    }

    delete blocks;
    return grpc::Status();
  }

} // namespace ECProject