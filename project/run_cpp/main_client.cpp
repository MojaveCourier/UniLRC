#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"

int main(int argc, char **argv)
{
    char buff[256];
    getcwd(buff, 256);
    std::string cwf = std::string(argv[0]);
    // std::string sys_config_path = std::string(buff) + cwf.substr(1, cwf.rfind('/') - 1) + "/../../config/parameterConfiguration.xml";
    std::string sys_config_path = "/home/GuanTian/lql/UniEC/project/config/parameterConfiguration.xml";
    std::cout << "Current working directory: " << sys_config_path << std::endl;

    ECProject::Config *config = ECProject::Config::getInstance(sys_config_path);
    std::string client_ip = "0.0.0.0";
    int client_port = 44444;
    ECProject::Client client(client_ip, client_port, config->CoordinatorIP + ":" + std::to_string(config->CoordinatorPort), sys_config_path);
    std::cout << client.sayHelloToCoordinatorByGrpc("Client ID: " + client_ip + ":" + std::to_string(client_port)) << std::endl;

    // // Test multiple append operations with different sizes
    // // std::vector<int> append_sizes = {
    // //     4 * 1024,
    // //     8 * 1024,
    // //     12 * 1024,
    // //     16 * 1024,
    // //     24 * 1024,
    // //     4 * 1024};
    // std::vector<int> append_sizes = {
    //     480 * 1024};

    // // n=10, k=4, r=4. z=2, group_size=5
    // std::cout << "Starting multiple append tests with different sizes..." << std::endl;

    // // Test regular append with different sizes
    // for (size_t i = 0; i < append_sizes.size(); i++)
    // {
    //     std::cout << "\n[Test " << i + 1 << "/" << append_sizes.size() << "]" << std::endl;
    //     std::cout << "Testing append with size: " << append_sizes[i] << " bytes" << std::endl;

    //     bool append_result = client.append(append_sizes[i]);
    //     if (append_result)
    //     {
    //         std::cout << "Append operation succeeded for size " << append_sizes[i] << " bytes" << std::endl;
    //     }
    //     else
    //     {
    //         std::cout << "Append operation failed for size " << append_sizes[i] << " bytes" << std::endl;
    //         break;
    //     }
    // }

    // Test set() interface with three calls using loop
    std::cout << "\nTesting set() interface..." << std::endl;

    const int NUM_TESTS = 2;
    for (int i = 0; i < NUM_TESTS; i++)
    {
        std::cout << "\n[Test " << i + 1 << "/" << NUM_TESTS << "] Calling set()" << std::endl;
        bool set_result = client.set();
        if (set_result)
        {
            std::cout << "Set operation " << i + 1 << " succeeded" << std::endl;
        }
        else
        {
            std::cout << "Set operation " << i + 1 << " failed" << std::endl;
            break;
        }
    }

    return 0;
}