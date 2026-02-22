/**
 * Data layout / placement: per-group block counts by code type (AzureLRC, OptimalLRC, UniformLRC, UniLRC).
 * Used by client and coordinator; no dependency on proto or RPC.
 */
#include "encoder.h"
#include <string>
#include <vector>

namespace ECProject
{

std::vector<int> get_data_block_num_per_group(int k, int r, int z, const std::string &code_type)
{
  std::vector<int> data_block_num_per_group;
  if (code_type == "AzureLRC")
  {
    for (int i = 0; i < z; i++)
    {
      data_block_num_per_group.push_back((k / z));
    }
    data_block_num_per_group.push_back(0);
  }
  else if (code_type == "OptimalLRC")
  {
    int group_size = r + 1;
    int local_group_size = (k / z);
    int group_num_of_one_local_group = local_group_size / group_size + 1;
    int group_num = z * group_num_of_one_local_group + 1;
    for (int i = 0; i < group_num - 1; i++)
    {
      if ((i + 1) % group_num_of_one_local_group)
      {
        data_block_num_per_group.push_back(group_size);
      }
      else
      {
        data_block_num_per_group.push_back(local_group_size % group_size);
      }
    }
    data_block_num_per_group.push_back(0);
  }
  else if (code_type == "UniformLRC")
  {
    for (int i = 0; i < z - 1; i++)
    {
      data_block_num_per_group.push_back((k + r) / z);
    }
    data_block_num_per_group.push_back(0);
  }
  else if (code_type == "UniLRC")
  {
    int local_data_num = k / z;
    for (int i = 0; i < z; i++)
    {
      data_block_num_per_group.push_back(local_data_num);
    }
  }
  return data_block_num_per_group;
}

std::vector<int> get_global_parity_block_num_per_group(int k, int r, int z, const std::string &code_type)
{
  std::vector<int> global_parity_block_num_per_group;
  if (code_type == "AzureLRC")
  {
    for (int i = 0; i < z; i++)
    {
      global_parity_block_num_per_group.push_back(0);
    }
    global_parity_block_num_per_group.push_back(r);
  }
  else if (code_type == "OptimalLRC")
  {
    int group_size = r + 1;
    int local_group_size = (k / z);
    int group_num_of_one_local_group = local_group_size / group_size + 1;
    int group_num = z * group_num_of_one_local_group + 1;
    for (int i = 0; i < group_num - 1; i++)
    {
      global_parity_block_num_per_group.push_back(0);
    }
    global_parity_block_num_per_group.push_back(r);
  }
  else if (code_type == "UniformLRC")
  {
    for (int i = 0; i < z - 1; i++)
    {
      global_parity_block_num_per_group.push_back(0);
    }
    global_parity_block_num_per_group.push_back(r);
  }
  else if (code_type == "UniLRC")
  {
    int local_global_parity_num = r / z;
    for (int i = 0; i < z; i++)
    {
      global_parity_block_num_per_group.push_back(local_global_parity_num);
    }
  }
  return global_parity_block_num_per_group;
}

std::vector<int> get_local_parity_block_num_per_group(int k, int r, int z, const std::string &code_type)
{
  std::vector<int> local_parity_block_num_per_group;
  if (code_type == "AzureLRC")
  {
    for (int i = 0; i < z; i++)
    {
      local_parity_block_num_per_group.push_back(1);
    }
    local_parity_block_num_per_group.push_back(0);
  }
  else if (code_type == "OptimalLRC")
  {
    int group_size = r + 1;
    int local_group_size = (k / z);
    int group_num_of_one_local_group = local_group_size / group_size + 1;
    int group_num = z * group_num_of_one_local_group + 1;
    for (int i = 0; i < group_num - 1; i++)
    {
      if ((i + 1) % group_num_of_one_local_group)
      {
        local_parity_block_num_per_group.push_back(0);
      }
      else
      {
        local_parity_block_num_per_group.push_back(1);
      }
    }
    local_parity_block_num_per_group.push_back(0);
  }
  else if (code_type == "UniformLRC")
  {
    for (int i = 0; i < z; i++)
    {
      local_parity_block_num_per_group.push_back(1);
    }
  }
  else if (code_type == "UniLRC")
  {
    for (int i = 0; i < z; i++)
    {
      local_parity_block_num_per_group.push_back(1);
    }
  }
  return local_parity_block_num_per_group;
}

} // namespace ECProject
