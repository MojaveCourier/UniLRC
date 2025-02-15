#include "unilrc_encoder.h"
#include <iostream>

extern "C" {
    void gf_vect_dot_prod_avx2(int len, int vec, unsigned char *g_tbls, unsigned char **buffs, unsigned char*dests);
    void gf_2vect_dot_prod_avx2(int len, int vec, unsigned char *g_tbls, unsigned char **buffs, unsigned char**dests);
    void gf_3vect_dot_prod_avx2(int len, int vec, unsigned char *g_tbls, unsigned char **buffs, unsigned char**dests);
    void gf_4vect_dot_prod_avx2(int len, int vec, unsigned char *g_tbls, unsigned char **buffs, unsigned char**dests);
    void gf_5vect_dot_prod_avx2(int len, int vec, unsigned char *g_tbls, unsigned char **buffs, unsigned char**dests);
    void gf_6vect_dot_prod_avx2(int len, int vec, unsigned char *g_tbls, unsigned char **buffs, unsigned char**dests);
}

void 
ECProject::gf_xor_mul_64(unsigned char *a, unsigned char *b, const unsigned char *mul_table)
{
    a[0] ^= mul_table[b[0]];
    a[1] ^= mul_table[b[1]];
    a[2] ^= mul_table[b[2]];
    a[3] ^= mul_table[b[3]];
    a[4] ^= mul_table[b[4]];
    a[5] ^= mul_table[b[5]];
    a[6] ^= mul_table[b[6]];
    a[7] ^= mul_table[b[7]];
    a[8] ^= mul_table[b[8]];
    a[9] ^= mul_table[b[9]];
    a[10] ^= mul_table[b[10]];
    a[11] ^= mul_table[b[11]];
    a[12] ^= mul_table[b[12]];
    a[13] ^= mul_table[b[13]];
    a[14] ^= mul_table[b[14]];
    a[15] ^= mul_table[b[15]];
    a[16] ^= mul_table[b[16]];
    a[17] ^= mul_table[b[17]];
    a[18] ^= mul_table[b[18]];
    a[19] ^= mul_table[b[19]];
    a[20] ^= mul_table[b[20]];
    a[21] ^= mul_table[b[21]];
    a[22] ^= mul_table[b[22]];
    a[23] ^= mul_table[b[23]];
    a[24] ^= mul_table[b[24]];
    a[25] ^= mul_table[b[25]];
    a[26] ^= mul_table[b[26]];
    a[27] ^= mul_table[b[27]];
    a[28] ^= mul_table[b[28]];
    a[29] ^= mul_table[b[29]];
    a[30] ^= mul_table[b[30]];
    a[31] ^= mul_table[b[31]];
    a[32] ^= mul_table[b[32]];
    a[33] ^= mul_table[b[33]];
    a[34] ^= mul_table[b[34]];
    a[35] ^= mul_table[b[35]];
    a[36] ^= mul_table[b[36]];
    a[37] ^= mul_table[b[37]];
    a[38] ^= mul_table[b[38]];
    a[39] ^= mul_table[b[39]];
    a[40] ^= mul_table[b[40]];
    a[41] ^= mul_table[b[41]];
    a[42] ^= mul_table[b[42]];
    a[43] ^= mul_table[b[43]];
    a[44] ^= mul_table[b[44]];
    a[45] ^= mul_table[b[45]];
    a[46] ^= mul_table[b[46]];
    a[47] ^= mul_table[b[47]];
    a[48] ^= mul_table[b[48]];
    a[49] ^= mul_table[b[49]];
    a[50] ^= mul_table[b[50]];
    a[51] ^= mul_table[b[51]];
    a[52] ^= mul_table[b[52]];
    a[53] ^= mul_table[b[53]];
    a[54] ^= mul_table[b[54]];
    a[55] ^= mul_table[b[55]];
    a[56] ^= mul_table[b[56]];
    a[57] ^= mul_table[b[57]];
    a[58] ^= mul_table[b[58]];
    a[59] ^= mul_table[b[59]];
    a[60] ^= mul_table[b[60]];
    a[61] ^= mul_table[b[61]];
    a[62] ^= mul_table[b[62]];
    a[63] ^= mul_table[b[63]];
}

void
ECProject::gf_xor_64(unsigned char *a, unsigned char *b)
{
    a[0] ^= b[0];
    a[1] ^= b[1];
    a[2] ^= b[2];
    a[3] ^= b[3];
    a[4] ^= b[4];
    a[5] ^= b[5];
    a[6] ^= b[6];
    a[7] ^= b[7];
    a[8] ^= b[8];
    a[9] ^= b[9];
    a[10] ^= b[10];
    a[11] ^= b[11];
    a[12] ^= b[12];
    a[13] ^= b[13];
    a[14] ^= b[14];
    a[15] ^= b[15];
    a[16] ^= b[16];
    a[17] ^= b[17];
    a[18] ^= b[18];
    a[19] ^= b[19];
    a[20] ^= b[20];
    a[21] ^= b[21];
    a[22] ^= b[22];
    a[23] ^= b[23];
    a[24] ^= b[24];
    a[25] ^= b[25];
    a[26] ^= b[26];
    a[27] ^= b[27];
    a[28] ^= b[28];
    a[29] ^= b[29];
    a[30] ^= b[30];
    a[31] ^= b[31];
    a[32] ^= b[32];
    a[33] ^= b[33];
    a[34] ^= b[34];
    a[35] ^= b[35];
    a[36] ^= b[36];
    a[37] ^= b[37];
    a[38] ^= b[38];
    a[39] ^= b[39];
    a[40] ^= b[40];
    a[41] ^= b[41];
    a[42] ^= b[42];
    a[43] ^= b[43];
    a[44] ^= b[44];
    a[45] ^= b[45];
    a[46] ^= b[46];
    a[47] ^= b[47];
    a[48] ^= b[48];
    a[49] ^= b[49];
    a[50] ^= b[50];
    a[51] ^= b[51];
    a[52] ^= b[52];
    a[53] ^= b[53];
    a[54] ^= b[54];
    a[55] ^= b[55];
    a[56] ^= b[56];
    a[57] ^= b[57];
    a[58] ^= b[58];
    a[59] ^= b[59];
    a[60] ^= b[60];
    a[61] ^= b[61];
    a[62] ^= b[62];
    a[63] ^= b[63];
}

void
ECProject::gf_xor_mul_idx_64(unsigned char *a, unsigned char *b, int *idx ,const unsigned char *mul_table)

{
    a[0] ^= mul_table[b[idx[0]]];
    a[1] ^= mul_table[b[idx[1]]];
    a[2] ^= mul_table[b[idx[2]]];
    a[3] ^= mul_table[b[idx[3]]];
    a[4] ^= mul_table[b[idx[4]]];
    a[5] ^= mul_table[b[idx[5]]];
    a[6] ^= mul_table[b[idx[6]]];
    a[7] ^= mul_table[b[idx[7]]];
    a[8] ^= mul_table[b[idx[8]]];
    a[9] ^= mul_table[b[idx[9]]];
    a[10] ^= mul_table[b[idx[10]]];
    a[11] ^= mul_table[b[idx[11]]];
    a[12] ^= mul_table[b[idx[12]]];
    a[13] ^= mul_table[b[idx[13]]];
    a[14] ^= mul_table[b[idx[14]]];
    a[15] ^= mul_table[b[idx[15]]];
    a[16] ^= mul_table[b[idx[16]]];
    a[17] ^= mul_table[b[idx[17]]];
    a[18] ^= mul_table[b[idx[18]]];
    a[19] ^= mul_table[b[idx[19]]];
    a[20] ^= mul_table[b[idx[20]]];
    a[21] ^= mul_table[b[idx[21]]];
    a[22] ^= mul_table[b[idx[22]]];
    a[23] ^= mul_table[b[idx[23]]];
    a[24] ^= mul_table[b[idx[24]]];
    a[25] ^= mul_table[b[idx[25]]];
    a[26] ^= mul_table[b[idx[26]]];
    a[27] ^= mul_table[b[idx[27]]];
    a[28] ^= mul_table[b[idx[28]]];
    a[29] ^= mul_table[b[idx[29]]];
    a[30] ^= mul_table[b[idx[30]]];
    a[31] ^= mul_table[b[idx[31]]];
    a[32] ^= mul_table[b[idx[32]]];
    a[33] ^= mul_table[b[idx[33]]];
    a[34] ^= mul_table[b[idx[34]]];
    a[35] ^= mul_table[b[idx[35]]];
    a[36] ^= mul_table[b[idx[36]]];
    a[37] ^= mul_table[b[idx[37]]];
    a[38] ^= mul_table[b[idx[38]]];
    a[39] ^= mul_table[b[idx[39]]];
    a[40] ^= mul_table[b[idx[40]]];
    a[41] ^= mul_table[b[idx[41]]];
    a[42] ^= mul_table[b[idx[42]]];
    a[43] ^= mul_table[b[idx[43]]];
    a[44] ^= mul_table[b[idx[44]]];
    a[45] ^= mul_table[b[idx[45]]];
    a[46] ^= mul_table[b[idx[46]]];
    a[47] ^= mul_table[b[idx[47]]];
    a[48] ^= mul_table[b[idx[48]]];
    a[49] ^= mul_table[b[idx[49]]];
    a[50] ^= mul_table[b[idx[50]]];
    a[51] ^= mul_table[b[idx[51]]];
    a[52] ^= mul_table[b[idx[52]]];
    a[53] ^= mul_table[b[idx[53]]];
    a[54] ^= mul_table[b[idx[54]]];
    a[55] ^= mul_table[b[idx[55]]];
    a[56] ^= mul_table[b[idx[56]]];
    a[57] ^= mul_table[b[idx[57]]];
    a[58] ^= mul_table[b[idx[58]]];
    a[59] ^= mul_table[b[idx[59]]];
    a[60] ^= mul_table[b[idx[60]]];
    a[61] ^= mul_table[b[idx[61]]];
    a[62] ^= mul_table[b[idx[62]]];
    a[63] ^= mul_table[b[idx[63]]];
}

void
ECProject::gf_xor_idx_64(unsigned char *a, unsigned char *b, int *idx)
{
    a[0] ^= b[idx[0]];
    a[1] ^= b[idx[1]];
    a[2] ^= b[idx[2]];
    a[3] ^= b[idx[3]];
    a[4] ^= b[idx[4]];
    a[5] ^= b[idx[5]];
    a[6] ^= b[idx[6]];
    a[7] ^= b[idx[7]];
    a[8] ^= b[idx[8]];
    a[9] ^= b[idx[9]];
    a[10] ^= b[idx[10]];
    a[11] ^= b[idx[11]];
    a[12] ^= b[idx[12]];
    a[13] ^= b[idx[13]];
    a[14] ^= b[idx[14]];
    a[15] ^= b[idx[15]];
    a[16] ^= b[idx[16]];
    a[17] ^= b[idx[17]];
    a[18] ^= b[idx[18]];
    a[19] ^= b[idx[19]];
    a[20] ^= b[idx[20]];
    a[21] ^= b[idx[21]];
    a[22] ^= b[idx[22]];
    a[23] ^= b[idx[23]];
    a[24] ^= b[idx[24]];
    a[25] ^= b[idx[25]];
    a[26] ^= b[idx[26]];
    a[27] ^= b[idx[27]];
    a[28] ^= b[idx[28]];
    a[29] ^= b[idx[29]];
    a[30] ^= b[idx[30]];
    a[31] ^= b[idx[31]];
    a[32] ^= b[idx[32]];
    a[33] ^= b[idx[33]];
    a[34] ^= b[idx[34]];
    a[35] ^= b[idx[35]];
    a[36] ^= b[idx[36]];
    a[37] ^= b[idx[37]];
    a[38] ^= b[idx[38]];
    a[39] ^= b[idx[39]];
    a[40] ^= b[idx[40]];
    a[41] ^= b[idx[41]];
    a[42] ^= b[idx[42]];
    a[43] ^= b[idx[43]];
    a[44] ^= b[idx[44]];
    a[45] ^= b[idx[45]];
    a[46] ^= b[idx[46]];
    a[47] ^= b[idx[47]];
    a[48] ^= b[idx[48]];
    a[49] ^= b[idx[49]];
    a[50] ^= b[idx[50]];
    a[51] ^= b[idx[51]];
    a[52] ^= b[idx[52]];
    a[53] ^= b[idx[53]];
    a[54] ^= b[idx[54]];
    a[55] ^= b[idx[55]];
    a[56] ^= b[idx[56]];
    a[57] ^= b[idx[57]];
    a[58] ^= b[idx[58]];
    a[59] ^= b[idx[59]];
    a[60] ^= b[idx[60]];
    a[61] ^= b[idx[61]];
    a[62] ^= b[idx[62]];
    a[63] ^= b[idx[63]];
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
            a[i][j] = gf_mul_table_base[a[i - 1][j]][gen];
        }
    }
}

void
ECProject::gf_gen_rs_matrix1(unsigned char *a, int m, int k)
{
        int i, j;
        unsigned char p, gen = 2;

        memset(a, 0, k * m);
        for (i = 0; i < k; i++)
                a[k * i + i] = 1;

        for (i = k; i < m; i++) {
                p = 1;
                for (j = 0; j < k; j++) {
                        a[k * i + j] = p;
                        p = gf_mul(p, gen);
                }
                gen = gf_mul(gen, 2);
        }
}

void
ECProject::gf_gen_cauchy_matrix1(unsigned char *a, int m, int k)
{
        int i, j;
        unsigned char *p;

        // Identity matrix in high position
        memset(a, 0, k * m);
        for (i = 0; i < k; i++)
                a[k * i + i] = 1;

        // For the rest choose 1/(i + j) | i != j
        p = &a[k * k];
        for (i = k; i < m; i++)
                for (j = 0; j < k; j++)
                        *p++ = gf_inv(i ^ j);
}

unsigned char
ECProject::gf_mul(unsigned char a, unsigned char b)
{
#ifndef GF_LARGE_TABLES
        int i;

        if ((a == 0) || (b == 0))
                return 0;

        return gff_base[(i = gflog_base[a] + gflog_base[b]) > 254 ? i - 255 : i];
#else
        return gf_mul_table_base[b * 256 + a];
#endif
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
        for (int i = 0; i < r; i++)
        {
            memset(global_ptrs[i], 0, parity_size);
        }
        for (int i = 0; i < z; i++)
        {
            memset(local_ptrs[i], 0, parity_size);
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
        
        for (int j = 0; j < r; j++){
            const unsigned char *mul_table = gf_mul_table_base[rs_matrix[j][block_idx[i]]];
            for(int l = 0; l <data_sizes->at(i) / 64; l+= 64){
                gf_xor_mul_64(global_ptrs[j] + l + block_start[i], data_ptrs[i] + l, mul_table);
            }
            for(int l = data_sizes->at(i) / 64 * 64; l < data_sizes->at(i); l++){
                global_ptrs[j][l + block_start[i]] ^= mul_table[data_ptrs[i][l]];
            }
        }

        for(int j = 0; j < data_sizes->at(i) / 64; j+= 64){
            int parity_index = j + block_start[i];
            gf_xor_64(local_ptrs[local_group] + block_start[i] + j, data_ptrs[i] + j);
        }
        for(int j = data_sizes->at(i) / 64 * 64; j < data_sizes->at(i); j++){
            local_ptrs[local_group][j + block_start[i]] ^= data_ptrs[i][j];
        }


    }

    for (int i = 0; i < z; i++)
    {
        for (int j = 0; j < r / z; j++)
        {
            int global_idx = r / z * i + j;
            
            const unsigned char *mul_table = gf_mul_table_base[rs_matrix[global_idx][block_idx[i]]];
            for (int l = 0; l < parity_size / 64; l+=64)
            {
                gf_xor_mul_64(local_ptrs[i] + l, global_ptrs[global_idx] + l, mul_table);
            }
            for(int l = parity_size / 64 * 64; l < parity_size; l++){
                local_ptrs[i][l] ^= global_ptrs[global_idx][l];
            }
        }
    }

    for (int i = 0; i < r; i++)
    {
        delete[] rs_matrix[i];
    }
    delete[] rs_matrix;
}

void ECProject::encode_azure_lrc_w_append_mode(int k, int r, int z, int data_num, unsigned char **data_ptrs,
                                                const std::vector<int> *data_sizes, unsigned char **global_ptrs,
                                                unsigned char **local_ptrs, int start_offset, int unit_size)
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


    for (int i = 0; i < r; i++)
    {
        memset(global_ptrs[i], 0, parity_size);
    }
    for (int i = 0; i < z; i++)
    {
        memset(local_ptrs[i], 0, parity_size);
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
        
        for (int j = 0; j < r; j++){
            const unsigned char *mul_table = gf_mul_table_base[rs_matrix[j][block_idx[i]]];
            for(int l = 0; l <data_sizes->at(i) / 64; l+= 64){
                gf_xor_mul_64(global_ptrs[j] + l + block_start[i], data_ptrs[i] + l, mul_table);
            }
            for(int l = data_sizes->at(i) / 64 * 64; l < data_sizes->at(i); l++){
                global_ptrs[j][l + block_start[i]] ^= mul_table[data_ptrs[i][l]];
            }
        }

        for(int j = 0; j < data_sizes->at(i) / 64; j+= 64){
            int parity_index = j + block_start[i];
            gf_xor_64(local_ptrs[local_group] + block_start[i] + j, data_ptrs[i] + j);
        }
        for(int j = data_sizes->at(i) / 64 * 64; j < data_sizes->at(i); j++){
            local_ptrs[local_group][j + block_start[i]] ^= data_ptrs[i][j];
        }
    }

    for (int i = 0; i < r; i++)
    {
        delete[] rs_matrix[i];
    }
    delete[] rs_matrix;
}

void ECProject::encode_optimal_lrc_w_append_mode(int k, int r, int z, int data_num, unsigned char **data_ptrs,
                                                const std::vector<int> *data_sizes, unsigned char **global_ptrs,
                                                unsigned char **local_ptrs, int start_offset, int unit_size)
{
    unsigned char **cauchy_matrix;
    cauchy_matrix = new unsigned char *[r];
    unsigned char *local_vector;
    local_vector = new unsigned char[k];

    for (int i = 0; i < r; i++)
    {
        cauchy_matrix[i] = new unsigned char[k];
    }
    ECProject::gf_gen_cauchy_matrix(cauchy_matrix, k + r, k);
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


    for (int i = 0; i < r; i++)
    {
        memset(global_ptrs[i], 0, parity_size);
    }
    for (int i = 0; i < z; i++)
    {
        memset(local_ptrs[i], 0, parity_size);
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
        
        for (int j = 0; j < r; j++){
            const unsigned char *mul_table = gf_mul_table_base[cauchy_matrix[j][block_idx[i]]];
            for(int l = 0; l <data_sizes->at(i) / 64; l+= 64){
                gf_xor_mul_64(global_ptrs[j] + l + block_start[i], data_ptrs[i] + l, mul_table);
            }
            for(int l = data_sizes->at(i) / 64 * 64; l < data_sizes->at(i); l++){
                global_ptrs[j][l + block_start[i]] ^= mul_table[data_ptrs[i][l]];
            }
        }

        const unsigned char *mul_table_local = gf_mul_table_base[local_vector[block_idx[i]]];
        for(int j = 0; j < data_sizes->at(i) / 64; j+= 64){
            
            gf_xor_mul_64(local_ptrs[local_group] + block_start[i] + j, data_ptrs[i] + j, mul_table_local);
        }
        for(int j = data_sizes->at(i) / 64 * 64; j < data_sizes->at(i); j++){
            local_ptrs[local_group][j + block_start[i]] ^= mul_table_local[data_ptrs[i][j]];
        }
    }

    for(int i = 0; i < z; i++){
        for(int j = 0; j < r / z; j++){
            int global_idx = r / z * i + j;
            const unsigned char *mul_table = gf_mul_table_base[cauchy_matrix[global_idx][block_idx[i]]];
            for(int l = 0; l < parity_size / 64; l+=64){
                gf_xor_mul_64(local_ptrs[i] + l, global_ptrs[global_idx] + l, mul_table);
            }
            for(int l = parity_size / 64 * 64; l < parity_size; l++){
                local_ptrs[i][l] ^= global_ptrs[global_idx][l];
            }
        }
    }

    for (int i = 0; i < r; i++)
    {
        delete[] cauchy_matrix[i];
    }
    delete[] cauchy_matrix;
}

void ECProject::encode_uniform_lrc_w_append_mode(int k, int r, int z, int data_num, unsigned char **data_ptrs,
                                                const std::vector<int> *data_sizes, unsigned char **global_ptrs,
                                                unsigned char **local_ptrs, int start_offset, int unit_size)
{
    unsigned char **cauchy_matrix;
    cauchy_matrix = new unsigned char *[r];
    unsigned char *local_vector;
    local_vector = new unsigned char[k];

    for (int i = 0; i < r; i++)
    {
        cauchy_matrix[i] = new unsigned char[k];
    }
    ECProject::gf_gen_cauchy_matrix(cauchy_matrix, k + r, k);
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


    for (int i = 0; i < r; i++)
    {
        memset(global_ptrs[i], 0, parity_size);
    }
    for (int i = 0; i < z; i++)
    {
        memset(local_ptrs[i], 0, parity_size);
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
        
        for (int j = 0; j < r; j++){
            const unsigned char *mul_table = gf_mul_table_base[cauchy_matrix[j][block_idx[i]]];
            for(int l = 0; l <data_sizes->at(i) / 64; l+= 64){
                gf_xor_mul_64(global_ptrs[j] + l + block_start[i], data_ptrs[i] + l, mul_table);
            }
            for(int l = data_sizes->at(i) / 64 * 64; l < data_sizes->at(i); l++){
                global_ptrs[j][l + block_start[i]] ^= mul_table[data_ptrs[i][l]];
            }
        }

        const unsigned char *mul_table_local = gf_mul_table_base[local_vector[block_idx[i]]];
        for(int j = 0; j < data_sizes->at(i) / 64; j+= 64){
            
            gf_xor_mul_64(local_ptrs[local_group] + block_start[i] + j, data_ptrs[i] + j, mul_table_local);
        }
        for(int j = data_sizes->at(i) / 64 * 64; j < data_sizes->at(i); j++){
            local_ptrs[local_group][j + block_start[i]] ^= mul_table_local[data_ptrs[i][j]];
        }
    }

    int group_size = (k + r) / z;
    int larger_group_num = (k + r) % z;
    int larger_group_block_start = group_size * (z - larger_group_num);
    for(int i = 0; i < z - larger_group_num; i++){
        for(int j = 0; j < group_size; j++){
            int data_idx = i * group_size + j;
            for(int l = 0; l < parity_size / 64; l+=64){
                gf_xor_64(local_ptrs[i] + l, data_ptrs[data_idx] + l);
            }
            for(int l = parity_size / 64 * 64; l < parity_size; l++){
                local_ptrs[i][l] ^= data_ptrs[data_idx][l];
            }
        }
    }
    group_size++;
    for(int i = z - larger_group_num; i < z - 1; i++){
        for(int j = 0; j < group_size; j++){
            int data_idx = larger_group_block_start + (i - z + larger_group_num) * group_size + j;
            for(int l = 0; l < parity_size / 64; l+=64){
                gf_xor_64(local_ptrs[i] + l, data_ptrs[data_idx] + l);
            }
            for(int l = parity_size / 64 * 64; l < parity_size; l++){
                local_ptrs[i][l] ^= data_ptrs[data_idx][l];
            }
        }
    }

    for(int i = 0; i < r; i++){
        for(int j = 0; j < parity_size / 64; j+=64){
            gf_xor_64(local_ptrs[z - 1] + j, global_ptrs[i] + j);
        }
        for(int j = parity_size / 64 * 64; j < parity_size; j++){
            local_ptrs[z - 1][j] ^= global_ptrs[i][j];
        }
    }

    int data_idx_last_group = k - (group_size - r);
    for(int i = 0; i < group_size - r; i++){
        for(int j = 0; j < parity_size / 64; j+=64){
            gf_xor_64(local_ptrs[z - 1] + j, data_ptrs[data_idx_last_group + i] + j);
        }
        for(int j = parity_size / 64 * 64; j < parity_size; j++){
            local_ptrs[z - 1][j] ^= data_ptrs[data_idx_last_group + i][j];
        }
    }



    for (int i = 0; i < r; i++)
    {
        delete[] cauchy_matrix[i];
    }
    delete[] cauchy_matrix;
}

void ECProject::encode_unilrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **parity_ptrs, int block_size)
{
    for(int i = 0; i < r + z; i++){
        memset(parity_ptrs[i], 0, block_size);
    }


    int m = k + r;
    unsigned char *encode_matrix = new unsigned char[(m + z) * k];
    memset(encode_matrix, 0, (m + z) * k);
    gf_gen_rs_matrix1(encode_matrix, m, k);
    for(int i = 0; i < k; i++){
        int row = i / (k / z);
        encode_matrix[(m + row) * k + i] = 1;
    }
    for(int i = 0; i < z; i++){
        for(int j = 0; j < k; j++){
            for(int l = 0; l < r / z; l++){
                encode_matrix[(m + i) * k + j] ^= encode_matrix[(k + i * r / z + l) * k + j];
            }
        }
    }

    unsigned char *g_tbls = new unsigned char[k * (r + z) * 32];
    ec_init_tables(k, r + z, &encode_matrix[k * k], g_tbls);
    ec_encode_data_avx2(block_size, k, r + z, g_tbls, data_ptrs, parity_ptrs);

    delete[] encode_matrix;
}

void ECProject::encode_azure_lrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **parity_ptrs, int block_size)
{
    for(int i = 0; i < r + z; i++){
        memset(parity_ptrs[i], 0, block_size);
    }
    int m = k + r;
    unsigned char *encode_matrix = new unsigned char[(m + z)* k];
    memset(encode_matrix, 0, (m + z) * k);
    gf_gen_rs_matrix1(encode_matrix, m, k);
    for(int i = 0; i < k; i++){
        int row = i / (k / z);
        encode_matrix[(m + row) * k + i] = 1;
    }

    unsigned char *g_tbls = new unsigned char[k * (r + z) * 32];
    ec_init_tables(k, r + z, &encode_matrix[k * k], g_tbls);
    ec_encode_data_avx2(block_size, k, r + z, g_tbls, data_ptrs, parity_ptrs);

    delete[] encode_matrix;
}

void ECProject::encode_optimal_lrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **parity_ptrs, int block_size)
{
    for(int i = 0; i < r + z; i++){
        memset(parity_ptrs[i], 0, block_size);
    }
    int m = k + r;
    unsigned char *encode_matrix = new unsigned char[(m + z) * k];
    memset(encode_matrix, 0, (m + z) * k);
    gf_gen_cauchy_matrix1(encode_matrix, m, k);

    unsigned char *local_vector = new unsigned char[k];
    gf_gen_local_vector(local_vector, k, r);
    for(int i = 0; i < k; i++){
        int row = i / (k / z);
        encode_matrix[(m + row) * k + i] = local_vector[i];
    }
    for(int i = 0; i < z; i++){
        for(int j = 0; j < k; j++){
            for(int l = 0; l < r; l++){
                encode_matrix[(m + i) * k + j] ^= encode_matrix[(k + l) * k + j];
            }
        }
    }

    unsigned char *g_tbls = new unsigned char[k * (r + z)* 32];
    ec_init_tables(k, r + z, &encode_matrix[k * k], g_tbls);
    ec_encode_data_avx2(block_size, k, r + z, g_tbls, data_ptrs, parity_ptrs);
    delete[] encode_matrix;
    delete[] local_vector;
}

void ECProject::encode_uniform_lrc(int k, int r, int z, unsigned char **data_ptrs, unsigned char **parity_ptrs, int block_size)
{
    for(int i = 0; i < r + z; i++){
        memset(parity_ptrs[i], 0, block_size);
    }
    int m = k + r;
    unsigned char *encode_matrix = new unsigned char[(m + z) * k];
    memset(encode_matrix, 0, (m + z) * k);
    gf_gen_cauchy_matrix1(encode_matrix, m, k);

    unsigned char *local_vector = new unsigned char[k];
    gf_gen_local_vector(local_vector, k, r);
    int group_size = (k + r) / z;
    int larger_group_num = (k + r) % z;
    int larger_group_block_start = group_size * (z - larger_group_num);
    for(int i = 0; i < larger_group_block_start; i++){
        int row = i / group_size;
        encode_matrix[(m + row) * k + i] = local_vector[i];
    }
    for(int i = larger_group_block_start; i < k; i++){
        int row = (i - larger_group_block_start) / (group_size + 1) + z - larger_group_num;
        encode_matrix[(m + row) * k + i] = local_vector[i];
    }
    for(int i = 0; i < r; i++){
        for(int j = 0; j < k; j++){
            encode_matrix[(m + z - 1) * k + j] ^= encode_matrix[(k + i) * k + j];
        }
    }

    unsigned char *g_tbls = new unsigned char[k * (r + z)* 32];
    ec_init_tables(k, r + z, &encode_matrix[k * k], g_tbls);
    ec_encode_data_avx2(block_size, k, r + z, g_tbls, data_ptrs, parity_ptrs);
    delete[] encode_matrix;
    delete[] local_vector;




    /*for(int i = 0; i < r; i++){
        memset(global_ptrs[i], 0, block_size);
    }
    for(int i = 0; i < z; i++){
        memset(local_ptrs[i], 0, block_size);
    }

    int m = k + r;
    unsigned char *encode_matrix = new unsigned char[m * k];
    gf_gen_cauchy_matrix1(encode_matrix, m, k);
    unsigned char *g_tbls = new unsigned char[k * r * 32];
    ec_init_tables(k, r, &encode_matrix[k * k], g_tbls);
    ec_encode_data_avx2(block_size, k, r, g_tbls, data_ptrs, global_ptrs);


    unsigned char *local_vector = new unsigned char[k];
    gf_gen_local_vector(local_vector, k, r);


    int local_group_size = (k + r) / z;
    int larger_local_group_num = (k + r) % z;
    int node_num_in_small_group = local_group_size * (z - larger_local_group_num); 
    for(int i = 0; i < z - larger_local_group_num; i++){
        for(int j = 0; j < local_group_size; j++){
            int data_idx = i * local_group_size + j;
            const unsigned char *mul_table = gf_mul_table_base[local_vector[data_idx]];
            #pragma GCC unroll(64)
            #pragma GCC omp simd
            for(int l = block_size; l < block_size; l++){
                local_ptrs[i][l] ^= mul_table[data_ptrs[data_idx][l]];
            }
        }
    }
    local_group_size++;
    for(int i = z - larger_local_group_num; i < z - 1; i++){
        for(int j = 0; j < local_group_size; j++){
            int data_idx = node_num_in_small_group + (i - z + larger_local_group_num) * local_group_size + j;
            const unsigned char *mul_table = gf_mul_table_base[local_vector[data_idx + j]];
            #pragma GCC unroll(64)
            #pragma GCC omp simd
            for(int l = block_size; l < block_size; l++){
                local_ptrs[i][l] ^= mul_table[data_ptrs[data_idx + j][l]];
            }
        }
    }
    for(int i = 0; i < r; i++){
        #pragma GCC unroll(64)
        #pragma GCC omp simd
        for(int j = block_size; j < block_size; j++){
            local_ptrs[z - 1][j] ^= global_ptrs[i][j];
        }
    }

    delete[] local_vector;

    delete[] encode_matrix;*/
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

    memset(parity_ptr, 0, block_size);

    int block_idx[k][block_size];
    for (int i = 0; i < block_size; i++)
    {
        for (int j = 0; j < k; j++)
        {
            block_idx[j][i] = (i / unit_size * k + j) * unit_size + i % unit_size;
        }
    }


    if (parity_block_id >= k && parity_block_id < k + r)
    {
        int parity_idx = parity_block_id - k;
        
        for(int i = 0; i < k; i++){
            const unsigned char *mul_table = gf_mul_table_base[rs_matrix[parity_idx][i]];
            for(int j = 0; j < block_size / 64; j+=64){
                gf_xor_mul_idx_64(parity_ptr + j, data_ptrs, block_idx[i] + j, mul_table);
            }
            for(int j = block_size / 64 * 64; j < block_size; j++){
                parity_ptr[j] ^= mul_table[data_ptrs[block_idx[j][i]]];
            }
        }
    }
    else
    {
        int local_group = parity_block_id - k - r;
        
        for(int i = 0; i < k; i++){
            for(int j = 0; j < r / z; j++){
                const unsigned char *mul_table = gf_mul_table_base[rs_matrix[j + local_group * (r / z)][i]];
                for(int l = 0; l < block_size / 64; l+=64){
                    gf_xor_mul_idx_64(parity_ptr + l, data_ptrs, block_idx[i] + l, mul_table);
                }
                for(int l = block_size / 32 * 32; l < block_size; l++){
                    parity_ptr[l] ^= mul_table[data_ptrs[block_idx[l][i]]];
                }
            }

        }


        for(int i = local_group * (r / z); i < (local_group + 1) * (r / z); i++){
            for(int j = 0; j < block_size / 64; j+=64){
                gf_xor_idx_64(parity_ptr + j, data_ptrs, block_idx[i] + j);
            }
            for(int j = block_size / 64 * 64; j < block_size; j++){
                parity_ptr[j] ^= data_ptrs[block_idx[j][i]];
            }
        }

    }

    for (int i = 0; i < r; i++)
    {
        delete[] rs_matrix[i];
    }
    delete[] rs_matrix;
}

void ECProject::decode_unilrc(const int k, const int r, const int z, const int block_num,
                              const std::vector<int> *block_indexes, unsigned char **block_ptrs, unsigned char *res_ptr, int block_size)
{
    for (int i = 0; i < block_num; i++)
    {
        /*for(int j = 0; j < block_size / 64 * 64; j+=64){
            gf_xor_64(res_ptr + j, block_ptrs[block_indexes->at(i)] + j);
        }*/
        for(int j = 0; j < block_size; j++){
            res_ptr[j] ^= block_ptrs[i][j];
        }
    }
    std::cout << "decode unilrc done" << std::endl;
    return;
}

void ECProject::decode_azure_lrc(const int k, const int r, const int z, const int block_num,
                                 const std::vector<int> *block_indexes, unsigned char **block_ptrs, unsigned char *res_ptr, int block_size,
                                 int failed_block_id)
{
    if (failed_block_id >= k && failed_block_id < k + r)
    {
        unsigned char **rs_matrix;
        rs_matrix = new unsigned char *[r];
        for (int i = 0; i < r; i++)
        {
            rs_matrix[i] = new unsigned char[k];
        }
        gf_gen_rs_matrix(rs_matrix, k + r, k);
        for (int i = 0; i < block_num; i++)
        {
            int block_index = block_indexes->at(i);
            const unsigned char *mul_table = gf_mul_table_base[rs_matrix[failed_block_id - k][block_index]];
            for(int j = 0; j < block_size / 64; j+=64){
                gf_xor_mul_64(res_ptr + j, block_ptrs[block_index] + j, mul_table);
            }
            for(int j = block_size / 64 * 64; j < block_size; j++){
                res_ptr[j] ^= mul_table[block_ptrs[block_index][j]];
            }
        }
        for (int i = 0; i < r; i++)
        {
            delete[] rs_matrix[i];
        }
        delete[] rs_matrix;
    }
    else
    {
        for (int i = 0; i < block_num; i++)
        {
            for(int j = 0; j < block_size / 64; j+=64){
                gf_xor_64(res_ptr + j, block_ptrs[block_indexes->at(i)] + j);
            }
            for(int j = block_size / 64 * 64; j < block_size; j++){
                res_ptr[j] ^= block_ptrs[block_indexes->at(i)][j];
            }
        }
    }
}

void ECProject::decode_optimal_lrc(const int k, const int r, const int z, const int block_num,
                                   const std::vector<int> *block_indexes, unsigned char **block_ptrs, unsigned char *res_ptr, int block_size)
{
    unsigned char *local_vector;
    local_vector = new unsigned char[k];
    gf_gen_local_vector(local_vector, k, r);
    for (int i = 0; i < block_num; i++)
    {
        int block_index = block_indexes->at(i);
        if (block_index < k)
        {
            const unsigned char *mul_table = gf_mul_table_base[local_vector[block_index]];
            /*for (int j = 0; j < block_size / 64; j+=64){
                gf_xor_mul_64(res_ptr + j, block_ptrs[block_index] + j, mul_table);
            }*/
            for(int j = 0; j < block_size; j++){
                res_ptr[j] ^= mul_table[block_ptrs[i][j]];
            }
        }

        else
        {
            /*for (int j = 0; j < block_size / 64; j+=64){
                gf_xor_64(res_ptr + j, block_ptrs[block_index] + j);
            }*/
            for(int j = 0; j < block_size; j++){
                res_ptr[j] ^= block_ptrs[i][j];
            }
        }
    }
    delete[] local_vector;
}
void ECProject::decode_uniform_lrc(const int k, const int r, const int z, const int block_num,
                                   const std::vector<int> *block_indexes, unsigned char **block_ptrs, unsigned char *res_ptr, int block_size)
{
    unsigned char *local_vector;
    local_vector = new unsigned char[k];
    gf_gen_local_vector(local_vector, k, r);
    for (int i = 0; i < block_num; i++)
    {
        int block_index = block_indexes->at(i);
        if (block_index < k)
        {
            const unsigned char *mul_table = gf_mul_table_base[local_vector[block_index]];
            /*for (int j = 0; j < block_size / 64; j+=64){
                gf_xor_mul_64(res_ptr + j, block_ptrs[block_index] + j, mul_table);
            }*/
            for(int j = 0; j < block_size; j++){
                res_ptr[j] ^= mul_table[block_ptrs[i][j]];
            }
        }

        else
        {
            /*for (int j = 0; j < block_size / 64; j+=64){
                gf_xor_64(res_ptr + j, block_ptrs[block_index] + j);
            }*/
            for(int j = 0; j < block_size; j++){
                res_ptr[j] ^= block_ptrs[i][j];
            }
        }
    }
    delete[] local_vector;
}

void
ECProject::ec_encode_data_avx2(int len, int k, int rows, unsigned char *g_tbls, unsigned char **data,
                    unsigned char **coding)
{

        if (len < 32) {
                ec_encode_data_base(len, k, rows, g_tbls, data, coding);
                return;
        }

        while (rows >= 6) {
                gf_6vect_dot_prod_avx2(len, k, g_tbls, data, coding);
                g_tbls += 6 * k * 32;
                coding += 6;
                rows -= 6;
        }
        switch (rows) {
        case 5:
                gf_5vect_dot_prod_avx2(len, k, g_tbls, data, coding);
                break;
        case 4:
                gf_4vect_dot_prod_avx2(len, k, g_tbls, data, coding);
                break;
        case 3:
                gf_3vect_dot_prod_avx2(len, k, g_tbls, data, coding);
                break;
        case 2:
                gf_2vect_dot_prod_avx2(len, k, g_tbls, data, coding);
                break;
        case 1:
                gf_vect_dot_prod_avx2(len, k, g_tbls, data, *coding);
                break;
        case 0:
                break;
        }
}

void
ECProject::ec_encode_data_base(int len, int srcs, int dests, unsigned char *v, unsigned char **src,
                    unsigned char **dest)
{
        int i, j, l;
        unsigned char s;

        for (l = 0; l < dests; l++) {
                for (i = 0; i < len; i++) {
                        s = 0;
                        for (j = 0; j < srcs; j++)
                                s ^= gf_mul(src[j][i], v[j * 32 + l * srcs * 32 + 1]);

                        dest[l][i] = s;
                }
        }
}

void
ECProject::ec_init_tables(int k, int rows, unsigned char *a, unsigned char *g_tbls)
{
        int i, j;

        for (i = 0; i < rows; i++) {
                for (j = 0; j < k; j++) {
                        gf_vect_mul_init(*a++, g_tbls);
                        g_tbls += 32;
                }
        }
}

void
ECProject::gf_vect_mul_init(unsigned char c, unsigned char *tbl)
{
        unsigned char c2 = (c << 1) ^ ((c & 0x80) ? 0x1d : 0);   // Mult by GF{2}
        unsigned char c4 = (c2 << 1) ^ ((c2 & 0x80) ? 0x1d : 0); // Mult by GF{2}
        unsigned char c8 = (c4 << 1) ^ ((c4 & 0x80) ? 0x1d : 0); // Mult by GF{2}

#if (__WORDSIZE == 64 || _WIN64 || __x86_64__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        unsigned long long v1, v2, v4, v8, *t;
        unsigned long long v10, v20, v40, v80;
        unsigned char c17, c18, c20, c24;

        t = (unsigned long long *) tbl;

        v1 = c * 0x0100010001000100ull;
        v2 = c2 * 0x0101000001010000ull;
        v4 = c4 * 0x0101010100000000ull;
        v8 = c8 * 0x0101010101010101ull;

        v4 = v1 ^ v2 ^ v4;
        t[0] = v4;
        t[1] = v8 ^ v4;

        c17 = (c8 << 1) ^ ((c8 & 0x80) ? 0x1d : 0);   // Mult by GF{2}
        c18 = (c17 << 1) ^ ((c17 & 0x80) ? 0x1d : 0); // Mult by GF{2}
        c20 = (c18 << 1) ^ ((c18 & 0x80) ? 0x1d : 0); // Mult by GF{2}
        c24 = (c20 << 1) ^ ((c20 & 0x80) ? 0x1d : 0); // Mult by GF{2}

        v10 = c17 * 0x0100010001000100ull;
        v20 = c18 * 0x0101000001010000ull;
        v40 = c20 * 0x0101010100000000ull;
        v80 = c24 * 0x0101010101010101ull;

        v40 = v10 ^ v20 ^ v40;
        t[2] = v40;
        t[3] = v80 ^ v40;

#else // 32-bit or other
        unsigned char c3, c5, c6, c7, c9, c10, c11, c12, c13, c14, c15;
        unsigned char c17, c18, c19, c20, c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31;

        c3 = c2 ^ c;
        c5 = c4 ^ c;
        c6 = c4 ^ c2;
        c7 = c4 ^ c3;

        c9 = c8 ^ c;
        c10 = c8 ^ c2;
        c11 = c8 ^ c3;
        c12 = c8 ^ c4;
        c13 = c8 ^ c5;
        c14 = c8 ^ c6;
        c15 = c8 ^ c7;

        tbl[0] = 0;
        tbl[1] = c;
        tbl[2] = c2;
        tbl[3] = c3;
        tbl[4] = c4;
        tbl[5] = c5;
        tbl[6] = c6;
        tbl[7] = c7;
        tbl[8] = c8;
        tbl[9] = c9;
        tbl[10] = c10;
        tbl[11] = c11;
        tbl[12] = c12;
        tbl[13] = c13;
        tbl[14] = c14;
        tbl[15] = c15;

        c17 = (c8 << 1) ^ ((c8 & 0x80) ? 0x1d : 0);   // Mult by GF{2}
        c18 = (c17 << 1) ^ ((c17 & 0x80) ? 0x1d : 0); // Mult by GF{2}
        c19 = c18 ^ c17;
        c20 = (c18 << 1) ^ ((c18 & 0x80) ? 0x1d : 0); // Mult by GF{2}
        c21 = c20 ^ c17;
        c22 = c20 ^ c18;
        c23 = c20 ^ c19;
        c24 = (c20 << 1) ^ ((c20 & 0x80) ? 0x1d : 0); // Mult by GF{2}
        c25 = c24 ^ c17;
        c26 = c24 ^ c18;
        c27 = c24 ^ c19;
        c28 = c24 ^ c20;
        c29 = c24 ^ c21;
        c30 = c24 ^ c22;
        c31 = c24 ^ c23;

        tbl[16] = 0;
        tbl[17] = c17;
        tbl[18] = c18;
        tbl[19] = c19;
        tbl[20] = c20;
        tbl[21] = c21;
        tbl[22] = c22;
        tbl[23] = c23;
        tbl[24] = c24;
        tbl[25] = c25;
        tbl[26] = c26;
        tbl[27] = c27;
        tbl[28] = c28;
        tbl[29] = c29;
        tbl[30] = c30;
        tbl[31] = c31;

#endif //__WORDSIZE == 64 || _WIN64 || __x86_64__
}