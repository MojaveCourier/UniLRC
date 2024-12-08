#include "config.h"

namespace ECProject
{
  Config *Config::instance = nullptr;

  Config::Config()
  {
  }

  Config *Config::getInstance()
  {
    if (instance == nullptr)
      {
        instance = new Config();
      }
      return instance;
    }

  void Config::printConfigs() const
  {
    std::cout << "Configuration Parameters:" << std::endl;
      std::cout << "  UnitSize: " << UnitSize << " bytes" << std::endl;
      std::cout << "  BlockSize: " << BlockSize << " bytes" << std::endl;
      std::cout << "  alpha: " << (int)alpha << std::endl;
      std::cout << "  z: " << (int)z << std::endl;
      std::cout << "  n: " << n << std::endl;
      std::cout << "  k: " << k << std::endl;
      std::cout << "  r: " << r << std::endl;
      std::cout << "  (n, k, r, z): (" << n << ", " << k << ", " << r << ", " << (int)z << ")" << std::endl;
      std::cout << "  DataBlockNumPerGroup: " << DataBlockNumPerGroup << " blocks/group" << std::endl;
      std::cout << "  GlobalParityBlockNumPerGroup: " << GlobalParityBlockNumPerGroup << " blocks/group" << std::endl;
      std::cout << "  DatanodeNumPerCluster: " << DatanodeNumPerCluster << " nodes/cluster" << std::endl;
      std::cout << "  ClusterNum: " << (int)ClusterNum << " clusters" << std::endl;
  }
}