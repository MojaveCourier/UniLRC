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
    }

    ~Client()
    {
      delete[] m_pre_allocated_buffer;
    }

    std::string sayHelloToCoordinatorByGrpc(std::string hello);
    bool append(int append_size);
    bool sub_append(int append_size);
    bool set(std::string key, std::string value);
    bool SetParameterByGrpc(ECSchema input_ecschema);
    bool get(std::string key, std::string &value);
    bool delete_key(std::string key);
    bool delete_stripe(int stripe_id);
    bool delete_all_stripes();
    void encode();

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
  };

} // namespace ECProject

#endif // CLIENT_H