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
#include <random>

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
    double block_size = parameters[3] / 1024 / 1024; //MB
    int n = k + r + z;
    //for read test
    /*for(int i = 0; i < 5; i++){
        client.set();
    }

    std::vector<std::chrono::duration<double>> time_spans;
    std::cout << "Set operation succeeded" << std::endl;
    for(int i = 0; i < 5; i++){
        size_t data_size;

        int id = i;
        std::string key = std::to_string(id);
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        std::shared_ptr<char[]> data = client.get(key, data_size);
        if(!data){
            std::cout << "Get operation failed" << std::endl;
            continue;
        }
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(time_span);
        std::cout << "get time: " << time_span.count() << std::endl;
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
    /*client.set();
    std::cout << "Set operation succeeded" << std::endl;
    std::vector<std::chrono::duration<double>> time_spans;
    std::cout << "Degraded read test" << std::endl;
    for(int i = 0; i < k; i++){
        size_t data_size;
        int id = i;
        std::string key = std::to_string(id);
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        std::shared_ptr<char[]> data = client.get_degraded_read_block(0, i);
        if(!data){
            std::cout << "Degraded read operation failed" << std::endl;
            continue;
        }
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(time_span);
        std::cout << "get time: " << time_span.count() << std::endl;
    }
    std::cout << "Degraded read operation succeeded" << std::endl;
    std::chrono::duration<double> total_time_span = std::accumulate(time_spans.begin(), time_spans.end(), std::chrono::duration<double>(0));
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    std::chrono::duration<double> max_time_span = *std::max_element(time_spans.begin(), time_spans.end());
    std::chrono::duration<double> min_time_span = *std::min_element(time_spans.begin(), time_spans.end());
    std::cout << "Max time: "<< max_time_span.count() << std::endl;
    std::cout << "Min time: "<< min_time_span.count() << std::endl;
    std::cout << "Throughput: " << time_spans.size() / total_time_span.count() << std::endl;
    std::cout << "Speed" << static_cast<size_t>(block_size)  / (total_time_span.count() / time_spans.size()) << "MB/s" << std::endl;
    std::cout << "Max speed: " << static_cast<size_t>(block_size)  / min_time_span.count() << "MB/s" << std::endl;
    std::cout << "Min speed: " << static_cast<size_t>(block_size)  / max_time_span.count() << "MB/s" << std::endl;*/


    //for degraded read breakdown test
    /*client.set();
    std::vector<std::chrono::duration<double>> time_spans;
    std::vector<std::chrono::duration<double>> disk_io_time_spans;
    std::vector<std::chrono::duration<double>> network_time_spans;
    std::vector<std::chrono::duration<double>> decode_time_spans;
    for(int i = 0; i < k; i++){
        
        double total_time, disk_io_time, network_time, decode_time;
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        std::shared_ptr<char[]> data = client.get_degraded_read_block_breakdown(0, i, total_time, disk_io_time, network_time, decode_time);
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(std::chrono::duration<double>(total_time));
        disk_io_time_spans.push_back(std::chrono::duration<double>(disk_io_time));
        network_time_spans.push_back(std::chrono::duration<double>(network_time));
        decode_time_spans.push_back(std::chrono::duration<double>(decode_time));
    }
    std::chrono::duration<double> total_time_span = std::accumulate(time_spans.begin(), time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_disk_io_time_span = std::accumulate(disk_io_time_spans.begin(), disk_io_time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_network_time_span = std::accumulate(network_time_spans.begin(), network_time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_decode_time_span = std::accumulate(decode_time_spans.begin(), decode_time_spans.end(), std::chrono::duration<double>(0));
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    std::chrono::duration<double> max_time_span = *std::max_element(time_spans.begin(), time_spans.end());
    std::chrono::duration<double> min_time_span = *std::min_element(time_spans.begin(), time_spans.end());
    std::cout << "Max time: "<< max_time_span.count() << std::endl;
    std::cout << "Min time: "<< min_time_span.count() << std::endl;


    std::cout << "Average disk io time: " << total_disk_io_time_span.count() / disk_io_time_spans.size() << std::endl;
    std::cout << "Average network time: " << total_network_time_span.count() / network_time_spans.size() << std::endl;
    std::cout << "Average decode time: " << total_decode_time_span.count() / decode_time_spans.size() << std::endl;
    std::chrono::duration<double> max_disk_io_time_span = *std::max_element(disk_io_time_spans.begin(), disk_io_time_spans.end());
    std::chrono::duration<double> min_disk_io_time_span = *std::min_element(disk_io_time_spans.begin(), disk_io_time_spans.end());
    std::chrono::duration<double> max_network_time_span = *std::max_element(network_time_spans.begin(), network_time_spans.end());
    std::chrono::duration<double> min_network_time_span = *std::min_element(network_time_spans.begin(), network_time_spans.end());
    std::chrono::duration<double> max_decode_time_span = *std::max_element(decode_time_spans.begin(), decode_time_spans.end());
    std::chrono::duration<double> min_decode_time_span = *std::min_element(decode_time_spans.begin(), decode_time_spans.end());
    std::cout << "Max disk io time: "<< max_disk_io_time_span.count() << std::endl;
    std::cout << "Min disk io time: "<< min_disk_io_time_span.count() << std::endl;
    std::cout << "Max network time: "<< max_network_time_span.count() << std::endl;
    std::cout << "Min network time: "<< min_network_time_span.count() << std::endl;
    std::cout << "Max decode time: "<< max_decode_time_span.count() << std::endl;
    std::cout << "Min decode time: "<< min_decode_time_span.count() << std::endl;*/


    //for single block repair
    /*client.set();

    std::vector<std::chrono::duration<double>> time_spans;
    for(int i = 0; i < n; i++){
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
    //std::cout << "Total time: " << total_time_span.count() << std::endl;
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    std::cout << "Max time: "<< max_time_span.count() << std::endl;
    std::cout << "Min time: "<< min_time_span.count() << std::endl;*/

    //for single block repair breakdown
    /*client.set();
    std::vector<std::chrono::duration<double>> time_spans;
    std::vector<std::chrono::duration<double>> disk_read_time_spans;
    std::vector<std::chrono::duration<double>> network_time_spans;
    std::vector<std::chrono::duration<double>> decode_time_spans;
    std::vector<std::chrono::duration<double>> disk_write_time_spans;
    for(int i = 0; i < n; i++){
        double total_time, disk_read_time, network_time, decode_time, disk_write_time;
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        client.recovery_breakdown(0, i, disk_read_time, network_time, decode_time, disk_write_time);
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        time_spans.push_back(std::chrono::duration<double>(time_span.count()));
        disk_read_time_spans.push_back(std::chrono::duration<double>(disk_read_time));
        network_time_spans.push_back(std::chrono::duration<double>(network_time));
        decode_time_spans.push_back(std::chrono::duration<double>(decode_time));
        disk_write_time_spans.push_back(std::chrono::duration<double>(disk_write_time));
    }
    std::chrono::duration<double> total_time_span = std::accumulate(time_spans.begin(), time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_disk_read_time_span = std::accumulate(disk_read_time_spans.begin(), disk_read_time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_network_time_span = std::accumulate(network_time_spans.begin(), network_time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_decode_time_span = std::accumulate(decode_time_spans.begin(), decode_time_spans.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> total_disk_write_time_span = std::accumulate(disk_write_time_spans.begin(), disk_write_time_spans.end(), std::chrono::duration<double>(0));
    std::cout << "Average time: " << total_time_span.count() / time_spans.size() << std::endl;
    std::cout << "Average disk read time: " << total_disk_read_time_span.count() / disk_read_time_spans.size() << std::endl;
    std::cout << "Average network time: " << total_network_time_span.count() / network_time_spans.size() << std::endl;
    std::cout << "Average decode time: " << total_decode_time_span.count() / decode_time_spans.size() << std::endl;
    std::cout << "Average disk write time: " << total_disk_write_time_span.count() / disk_write_time_spans.size() << std::endl;*/

    //for full node repair
    //size_t total_size = 20000; //MB
    //int stripe_num = total_size / (block_size * n);
    double total_size = 500 * n * block_size; //MB
    for(int i = 0; i < 500; i++){
        client.set();
    }
    std::vector<std::chrono::duration<double>> time_spans;
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    int block_num = client.recovery_full_node(0);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << "full node repair time: " << time_span.count() << std::endl;
    std::cout << "block num: " << block_num << std::endl;
    std::cout << "Speed: " << total_size / time_span.count() << "MB/s" << std::endl;
    
    //for workload test
    /*int stripe_num = 200;
    int workload = 100;

    // 随机数生成器（Mersenne Twister算法）
    std::mt19937 rng(std::random_device{}());

    // 均匀分布
    std::uniform_int_distribution<int> dist_64(0, k*stripe_num - 64);
    std::uniform_int_distribution<int> dist_32(0, k*stripe_num - 32);
    std::uniform_int_distribution<int> dist_1(0, k*stripe_num - 1);
    std::uniform_real_distribution<double> dist_double(0.0, 1.0);

    for(int i = 0; i < stripe_num; i++){
        client.set();
    }
    for(int i = 0; i < workload; i++){
        double random_double = dist_double(rng);
        if(random_double < 0.85){
            int start_block_id = dist_64(rng);
            int end_block_id = start_block_id + 64 - 1;
            std::string value;
            client.get_blocks(start_block_id, end_block_id, value);
        }
        else if(random_double < 0.925){

        }
        else{

        }
    }*/
    
    
    return 0;
}