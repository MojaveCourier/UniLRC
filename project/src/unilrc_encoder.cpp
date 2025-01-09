#include "unilrc_encoder.h"
#include <iostream>

unsigned char
ECProject::gf_mul(unsigned char a, unsigned char b)
{
    int i;

    if ((a == 0) || (b == 0))
        return 0;

    return ECProject::gff_base[(i = ECProject::gflog_base[a] + ECProject::gflog_base[b]) > 254 ? i - 255 : i];
}

unsigned char
ECProject::gf_inv(unsigned char a)
{
    if (a == 0)
        return 0;
    return ECProject::gff_base[255 - ECProject::gflog_base[a]];
}

void ECProject::gf_gen_cauchy_matrix(unsigned char **a, int m, int k)
{
    for (int i = 0; i < m - k; i++)
        for (int j = 0; j < k; j++)
        {
            a[i][j] = ECProject::gf_inv((i + k) ^ j);
        }
}

void ECProject::gf_gen_local_vector(unsigned char *a, int k, int p)
{
    int i;

    for (i = 0; i < k; i++)
    {
        a[i] = ECProject::gf_inv(i ^ (k + p));
    }
}

void ECProject::gf_gen_rs_matrix(unsigned char **a, int m, int k)
{
    for (int j = 0; j < k; j++)
    {
        int gen = gff_base[j];
        a[0][j] = gen;
        for (int i = 1; i < m - k; i++)
        {
            a[i][j] = gf_mul(a[i - 1][j], gen);
        }
    }
}

void ECProject::encode_unilrc_w_append_mode(int k, int r, int z, int data_num, unsigned char **data_ptrs,
                                            const std::vector<int> *data_sizes, unsigned char **global_ptrs,
                                            unsigned char **local_ptrs, int start_offset, int unit_size, bool is_cached)
{
    unsigned char **rs_matrix;
    rs_matrix = new unsigned char *[r];
    for (int i = 0; i < r; i++)
    {
        rs_matrix[i] = new unsigned char[k];
    }
    ECProject::gf_gen_rs_matrix(rs_matrix, k + r, k);
    std::vector<int> block_idx;
    if (data_num + start_offset < k)
    {
        for (int i = 0; i < data_num; i++)
        {
            block_idx.push_back(i + start_offset);
        }
    }
    else
    {
        for (int i = 0; i < data_num + start_offset - k; i++)
        {
            block_idx.push_back(i);
        }
        for (int i = start_offset; i < k; i++)
        {
            block_idx.push_back(i);
        }
    }

    int parity_size = 0;
    for (int i = 0; i < data_num; i++)
    {
        if (block_idx[i] < start_offset)
        {
            if (parity_size < data_sizes->at(i) + unit_size)
            {
                parity_size = data_sizes->at(i) + unit_size;
            }
        }
        else
        {
            if (parity_size < data_sizes->at(i))
            {
                parity_size = data_sizes->at(i);
            }
        }

        if (block_idx[i] == start_offset && data_num > 1)
        {
            if (data_sizes->at(i) % unit_size != 0 && parity_size < (data_sizes->at(i) / unit_size + 1) * unit_size)
            {
                parity_size = (data_sizes->at(i) / unit_size + 1) * unit_size;
            }
        }
    }

    if (!is_cached)
    {
        for (int i = 0; i < parity_size; i++)
        {
            for (int j = 0; j < r; j++)
            {
                global_ptrs[j][i] = 0;
            }
            for (int j = 0; j < z; j++)
            {
                local_ptrs[j][i] = 0;
            }
        }
    }

    std::vector<int> block_start;
    if (data_num == 1)
    {
        block_start.push_back(0);
    }
    else
    {
        for (int i = 0; i < data_num; i++)
        {
            if (block_idx[i] < start_offset)
            {
                block_start.push_back(unit_size);
            }

            else if (block_idx[i] == start_offset)
            {
                int remain = data_sizes->at(i) % unit_size;
                if (remain == 0)
                {
                    block_start.push_back(0);
                }

                else
                {
                    block_start.push_back(unit_size - remain);
                }
            }

            else
            {
                block_start.push_back(0);
            }
        }
    }

    for (int i = 0; i < data_num; i++)
    {
        int local_group = block_idx[i] / (k / z);
        for (int j = 0; j < data_sizes->at(i); j++)
        {
            int parity_index = j + block_start[i];
            for (int l = 0; l < r; l++)
            {
                global_ptrs[l][parity_index] ^= gf_mul(rs_matrix[l][block_idx[i]], data_ptrs[i][j]);
            }
            local_ptrs[local_group][parity_index] ^= data_ptrs[i][j];
        }
    }

    for (int i = 0; i < z; i++)
    {
        for (int j = 0; j < r / z; j++)
        {
            for (int l = 0; l < parity_size; l++)
            {
                local_ptrs[i][l] ^= global_ptrs[r / z * i + j][l];
            }
        }
    }
}

void ECProject::encode_unilrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **global_ptrs,
                              unsigned char **local_ptrs, int block_size)
{
    unsigned char **rs_matrix;
    rs_matrix = new unsigned char *[r];
    for (int i = 0; i < r; i++)
    {
        rs_matrix[i] = new unsigned char[k];
    }
    gf_gen_rs_matrix(rs_matrix, k + r, k);

    for (int i = 0; i < block_size; i++)
    {
        for (int j = 0; j < r; j++)
        {
            global_ptrs[j][i] = 0;
        }
        for (int j = 0; j < z; j++)
        {
            local_ptrs[j][i] = 0;
        }
    }

    for (int i = 0; i < k; i++)
    {
        int local_group = i / (k / z);
        for (int j = 0; j < block_size; j++)
        {
            for (int l = 0; l < r; l++)
            {
                global_ptrs[l][j] ^= gf_mul(rs_matrix[l][i], data_ptrs[i][j]);
            }
            local_ptrs[local_group][j] ^= data_ptrs[i][j];
        }
    }

    for (int i = 0; i < z; i++)
    {
        for (int j = 0; j < r / z; j++)
        {
            for (int l = 0; l < block_size; l++)
            {
                local_ptrs[i][l] ^= global_ptrs[r / z * i + j][l];
            }
        }
    }
}

void ECProject::encode_azure_lrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **global_ptrs,
                                 unsigned char **local_ptrs, int block_size)
{
    unsigned char **cauchy_matrix;
    cauchy_matrix = new unsigned char *[r];
    for (int i = 0; i < r; i++)
    {
        cauchy_matrix[i] = new unsigned char[k];
    }
    gf_gen_cauchy_matrix(cauchy_matrix, k + r, k);

    for (int i = 0; i < block_size; i++)
    {
        for (int j = 0; j < r; j++)
        {
            global_ptrs[j][i] = 0;
        }
        for (int j = 0; j < z; j++)
        {
            local_ptrs[j][i] = 0;
        }
    }

    for (int i = 0; i < k; i++)
    {
        int local_group = i / (k / z);
        for (int j = 0; j < block_size; j++)
        {
            for (int l = 0; l < r; l++)
            {
                global_ptrs[l][j] ^= gf_mul(cauchy_matrix[l][i], data_ptrs[i][j]);
            }
            local_ptrs[local_group][j] ^= data_ptrs[i][j];
        }
    }
}

void ECProject::encode_optimal_lrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **global_ptrs,
                                   unsigned char **local_ptrs, int block_size)
{
    unsigned char **cauchy_matrix;
    unsigned char *local_vector;
    cauchy_matrix = new unsigned char *[r];
    for (int i = 0; i < r; i++)
    {
        cauchy_matrix[i] = new unsigned char[k];
    }
    local_vector = new unsigned char[k];
    gf_gen_cauchy_matrix(cauchy_matrix, k + r, k);
    gf_gen_local_vector(local_vector, k, r);

    for (int i = 0; i < block_size; i++)
    {
        for (int j = 0; j < r; j++)
        {
            global_ptrs[j][i] = 0;
        }
        for (int j = 0; j < z; j++)
        {
            local_ptrs[j][i] = 0;
        }
    }

    for (int i = 0; i < k; i++)
    {
        int local_group = i / (k / z);
        for (int j = 0; j < block_size; j++)
        {
            for (int l = 0; l < r; l++)
            {
                global_ptrs[l][j] ^= gf_mul(cauchy_matrix[l][i], data_ptrs[i][j]);
            }
            local_ptrs[local_group][j] ^= gf_mul(local_vector[i], data_ptrs[i][j]);
        }
    }

    for (int i = 0; i < z; i++)
    {
        for (int j = 0; j < r; j++)
        {
            for (int l = 0; l < block_size; l++)
            {
                local_ptrs[i][l] ^= global_ptrs[j][l];
            }
        }
    }
}

void ECProject::encode_uniform_lrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **global_ptrs,
                                   unsigned char **local_ptrs, int block_size)
{
    unsigned char **cauchy_matrix;
    unsigned char *local_vector;
    cauchy_matrix = new unsigned char *[r];
    for (int i = 0; i < r; i++)
    {
        cauchy_matrix[i] = new unsigned char[k];
    }
    local_vector = new unsigned char[k];
    gf_gen_cauchy_matrix(cauchy_matrix, k + r, k);
    gf_gen_local_vector(local_vector, k, r);

    for (int i = 0; i < block_size; i++)
    {
        for (int j = 0; j < r; j++)
        {
            global_ptrs[j][i] = 0;
        }
        for (int j = 0; j < z; j++)
        {
            local_ptrs[j][i] = 0;
        }
    }

    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < block_size; j++)
        {
            for (int l = 0; l < r; l++)
            {
                global_ptrs[l][j] ^= gf_mul(cauchy_matrix[l][i], data_ptrs[i][j]);
            }
        }
    }

    int local_group_size = (k + r) / z;
    int larger_local_group_num = (k + r) % z;
    int block_num = 0;
    for (int i = 0; i < z; i++)
    {
        if (i + larger_local_group_num == z)
        {
            local_group_size++;
        }
        for (int j = 0; j < local_group_size; j++)
        {
            for (int l = 0; l < block_size; l++)
            {
                local_ptrs[i][l] ^= gf_mul(local_vector[block_num], data_ptrs[block_num][l]);
            }
            block_num++;
            if (block_num == k)
            {
                break;
            }
        }
    }
    for (int j = 0; j < r; j++)
    {
        for (int l = 0; l < block_size; l++)
        {
            local_ptrs[z - 1][l] ^= global_ptrs[j][l];
        }
    }
}

void ECProject::encode_unilrc_w_rep_mode(int k, int r, int z, unsigned char *data_ptrs, unsigned char *parity_ptr,
                                         int block_size, int unit_size, int parity_block_id)
{
    unsigned char **rs_matrix;
    rs_matrix = new unsigned char *[r];
    for (int i = 0; i < r; i++)
    {
        rs_matrix[i] = new unsigned char[k];
    }

    gf_gen_rs_matrix(rs_matrix, k + r, k);

    for (int i = 0; i < block_size; i++)
    {
        parity_ptr[i] = 0;
    }

    int block_idx[block_size][k];
    for (int i = 0; i < block_size; i++)
    {
        for (int j = 0; j < k; j++)
        {
            // 计算出第i,j个数据块在data_ptrs中的实际位置
            block_idx[i][j] = (i / unit_size * k + j) * unit_size + i % unit_size;
        }
    }

    if (parity_block_id >= k && parity_block_id < k + r)
    {
        for (int i = 0; i < block_size; i++)
        {
            for (int j = 0; j < k; j++)
            {
                parity_ptr[i] ^= gf_mul(rs_matrix[parity_block_id - k][j], data_ptrs[block_idx[i][j]]);
            }
        }
    }
    else
    {
        int local_group = parity_block_id - k - r;
        for (int i = 0; i < block_size; i++)
        {
            for (int j = local_group * (k / z); j < (local_group + 1) * (k / z); j++)
            {
                parity_ptr[i] ^= data_ptrs[block_idx[i][j]];
            }
            for (int j = 0; j < k; j++)
            {
                for (int l = 0; l < r / z; l++)
                {
                    parity_ptr[i] ^= gf_mul(rs_matrix[l + local_group * (r / z)][j], data_ptrs[block_idx[i][j]]);
                }
            }
        }
    }
}
