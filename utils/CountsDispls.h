//
//  CountsDispls.h
//  ScaPSRS
//
//  Created by Estella Wong on 2021/6/8.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef CountsDispls_h
#define CountsDispls_h

#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>

using namespace std;

template <class T1>
static T1 create_displs(T1 counts[], T1 displs[], int comm_sz) {
    T1 offset = (T1)0;
    for (int q = 0; q < comm_sz; q++) {
        displs[q] = offset;
        if (counts[q]) offset += counts[q];
    }
    return offset; //return sum
};

template <class T1>
static void create_counts(T1 counts[], T1 displs[], int comm_sz, T1 sum) {
    for (int q = 0; q < comm_sz - 1; q++) {
        counts[q] = displs[q+1] - displs[q];
        if (counts[q] < 0) counts[q] = 0;
    }
    counts[comm_sz-1] = sum - displs[comm_sz-1];
    if (counts[comm_sz-1] < 0) {
        counts[comm_sz-1] = 0;
        displs[comm_sz-1] = sum;
    }
};

template <class T1>
static void init_counts(T1 counts[], T1 sum, int comm_sz) {
   T1 offset, q, quotient, remainder;

   quotient = sum / comm_sz;
   remainder = sum % comm_sz;
   offset = 0;
   for (q = 0; q < comm_sz; q++) {
      if (q < remainder)
         counts[q] = quotient+1;
      else
         counts[q] = quotient;
      offset += counts[q];
   }
};

template <class T1>
static void init_displs(T1 displs[], T1 sum, int comm_sz) {
   T1 offset, q, quotient, remainder;

   quotient = sum / comm_sz;
   remainder = sum % comm_sz;
   offset = 0;
   for (q = 0; q < comm_sz; q++) {
      if (q < remainder)
         offset += quotient+1;
      else
        offset += quotient;
      displs[q] = offset;
   }
}; 

template <class T1>
static void init_counts_displs(T1 counts[], T1 displs[], T1 sum, int comm_sz) {
   T1 offset, q, quotient, remainder;

   quotient = sum / comm_sz;
   remainder = sum % comm_sz;
   offset = 0;
   for (q = 0; q < comm_sz; q++) {
      if (q < remainder)
         counts[q] = quotient+1;
      else
         counts[q] = quotient;
      displs[q] = offset;
      offset += counts[q];
   }
};  /* init_counts_displs */

template <class T1>
static void init_counts_displs(T1 counts[], T1 displs[], T1 sum, int offset, int comm_sz) {
   T1 q, quotient, remainder;

   quotient = sum / comm_sz;
   remainder = sum % comm_sz;
   for (q = 0; q < comm_sz; q++) {
      if (q < remainder)
         counts[q] = quotient+1;
      else
         counts[q] = quotient;
      displs[q] = offset;
      offset += counts[q];
   }
};  /* init_counts_displs */

template <class T1>
static void init_counts_displs_with_dim(T1 counts[], T1 displs[], int dim, T1 sum, int comm_sz) {
   T1 offset, q, quotient, remainder;

   quotient = sum / comm_sz;
   remainder = sum % comm_sz;
   offset = 0;
   for (q = 0; q < comm_sz; q++) {
      if (q < remainder)
         counts[q] = (quotient+1) * dim;
      else
         counts[q] = quotient * dim;
      displs[q] = offset;
      offset += counts[q];
   }
};  /* init_counts_displs_with_dim */

template <class T1, class T2>
static void create_counts_displs(int counts[], int displs[], T1 samples[], T2 min_sample[], int samples_len, int comm_sz) {
    int r = 0; //rankID
    int i = 0; //the index in samples
    for (; i < samples_len; i++) {
        if (r >= comm_sz) break;
        if (r + 1 < comm_sz) {
            if (samples[i] >= min_sample[r+1]) {
                i--;
                r++;
                continue;
            }
        }
        if (samples[i] >= min_sample[r]) counts[r]++;
    }
    create_displs(counts, displs, comm_sz);
};
template <class T1, class T2>
static void create_counts_displs(int counts[], int displs[], vector<pair<T1,T1>>& samples, T2 min_sample[], int samples_len, int comm_sz) {
    int r = 0; //rankID
    int i = 0; //the index in samples
    for (; i < samples_len; i++) {
        if (r >= comm_sz) break;
        if (r + 1 < comm_sz) {
            if (samples[i].first >= min_sample[r+1]) {
                i--;
                r++;
                continue;
            }
        }
        if (samples[i].first >= min_sample[r]) counts[r] += 2;
    }
    create_displs(counts, displs, comm_sz);
};

template <class T1, class T2>
static void create_counts_displs(T1 counts[], T1 displs[], T2 samples[], T2 min_sample[], T2 max_sample[], int samples_len, int comm_sz) {
    int r = 0; //rankID
    int i = 0; //the index in samples
    for (; i < samples_len; i++) {
        if (r >= comm_sz) {
            r = comm_sz - 1;
            counts[r]++;
            continue;
        }
        if (r < 0) {
            r = 0;
            counts[r]++;
            continue;
        }
        if (r + 1 < comm_sz) {
            if ((samples[i] > max_sample[r]) && (samples[i] < min_sample[r+1])) {
                counts[r]++;
                continue;
            }
        }
        if (samples[i] > max_sample[r]) {
            i--;
            r++;
            continue;
        }
        else if (samples[i] < min_sample[r]) {
            i--;
            r--;
            continue;
        }
        else counts[r]++;
        
    }
    create_displs(counts, displs, comm_sz);
};
template <class T1, class T2>
static void create_counts_displs(T1 counts[], T1 displs[], vector<pair<T2,T2>>& samples, T2 min_sample[], T2 max_sample[], int samples_len, int comm_sz) {
    int r = 0; //rankID
    int i = 0; //the index in samples
    for (; i < samples_len; i++) {
        if (r >= comm_sz) {
            r = comm_sz - 1;
            counts[r] += 2;
            continue;
        }
        if (r < 0) {
            r = 0;
            counts[r] += 2;
            continue;
        }
        if (r + 1 < comm_sz) {
            if ((samples[i].first > max_sample[r]) && (samples[i].first < min_sample[r+1])) {
                counts[r] += 2;
                continue;
            }
        }
        if (samples[i].first > max_sample[r]) {
            i--;
            r++;
            continue;
        }
        else if (samples[i].first < min_sample[r]) {
            i--;
            r--;
            continue;
        }
        else counts[r] += 2;
        
    }
    create_displs(counts, displs, comm_sz);
};

/*---------------------------------------*/
/* kmer<->myrank */
static void init_task_rank(int* rank_t, int* task_r, int comm_sz) {
    if (comm_sz == 2) {
        task_r[2] = 1; task_r[3] = 1;
        rank_t[1] = 2;
    }
    else if (comm_sz == 3) {
        task_r[2] = 1; task_r[3] = 2;
        rank_t[1] = 2; rank_t[2] = 3;
    }
    else if (comm_sz >= 4) {
        int quotient = comm_sz / 4;
        int remainder = comm_sz % 4;
        int offset = 0;
        for (int i = 0; i < 4; i++) {
            task_r[i] = offset;
            if (i < remainder) offset += quotient+1;
            else offset += quotient;
            for (int j = task_r[i]; j < offset; j++) {
                rank_t[j] = i;
            }
        }
    }
};

    /// create color_t[], key_t[] by comm_sz and my_rank.
    /// (to MPI_Comm_split) partitions the group(comm_sz processes) into disjoint subgroup, one for each value of color
    /// @param color_t Each subgroup contains all processes of the same color
    /// @param key_t In each subgroup, the processes are ranked in the order defined by the value of key
    /// @param comm_sz the number of processes in the old group
    /// @param group_num the number of subgroup
static void init_color_key(int* color_t, int* key_t, int comm_sz, int group_num) {
    group_num = group_num > comm_sz ? comm_sz : group_num; //以确保group_num有效，<=comm_sz
    int quotient = comm_sz / group_num;
    int remainder = comm_sz % group_num;
    int offset = 0;
    for (int i = 0; i < group_num; i++) {
        for (int j = offset; j < offset + quotient; j++) {
            color_t[j] = i;
            key_t[j] = j - offset;
        }
        offset += quotient;
        if (i < remainder) {
            color_t[offset] = i;
            key_t[offset] = quotient;
            offset++;
        }
    }
};

    /// create master_t[], color_, key_ just by comm_sz and my_rank
    /// @param master_t the rank id which is master in each subgroup
    /// @param color_ the color in subgroup
    /// @param key_ the key in subgroup
    /// @param comm_sz the number of processes in the old group
    /// @param my_rank the rank in the old group
static void init_master_color_key(int* master_t, int& color_, int& key_, int comm_sz, int my_rank) {
    unsigned AlphabetSize = 4;
    color_ = 0;
    key_ = 0;

    if (comm_sz == 2) {
        master_t[2] = 1; master_t[3] = 1;
        if (my_rank == 1) color_ = 2;
    }
    else if (comm_sz == 3) {
        master_t[2] = 1; master_t[3] = 2;
        if (my_rank == 1) color_ = 2;
        else if (my_rank == 2) color_ = 3;
    }
    else if (comm_sz >= AlphabetSize) {
        int quotient = comm_sz / AlphabetSize;
        int remainder = comm_sz % AlphabetSize;
        int offset = 0;
        for (int i = 0; i < AlphabetSize; i++) {
            master_t[i] = offset;
            if (i < remainder) offset += quotient+1;
            else offset += quotient;
            if (my_rank >= master_t[i] && my_rank < offset) {
                color_ = i;
                key_ = my_rank - master_t[i];
            }
        }
    }
};

/*---------------------------------------*/
static void Init_counts_alltoallv(int bufsize[], int new_bufsize[], int sendcounts[], int recvcounts[], int comm_sz, int my_rank, int nlen) {
    int quotient, remainder, before_size;
    int *color_, *key_, *bufsize_tmp;
    color_ = new int[comm_sz];
    key_ = new int[comm_sz];
    bufsize_tmp = new int[comm_sz];
    init_color_key(color_, key_, comm_sz, 4);
    int m = pow(4, nlen-1);
        //初始化group_size
    int group_size[4]{0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < m; j++) {
            group_size[i] += bufsize[i*m+j];
        }
    }
        //初始化new_bufsize
    int before_group_num = 0;
    for (int i = 0; i < 4; i++) {
        int group_num = (int)count(color_, color_+comm_sz, i);
        quotient = group_size[i] / group_num;
        remainder = group_size[i] % group_num;
        for (int j = 0; j < group_num; j++) {
            if (j < remainder) {
                new_bufsize[before_group_num+j] = quotient + 1;
                bufsize_tmp[before_group_num+j] = quotient + 1;
            }
            else {
                new_bufsize[before_group_num+j] = quotient;
                bufsize_tmp[before_group_num+j] = quotient;
            }
        }
        before_group_num += group_num;
    }
        //初始化sendcounts和recvcounts
    int j = 0;
    before_size = 0;
    for (int i = 0; i < comm_sz; i++) {
        before_size += bufsize[i];
        while (bufsize_tmp[j] <= before_size) {
            if (i==my_rank) sendcounts[j] = bufsize_tmp[j];
            if (j==my_rank) recvcounts[i] = bufsize_tmp[j];
            before_size -= bufsize_tmp[j];
            bufsize_tmp[j] = 0;
            if (j+1<comm_sz) j++;
            else break;
        }
        if (bufsize_tmp[j] > before_size) {
            if (i==my_rank) sendcounts[j] = before_size;
            if (j==my_rank) recvcounts[i] = before_size;
            bufsize_tmp[j] -= before_size;
            before_size = 0;
        }
    }
};


#endif /* CountsDispls_h */
