#include "datanode.h"

int main(int argc, char **argv)
{
    // pid_t pid = fork();
    // if (pid > 0)
    // {
    //     exit(0);
    // }
    // setsid();
    if (true)
    {
        umask(0);
        close(STDIN_FILENO);
        // close(STDOUT_FILENO);
        // close(STDERR_FILENO);
    }

    std::string ip_and_port(argv[1]);
    // std::string ip = ip_and_port.substr(0, ip_and_port.find(":"));
    // int port = std::stoi(ip_and_port.substr(ip_and_port.find(":") + 1, ip_and_port.size()));
    char buff[256];
    getcwd(buff, 256);
    std::string exe_path(argv[0]);
    std::string exe_dir;
    size_t last_slash = exe_path.rfind('/');
    if (last_slash != std::string::npos) {
        exe_dir = exe_path.substr(0, last_slash);
        if (exe_dir.empty() || exe_dir[0] != '/')
            exe_dir = std::string(buff) + "/" + exe_dir;
    } else {
        exe_dir = buff;
    }
    std::string sys_config_path = exe_dir + "/../../config/parameterConfiguration.xml";
    // std::string sys_config_path = "/home/GuanTian/lql/UniLRC/project/config/parameterConfiguration.xml";
    ECProject::DataNode datanode(ip_and_port, sys_config_path);
    datanode.Run();
    return 0;
}
