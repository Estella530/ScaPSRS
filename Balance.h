//
//  Balance.h
//  ScaPSRS
//
//  Created by Estella Wong on 2021/6/8.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef Balance_h
#define Balance_h

#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>

#include "utils/CountsDispls.h"
#include "utils/Log.hpp"
#include "utils/mpi_utils.h"
#include "utils/utils.h"

using namespace std;

    ///数据负载不均衡，通过 send_counts 来一次 alltoallv 交换数据，来实现数据负载均衡
    ///@param my_ori_num the number of data in my_rank
    ///@param my_ori_displs the number of data before my_rank
    ///@param sum the total number of data of all ranks
    ///@param comm_sz the number of processes
    ///@param send_counts output
template <class T1>
static void balance_sendcounts(uint64_t my_ori_num, uint64_t my_ori_displs, uint64_t sum, int comm_sz, T1 send_counts[]) {
    uint64_t* new_counts = new uint64_t[comm_sz]{0};
    init_counts(new_counts, sum, comm_sz);
    uint64_t my_ori_last = my_ori_displs + my_ori_num;
    uint64_t my_send = 0;
    uint64_t acc = 0;
    for (int i = 0; i < comm_sz; i++) {
        acc += new_counts[i];
        if (acc > my_ori_displs) {
            my_send = acc - my_ori_displs;
            if (my_send > my_ori_num) my_send = my_ori_num;
            my_ori_num -= my_send;
            my_ori_displs = acc;
            send_counts[i] += (T1)my_send;
            if (send_counts[i] < 0) send_counts[i] = 0;
        }
        if (acc > my_ori_last) break;
    }
    delete [] new_counts;
};

template <class T1, class T2>
static void check_senddispls(int comm_sz, uint64_t bufsize, vector<pair<T1,T1>>& bufdata, T2 send_counts[], T2 send_displs[]) {
    T1 prev, now;
    int prev_rank = 0;
    for (int i = 0; i < comm_sz; i++) {
        if (send_counts[i] > 0) {
            if (send_displs[i] == 0) {
                prev_rank = i;
                continue;
            }
            uint64_t offset = send_displs[i];
            prev = bufdata.at(offset - 1).first;
            prev |= 0xf;
            now = bufdata.at(offset).first;
            while (prev >= now) {
                Log::Debug("[CheckDispls] i:", i, "offset:", offset, "prev:", prev, "now:", now);
                send_displs[i]++;
                send_counts[prev_rank]++;
                send_counts[i]--;
                if (send_counts[i] == 0) break;
                offset++;
                if (offset >= bufsize) break;
                now = bufdata.at(offset).first;
            }
            prev_rank = i;
        }
    }
};
template <class T1, class T2>
static void check_senddispls(int comm_sz, uint64_t bufsize, T1 bufdata[], T2 send_counts[], T2 send_displs[]) {
    T1 prev, now;
    int prev_rank = 0;
    for (int i = 0; i < comm_sz; i++) {
        if (send_counts[i] > 0) {
            if (send_displs[i] == 0) {
                prev_rank = i;
                continue;
            }
            uint64_t offset = send_displs[i];
            prev = bufdata[offset - 1];
            prev |= 0xf;
            now = bufdata[offset];
            while (prev >= now) {
                send_displs[i]++;
                send_counts[prev_rank]++;
                send_counts[i]--;
                if (send_counts[i] == 0) break;
                offset++;
                if (offset >= bufsize) break;
                now = bufdata[offset];
            }
            prev_rank = i;
        }
    }
};

template <class T1>
static void DataBalance(vector<T1>& ori_data, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* ori_counts = new int[comm_sz]{0};
    int my_ori_num = ori_data.size();
    MPI_Allgather(&my_ori_num, 1, MPI_INT, ori_counts, 1, MPI_INT, COMM_SHARED);
    StaticsNum(my_rank, comm_sz, ori_counts, "beforeDataBalance");
    uint64_t my_ori_displs = accumulate(ori_counts, ori_counts + my_rank, 0);
    uint64_t sum = my_ori_displs + accumulate(ori_counts + my_rank, ori_counts + comm_sz, 0);
    int* bcounts = new int[comm_sz]{0}; //to_be_balance_sendcounts
    int* bdispls = new int[comm_sz]{0}; //to_be_balance_sendcounts
    balance_sendcounts(my_ori_num, my_ori_displs, sum, comm_sz, bcounts);
    create_displs(bcounts, bdispls, comm_sz);
    check_senddispls(comm_sz, ori_data.size(), ori_data.data(), bcounts, bdispls);
    AlltoallvMsg(ori_data, bcounts, bdispls, COMM_SHARED);
    StaticsNum(ori_data.size(), "afterDataBalance");
};


#endif