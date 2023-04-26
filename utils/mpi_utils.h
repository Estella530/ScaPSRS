//
//  mpi_utils.h
//  ScaPSRS
//
//  Created by Estella Wong on 2022/7/7.
//  Copyright © 2022 Estella Wong. All rights reserved.
//

#ifndef mpi_utils_h
#define mpi_utils_h

#pragma once
#include <string.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <typeinfo>
#include "mpi.h"
#include "utils.h"
#include "Log.hpp"
#include "CountsDispls.h"

template <typename T1>
static void StaticsNum(int my_rank, vector<T1>& recv_num, string title="", bool print_to_screen = false, bool print_detail = false) {
    if (my_rank != 0) return;
    if (print_to_screen) cout << "---------- StaticsNum(" << title << ") ----------" << endl;
    Log::Info("---------- StaticsNum(", title, ") ----------");
    auto max_id = max_element(recv_num.begin(), recv_num.end());
    auto max_n = *max_id;
    auto min_id = min_element(recv_num.begin(), recv_num.end());
    auto min_n = *min_id;
    if (print_to_screen) cout << "max:" << max_n << ", at:" << max_id - recv_num.begin();
    if (print_to_screen) cout << " min:" << min_n << ", at:" << min_id - recv_num.begin();
    Log::Info("max:", max_n, ", at:", max_id - recv_num.begin());
    Log::Info("min:", min_n, ", at:", min_id - recv_num.begin());
    double rate = 0.0;
    if (min_n != 0) {
        rate = (double)max_n/(double)min_n;
        if (print_to_screen) cout << " max/min:" << rate;
        Log::Info("max/min:", rate);
    }
    else {
        if (print_to_screen) cout << " max/min: infinite";
        Log::Info("max/min:", "infinite");
    }
    if (print_to_screen) cout << endl;
    if (print_detail) {
        Log::Info("[StaticsNum] Detail of", title.c_str(), ":");
        Log::Vector1Detail(recv_num);
        if (print_to_screen) cout << "--------------------" << endl;
        Log::Line();
    }
};
template <typename T1>
static void StaticsNum(int my_rank, int comm_sz, T1* recv_num, string title="", bool print_to_screen = false, bool print_detail = false) {
    if (my_rank != 0) return;
    if (print_to_screen) cout << "---------- StaticsNum(" << title << ") ----------" << endl;
    Log::Info("---------- StaticsNum(", title, ") ----------");
    auto max_id = max_element(recv_num, recv_num + comm_sz);
    auto max_n = *max_id;
    auto min_id = min_element(recv_num, recv_num + comm_sz);
    auto min_n = *min_id;
    if (print_to_screen) cout << "max:" << max_n << ", at:" << max_id - recv_num;
    if (print_to_screen) cout << " min:" << min_n << ", at:" << min_id - recv_num;
    Log::Info("max:", max_n, ", at:", max_id - recv_num);
    Log::Info("min:", min_n, ", at:", min_id - recv_num);
    double rate = 0.0;
    if (min_n != 0) {
        rate = (double)max_n/(double)min_n;
        if (print_to_screen) cout << " max/min:" << rate;
        Log::Info("max/min:", rate);
    }
    else {
        if (print_to_screen) cout << " max/min: infinite";
        Log::Info("max/min:", "infinite");
    }
    if (print_to_screen) cout << endl;
    if (print_detail) {
        Log::Info("[StaticsNum] Detail of", title.c_str(), ":");
        Log::Array1Detail(comm_sz, recv_num);
        if (print_to_screen) cout << "--------------------" << endl;
        Log::Line();
    }
};
static void StaticsNum(int num, string title="", bool print_to_screen = false, bool print_detail = false) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    vector<int> recv_num;
    if (my_rank == 0) recv_num.resize(comm_sz);
    MPI_Gather(&num, 1, MPI_INT, recv_num.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    StaticsNum(my_rank, recv_num, title, print_to_screen, print_detail);
};


// if (typeid(a) == typeid(int))
template <typename T>
static MPI_Datatype GetMpiDataType(T data) {
    MPI_Datatype mpitype;
    if (typeid(T) == typeid(char)) mpitype = MPI_CHAR;
    else if (typeid(T) == typeid(int)) mpitype = MPI_INT;
    else if (typeid(T) == typeid(long)) mpitype = MPI_LONG;
    else if (typeid(T) == typeid(uint8_t)) mpitype = MPI_UINT8_T;
    else if (typeid(T) == typeid(uint32_t)) mpitype = MPI_UINT32_T;
    else if (typeid(T) == typeid(uint64_t)) mpitype = MPI_UINT64_T;
    return mpitype;
};
template <typename T>
static MPI_Datatype GetMpiDataType(vector<T>& data) {
    MPI_Datatype mpitype;
    if (typeid(T) == typeid(char)) mpitype = MPI_CHAR;
    else if (typeid(T) == typeid(int)) mpitype = MPI_INT;
    else if (typeid(T) == typeid(long)) mpitype = MPI_LONG;
    else if (typeid(T) == typeid(uint8_t)) mpitype = MPI_UINT8_T;
    else if (typeid(T) == typeid(uint32_t)) mpitype = MPI_UINT32_T;
    else if (typeid(T) == typeid(uint64_t)) mpitype = MPI_UINT64_T;
    return mpitype;
};

/*---------------------------------------*/
/* mpi - vector */
// MPI_Gatherv
template <typename T>
static int GathervMsg(vector<T>& send_msg, int send_size, vector<T>& recv_msg, int recv_size, int* rcounts, int* rdispls, int root_rank, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    recv_msg.resize(recv_size);
    MPI_Gatherv(send_msg.data(), send_size, mpitype, recv_msg.data(), rcounts, rdispls, mpitype, root_rank, COMM_SHARED);
    return recv_size;
};
template <typename T>
static int GathervMsg(vector<T>& send_msg, vector<T>& recv_msg, int root_rank, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int send_size = send_msg.size();
    int* rcounts = new int[comm_sz]{0};
    int* rdispls = new int[comm_sz]{0};
    MPI_Gather(&send_size, 1, MPI_INT, rcounts, 1, MPI_INT, root_rank, COMM_SHARED);
    int recv_size = create_displs(rcounts, rdispls, comm_sz);
    GathervMsg(send_msg, send_size, recv_msg, recv_size, rcounts, rdispls, root_rank, COMM_SHARED);
    delete [] rcounts;
    delete [] rdispls;
    return recv_size;
};
// MPI_Allgather
template <typename T>
static int AllGatherMsg(T send_msg, vector<T>& recv_msg, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    recv_msg.resize(comm_sz);
    MPI_Allgather(&send_msg, 1, mpitype, recv_msg.data(), 1, mpitype, COMM_SHARED);

};
template <typename T>
static int AllGatherMsg(vector<T>& send_msg, int send_size, vector<T>& recv_msg, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    int recv_size = send_size * comm_sz;
    recv_msg.resize(recv_size);
    MPI_Allgather(&send_msg, send_size, mpitype, recv_msg.data(), send_size, mpitype, COMM_SHARED);
    return recv_size;
};
// MPI_Allgatherv
template <typename T>
static int AllGathervMsg(vector<T>& send_msg, int send_size, vector<T>& recv_msg, int recv_size, int* rcounts, int* rdispls, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    recv_msg.resize(recv_size);
    MPI_Allgatherv(send_msg.data(), send_size, mpitype, recv_msg.data(), rcounts, rdispls, mpitype, COMM_SHARED);
    return recv_size;
};
template <typename T>
static int AllGathervMsg(vector<T>& send_msg, vector<T>& recv_msg, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int send_size = send_msg.size();
    int* rcounts = new int[comm_sz]{0};
    int* rdispls = new int[comm_sz]{0};
    MPI_Allgather(&send_size, 1, MPI_INT, rcounts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(rcounts, rdispls, comm_sz);
    AllGathervMsg(send_msg, send_size, recv_msg, recv_size, rcounts, rdispls, COMM_SHARED);
    delete [] rcounts;
    delete [] rdispls;
    return recv_size;
};

// MPI_Scatterv
template <typename T>
static int ScattervMsg(vector<T>& send_msg, int* scounts, int* sdispls, vector<T>& recv_msg, int recv_size, int root_rank, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    MPI_Scatterv(send_msg.data(), scounts, sdispls, mpitype, recv_msg.data(), recv_size, mpitype, root_rank, COMM_SHARED);
    return recv_size;
};
template <typename T>
static int ScattervMsg(vector<T>& send_msg, int* scounts, vector<T>& recv_msg, int root_rank, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* sdispls = new int[comm_sz]{0};
    int recv_size = 0;
    create_displs(scounts, sdispls, comm_sz);
    MPI_Scatter(scounts, 1, MPI_INT, &recv_size, 1, MPI_INT, 0, COMM_SHARED);
    recv_msg.resize(recv_size);
    ScattervMsg(send_msg, scounts, sdispls, recv_msg, recv_size, root_rank, COMM_SHARED);
    delete [] sdispls;
    return recv_size;
};

// MPI_Reduce
template <typename T>
static int ReduceMsg(vector<T>& send_msg, vector<T>& recv_msg, MPI_Op mpiop, int root_rank, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    int send_size = send_msg.size();
    recv_msg.resize(send_size);
    MPI_Reduce(send_msg.data(), recv_msg.data(), send_size, mpitype, mpiop, root_rank, COMM_SHARED);
    return send_size;
};

// MPI_Allreduce
template <typename T>
static int AllReduceMsg(vector<T>& send_msg, vector<T>& recv_msg, MPI_Op mpiop, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    int send_size = send_msg.size();
    recv_msg.resize(send_size);
    MPI_Allreduce(send_msg.data(), recv_msg.data(), send_size, mpitype, mpiop, COMM_SHARED);
    return send_size;
};

// MPI_Alltoall
template <typename T>
static int AlltoallMsg(vector<T>& send_msg, int send_count, vector<T>& recv_msg, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    int send_size = send_msg.size();
    recv_msg.resize(send_size);
    MPI_Alltoall(send_msg.data(), send_count, mpitype, recv_msg.data(), send_count, mpitype, COMM_SHARED);
    return send_size;
};
template <typename T>
static int AlltoallMsg(vector<T>& send_msg, int send_count, MPI_Comm COMM_SHARED) {
    vector<T> recv_msg;
    int send_size = AlltoallMsg(send_msg, send_count, recv_msg, COMM_SHARED);
    send_msg.swap(recv_msg);
    // FreeUpCompletely(recv_msg);
    return send_size;
};

// MPI_Alltoallv
template <typename T>
static int AlltoallvMsg(vector<T>& send_msg, int* scounts, int* sdispls, vector<T>& recv_msg, int recv_size, int* rcounts, int* rdispls, MPI_Comm COMM_SHARED) {
    MPI_Datatype mpitype = GetMpiDataType(send_msg);
    recv_msg.resize(recv_size);
    MPI_Alltoallv(send_msg.data(), scounts, sdispls, mpitype, recv_msg.data(), rcounts, rdispls, mpitype, COMM_SHARED);
    return recv_size;
};
template <typename T>
static int AlltoallvMsg(vector<T>& send_msg, int* scounts, int* sdispls, vector<T>& recv_msg, int* rcounts, int* rdispls, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int recv_size = rdispls[comm_sz-1] + rcounts[comm_sz-1];
    AlltoallvMsg(send_msg, scounts, sdispls, recv_msg, recv_size, rcounts, rdispls, COMM_SHARED);
    return recv_size;
};
template <typename T>
static int AlltoallvMsg(vector<T>& send_msg, int* scounts, int* sdispls, vector<T>& recv_msg, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* rcounts = new int[comm_sz]{0};
    int* rdispls = new int[comm_sz]{0};
    MPI_Alltoall(scounts, 1, MPI_INT, rcounts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(rcounts, rdispls, comm_sz);
    AlltoallvMsg(send_msg, scounts, sdispls, recv_msg, recv_size, rcounts, rdispls, COMM_SHARED);
    delete [] rcounts;
    delete [] rdispls;
    return recv_size;
};
template <typename T>
static int AlltoallvMsg(vector<T>& send_msg, int* scounts, int* sdispls, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    vector<T> recv_msg;
    int recv_size = AlltoallvMsg(send_msg, scounts, sdispls, recv_msg, COMM_SHARED);
    send_msg.swap(recv_msg);
    // FreeUpCompletely(recv_msg);
    return recv_size;
};
template <typename T>
static int AlltoallvMsg(vector<T>& send_msg, int* sdispls, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* scounts = new int[comm_sz]{0};
    create_counts(scounts, sdispls, comm_sz, (int)send_msg.size());
    int recv_size = AlltoallvMsg(send_msg, scounts, sdispls, COMM_SHARED);
    delete [] scounts;
    return recv_size;
};


#endif /* mpi_utils_h */
