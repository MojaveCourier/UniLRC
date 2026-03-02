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
    else if (code_type == "LotusLRC")
    {
      return get_data_block_num_per_group_lotuslrc(k, r, z);
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
    else if (code_type == "LotusLRC")
    {
      return get_global_parity_block_num_per_group_lotuslrc(k, r, z);
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
    else if (code_type == "LotusLRC")
    {
      return get_local_parity_block_num_per_group_lotuslrc(k, r, z);
    }
    return local_parity_block_num_per_group;
  }

  /* LotusLRC layout: stub for user implementation */
  std::vector<int> get_data_block_num_per_group_lotuslrc(int k, int r, int z)
  {
    std::vector<int> data_block_num_per_group;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    int local_group_num = local_group_sizes.size();
    std::vector<int> data_block_num_per_local_group = get_data_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> global_parity_block_num_per_local_group = get_global_parity_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> local_parity_block_num_per_local_group = get_local_parity_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> group_sizes = get_lotuslrc_group_sizes(k, r, z);
    std::vector<int> group_num_per_local_group = get_lotuslrc_group_num_per_local_group(k, r, z);
    int cur_group_id = 0;
    for (int i = 0; i < local_group_num; i++){
      int parity_num = global_parity_block_num_per_local_group[i] + local_parity_block_num_per_local_group[i];
      for (int j = 0; j < parity_num; j++){ // put parity blocks in the first positions
        data_block_num_per_group.push_back(group_sizes[cur_group_id] - 1);
        cur_group_id++;
      }
      for (int j = 0; j < group_num_per_local_group[i] - parity_num; j++){
        data_block_num_per_group.push_back(group_sizes[cur_group_id]);
        cur_group_id++;
      }
    }
    return data_block_num_per_group;
  }

  std::vector<int> get_global_parity_block_num_per_group_lotuslrc(int k, int r, int z)
  {
    std::vector<int> global_parity_block_num_per_group;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    int local_group_num = local_group_sizes.size();
    std::vector<int> data_block_num_per_local_group = get_data_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> global_parity_block_num_per_local_group = get_global_parity_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> local_parity_block_num_per_local_group = get_local_parity_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> group_sizes = get_lotuslrc_group_sizes(k, r, z);
    std::vector<int> group_num_per_local_group = get_lotuslrc_group_num_per_local_group(k, r, z);
    for (int i = 0; i < local_group_num; i++){
      int global_parity_num = global_parity_block_num_per_local_group[i];
      for (int j = 0; j < global_parity_num; j++){ // put parity blocks in the first positions
        global_parity_block_num_per_group.push_back(1);
      }
      for (int j = 0; j < group_num_per_local_group[i] - global_parity_num; j++){
        global_parity_block_num_per_group.push_back(0);
      }
    }
    return global_parity_block_num_per_group;
  }

  std::vector<int> get_local_parity_block_num_per_group_lotuslrc(int k, int r, int z)
  {
    std::vector<int> local_parity_block_num_per_group;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    int local_group_num = local_group_sizes.size();
    std::vector<int> data_block_num_per_local_group = get_data_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> global_parity_block_num_per_local_group = get_global_parity_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> local_parity_block_num_per_local_group = get_local_parity_block_num_per_local_group_lotuslrc(k, r, z);
    std::vector<int> group_sizes = get_lotuslrc_group_sizes(k, r, z);
    std::vector<int> group_num_per_local_group = get_lotuslrc_group_num_per_local_group(k, r, z);
    for (int i = 0; i < local_group_num; i++){
      int local_parity_num = local_parity_block_num_per_local_group[i];
      int global_parity_num = global_parity_block_num_per_local_group[i];
      for (int j = 0; j < global_parity_num; j++){
        local_parity_block_num_per_group.push_back(0);
      }
      for (int j = 0; j < local_parity_num; j++){ // put parity blocks in the first positions
        local_parity_block_num_per_group.push_back(1);
      }
      for (int j = 0; j < group_num_per_local_group[i] - local_parity_num - global_parity_num; j++){
        local_parity_block_num_per_group.push_back(0);
      }
    }
    return local_parity_block_num_per_group;  
  }

  std::unordered_map<int, int> get_lotuslrc_block_id_to_group_id(int k, int r, int z){
    std::unordered_map<int, int> block_id_to_group_id;
    std::unordered_map<int, std::vector<int>> group_id_to_block_ids = get_lotuslrc_group_id_to_block_ids(k, r, z);
    for (int i = 0; i < group_id_to_block_ids.size(); i++){
      for (int j = 0; j < group_id_to_block_ids[i].size(); j++){
        block_id_to_group_id[group_id_to_block_ids[i][j]] = i;
      }
    }
    return block_id_to_group_id;
  }

  std::unordered_map<int, std::vector<int>> get_lotuslrc_group_id_to_block_ids(int k, int r, int z){
    std::vector<int> data_block_num_per_group = get_data_block_num_per_group_lotuslrc(k, r, z);
    std::vector<int> global_parity_block_num_per_group = get_global_parity_block_num_per_group_lotuslrc(k, r, z);
    std::vector<int> local_parity_block_num_per_group = get_local_parity_block_num_per_group_lotuslrc(k, r, z);
    std::unordered_map<int, std::vector<int>> group_id_to_block_ids;
    int cur_data_block_id = 0;
    int cur_global_parity_block_id = k;
    int cur_local_parity_block_id = k + r;
    for (int i = 0; i < data_block_num_per_group.size(); i++){
      std::vector<int> block_ids;
      for (int j = 0; j < data_block_num_per_group[i]; j++){
        block_ids.push_back(cur_data_block_id);
        cur_data_block_id++;
      }
      for (int j = 0; j < global_parity_block_num_per_group[i]; j++){
        block_ids.push_back(cur_global_parity_block_id);
        cur_global_parity_block_id++;
      }
      for (int j = 0; j < local_parity_block_num_per_group[i]; j++){
        block_ids.push_back(cur_local_parity_block_id);
        cur_local_parity_block_id++;
      }
      group_id_to_block_ids[i] = block_ids;
    }
    return group_id_to_block_ids;
  }

  std::vector<int> get_data_block_num_per_local_group_lotuslrc(int k, int r, int z){
    std::vector<int> data_block_num_per_local_group;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    int local_group_num = local_group_sizes.size();
    if (local_group_num != z / 2){
      throw std::runtime_error("local group num is not equal to z / 2");
    }
    for (int i = 0; i < local_group_num; i++){
      if (i < local_group_num - r ){
        data_block_num_per_local_group.push_back(local_group_sizes[i] - 2);
      }
      else{
        data_block_num_per_local_group.push_back(local_group_sizes[i] - 3); // global parity blocks are in the last positions
      }
    }
    return data_block_num_per_local_group;
  }
  std::vector<int> get_global_parity_block_num_per_local_group_lotuslrc(int k, int r, int z){
    std::vector<int> global_parity_block_num_per_local_group;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    int local_group_num = local_group_sizes.size();
    if (local_group_num != z / 2){
      throw std::runtime_error("local group num is not equal to z / 2");
    }
    for (int i = 0; i < local_group_num; i++){
      //global_parity_block_num_per_local_group.push_back(local_group_sizes[i] - 2);
      if (i < local_group_num - r ){
        global_parity_block_num_per_local_group.push_back(0);
      }
      else{
        global_parity_block_num_per_local_group.push_back(1); // global parity blocks are in the last positions
      }
    }
    return global_parity_block_num_per_local_group;
  }
  std::vector<int> get_local_parity_block_num_per_local_group_lotuslrc(int k, int r, int z){
    std::vector<int> local_parity_block_num_per_local_group;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    int local_group_num = local_group_sizes.size();
    if (local_group_num != z / 2){
      throw std::runtime_error("local group num is not equal to z / 2");
    }
    for (int i = 0; i < local_group_num; i++){
      local_parity_block_num_per_local_group.push_back(2); // lotuslrc has 2 local parity blocks in each local group
    }
    return local_parity_block_num_per_local_group;
  }

  int get_lotuslrc_local_group_id_to_block_id(int k, int r, int z, int local_group_id){
    
  }

  int get_lotuslrc_block_id_to_local_group_id(int k, int r, int z, int block_id){
    if (block_id < k){
      std::vector<int> data_block_num_per_local_group = get_data_block_num_per_local_group_lotuslrc(k, r, z);
      int cur_sum = 0;
      for (int i = 0; i < data_block_num_per_local_group.size(); i++){
        cur_sum += data_block_num_per_local_group[i];
        if (block_id < cur_sum){
          return i;
        }
      }
    }
    else if (block_id < k + r){
      std::vector<int> global_parity_block_num_per_local_group = get_global_parity_block_num_per_local_group_lotuslrc(k, r, z);
      int cur_sum = 0;
      for (int i = 0; i < global_parity_block_num_per_local_group.size(); i++){
        cur_sum += global_parity_block_num_per_local_group[i];
        if (block_id < cur_sum){
          return i;
        }
      }
    }
    else{
      return (block_id - k - r) % (z / 2);
    }
    throw std::runtime_error("block id is out of range");
  }

  std::vector<int> get_lotuslrc_group_sizes(int k, int r, int z){
    int max_capacity = r + 2;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    std::vector<int> group_sizes;
    for (int i = 0; i < local_group_sizes.size(); i++){
      int group_num = local_group_sizes[i] / max_capacity + bool(local_group_sizes[i] % max_capacity);
      int group_size = local_group_sizes[i] / group_num;
      int larger_group_num = local_group_sizes[i] % group_num;
      for (int j = 0; j < larger_group_num; j++)
      {
        group_sizes.push_back(group_size + 1);
      }
      for (int j = 0; j < group_num - larger_group_num; j++)
      {
        group_sizes.push_back(group_size);
      }
    }
    return group_sizes;
  }
  std::vector<int> get_lotuslrc_local_group_sizes(int k, int r, int z){
    int group_num = z / 2;
    int group_size = (k + r) / group_num + 2; // plus 2 local parity blocks
    int larger_group_num = (k + r) % group_num;
    std::vector<int> group_sizes;
    for (int i = 0; i < group_num - larger_group_num; i++)
    {
      group_sizes.push_back(group_size);
    }
    for (int i = 0; i < larger_group_num; i++)
    {
      group_sizes.push_back(group_size + 1);
    }
    return group_sizes;
  }
  std::vector<int> get_lotuslrc_group_num_per_local_group(int k, int r, int z){
    std::vector<int> group_num_per_local_group;
    int max_capacity = r + 2;
    std::vector<int> local_group_sizes = get_lotuslrc_local_group_sizes(k, r, z);
    for (int i = 0; i < local_group_sizes.size(); i++){
      int group_num = local_group_sizes[i] / max_capacity + bool(local_group_sizes[i] % max_capacity);
      group_num_per_local_group.push_back(group_num);
    }
    return group_num_per_local_group;
  }
} // namespace ECProject
