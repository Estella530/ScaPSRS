//
//  ScaPSRS.hpp
//  ScaPSRS
//
//  Created by Estella Wong on 2023/2/16.
//  Copyright © 2023 Estella Wong. All rights reserved.
//

#ifndef ScaPSRS_hpp
#define ScaPSRS_hpp

#pragma once
#include <stdio.h>
#include <limits.h>
#include "utils/utils.h"
#include "utils/mpi_utils.h"
#include "utils/CountsDispls.h"
#include "utils/Log.hpp"
#include "utils/ProcessConfig.hpp"
#include "Balance.h"
#include "Dividing.h"
#include "FinalRound.h"
#include "Sampling.h"

template <class T>
class ScaPSRS {
public:
    ScaPSRS() = default;
    ScaPSRS(double imba_) : imba(imba_) {
        COMM_SHARED = MPI_COMM_WORLD;
    };
    ScaPSRS(MPI_Comm COMM_SHARED_, double imba_ = 0.05) : COMM_SHARED(COMM_SHARED_), imba(imba_){};
    ScaPSRS(vector<T>& local_data, MPI_Comm COMM_SHARED_, double imba_ = 0.05);
    ~ScaPSRS() = default;

    void Partition_ScaPSRS(vector<T>& local_data);
    void InitBounds();
    void SelectPivots(vector<T>& local_data);
    T SelectPivot_FromSamples();

    void UpdatePivot_add(T &pivot, T &last_pivot);
    void UpdatePivot_minus(T &pivot, T &last_pivot);

    void UpdatePivot_to_be_larger(T &pivot, T &last_pivot);
    void UpdatePivot_to_be_smaller(T &pivot, T &last_pivot);

    void LocatePos_to_be_larger(T &pivot);
    void LocatePos_to_be_smaller(T &pivot);

public:
    uint64_t local_n = 0; // the number of local data
    double imba = 0.05; // the acceptable rate of imbalance

    int lpos = 0;
    int hpos = 0;
    int mpos = 0;

    vector<T> samples;
    vector<T> pivots;

    vector<uint64_t> lower_bounds; //the acceptable lower bounds, according to imba
    vector<uint64_t> upper_bounds; //the acceptable upper bounds, according to imba
    vector<uint64_t> candi_s; 
    vector<uint64_t> b; 

    // uint64_t *candi_s;
    // uint64_t *b; //all-reduce-sum(s)

private:
    MPI_Comm COMM_SHARED; // the communicator
    commID my_;
    bool from_samples = true;
    bool to_be_larger = false; // true: pivot to be larger; false: to be smaller
    bool candi_sorted = true; 

    int complete = 0; //记录该轮是否完成
    int complete_num = 0; //记录有多少个进程完成该轮
};


#endif /* ScaPSRS_hpp */

