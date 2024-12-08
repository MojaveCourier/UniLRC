#include "client.h"
#include "toolbox.h"
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"


int main(int argc, char **argv)
{
    ECProject::Config *config = ECProject::Config::getInstance();
    config->printConfigs();
    return 0;
}