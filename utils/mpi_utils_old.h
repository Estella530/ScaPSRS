//
//  mpi_utils.h
//  Exassembler
//
//  Created by Estella Wong on 2022/7/7.
//  Copyright © 2022 Estella Wong. All rights reserved.
//

#ifndef mpi_utils_old_h
#define mpi_utilsold__h

#pragma once
#include <string.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <map>
#include "mpi.h"
#include "Log.hpp"


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

//Memory
    /// 实时获取当前进程占用的内存，单位：kb
static int physical_memory_used_by_process() {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];
    while (fgets(line, 128, file) != nullptr) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            int len = strlen(line);
            const char* p = line;
            for (; std::isdigit(*p) == false; ++p) {}
            line[len - 3] = 0;
            result = atoi(p);
            break;
        }
    }
    fclose(file);
    return result;
};

static int GetMemory(int masterId = 0, string title="", bool print_detail = false) {
    int myid;
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    int mem = 0;
    if (myid == masterId) {
        mem = physical_memory_used_by_process();
        Log::Info("Memory of", title.c_str(), "used by process", myid, "is", mem, "KB.");
    }
    return mem;
};
static int GetMemory(int myid, int masterId = 0, string title="", bool print_detail = false) {
    int mem = 0;
    if (myid == masterId) {
        mem = physical_memory_used_by_process();
        Log::Info("Memory of", title.c_str(), "used by process", myid, "is", mem, "KB.");
    }
    return mem;
};

static int PrintMemory(int myid, int comm_sz, string title="") {
    int mem = 0;
    if (myid == 0 || myid == comm_sz-1) {
        mem = physical_memory_used_by_process();
        Log::Info("Memory of", title.c_str(), "used by process", myid, "is", mem, "KB.");
    }
    return mem;
};


/*---------------------------------------*/
/* mpi */
/* Scatterv */
static int ScattervMessage(vector<char> messages, int* send_counts, vector<char>& recv_messages, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int recv_size = 0;
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Scatter(send_counts, 1, MPI_INT, &recv_size, 1, MPI_INT, 0, COMM_SHARED);
    recv_messages.resize(recv_size);
    MPI_Scatterv(messages.data(), send_counts, send_displs, MPI_CHAR, recv_messages.data(), recv_size, MPI_CHAR, 0, COMM_SHARED);
    
    return recv_size;
};

/* Gatherv */
static int GathervMessage(vector<int> messages, vector<int>& recv_messages, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    int send_size = messages.size();
    MPI_Gather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Gatherv(messages.data(), send_size, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, 0, COMM_SHARED);
    
    return recv_size;
};
static int GathervMessage(vector<uint64_t> messages, vector<uint64_t>& recv_messages, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    int send_size = messages.size();
    MPI_Gather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Gatherv(messages.data(), send_size, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, 0, COMM_SHARED);
    
    return recv_size;
};
static int AllGathervMessage(vector<int> messages, vector<int>& recv_messages, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    int send_size = messages.size();
    MPI_Allgather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Allgatherv(messages.data(), send_size, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, COMM_SHARED);
    
    return recv_size;
};
static int AllGathervMessage(vector<uint32_t> messages, vector<uint32_t>& recv_messages, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    int send_size = messages.size();
    MPI_Allgather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Allgatherv(messages.data(), send_size, MPI_UINT32_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT32_T, COMM_SHARED);
    
    return recv_size;
};
static int AllGathervMessage(vector<uint64_t> messages, vector<uint64_t>& recv_messages, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    int send_size = messages.size();
    MPI_Allgather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Allgatherv(messages.data(), send_size, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    
    return recv_size;
};
static int AllGathervMessage(vector<uint64_t> messages, vector<uint64_t>& recv_messages, int* recv_counts, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    
    int* recv_displs = new int[comm_sz]{0};
    int send_size = messages.size();
    MPI_Allgather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Allgatherv(messages.data(), send_size, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    
    return recv_size;
};
static int AllGathervMessage(vector<uint64_t> messages, vector<uint64_t>& recv_messages, int* recv_counts, int* recv_displs, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);

    int send_size = messages.size();
    MPI_Allgather(&send_size, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    MPI_Allgatherv(messages.data(), send_size, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    
    return recv_size;
};

/* Alltoallv */
/* send_msg, send_counts, recv_msg */
static int AlltoallvMessage(vector<uint8_t> messages, int* send_counts, vector<uint8_t>& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT8_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT8_T, MPI_COMM_WORLD);
    
    return recv_size;
};
static int AlltoallvMessage(vector<int> messages, int* send_counts, vector<int>& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    
    return recv_size;
};
static int AlltoallvMessage(vector<uint32_t> messages, int* send_counts, vector<uint32_t>& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT32_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT32_T, MPI_COMM_WORLD);
    
    return recv_size;
};
static int AlltoallvMessage(vector<uint64_t> messages, int* send_counts, vector<uint64_t>& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    
    return recv_size;
};
static int AlltoallvMessage(vector< pair<uint64_t, uint64_t> > messages, int* send_counts, vector< pair<uint64_t, uint64_t> >& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_size /= 2;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg, send_counts, recv_msg, recv_counts */
static int AlltoallvMessage(vector<int> messages, int* send_counts, vector<int>& recv_messages, int* recv_counts) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    
    return recv_size;
};
static int AlltoallvMessage(vector<uint32_t> messages, int* send_counts, vector<uint32_t>& recv_messages, int* recv_counts) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT32_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT32_T, MPI_COMM_WORLD);
    
    return recv_size;
};
static int AlltoallvMessage(vector<uint64_t> messages, int* send_counts, vector<uint64_t>& recv_messages, int* recv_counts) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    
    return recv_size;
};

/* send_msg, send_counts, send_displs, recv_msg */
static int AlltoallvMessage(vector<int> messages, int* send_counts, int* send_displs, vector<int>& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    return recv_size;
};
static int AlltoallvMessage(vector<uint64_t> messages, int* send_counts, int* send_displs, vector<uint64_t>& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};
static int AlltoallvMessage(vector< pair<uint64_t, uint64_t> > messages, int* send_counts, int* send_displs, vector< pair<uint64_t, uint64_t> >& recv_messages) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_size /= 2;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg, send_counts, send_displs */
static int AlltoallvMessage(vector<int>& messages, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<int> recv_messages;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    messages.swap(recv_messages);
    recv_messages.clear();
    return recv_size;
};
static int AlltoallvMessage(vector<uint64_t>& messages, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<uint64_t> recv_messages;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    messages.swap(recv_messages);
    recv_messages.clear();
    return recv_size;
};
static int AlltoallvMessage(vector< pair<uint64_t, uint64_t> >& messages, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_size /= 2;
    vector< pair<uint64_t, uint64_t> > recv_messages;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    messages.swap(recv_messages);
    recv_messages.clear();
    return recv_size;
};

/* send_msg, send_counts, send_displs, recv_msg, recv_displs */
static int AlltoallvMessage(vector<uint8_t> messages, int* send_counts, int* send_displs, vector<uint8_t>& recv_messages, int* recv_counts, int* recv_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT8_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT8_T, MPI_COMM_WORLD);
    return recv_size;
};
static int AlltoallvMessage(vector<int> messages, int* send_counts, int* send_displs, vector<int>& recv_messages, int* recv_counts, int* recv_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_INT, recv_messages.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    return recv_size;
};
static int AlltoallvMessage(vector<uint64_t> messages, int* send_counts, int* send_displs, vector<uint64_t>& recv_messages, int* recv_counts, int* recv_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg, send_displs, recv_msg, recv_displs */
static int AlltoallvMessage_d(vector<uint64_t>& messages, int* send_displs, vector<uint64_t>& recv_messages, int* recv_displs, MPI_Comm COMM_SHARED = MPI_COMM_WORLD) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);

    int* send_counts = new int[comm_sz]{0};
    create_counts(send_counts, send_displs, comm_sz, (int)messages.size());
    int* recv_counts = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    return recv_size;
};
static int AlltoallvMessage_d(vector< pair<uint64_t, uint64_t> >& messages, int* send_displs, vector< pair<uint64_t, uint64_t> >& recv_messages, int* recv_displs, MPI_Comm COMM_SHARED = MPI_COMM_WORLD) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    int local_size = messages.size() * 2;
    int* send_counts = new int[comm_sz]{0};
    create_counts(send_counts, send_displs, comm_sz, local_size);
    int* recv_counts = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_size /= 2;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    return recv_size;
};

/* send_msg, send_displs, recv_displs */
static int AlltoallvMessage_d(vector<uint64_t>& messages, int* send_displs, int* recv_displs, MPI_Comm COMM_SHARED = MPI_COMM_WORLD) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);

    int* send_counts = new int[comm_sz]{0};
    create_counts(send_counts, send_displs, comm_sz, (int)messages.size());
    int* recv_counts = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<uint64_t> recv_messages;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    messages.swap(recv_messages);
    recv_messages.clear();
    return recv_size;
};
static int AlltoallvMessage_d(vector< pair<uint64_t, uint64_t> >& messages, int* send_displs, int* recv_displs, MPI_Comm COMM_SHARED = MPI_COMM_WORLD) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);

    int local_size = messages.size() * 2;
    int* send_counts = new int[comm_sz]{0};
    create_counts(send_counts, send_displs, comm_sz, local_size);
    // Log::Array1Detail("send_displs", comm_sz, send_displs);
    // Log::Array1Detail("send_counts", comm_sz, send_counts);

    int* recv_counts = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    // Log::Array1Detail("recv_counts", comm_sz, recv_counts);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    // Log::Array1Detail("recv_displs", comm_sz, recv_displs);

    recv_size /= 2;
    vector< pair<uint64_t, uint64_t> > recv_messages;
    recv_messages.resize(recv_size);
    // if (recv_size == 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    messages.swap(recv_messages);
    return recv_size;
};

/* send_msg, send_displs */
static int AlltoallvMessage_d(vector< pair<uint64_t, uint64_t> >& messages, int* send_displs, MPI_Comm COMM_SHARED = MPI_COMM_WORLD) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);

    int local_size = messages.size() * 2;
    // Log::Info("[AlltoallvMessage_d] local_size=", local_size, " messages.size=",  messages.size());
    int* send_counts = new int[comm_sz]{0};
    create_counts(send_counts, send_displs, comm_sz, local_size);
    // Log::Array1Detail("send_displs", comm_sz, send_displs);
    // Log::Array1Detail("send_counts", comm_sz, send_counts);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, COMM_SHARED);
    // Log::Array1Detail("recv_counts", comm_sz, recv_counts);
    uint64_t recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    // Log::Info("[AlltoallvMessage_d] last_recv_displs=", recv_displs[comm_sz-1]);
    // Log::Array1Detail("recv_displs", comm_sz, recv_displs);
    recv_size /= 2;
    Log::Warn("[AlltoallvMessage_d] recv_size=", recv_size);
    StaticsNum(recv_size, "recv_size", true);
    vector< pair<uint64_t, uint64_t> > recv_messages;
    recv_messages.resize(recv_size);
    if (recv_size < 0) recv_messages.resize(1);
    MPI_Alltoallv(messages.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages.data(), recv_counts, recv_displs, MPI_UINT64_T, COMM_SHARED);
    messages.swap(recv_messages);
    return recv_size;
};

/* send_msg_1, send_msg_2, send_counts, recv_msg_1, recv_msg_2 */
static int AlltoallvMessage2(vector<uint64_t> messages_1, vector<int> messages_2, int* send_counts,  vector<uint64_t>& recv_messages_1, vector<int>& recv_messages_2) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    recv_counts = new int[comm_sz]{0};
    recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);

    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_INT, recv_messages_2.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    return recv_size;
};
static int AlltoallvMessage2(vector<uint64_t> messages_1, vector<uint64_t> messages_2, int* send_counts,  vector<uint64_t>& recv_messages_1, vector<uint64_t>& recv_messages_2) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    recv_counts = new int[comm_sz]{0};
    recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);

    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_2.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg_1, send_msg_2, send_counts, send_displs, recv_msg_1, recv_msg_2 */
static int AlltoallvMessage2(vector<uint64_t> messages_1, vector<int> messages_2, int* send_counts, int* send_displs, vector<uint64_t>& recv_messages_1, vector<int>& recv_messages_2) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    recv_counts = new int[comm_sz]{0};
    recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);

    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_INT, recv_messages_2.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    return recv_size;
};

static int AlltoallvMessage2(vector<uint64_t> messages_1, vector<uint64_t> messages_2, int* send_counts, int* send_displs, vector<uint64_t>& recv_messages_1, vector<uint64_t>& recv_messages_2) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    recv_counts = new int[comm_sz]{0};
    recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);

    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_2.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg_1, send_msg_2, send_counts, recv_msg_1, recv_msg_2, recv_counts, recv_displs */
static int AlltoallvMessage2(vector<uint64_t> messages_1, vector<uint64_t> messages_2, int* send_counts,  vector<uint64_t>& recv_messages_1, vector<uint64_t>& recv_messages_2, int* recv_counts, int* recv_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_displs = new int[comm_sz]{0};
    recv_counts = new int[comm_sz]{0};
    recv_displs = new int[comm_sz]{0};
    create_displs(send_counts, send_displs, comm_sz);
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);
    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);

    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_2.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg_1, send_msg_2, send_counts, send_displs, recv_msg_1, recv_msg_2, recv_counts, recv_displs */
static int AlltoallvMessage2(vector<uint64_t> messages_1, vector<uint64_t> messages_2, int* send_counts, int* send_displs, vector<uint64_t>& recv_messages_1, vector<uint64_t>& recv_messages_2, int* recv_counts, int* recv_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);

    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_2.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    return recv_size;
};

/* send_msg_1, send_msg_2, send_counts, send_displs */
static int AlltoallvMessage2(vector<int>& messages_1, vector<int>& messages_2, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<int> recv_messages_1;
    vector<int> recv_messages_2;
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);
    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_INT, recv_messages_1.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_INT, recv_messages_2.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    messages_1.swap(recv_messages_1);
    messages_2.swap(recv_messages_2);
    recv_messages_1.clear();
    recv_messages_2.clear();
    return recv_size;
};
static int AlltoallvMessage2(vector<uint64_t>& messages_1, vector<uint64_t>& messages_2, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<uint64_t> recv_messages_1;
    vector<uint64_t> recv_messages_2;
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);
    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_2.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    messages_1.swap(recv_messages_1);
    messages_2.swap(recv_messages_2);
    recv_messages_1.clear();
    recv_messages_2.clear();
    return recv_size;
};
static int AlltoallvMessage2(vector<uint64_t>& messages_1, vector<int>& messages_2, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<uint64_t> recv_messages_1;
    vector<int> recv_messages_2;
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);
    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_INT, recv_messages_2.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    messages_1.swap(recv_messages_1);
    messages_2.swap(recv_messages_2);
    recv_messages_1.clear();
    recv_messages_2.clear();
    return recv_size;
};

/* send_msg_1, send_msg_2, send_displs */
static int AlltoallvMessage2_d(vector<uint64_t>& messages_1, vector<int>& messages_2, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* send_counts = new int[comm_sz]{0};
    create_counts(send_counts, send_displs, comm_sz, (int)messages_1.size());

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<uint64_t> recv_messages_1;
    vector<int> recv_messages_2;
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);
    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_INT, recv_messages_2.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    messages_1.swap(recv_messages_1);
    messages_2.swap(recv_messages_2);
    recv_messages_1.clear();
    recv_messages_2.clear();
    return recv_size;
};

/* send_msg_1, send_msg_2, send_msg_3, send_counts, send_displs */
static int AlltoallvMessage3(vector<uint64_t>& messages_1, vector<int>& messages_2, vector<uint8_t>& messages_3, int* send_counts, int* send_displs) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int* recv_counts = new int[comm_sz]{0};
    int* recv_displs = new int[comm_sz]{0};
    MPI_Alltoall(send_counts, 1, MPI_INT, recv_counts, 1, MPI_INT, MPI_COMM_WORLD);

    int recv_size = create_displs(recv_counts, recv_displs, comm_sz);
    vector<uint64_t> recv_messages_1;
    vector<int> recv_messages_2;
    vector<uint8_t> recv_messages_3;
    recv_messages_1.resize(recv_size);
    recv_messages_2.resize(recv_size);
    recv_messages_3.resize(recv_size);
    MPI_Alltoallv(messages_1.data(), send_counts, send_displs, MPI_UINT64_T, recv_messages_1.data(), recv_counts, recv_displs, MPI_UINT64_T, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_2.data(), send_counts, send_displs, MPI_INT, recv_messages_2.data(), recv_counts, recv_displs, MPI_INT, MPI_COMM_WORLD);
    MPI_Alltoallv(messages_3.data(), send_counts, send_displs, MPI_UINT8_T, recv_messages_3.data(), recv_counts, recv_displs, MPI_UINT8_T, MPI_COMM_WORLD);
    messages_1.swap(recv_messages_1);
    messages_2.swap(recv_messages_2);
    messages_3.swap(recv_messages_3);
    recv_messages_1.clear();
    recv_messages_2.clear();
    recv_messages_3.clear();
    return recv_size;
};



#endif /* mpi_utils_old_h */
