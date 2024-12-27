#ifndef CONFIG_H
#define CONFIG_H

#include "devcommon.h"

namespace ECProject
{
  const int DATANODE_PORT_SHIFT = 20;
  const int PROXY_PORT_SHIFT = 1;

  class Config
  {
  private:
    static Config *instance;
    Config(const std::string &configPath);

  public:
    static Config *getInstance(const std::string &configPath);
    void loadConfig(const std::string &configPath);
    void printConfigs() const;
    void validateConfig() const;

    int AlignedSize = 4096;
    int UnitSize = 8 * 1024;
    int BlockSize = 1024 * 1024;
    int alpha = 2;
    int z = 2;
    int n = alpha * z * z + z;
    int k = alpha * z * z - alpha * z;
    int r = alpha * z;
    int DataBlockNumPerGroup = k / z;
    int GlobalParityBlockNumPerGroup = r / z;
    int DatanodeNumPerCluster = 0;
    int ClusterNum = 0;
    std::string CoordinatorIP = "0.0.0.0";
    int CoordinatorPort = 55555;
  };
}

#endif // CONFIG_H