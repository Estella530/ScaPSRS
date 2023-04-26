//
//  mpi_new_utils.h
//  Exassembler
//
//  Created by Estella Wong on 2023/3/20.
//  Copyright © 2023 Estella Wong. All rights reserved.
//

#ifndef mpi_new_utils_h
#define mpi_new_utils_h

#pragma once
#include <string.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <typeinfo>
#include "mpi.h"

// if (typeid(a) == typeid(int))

template <typename T>
static int New_Alltoallv(T* sendbuf, uint64_t* scounts, uint64_t* sdispls, T* recvbuf, uint64_t* rcounts, uint64_t* rdispls, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    MPI_Status status[comm_sz];

    MPI_Datatype mpitype;
    if (typeid(T) == typeid(int)) mpitype = MPI_INT;
    else if (typeid(T) == typeid(uint64_t)) mpitype = MPI_UINT64_T;
    if (scounts[my_rank] > 0) {
        memcpy(recvbuf + rdispls[my_rank], sendbuf + sdispls[my_rank], scounts[my_rank] * sizeof(T));
    }
    int i = 0;
    for (i = 0; i < comm_sz; i++) {
        if (i == my_rank) continue;
        MPI_Send(sendbuf + sdispls[i], scounts[i], mpitype, i, i, COMM_SHARED);
    }
    for (i = 0; i < comm_sz; i++) {
        if (i == my_rank) continue;
        MPI_Recv(recvbuf + rdispls[i], rcounts[i], mpitype, i, my_rank, COMM_SHARED, &status[i]);
    }
};


template <typename T>
static int New_Ialltoallv(T* sendbuf, uint64_t* scounts, uint64_t* sdispls, T* recvbuf, uint64_t* rcounts, uint64_t* rdispls, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    MPI_Request request[(comm_sz-1)*2];
    // MPI_Status status[comm_sz];

    MPI_Datatype mpitype;
    if (typeid(T) == typeid(int)) mpitype = MPI_INT;
    else if (typeid(T) == typeid(uint64_t)) mpitype = MPI_UINT64_T;
    if (scounts[my_rank] > 0) {
        memcpy(recvbuf + rdispls[my_rank], sendbuf + sdispls[my_rank], scounts[my_rank] * sizeof(T));
    }
    int i = 0;
    int cnt = 0;
    for (int i = 0; i < comm_sz; i++) {
        if (i != my_rank) {
            MPI_Irecv(recvbuf + rdispls[i], rcounts[i], mpitype, i, my_rank, COMM_SHARED, &request[cnt]);
            cnt++;
        }
    }
    for (int i = 0; i < comm_sz; i++) {
        if (i != my_rank) {
            MPI_Isend(sendbuf + sdispls[i], scounts[i], mpitype, i, i, COMM_SHARED, &request[cnt]);
            cnt++;
        }
    }
    MPI_Waitall(cnt, request, MPI_STATUS_IGNORE);
};

template <typename T>
static int New_Ialltoallv(vector< pair<T, T> >& sendbuf, uint64_t* scounts, uint64_t* sdispls, vector< pair<T, T> >& recvbuf, uint64_t* rcounts, uint64_t* rdispls, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    for (int i = 0; i < comm_sz; i++) {
        scounts[i] *= 2;
        sdispls[i] *= 2;
        rcounts[i] *= 2;
        rdispls[i] *= 2;
    }
    New_Ialltoallv(sendbuf, scounts, sdispls, recvbuf, rcounts, rdispls, COMM_SHARED);
};

template <typename T>
static int New_Ialltoallv_displs(vector< pair<T, T> >& sendbuf, uint64_t* sdispls, vector< pair<T, T> >& recvbuf, uint64_t* rdispls, MPI_Comm COMM_SHARED) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(COMM_SHARED, &my_rank);
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    uint64_t local_size = sendbuf.size() * 2;
    // Log::Info("[New_Ialltoallv_displs] local_size=", local_size, " messages.size=",  sendbuf.size());
    uint64_t* scounts = new uint64_t[comm_sz]{0};
    uint64_t* rcounts = new uint64_t[comm_sz]{0};
    create_counts(scounts, sdispls, comm_sz, local_size);
    MPI_Alltoall(scounts, 1, MPI_UINT64_T, rcounts, 1, MPI_UINT64_T, COMM_SHARED);
    uint64_t recv_size = create_displs(rcounts, rdispls, comm_sz);
    // if (my_rank == 1018 || my_rank == 1022) {
    //     uint64_t recv_acc = accumulate(rcounts, rcounts + comm_sz, (uint64_t)0);
    //     Log::Info("[New_Ialltoallv_displs] recv_acc=", recv_acc);
    // }
    // Log::Info("[New_Ialltoallv_displs] last_rdispls=", rdispls[comm_sz-1], recv_size);
    recv_size /= 2;
    // Log::Info("[New_Ialltoallv_displs] recv_size=", recv_size);
    // StaticsNum(recv_size, "recv_size", true);
    recvbuf.resize(recv_size);
    New_Ialltoallv(sendbuf, scounts, sdispls, recvbuf, rcounts, rdispls, COMM_SHARED);
};

template <typename T>
static int New_Ialltoallv_displs(vector< pair<T, T> >& sendbuf, uint64_t* sdispls, uint64_t* rdispls, MPI_Comm COMM_SHARED) {
    vector< pair<T, T> > recvbuf;
    New_Ialltoallv_displs(sendbuf, sdispls, recvbuf, rdispls, COMM_SHARED);
    sendbuf.swap(recvbuf);
    recvbuf.clear();
};

template <typename T>
static int New_Ialltoallv_displs(vector< pair<T, T> >& sendbuf, uint64_t* sdispls, MPI_Comm COMM_SHARED) {
    int comm_sz;
    MPI_Comm_size(COMM_SHARED, &comm_sz);
    uint64_t* rdispls = new uint64_t[comm_sz]{0};
    New_Ialltoallv_displs(sendbuf, sdispls, rdispls, COMM_SHARED);
    delete [] rdispls;
};


#endif /* mpi_new_utils_h */
