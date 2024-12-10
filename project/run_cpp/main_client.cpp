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
    std::string sys_config_path = std::string(buff) + cwf.substr(1, cwf.rfind('/') - 1) + "/../../config/parameterConfiguation.xml";
    std::cout << "Current working directory: " << sys_config_path << std::endl;

    ECProject::Config *config = ECProject::Config::getInstance(sys_config_path);
    config->printConfigs();
    return 0;
}