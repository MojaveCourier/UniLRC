#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"
#include <iomanip>
#include <iostream>
#include <chrono>
#include <algorithm>

int main(int argc, char **argv)
{
    char buff[256];
    getcwd(buff, 256);
    std::string cwf = std::string(argv[0]);
    std::string sys_config_path = std::string(buff) + cwf.substr(1, cwf.rfind('/') - 1) + "/../../config/parameterConfiguration.xml";
    //std::string sys_config_path = "/home/GuanTian/lql/UniEC/project/config/parameterConfiguration.xml";
    std::cout << "Current working directory: " << sys_config_path << std::endl;

    const ECProject::Config *config = ECProject::Config::getInstance(sys_config_path);
    std::string client_ip = "10.10.1.1";
    int client_port = 44444;
    ECProject::Client client(client_ip, client_port, config->CoordinatorIP + ":" + std::to_string(config->CoordinatorPort), sys_config_path);
    std::cout << client.sayHelloToCoordinatorByGrpc("Client ID: " + client_ip + ":" + std::to_string(client_port)) << std::endl;

    std::vector<int> parameters = client.get_parameters();
    int k = parameters[0];
    int r = parameters[1];
    int z = parameters[2];
    int block_size = parameters[3] / 1024 / 1024; //MB
    int n = k + r + z;
    //for read test
    /*for(int i = 0; i < 5; i++){
        client.set();
    }

    std::vector<std::chrono::duration<double>> time_spans;
    std::cout << "Set operation succeeded" << std::endl;
    for(int i = 0; i < 5; i++){
        std::string value;
        int id = i;
        std::string key = std::to_string(id);
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        bool get_result = client.get(key, value);
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(time_span);
        std::cout << "get time: " << time_span.count() << std::endl;
        if(!get_result)
            std::cout << "Get operation failed" << std::endl;
    }
    std::cout << "Get operation succeeded" << std::endl;
    std::chrono::duration<double> total_time_span = std::accumulate(time_spans.begin(), time_spans.end(), std::chrono::duration<double>(0));
    std::cout << "Total time: " << total_time_span.count() << std::endl;
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    std::cout << "Throughput: " << time_spans.size() / total_time_span.count() << std::endl;
    std::cout << "Speed" << static_cast<size_t>(block_size) * k / (total_time_span.count() / time_spans.size()) << "MB/s" << std::endl;
    std::chrono::duration<double> max_time_span = *std::max_element(time_spans.begin(), time_spans.end());
    std::chrono::duration<double> min_time_span = *std::min_element(time_spans.begin(), time_spans.end());
    std::cout << "Max speed: " << static_cast<size_t>(block_size) * k / min_time_span.count() << "MB/s" << std::endl;
    std::cout << "Min speed: " << static_cast<size_t>(block_size) * k / max_time_span.count() << "MB/s" << std::endl;*/
    
    //for degraded read test

    client.set();
    std::vector<std::chrono::duration<double>> time_spans;
    for(int i = 0; i < k; i++){
        std::string value;
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        client.get_degraded_read_block(0, i, value);
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(time_span);
    }
    std::chrono::duration<double> total_time_span = std::accumulate(time_spans.begin(), time_spans.end(), std::chrono::duration<double>(0));
    std::cout << "Total time: " << total_time_span.count() << std::endl;
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    //std::cout << "Throughput: " << time_spans.size() / total_time_span.count() << std::endl;
    //std::cout << "Speed" << block_size / (total_time_span.count() / time_spans.size()) << "MB/s" << std::endl;
    std::chrono::duration<double> max_time_span = *std::max_element(time_spans.begin(), time_spans.end());
    std::chrono::duration<double> min_time_span = *std::min_element(time_spans.begin(), time_spans.end());
    std::cout << "Max time: "<< max_time_span.count() << std::endl;
    std::cout << "Min time: "<< min_time_span.count() << std::endl;
    //std::cout << "Max speed: " << block_size / min_time_span.count() << "MB/s" << std::endl;
    //std::cout << "Min speed: " << block_size / max_time_span.count() << "MB/s" << std::endl;

    //for single block repair
    /*client.set();

    std::vector<std::chrono::duration<double>> time_spans;
    for(int i = 0; i < n; i++){
        std::string value;
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        client.recovery(0, i);
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(time_span);
        std::cout << "single block repair time: " << time_span.count() << std::endl;
    }
    std::chrono::duration<double> total_time_span = std::accumulate(time_spans.begin(), time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> max_time_span = *std::max_element(time_spans.begin(), time_spans.end());
    std::chrono::duration<double> min_time_span = *std::min_element(time_spans.begin(), time_spans.end());
    std::cout << "Total time: " << total_time_span.count() << std::endl;
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    std::cout << "Max time: "<< max_time_span.count() << std::endl;
    std::cout << "Min time: "<< min_time_span.count() << std::endl;*/

    //for full node repair
    /*for(int i = 0; i < 500; i++){
        client.set();
    }
    std::vector<std::chrono::duration<double>> time_spans;
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    int block_num = client.recovery_full_node(0);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << "full node repair time: " << time_span.count() << std::endl;
    std::cout << "block num: " << block_num << std::endl;
    size_t total_size = static_cast<size_t>(block_size) * block_num;
    std::cout << "Speed: " << total_size / time_span.count() << "MB/s" << std::endl;
    return 0;*/
}