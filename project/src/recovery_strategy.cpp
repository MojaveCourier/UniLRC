/**
 * Recovery strategy: multi-block decode plan and single-block recovery group/block ids.
 * Moved from encoder.cpp (get_multi_decode_plan) and encoder_layout.cpp (get_recovery_group_and_block_ids*).
 */
#include "encoder.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ECProject {

bool get_multi_decode_plan(int k, int r, int z, std::string code_type, const std::vector<int> failed_block_indexes, std::vector<int> &decode_block_indexes, std::vector<std::vector<int>> &decode_factors)
{
    int m = k + r;
    int nrows = k + r + z;
    unsigned char gen_matrix[k * (k + r + z)];
    memset(gen_matrix, 0, k * (k + r + z));
    if(code_type == "UniLRC"){
        gen_unilrc_matrix(gen_matrix, k, r, z);
    }
    else if(code_type == "AzureLRC"){
        gen_azure_lrc_matrix(gen_matrix, k, r, z);
    }
    else if(code_type == "OptimalLRC"){
        gen_optimal_lrc_matrix(gen_matrix, k, r, z);
    }
    else if(code_type == "UniformLRC"){
        gen_uniform_lrc_matrix(gen_matrix, k, r, z);
    }
    else{
        std::cerr << "Error: Unsupported code type " << code_type << std::endl;
        return false;
    }

    // build failed set
    std::unordered_map<int, bool> failed_map;
    for (int idx : failed_block_indexes) failed_map[idx] = true;

    // Prefer first m rows (global rows). Collect candidate rows (non-failed).
    std::vector<int> candidates;
    for (int i = 0; i < m; i++) {
        if (!failed_map.count(i)) candidates.push_back(i);
    }
    // If not enough, append remaining non-failed rows (local parity rows)
    if ((int)candidates.size() < k) {
        for (int i = m; i < nrows; i++) {
            if (!failed_map.count(i)) candidates.push_back(i);
            if ((int)candidates.size() >= k) break;
        }
    }

    // If still fewer than k rows, cannot form left-inverse
    if ((int)candidates.size() < k) {
        decode_block_indexes.clear();
        decode_factors.clear();
        return false;
    }

    // Find k linearly independent rows among candidates using Gaussian elimination (GF)
    int M = (int)candidates.size();
    unsigned char *mat = new unsigned char[M * k];
    // copy candidate rows into mat (row-major: M x k)
    for (int i = 0; i < M; i++) {
        int gro = candidates[i];
        for (int c = 0; c < k; c++) mat[i * k + c] = gen_matrix[gro * k + c];
    }

    int cur = 0; // current pivot row index in mat
    for (int col = 0; col < k && cur < M; col++) {
        // find row with non-zero in this column
        int sel = -1;
        for (int row = cur; row < M; row++) {
            if (mat[row * k + col] != 0) { sel = row; break; }
        }
        if (sel == -1) continue; // no pivot in this column

        // swap sel and cur (both in mat and candidates)
        if (sel != cur) {
            for (int c = 0; c < k; c++) {
                unsigned char tmp = mat[cur * k + c];
                mat[cur * k + c] = mat[sel * k + c];
                mat[sel * k + c] = tmp;
            }
            int tmpidx = candidates[cur];
            candidates[cur] = candidates[sel];
            candidates[sel] = tmpidx;
        }

        // normalize pivot row: make pivot == 1
        unsigned char pivot = mat[cur * k + col];
        unsigned char inv_pivot = gf_inv(pivot);
        for (int c = col; c < k; c++) mat[cur * k + c] = gf_mul(mat[cur * k + c], inv_pivot);

        // eliminate this column in all other rows
        for (int row = 0; row < M; row++) {
            if (row == cur) continue;
            unsigned char factor = mat[row * k + col];
            if (factor == 0) continue;
            for (int c = col; c < k; c++) {
                mat[row * k + c] ^= gf_mul(factor, mat[cur * k + c]);
            }
        }

        cur++;
    }

    // check if we found k independent rows (need cur >= k)
    if (cur < k) {
        delete[] mat;
        decode_block_indexes.clear();
        decode_factors.clear();
        return false;
    }

    // selected rows are candidates[0..k-1]
    std::vector<int> chosen(k);
    for (int i = 0; i < k; i++) chosen[i] = candidates[i];

    // build tempM from chosen rows (k x k) and compute inverse
    unsigned char *tempM = new unsigned char[k * k];
    for (int row = 0; row < k; row++) {
        int global_row = chosen[row];
        for (int col = 0; col < k; col++) {
            tempM[row * k + col] = gen_matrix[global_row * k + col];
        }
    }
    unsigned char *invM = new unsigned char[k * k];
    if (gf_invert_matrix(tempM, invM, k) != 0) {
        // unexpected: invert failed though rows are independent; return empty
        delete[] mat;
        delete[] tempM;
        delete[] invM;
        decode_block_indexes.clear();
        decode_factors.clear();
        return false;
    }

    // decode_block_indexes = chosen (sources)
    decode_block_indexes = chosen;

    // For each failed block, compute coefficients c = G_row_failed * invM
    decode_factors.clear();
    for (int fidx : failed_block_indexes) {
        std::vector<int> factors(k);
        unsigned char *coeff = new unsigned char[k];
        gf_mul_vect_matrix(gen_matrix + fidx * k, invM, coeff, k);
        for (int i = 0; i < k; i++) factors[i] = (int)coeff[i];
        decode_factors.push_back(std::move(factors));
        delete[] coeff;
    }

    delete[] mat;
    delete[] tempM;
    delete[] invM;
    // done
    return true;
}

/* ----- Recovery group and block ids ----- */

std::vector<std::pair<int, std::vector<int>>> get_recovery_group_and_block_ids(const std::string &code_type, int k, int r, int z, int failed_block_id)
{
  if (code_type == "AzureLRC")
    return get_recovery_group_and_block_ids_azurelrc(k, r, z, failed_block_id);
  else if (code_type == "OptimalLRC")
    return get_recovery_group_and_block_ids_optimal_lrc(k, r, z, failed_block_id);
  else if (code_type == "UniformLRC")
    return get_recovery_group_and_block_ids_uniform_lrc(k, r, z, failed_block_id);
  else if (code_type == "UniLRC")
    return get_recovery_group_and_block_ids_unilrc(k, r, z, failed_block_id);
  else if (code_type == "LotusLRC")
    return get_recovery_group_and_block_ids_lotuslrc(k, r, z, failed_block_id);
  else
    throw std::runtime_error("unknown code type");
}

std::vector<std::pair<int, std::vector<int>>> get_recovery_group_and_block_ids_azurelrc(int k, int r, int z, int failed_block_id)
{
  std::vector<std::pair<int, std::vector<int>>> recovery_group_and_block_ids;
  std::vector<int> recovery_block_ids;
  if(failed_block_id < k)
  {
    int local_group_size = k / z;
    int local_group_id = failed_block_id / local_group_size;
    for (int i = local_group_id * local_group_size; i < (local_group_id + 1) * local_group_size; i++)
    {
      if (i != failed_block_id)
        recovery_block_ids.push_back(i);
    }
    recovery_block_ids.push_back(k + r + local_group_id); // plus one local parity block
  }
  else if(failed_block_id < k + r)
  {
    for(int i = r - 1; i < k + r; i++) // global parity blocks need global recovery;
    {
      if (i != failed_block_id)
        recovery_block_ids.push_back(i);
    }
  }
  else
  {
    int local_group_size = k / z;
    int local_group_id = failed_block_id - k - r;
    for (int i = local_group_id * local_group_size; i < (local_group_id + 1) * local_group_size; i++)
    {
      if (i != failed_block_id)
        recovery_block_ids.push_back(i);
    }
  }
  std::unordered_map<int, int> block_id_to_group_id = get_azurelrc_block_id_to_group_id(k, r, z);
  for (size_t i = 0; i < recovery_block_ids.size(); i++)
  {
    int gid = block_id_to_group_id[recovery_block_ids[i]];
    auto it = std::find_if(recovery_group_and_block_ids.begin(), recovery_group_and_block_ids.end(),
        [gid](const std::pair<int, std::vector<int>> &p) { return p.first == gid; });
    if (it == recovery_group_and_block_ids.end())
      recovery_group_and_block_ids.push_back({gid, {recovery_block_ids[i]}});
    else
      it->second.push_back(recovery_block_ids[i]);
  }
  return recovery_group_and_block_ids;
}

std::vector<std::pair<int, std::vector<int>>> get_recovery_group_and_block_ids_lotuslrc(int k, int r, int z, int failed_block_id)
{
  std::vector<std::pair<int, std::vector<int>>> recovery_group_and_block_ids;
  int local_group_id = get_lotuslrc_block_id_to_local_group_id(k, r, z, failed_block_id);
  std::vector<int> group_num_per_local_group = get_lotuslrc_group_num_per_local_group(k, r, z);
  std::vector<int> group_ids;
  int start_group_id = std::accumulate(group_num_per_local_group.begin(), group_num_per_local_group.begin() + local_group_id, 0);
  for (size_t i = 0; i < (size_t)group_num_per_local_group[local_group_id]; i++)
  {
    group_ids.push_back((int)(start_group_id + i));
  }
  std::unordered_map<int, std::vector<int>> group_id_to_block_ids = get_lotuslrc_group_id_to_block_ids(k, r, z);
  for (size_t i = 0; i < group_ids.size(); i++)
  {
    recovery_group_and_block_ids.push_back({group_ids[i], group_id_to_block_ids[group_ids[i]]});
  }
  // remove the failed block
  for (size_t i = 0; i < recovery_group_and_block_ids.size(); i++)
  {
    for (size_t j = 0; j < recovery_group_and_block_ids[i].second.size(); j++)
    {
      if (recovery_group_and_block_ids[i].second[j] == failed_block_id)
        recovery_group_and_block_ids[i].second.erase(recovery_group_and_block_ids[i].second.begin() + (std::ptrdiff_t)j);
    }
  }
  // remove one the last block of the last group because lotuslrc has two local parity blocks, if the last group is empty, remove the group
  if (recovery_group_and_block_ids.size() > 0)
  {
    if (recovery_group_and_block_ids[recovery_group_and_block_ids.size() - 1].second.size() > 0)
      recovery_group_and_block_ids[recovery_group_and_block_ids.size() - 1].second.pop_back();
    if (recovery_group_and_block_ids[recovery_group_and_block_ids.size() - 1].second.size() == 0)
      recovery_group_and_block_ids.erase(recovery_group_and_block_ids.begin() + (std::ptrdiff_t)recovery_group_and_block_ids.size() - 1);
  }
  return recovery_group_and_block_ids;
}

std::vector<std::pair<int, std::vector<int>>> get_recovery_group_and_block_ids_optimal_lrc(int k, int r, int z, int failed_block_id)
{
  std::vector<std::pair<int, std::vector<int>>> recovery_group_and_block_ids;
  std::vector<int> recovery_block_ids;
  if(failed_block_id < k)
  {
    int local_group_size = k / z;
    int local_group_id = failed_block_id / local_group_size;
    for (int i = local_group_id * local_group_size; i < (local_group_id + 1) * local_group_size; i++)
    {
      if (i != failed_block_id)
        recovery_block_ids.push_back(i);
    }
    recovery_block_ids.push_back(k + r + local_group_id); // plus one local parity block
    for(int i = k; i < k + r; i++)
    {
      recovery_block_ids.push_back(i); // optimal lrc need global parity blocks for local group recovery
    }
  }
  else if(failed_block_id < k + r)
  {
    // use the first local group for recovery
    for(int i = 0; i < k / z; i++)
    {
      recovery_block_ids.push_back(i);
    }
    recovery_block_ids.push_back(k + r); // plus one local parity block
    for(int i = k; i < k + r; i++)
    {
      if(i != failed_block_id)
      recovery_block_ids.push_back(i); // optimal lrc need global parity blocks for local group recovery
    }
  }
  else
  {
    int local_group_size = k / z;
    int local_group_id = failed_block_id - k - r;
    for (int i = local_group_id * local_group_size; i < (local_group_id + 1) * local_group_size; i++)
    {
      if (i != failed_block_id)
        recovery_block_ids.push_back(i);
    }
    for(int i = k; i < k + r; i++)
    {
      recovery_block_ids.push_back(i); // optimal lrc need global parity blocks for local group recovery
    }
  }
  std::unordered_map<int, int> block_id_to_group_id = get_optimal_lrc_block_id_to_group_id(k, r, z);
  for (size_t i = 0; i < recovery_block_ids.size(); i++)
  {
    int gid = block_id_to_group_id[recovery_block_ids[i]];
    auto it = std::find_if(recovery_group_and_block_ids.begin(), recovery_group_and_block_ids.end(),
        [gid](const std::pair<int, std::vector<int>> &p) { return p.first == gid; });
    if (it == recovery_group_and_block_ids.end())
      recovery_group_and_block_ids.push_back({gid, {recovery_block_ids[i]}});
    else
      it->second.push_back(recovery_block_ids[i]);
  }
  return recovery_group_and_block_ids;
}

std::vector<std::pair<int, std::vector<int>>> get_recovery_group_and_block_ids_uniform_lrc(int k, int r, int z, int failed_block_id)
{
  std::vector<std::pair<int, std::vector<int>>> recovery_group_and_block_ids;
  int local_group_id = get_uniform_lrc_block_id_to_local_group_id(k, r, z, failed_block_id);
  std::vector<int> group_num_per_local_group = get_uniform_lrc_group_num_per_local_group(k, r, z);
  std::vector<int> group_ids;
  int start_group_id = std::accumulate(group_num_per_local_group.begin(), group_num_per_local_group.begin() + local_group_id, 0);
  for (size_t i = 0; i < (size_t)group_num_per_local_group[local_group_id]; i++)
  {
    group_ids.push_back((int)(start_group_id + i));
  }
  std::unordered_map<int, std::vector<int>> group_id_to_block_ids = get_uniform_lrc_group_id_to_block_ids(k, r, z);
  for (size_t i = 0; i < group_ids.size(); i++)
  {
    recovery_group_and_block_ids.push_back({group_ids[i], group_id_to_block_ids[group_ids[i]]});
  }
  // remove the failed block, if the failed block is the last block of the group, remove the group
  for (size_t i = 0; i < recovery_group_and_block_ids.size(); i++)
  {
    for (size_t j = 0; j < recovery_group_and_block_ids[i].second.size(); j++)
    {
      if (recovery_group_and_block_ids[i].second[j] == failed_block_id)
        recovery_group_and_block_ids[i].second.erase(recovery_group_and_block_ids[i].second.begin() + (std::ptrdiff_t)j);
      if (recovery_group_and_block_ids[i].second.size() == 0)
        recovery_group_and_block_ids.erase(recovery_group_and_block_ids.begin() + (std::ptrdiff_t)i);
    }
  }
  return recovery_group_and_block_ids;
}

std::vector<std::pair<int, std::vector<int>>> get_recovery_group_and_block_ids_unilrc(int k, int r, int z, int failed_block_id)
{
  return {{0, {failed_block_id}}}; // TODO
}

} // namespace ECProject
