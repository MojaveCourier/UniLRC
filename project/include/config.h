#ifndef CONFIG_H
#define CONFIG_H

#include "devcommon.h"

namespace ECProject
{
  const int PORT_SHIFT = 20;

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

    uint32_t UnitSize = 8 * 1024;
    uint32_t BlockSize = 1024 * 1024;
    uint8_t alpha = 2;
    uint8_t z = 2;
    uint32_t n = alpha * z * z + z;
    uint32_t k = alpha * z * z - alpha * z;
    uint32_t r = alpha * z;
    uint32_t DataBlockNumPerGroup = k / z;
    uint32_t GlobalParityBlockNumPerGroup = r / z;
    uint32_t DatanodeNumPerCluster = 0;
    uint8_t ClusterNum = 0;
  };
}

#endif // CONFIG_H