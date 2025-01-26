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
    std::string sys_config_path = std::string(buff) + cwf.substr(1, cwf.rfind('/') - 1) + "/../../config/parameterConfiguration.xml";
    //std::string sys_config_path = "/home/GuanTian/lql/UniEC/project/config/parameterConfiguration.xml";
    std::cout << "Current working directory: " << sys_config_path << std::endl;

    ECProject::Config *config = ECProject::Config::getInstance(sys_config_path);
    std::string client_ip = "0.0.0.0";
    int client_port = 44444;
    ECProject::Client client(client_ip, client_port, config->CoordinatorIP + ":" + std::to_string(config->CoordinatorPort), sys_config_path);
    std::cout << client.sayHelloToCoordinatorByGrpc("Client ID: " + client_ip + ":" + std::to_string(client_port)) << std::endl;

    // Test multiple append operations with different sizes
    // std::vector<int> append_sizes = {
    //     4 * 1024,
    //     8 * 1024,
    //     12 * 1024,
    //     16 * 1024,
    //     24 * 1024,
    //     4 * 1024};

    // alicloud trace
    /*
    std::vector<int> append_sizes = {
        77824,
        12288,
        12288,
        4096,
        4096,
        12288,
        4096,
        4096,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        40960,
        8192,
        12288,
        4096,
        4096,
        8192,
        8192,
        16384,
        24576,
        4096,
        4096,
        4096,
        20480,
        4096,
        12288,
        4096,
        8192,
        4096,
        12288,
        28672,
        4096,
        61440,
        4096,
        12288,
        4096,
        4096,
        12288,
        4096,
        4096,
        4096,
        12288,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        4096,
        4096,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        20480,
        4096,
        192512,
        8192,
        4096,
        4096,
        4096,
        4096,
        16384,
        32768,
        4096,
        8192,
        20480,
        4096,
        16384,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        131072,
        45056,
        8192,
        4096,
        4096,
        4096,
        1536,
        4096,
        32768,
        57344,
        4096,
        4096,
        4096,
        4096,
        11264,
        4096,
        266240,
        4096,
        4096,
        1024,
        266240,
        4096,
        4096,
        4096,
        4096,
        393216,
        98304,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        1024,
        28672,
        4096,
        4096,
        4096,
        4096,
        65536,
        61440,
        4096,
        12288,
        12288,
        65536,
        98304,
        12288,
        4096,
        4096,
        4096,
        12288,
        4096,
        1024,
        8192,
        4096,
        4096,
        8192,
        53248,
        16384,
        4096,
        1024,
        4096,
        4096,
        4096,
        32768,
        4096,
        1536,
        4096,
        8192,
        4096,
        4096,
        12288,
        8192,
        4096,
        8192,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        16384,
        8192,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        8192,
        4096,
        8192,
        12288,
        4096,
        28672,
        1024,
        4096,
        8192,
        4096,
        20480,
        4096,
        45056,
        4096,
        12288,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        8192,
        8192,
        12288,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        1024,
        8192,
        69632,
        4096,
        4096,
        4096,
        4096,
        4096,
        1024,
        4096,
        28672,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        1536,
        1024,
        4096,
        4096,
        8192,
        4096,
        1024,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        4096,
        8192,
        4096,
        16384,
        8192,
        20480,
        4096,
        4096,
        4096,
        24576,
        4096,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        4096,
        12288,
        4096,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        1536,
        4096,
        4096,
        4096,
        4096,
        24576,
        4096,
        4096,
        4096,
        77824,
        4096,
        8192,
        4096,
        4096,
        4096,
        32768,
        28672,
        4096,
        8192,
        32256,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        4096,
        28672,
        65536,
        4096,
        4096,
        20480,
        4096,
        4096,
        12288,
        4096,
        4096,
        4096,
        4096,
        12288,
        8192,
        12288,
        4096,
        4096,
        24576,
        8192,
        4096,
        1536,
        118784,
        4096,
        4096,
        4096,
        4096,
        4096,
        524288,
        12288,
        4096,
        458752,
        16384,
        49152,
        16384,
        458752,
        12288,
        524288,
        28672,
        458752,
        4096,
        8192,
        57344,
        524288,
        524288,
        4096,
        520192,
        499712,
        4096,
        65536,
        24576,
        524288,
        458752,
        4096,
        458752,
        69632,
        4096,
        4096,
        524288,
        516096,
        73728,
        495616,
        28672,
        483328,
        524288,
        40960,
        8192,
        458752,
        458752,
        524288,
        4096,
        475136,
        32768,
        512000,
        49152,
        495616,
        32768,
        4096,
        1536,
        479232,
        77824,
        94208,
        45056,
        524288,
        4096,
        4096,
        524288,
        491520,
        6656,
        32768,
        397312,
        4096,
        4096,
        36864,
        4096,
        8192,
        8192,
        4096,
        20480,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        12288,
        1536,
        4096,
        4096,
        16384,
        4096,
        73728,
        225280,
        8192,
        4096,
        4096,
        8192,
        40960,
        1024,
        12288,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        103424,
        142336,
        12288,
        4096,
        12288,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        155648,
        4096,
        4096,
        24576,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        16384,
        4096,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        73728,
        4096,
        4096,
        4096,
        12288,
        8192,
        4096,
        4096,
        4096,
        1536,
        4096,
        4096,
        4096,
        8192,
        40960,
        1536,
        4096,
        16384,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        16384,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        20480,
        4096,
        4096,
        8192,
        4096,
        4096,
        20480,
        12288,
        4096,
        4096,
        32768,
        4096,
        4096,
        4096,
        16384,
        4096,
        4096,
        1536,
        8192,
        372736,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        40960,
        24576,
        45056,
        4096,
        20480,
        4096,
        4096,
        4096,
        16384,
        8192,
        12288,
        4096,
        12288,
        12288,
        4096,
        4096,
        61440,
        4096,
        4096,
        4096,
        4096,
        8192,
        2048,
        4096,
        20480,
        53248,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        16384,
        4096,
        1024,
        8192,
        4096,
        1024,
        4096,
        3584,
        3072,
        4096,
        8192,
        1024,
        1024,
        1024,
        1536,
        3584,
        12288,
        1024,
        1024,
        4096,
        4096,
        8192,
        4096,
        4096,
        1024,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        1024,
        4096,
        4096,
        1024,
        4096,
        4096,
        4096,
        4096,
        4096,
        1024,
        1024,
        4096,
        4096,
        4096,
        4096,
        1024,
        1024,
        1024,
        1024,
        1024,
        4096,
        4096,
        1024,
        1024,
        32768,
        1024,
        1024,
        1024,
        1024,
        1024,
        1024,
        8192,
        4096,
        1024,
        4096,
        1024,
        1024,
        1024,
        4096,
        1024,
        1024,
        1024,
        1024,
        5632,
        1024,
        4096,
        1024,
        1024,
        1536,
        1024,
        1536,
        4096,
        4096,
        4096,
        1024,
        4096,
        1024,
        1536,
        2048,
        1536,
        1536,
        4096,
        1536,
        4096,
        1536,
        1536,
        1024,
        1024,
        4096,
        4096,
        1024,
        1024,
        1536,
        1536,
        4096,
        1024,
        1536,
        1024,
        1536,
        4096,
        4096,
        1536,
        4096,
        1024,
        4096,
        16384,
        57344,
        57344,
        4096,
        32768,
        4096,
        16384,
        4096,
        4096,
        4096,
        28672,
        4096,
        4096,
        4096,
        4096,
        147456,
        4096,
        8192,
        4096,
        16384,
        4096,
        4096,
        12288,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        8192,
        4096,
        12288,
        65536,
        4096,
        4096,
        8192,
        4096,
        12288,
        4096,
        4096,
        4096,
        4096,
        16384,
        4096,
        65536,
        4096,
        4096,
        12288,
        4096,
        8192,
        20480,
        4096,
        40960,
        28672,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        57344,
        4096,
        4096,
        4096,
        4096,
        4096,
        12288,
        8192,
        20480,
        4096,
        8192,
        4096,
        28672,
        4096,
        4096,
        1536,
        20480,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        49152,
        8192,
        4096,
        45056,
        4096,
        4096,
        8192,
        8192,
        4096,
        4096,
        4096,
        4096,
        8192,
        8192,
        16384,
        12288,
        24576,
        4096,
        8192,
        32768,
        20480,
        4096,
        4096,
        4096,
        32768,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        4096,
        36864,
        4096,
        4096,
        8192,
        4096,
        4096,
        4096,
        4096,
        81920,
        32768,
        4096,
        16384,
        4096,
        16384,
        4096,
        4096,
        4096,
        17920,
        4096,
        8192,
        4096,
        4096,
        1536,
        1536,
        16384,
        4096,
        4096,
        28672,
        65536,
        4096,
        4096,
        12288,
        4096,
        4096,
        4096,
        4096,
        40960,
        139264,
        4096,
        335872,
        4096,
        4096,
        4096,
        8192,
        1024,
        8192,
        471040,
        12288,
        12288,
        4096,
        393216,
        8192,
        184320,
        458752,
        4096,
        4096,
        16384,
        4096,
        1536,
        4096,
        524288,
        4096,
        4096,
        4096,
        516096,
        4096,
        8192,
        73728,
        6656,
        4096,
        524288,
        4096,
        4096,
        524288,
        16384,
        4096,
        458752,
        126976,
        8192,
        49152,
        458752,
        40960,
        4096,
        4096,
        491520,
        4096,
        524288,
        512,
        32768,
        495616,
        4096,
        4096,
        24576,
        40960,
        28672,
        4096,
        4096,
        4096,
        8192,
        503808,
        454656,
        16384,
        4096,
        4096,
        4096,
        4096,
        8192,
        458752,
        4096,
        4096,
        4096,
        4096,
        12288,
        4096,
        4096,
        4096,
        4096,
        524288,
        4096,
        4096,
        4096,
        393216,
        4096,
        4096,
        8192,
        8192,
        4096,
        4096,
        20480,
        524288,
        200704,
        24576,
        524288,
        40960,
        1024,
        4096,
        4096,
        524288,
        12288,
        4096,
        4096,
        503808,
        524288,
        8192,
        4096};

    // n=10, k=4, r=4. z=2, group_size=5
    std::cout << "Starting multiple append tests with different sizes..." << std::endl;

    // Test regular append with different sizes
    for (size_t i = 0; i < append_sizes.size(); i++)
    {
        std::cout << "\n[Test " << i + 1 << "/" << append_sizes.size() << "]" << std::endl;
        std::cout << "Testing append with size: " << append_sizes[i] << " bytes" << std::endl;

        bool append_result = client.append(append_sizes[i]);
        if (append_result)
        {
            std::cout << "Append operation succeeded for size " << append_sizes[i] << " bytes" << std::endl;
        }
        else
        {
            std::cout << "Append operation failed for size " << append_sizes[i] << " bytes" << std::endl;
            break;
        }
    }
    */

    //Test set() interface with three calls using loop
    std::cout << "\nTesting set() interface..." << std::endl;
    //std::cout << "编码时间: " << encoding_duration.count() << " 微秒" << std::endl;
    //std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    //auto encoding_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    const int NUM_TESTS = 10;
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
/*
        // get
    std::cout << "[GET BEGIN]" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        std::string value;
        std::string key = "Object" + std::to_string(i);
        std::string targetdir = "./client_get/";
        std::string writepath = targetdir + key;
        // if (!std::filesystem::exists(std::filesystem::path{"./client_get/"}))
        // {
        //   std::filesystem::create_directory("./client_get/");
        // }
        if (access(targetdir.c_str(), 0) == -1)
        {
        mkdir(targetdir.c_str(), S_IRWXU);
        }
        client.get(key, value);
        std::cout << "[run_client] value size " << value.size() << std::endl;
        std::ofstream ofs(writepath, std::ios::binary | std::ios::out | std::ios::trunc);
        ofs.write(value.c_str(), value.size());
        ofs.flush();
        ofs.close();
    }
    std::cout << "[GET END]" << std::endl
                << std::endl;

*/

    return 0;
}