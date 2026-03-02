/**
 * Layout 相关单元测试，LotusLRC 输入参数打印
 */
#include "encoder.h"
#include <iostream>
#include <vector>
#include <unordered_map>

static void print_vector(const std::string &name, const std::vector<int> &v)
{
  std::cout << "  " << name << ": [";
  for (size_t i = 0; i < v.size(); ++i)
  {
    if (i > 0)
      std::cout << ", ";
    std::cout << v[i];
  }
  std::cout << "]" << std::endl;
}

static void lotuslrc_print_params(int k, int r, int z)
{
  std::cout << "=== LotusLRC 输入参数 ===" << std::endl;
  std::cout << "  k (数据块数): " << k << std::endl;
  std::cout << "  r (全局校验块数): " << r << std::endl;
  std::cout << "  z (本地校验块数 / local group 数*2): " << z << std::endl;
  std::cout << "  n (总块数 k+r+z): " << (k + r + z) << std::endl;
  std::cout << "  local_group_num (z/2): " << (z / 2) << std::endl;

  std::cout << "\n--- LotusLRC layout 输出 ---" << std::endl;
  auto local_group_sizes = ECProject::get_lotuslrc_local_group_sizes(k, r, z);
  print_vector("local_group_sizes", local_group_sizes);

  auto group_sizes = ECProject::get_lotuslrc_group_sizes(k, r, z);
  print_vector("group_sizes", group_sizes);

  auto group_num_per_local = ECProject::get_lotuslrc_group_num_per_local_group(k, r, z);
  print_vector("group_num_per_local_group", group_num_per_local);

  auto data_per_group = ECProject::get_data_block_num_per_group_lotuslrc(k, r, z);
  print_vector("data_block_num_per_group", data_per_group);

  auto global_per_group = ECProject::get_global_parity_block_num_per_group_lotuslrc(k, r, z);
  print_vector("global_parity_block_num_per_group", global_per_group);

  auto local_per_group = ECProject::get_local_parity_block_num_per_group_lotuslrc(k, r, z);
  print_vector("local_parity_block_num_per_group", local_per_group);

  std::cout << "\n--- LotusLRC per_local_group ---" << std::endl;
  auto data_per_local = ECProject::get_data_block_num_per_local_group_lotuslrc(k, r, z);
  print_vector("data_block_num_per_local_group", data_per_local);
  auto global_per_local = ECProject::get_global_parity_block_num_per_local_group_lotuslrc(k, r, z);
  print_vector("global_parity_block_num_per_local_group", global_per_local);
  auto local_parity_per_local = ECProject::get_local_parity_block_num_per_local_group_lotuslrc(k, r, z);
  print_vector("local_parity_block_num_per_local_group", local_parity_per_local);

  std::cout << "\n--- block_id -> group_id ---" << std::endl;
  auto bid2gid = ECProject::get_lotuslrc_block_id_to_group_id(k, r, z);
  for (int i = 0; i < k + r + z; ++i)
  {
    std::cout << "  block " << i << " -> group " << bid2gid[i] << std::endl;
  }
}

int main()
{
  int k = 12, r = 4, z = 4;
  std::cin >> k >> r >> z;
  lotuslrc_print_params(k, r, z);
  return 0;
}
