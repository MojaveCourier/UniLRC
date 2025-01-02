#include "unilrc_encoder.h"
#include <iostream>

unsigned char
gf_mul(unsigned char a, unsigned char b)
{
    int i;

    if ((a == 0) || (b == 0))
        return 0;

    return ECProject::gff_base[(i = ECProject::gflog_base[a] + ECProject::gflog_base[b]) > 254 ? i - 255 : i];
}

unsigned char
gf_inv(unsigned char a)
{
    if (a == 0)
        return 0;
    return ECProject::gff_base[255 - ECProject::gflog_base[a]];
}

void gf_gen_cauchy_matrix(unsigned char **a, int m, int k)
{
    for (int i = 0; i < m - k; i++)
        for (int j = 0; j < k; j++)
        {
            a[i][j] = gf_inv((i + k) ^ j);
        }
}

void gf_gen_local_vector(unsigned char *a, int k, int p)
{
    int i;

    for (i = 0; i < k; i++)
    {
        a[i] = gf_inv(i ^ (k + p));
    }
}

void encode(int k, int r, int z, int data_num, unsigned char **data_ptrs,
            const std::unique_ptr<std::vector<int>> &data_sizes, unsigned char **global_ptrs,
            unsigned char **local_ptrs, int start_offset, int unit_size)
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
                global_ptrs[l][parity_index] ^= gf_mul(cauchy_matrix[l][block_idx[i]], data_ptrs[i][j]);
            }
            local_ptrs[local_group][parity_index] ^= gf_mul(local_vector[block_idx[i]], data_ptrs[i][j]);
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
