//
//  statics_num.h
//  ScaPSRS
//
//  Created by Estella Wong on 2022/7/7.
//  Copyright © 2022 Estella Wong. All rights reserved.
//

#ifndef statics_num_h
#define statics_num_h

#pragma once
#include <string.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <map>
#include "mpi.h"
#include "Log.hpp"


template <typename T>
static void StaticsNum(int my_rank, vector<T>& recv_num, string title="", bool print_to_screen = false, bool print_detail = false) {
    if (my_rank != 0) return;
    if (print_to_screen) cout << "---------- StaticsNum(" << title << ") ----------" << endl;
    Log::Info("---------- StaticsNum(", title, ") ----------");
    auto max_id = max_element(recv_num.begin(), recv_num.end());
    auto max_n = *max_id;
    auto min_id = min_element(recv_num.begin(), recv_num.end());
    auto min_n = *min_id;
    double rate = 0.0;
    if (min_n != 0) {
        rate = (double)max_n/(double)min_n;
    }
    if (print_to_screen) {
        cout << "max:" << max_n << ", at:" << max_id - recv_num.begin();
        cout << " min:" << min_n << ", at:" << min_id - recv_num.begin();
        if (min_n != 0)  cout << " max/min:" << rate << endl;
        else cout << " max/min: infinite" << endl;
    }
    Log::Info("max:", max_n, ", at:", max_id - recv_num.begin());
    Log::Info("min:", min_n, ", at:", min_id - recv_num.begin());
    if (min_n != 0) Log::Info("max/min:", rate);
    else Log::Info("max/min:", "infinite");
    if (print_detail) {
        Log::Info("[StaticsNum] Detail of", title.c_str(), ":");
        Log::Vector1Detail(recv_num);
        Log::Line();
    }
};
template <typename T>
static void StaticsNum(int my_rank, int comm_sz, T* recv_num, string title="", bool print_to_screen = false, bool print_detail = false) {
    if (my_rank != 0) return;
    if (print_to_screen) cout << "---------- StaticsNum(" << title << ") ----------" << endl;
    Log::Info("---------- StaticsNum(", title, ") ----------");
    auto max_id = max_element(recv_num, recv_num + comm_sz);
    auto max_n = *max_id;
    auto min_id = min_element(recv_num, recv_num + comm_sz);
    auto min_n = *min_id;
    double rate = 0.0;
    if (min_n != 0) {
        rate = (double)max_n/(double)min_n;
    }
    if (print_to_screen) {
        cout << "max:" << max_n << ", at:" << max_id - recv_num;
        cout << " min:" << min_n << ", at:" << min_id - recv_num;
        if (min_n != 0)  cout << " max/min:" << rate << endl;
        else cout << " max/min: infinite" << endl;
    }
    Log::Info("max:", max_n, ", at:", max_id - recv_num);
    Log::Info("min:", min_n, ", at:", min_id - recv_num);
    if (min_n != 0) Log::Info("max/min:", rate);
    else Log::Info("max/min:", "infinite");
    if (print_detail) {
        Log::Info("[StaticsNum] Detail of", title.c_str(), ":");
        Log::Array1Detail(comm_sz, recv_num);
        Log::Line();
    }
};
template <typename T>
static void StaticsNum(T num, string title="", bool print_to_screen = false, bool print_detail = false) {
    int my_rank;
    int comm_sz;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    vector<int> recv_num;
    if (my_rank == 0) recv_num.resize(comm_sz);
    MPI_Gather(&num, 1, MPI_INT, recv_num.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    StaticsNum(my_rank, recv_num, title, print_to_screen, print_detail);
};



#endif /* statics_num_h */
