#ifndef CLIENT_H
#define CLIENT_H

#ifdef BAZEL_BUILD
#include "src/proto/coordinator.grpc.pb.h"
#else
#include "coordinator.grpc.pb.h"
#endif

#include "meta_definition.h"
#include <grpcpp/grpcpp.h>
#include <asio.hpp>
#include "config.h"
#include "toolbox.h"
#include <vector>
namespace ECProject
{
  class Client
  {
  public:
    Client(std::string ClientIP, int ClientPort, std::string CoordinatorIpPort) : m_coordinatorIpPort(CoordinatorIpPort),
                                                                                  m_clientIPForGet(ClientIP),
                                                                                  m_clientPortForGet(ClientPort),
                                                                                  acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(ClientIP.c_str()), m_clientPortForGet))
    {
      auto channel = grpc::CreateChannel(m_coordinatorIpPort, grpc::InsecureChannelCredentials());
      m_coordinator_ptr = coordinator_proto::coordinatorService::NewStub(channel);
      m_clientID = ClientIP + ":" + std::to_string(ClientPort);
    }

    Client(std::string ClientIP, int ClientPort, std::string CoordinatorIpPort, std::string config_path) : m_coordinatorIpPort(CoordinatorIpPort),
                                                                                                           m_clientIPForGet(ClientIP),
                                                                                                           m_clientPortForGet(ClientPort),
                                                                                                           acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(ClientIP.c_str()), m_clientPortForGet))
    {
      auto channel = grpc::CreateChannel(m_coordinatorIpPort, grpc::InsecureChannelCredentials());
      m_coordinator_ptr = coordinator_proto::coordinatorService::NewStub(channel);
      m_clientID = ClientIP + ":" + std::to_string(ClientPort);
      m_sys_config = ECProject::Config::getInstance(config_path);
      m_toolbox = ECProject::ToolBox::getInstance();
      m_pre_allocated_buffer = new char[m_sys_config->BlockSize * m_sys_config->n];
      if (m_sys_config->AppendMode == "CACHED_MODE")
      {
        m_cached_buffer = new char *[m_sys_config->r + m_sys_config->z];
        for (int i = 0; i < m_sys_config->r + m_sys_config->z; i++)
        {
          m_cached_buffer[i] = new char[m_sys_config->BlockSize];
          memset(m_cached_buffer[i], 0, m_sys_config->BlockSize);
        }
      }
    }

    ~Client()
    {
      delete[] m_pre_allocated_buffer;
      if (m_sys_config->AppendMode == "CACHED_MODE")
      {
        for (int i = 0; i < m_sys_config->r + m_sys_config->z; i++)
        {
          delete[] m_cached_buffer[i];
        }
        delete[] m_cached_buffer;
      }
    }

    std::string sayHelloToCoordinatorByGrpc(std::string hello);
    bool append(int append_size);
    bool sub_append(int append_size);
    bool sub_append_in_rep_mode(int append_size);
    bool set();
    bool degraded_read(int stripe_id, int failed_block_id, std::string &value);
    bool recovery(int stripe_id, int failed_block_id);
    std::vector<int> get_data_block_num_per_group(int k, int r, int z, std::string code_type);
    std::vector<int> get_global_parity_block_num_per_group(int k, int r, int z, std::string code_type);
    std::vector<int> get_local_parity_block_num_per_group(int k, int r, int z, std::string code_type);
    bool set(std::string key, std::string value);
    bool SetParameterByGrpc(ECSchema input_ecschema);
    bool get(std::string key, std::string &value);
    bool delete_key(std::string key);
    bool delete_stripe(int stripe_id);
    bool delete_all_stripes();
    int get_append_slice_plans(std::string append_mode, int curr_logical_offset, int append_size, std::vector<std::vector<int>> *node_slice_sizes_per_cluster, std::vector<int> *modified_data_block_nums_per_cluster, std::vector<int> *data_ptr_size_array, int &parity_slice_size, int &parity_slice_offset);
    void split_for_append_data_and_parity(const coordinator_proto::ReplyProxyIPsPorts *reply_proxy_ips_ports, const std::vector<char *> &cluster_slice_data, const std::vector<std::vector<int>> &node_slice_sizes_per_cluster, const std::vector<int> &modified_data_block_nums_per_cluster, std::vector<char *> &data_ptr_array, std::vector<char *> &global_parity_ptr_array, std::vector<char *> &local_parity_ptr_array);
    void split_for_set_data_and_parity(const coordinator_proto::ReplyProxyIPsPorts *reply_proxy_ips_ports, const std::vector<char *> &cluster_slice_data, const std::vector<int> &data_block_num_per_group, const std::vector<int> &global_parity_block_num_per_group, const std::vector<int> &local_parity_block_num_per_group, std::vector<char *> &data_ptr_array, std::vector<char *> &global_parity_ptr_array, std::vector<char *> &local_parity_ptr_array);
    void async_append_to_proxies(char *cluster_slice_data, std::string append_key, int cluster_slice_size, std::string proxy_ip, int proxy_port, int index, bool *if_commit_arr);
    void get_cached_parity_slices(std::vector<char *> &global_parity_ptr_array, std::vector<char *> &local_parity_ptr_array, const int parity_slice_size, const int parity_slice_offset);
    void cache_latest_parity_slices(std::vector<char *> &global_parity_ptr_array, std::vector<char *> &local_parity_ptr_array, const int parity_slice_size, const int parity_slice_offset);

  private:
    std::unique_ptr<coordinator_proto::coordinatorService::Stub> m_coordinator_ptr;
    std::string m_coordinatorIpPort;
    std::string m_clientIPForGet;
    int m_clientPortForGet;
    std::string m_clientID;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;

    int m_append_logical_offset = 0;
    ECProject::Config *m_sys_config;
    ECProject::ToolBox *m_toolbox;
    char *m_pre_allocated_buffer = nullptr;
    char **m_cached_buffer = nullptr;
  };

} // namespace ECProject

#endif // CLIENT_H