#include "config.h"
#include "tinyxml2.h"
#include <cassert>

namespace ECProject
{
  Config *Config::instance = nullptr;

  Config::Config(const std::string &configPath)
  {
    loadConfig(configPath);
    printConfigs();
    validateConfig();
  }

  void Config::validateConfig() const
  {
    assert(BlockSize % UnitSize == 0 && "Error: BlockSize must be divisible by UnitSize");
    assert(DatanodeNumPerCluster > n / z && "Error: DatanodeNumPerCluster must be greater than n / z");
    assert(ClusterNum > z && "Error: ClusterNum must be greater than z");
  }

  Config *Config::getInstance(const std::string &configPath)
  {
    if (instance == nullptr)
    {
      instance = new Config(configPath);
    }
    return instance;
  }

  void Config::loadConfig(const std::string &configPath)
  {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(configPath.c_str()) != tinyxml2::XML_SUCCESS)
    {
      std::cerr << "Failed to load config file: " << configPath << std::endl;
      return;
    }

    tinyxml2::XMLElement *root = doc.RootElement();
    if (root == nullptr)
    {
      std::cerr << "Invalid config file format" << std::endl;
      return;
    }

    if (auto elem = root->FirstChildElement("AlignedSize"))
      AlignedSize = std::stoi(elem->GetText());
    if (auto elem = root->FirstChildElement("UnitSize"))
      UnitSize = std::stoi(elem->GetText());
    if (auto elem = root->FirstChildElement("BlockSize"))
      BlockSize = std::stoi(elem->GetText());
    if (auto elem = root->FirstChildElement("alpha"))
      alpha = std::stoi(elem->GetText());
    if (auto elem = root->FirstChildElement("z"))
      z = std::stoi(elem->GetText());
    n = alpha * z * z + z;
    k = alpha * z * z - alpha * z;
    r = alpha * z;
    DataBlockNumPerGroup = k / z;
    GlobalParityBlockNumPerGroup = r / z;
    if (auto elem = root->FirstChildElement("DatanodeNumPerCluster"))
      DatanodeNumPerCluster = std::stoi(elem->GetText());
    if (auto elem = root->FirstChildElement("ClusterNum"))
      ClusterNum = std::stoi(elem->GetText());
    if (DatanodeNumPerCluster == 0)
      DatanodeNumPerCluster = 2 * n / z; // default DatanodeNumPerCluster: two times of local group number
    if (ClusterNum == 0)
      ClusterNum = 2 * z; // default ClusterNum: two times of z
    if (auto elem = root->FirstChildElement("CoordinatorIP"))
      CoordinatorIP = std::string(elem->GetText());
    if (auto elem = root->FirstChildElement("CoordinatorPort"))
      CoordinatorPort = std::stoi(elem->GetText());
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