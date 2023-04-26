//
//  ScaPSRS.h
//  ScaPSRS
//
//  Created by Estella Wong on 2023/2/16.
//  Copyright © 2023 Estella Wong. All rights reserved.
//

#ifndef ScaPSRS_h
#define ScaPSRS_h

#pragma once
#include <stdio.h>
#include <limits.h>
#include "utils/utils.h"
#include "utils/mpi_utils.h"
#include "utils/ProcessConfig.hpp"
#include "Dividing.h"
#include "FinalRound.h"

    ///@param samples the samples
    ///@param lpos the low position
    ///@param hpos the high position
    ///@param pivot (output) the pivot selected from the range lpos to hpos of samples
    ///@param mpos (output) the mid position, where pivot located in samples
template <typename T>
static void SelectPivot_FromSamples(vector<T>& samples, int lpos, int hpos, T &pivot, int &mpos) {
    if (lpos < 0) lpos = 0;
    if (hpos >= samples.size()) hpos = samples.size() - 1;
    mpos = hpos - lpos;
    if (mpos < 0) {
        mpos = lpos;
        lpos = hpos;
        hpos = mpos;
        mpos = hpos - lpos;
    }
    if (mpos <= 1) {
        pivot = samples.at(lpos) + (samples.at(hpos) - samples.at(lpos)) / 2;
        mpos = lpos;
    }
    else {
        mpos = lpos + mpos / 2;
        pivot = samples.at(mpos);
    }
};

// to_be_larger
template <typename T>
static void UpdatePivot_add(T &pivot, T &last_pivot) {
    T tmp = pivot;
    pivot += (last_pivot - pivot) / 2;
    last_pivot = tmp;
};
template <typename T>
static void UpdatePivot_to_be_larger(T &pivot, T &last_pivot, vector<T>& samples, int lpos) {
    if (pivot == UINT64_MAX) {
        last_pivot = pivot;
    }
    else if (pivot < last_pivot) {
        UpdatePivot_add(pivot, last_pivot);
    }
    else {
        if (lpos < 0) lpos = 0;
        else if (lpos >= samples.size()) lpos = samples.size() - 1;
        if (samples.at(lpos) > last_pivot) last_pivot = samples.at(lpos);
        if (pivot < last_pivot) {
            UpdatePivot_add(pivot, last_pivot);
        }
        else {
            last_pivot = pivot;
            pivot++;
        }
    }
};

template <typename T>
static int LocatePos(T &pivot, vector<T>& pivots, vector<T>& samples, int &lpos, int &hpos, int mpos, uint64_t* b, vector<uint64_t>& lower_bounds, vector<uint64_t>& upper_bounds, MPI_Comm COMM_SHARED) {
    commID my_;
    ProcessConfig(my_, COMM_SHARED);
    if (pivot < pivots.at(my_.global_rank_)) {
        lpos = find_max_right(samples.data(), my_.global_size_, pivots.at(my_.global_rank_));
        if (lpos <= mpos) lpos = mpos + 1;
    }
    else { //找出 pivot 的位置及其对应b，判断更新方向
        int real_rank_pos = scan_fwd(pivot, my_.global_rank_ + 1,  my_.global_size_ - 1, pivots.data());
        if (real_rank_pos >=  my_.global_size_) {
            real_rank_pos =  my_.global_size_ - 1;
        }
        if (b[real_rank_pos] < lower_bounds[my_.global_rank_]) {
            to_be_larger = true;
            lpos = mpos + 1;
        }
        else if (b[real_rank_pos] > upper_bounds[my_.global_rank_]) {
            to_be_larger = false;
            hpos = mpos - 1;
            mpos = find_max_right(samples.data(), my_.global_size_, pivots.at(my_.global_rank_));
            if (lpos <= mpos) lpos = mpos + 1;
        }
        else {
            pivot = pivots.at(real_rank_pos);
            // Log::Debug("j-", my_.global_rank_, "[caseA], mpos=", mpos, "candi_p:", candi_p, "bk:", b[real_rank_pos]);
            // complete = 1;
            return 1;
        }
    }
    return 0;
};

// to_be_smaller
template <typename T>
static void UpdatePivot_minus(T &pivot, T &last_pivot) {
    T tmp = pivot;
    pivot -= (pivot - last_pivot) / 2;
    last_pivot = tmp;
};
template <typename T>
static void UpdatePivot_to_be_smaller(T &pivot, T &last_pivot, vector<T>& samples, int hpos) {
    if (pivot == 0) {
        last_pivot = pivot;
    }
    else if (pivot > last_pivot) {
        UpdatePivot_minus(pivot, last_pivot);
    }
    else{
        if (hpos < 0) hpos = 0;
        else if (hpos >= samples.size()) hpos = samples.size() - 1;
        if (samples.at(hpos) < last_pivot) last_pivot = samples.at(hpos);
        if (pivot > last_pivot) {
            UpdatePivot_minus(pivot, last_pivot);
        }
        else {
            last_pivot = pivot;
            pivot--;
        }
    }
};

    ///减少for循环
    ///通过归约判断是否提前结束循环
    ///@param loca_data local raw data to be sorted
    ///@param local_n the number of local raw data
    ///@param lower_bounds the acceptable lower bounds for b
    ///@param upper_bounds the acceptable upper bounds for b 
    ///@param samples the samples gather from all processes in COMM_SHARED
    ///@param candi_pivots (output) candidates for pivots
    ///@param candi_s (output) candidates sdispls, according to candi_pivots
    ///@param b (output) allreduce-sum(candi_s)
    ///@param COMM_SHARED the communicator
template <typename T>
static void SelectPivots(vector<T>& local_data, uint64_t local_n, vector<uint64_t>& lower_bounds, vector<uint64_t>& upper_bounds, vector<T>& samples, vector<T>& candi_pivots, uint64_t* candi_s, uint64_t* b, MPI_Comm COMM_SHARED) {
    commID my_;
    ProcessConfig(my_, COMM_SHARED);
    uint64_t mid_bound = lower_bounds[my_.global_rank_] + (upper_bounds[my_.global_rank_] - lower_bounds[my_.global_rank_]) / 2;
    int rounds = log2( my_.global_size_) + 1;
    int lpos = 0;
    int hpos = samples.size() - 1;
    int mpos = 0;
    bool from_samples = true;
    bool to_be_larger = false; // true: pivot to be larger; false: to be smaller
    bool candi_sorted = true;
    T last_candi_p = 0;
    T candi_p = 0;
    int complete = 0; //记录该轮是否完成
    int complete_num = 0; //记录有多少个进程完成该轮
    for (int r = 0; r < rounds; r++) {
        if (r > 0) MPI_Allreduce(&complete, &complete_num, 1, MPI_INT, MPI_SUM, COMM_SHARED);
        if (complete_num >=  my_.global_size_) {
            break;
        }
        if (complete == 0) {
            if (lpos + 1 > hpos) {
                from_samples = false;
            }
            if (from_samples) {
                last_candi_p = candi_p;
                SelectPivot_FromSamples(samples, lpos, hpos, candi_p, mpos);
                if (lpos + 1 > hpos) {
                    from_samples = false;
                }
            }
            else {
                if (to_be_larger) {
                    UpdatePivot_to_be_larger(candi_p, last_candi_p, samples, lpos);
                }
                else { //to_be_smaller
                    UpdatePivot_to_be_smaller(candi_p, last_candi_p, samples, hpos);
                }
            }
        }
            //allgather candi_pivots
        AllGatherMsg(candi_p, candi_pivots, COMM_SHARED);
        sort(candi_pivots.begin(), candi_pivots.end());
        if (candi_p != candi_pivots[my_.global_rank_]) {
            candi_sorted = false;
        }
        else {
            candi_sorted = true;
        }
        uint64_t local_n_1 = local_n - 1;
        DivideByPivots_duplicated_2(local_data.data(), local_n, (uint64_t)0, local_n_1, candi_pivots.data() + 1, candi_pivots.size() - 1, 0,  my_.global_size_-2, candi_s,  my_.global_size_);
        
        MPI_Allreduce(candi_s, b,  my_.global_size_, MPI_UINT64_T, MPI_SUM, COMM_SHARED);
        if (complete >= 1) continue;
        //重新定位lpos和hpos
        if (b[my_.global_rank_] < lower_bounds[my_.global_rank_]) {
            to_be_larger = true;
            if (candi_sorted) lpos = mpos + 1;
            else {
                // complete = LocatePos(candi_p, candi_pivots, samples, lpos, hpos, COMM_SHARED);
                if (candi_p < candi_pivots[my_.global_rank_]) {
                    lpos = find_max_right(samples.data(),  my_.global_size_, candi_pivots[my_.global_rank_]);
                    if (lpos <= mpos) lpos = mpos + 1;
                }
                else { //找出candi_p的位置及其对应b，判断更新方向
                    int real_rank_pos = scan_fwd(candi_p, my_.global_rank_ + 1,  my_.global_size_ - 1, candi_pivots.data());
                    if (real_rank_pos >=  my_.global_size_) {
                        real_rank_pos =  my_.global_size_ - 1;
                    }
                    if (b[real_rank_pos] < lower_bounds[my_.global_rank_]) {
                        to_be_larger = true;
                        lpos = mpos + 1;
                    }
                    else if (b[real_rank_pos] > upper_bounds[my_.global_rank_]) {
                        to_be_larger = false;
                        hpos = mpos - 1;
                        mpos = find_max_right(samples.data(),  my_.global_size_, candi_pivots[my_.global_rank_]);
                        if (lpos <= mpos) lpos = mpos + 1;
                    }
                    else {
                        // Log::Debug("j-", my_.global_rank_, "[caseA], mpos=", mpos, "candi_p:", candi_p, "bk:", b[real_rank_pos]);
                        complete = 1;
                    }
                }
            }
        }
        else if (b[my_.global_rank_] > upper_bounds[my_.global_rank_]) {
            to_be_larger = false;
            if (candi_sorted) hpos = mpos - 1;
            else {
                if (candi_p > candi_pivots[my_.global_rank_]) {
                    hpos = BinarySearch_min_left(candi_pivots[my_.global_rank_],  my_.global_size_, samples.data());
                    if (hpos >= mpos) hpos = mpos - 1;
                }
                else { //找出candi_p的位置及其对应b，判断更新方向
                    int real_rank_pos = scan_bwd(candi_p, 0, my_.global_rank_ - 1, candi_pivots.data());
                    if (real_rank_pos < 0) {
                        real_rank_pos = 0;
                    }
                    if (b[real_rank_pos] < lower_bounds[my_.global_rank_]) {
                        to_be_larger = true;
                        lpos = mpos + 1;
                        mpos = BinarySearch_min_left(candi_pivots[my_.global_rank_],  my_.global_size_, samples.data());
                        if (hpos >= mpos) hpos = mpos - 1;
                    }
                    else if (b[real_rank_pos] > upper_bounds[my_.global_rank_]) {
                        to_be_larger = false;
                        hpos = mpos - 1;
                    }
                    else {
                        // Log::Debug("j-", my_.global_rank_, "[caseA], mpos=", mpos, "candi_p:", candi_p, "bk:", b[real_rank_pos]);
                        complete = 1;
                    }
                }
            }
        }
        else {
            // Log::Debug("j-", my_.global_rank_, "[caseA], mpos=", mpos, "candi_p:", candi_pivots[my_.global_rank_], "bk:", b[my_.global_rank_]);
            candi_p = candi_pivots[my_.global_rank_];
            complete = 1;
        }
    }
    AllGatherMsg(candi_p, candi_pivots, COMM_SHARED);
    sort(candi_pivots.begin(), candi_pivots.end());
};

    ///@param lower_bounds (output) the acceptable lower bounds, according to imba
    ///@param upper_bounds (output) the acceptable upper bounds, according to imba
    ///@param local_n the number of data
    ///@param imba the acceptable rate of imbalance
    ///@param COMM_SHARED the communicator
static void InitBounds(vector<uint64_t>& lower_bounds, vector<uint64_t>& upper_bounds, uint64_t local_n, double imba, MPI_Comm COMM_SHARED) {
    commID my_;
    ProcessConfig(my_, COMM_SHARED);
    uint64_t total_n = 0;
    MPI_Allreduce(&local_n, &total_n, 1, MPI_UINT64_T, MPI_SUM, COMM_SHARED);
    uint64_t avg_n = total_n / my_.global_size_;
    uint64_t imba_n = imba * avg_n;
    for (int i = 1; i <  my_.global_size_; i++) {
        lower_bounds[i] = i * avg_n - imba_n;
        upper_bounds[i] = i * avg_n + imba_n;
    }
};

    ///MARK: 记录历史pivots，记录IR变化
    ///@param loca_data local raw data to be sorted
    ///@param local_n the number of local raw data
    ///@param sdispls (output) candidates sdispls according to candi_pivots
    ///@param COMM_SHARED the communicator
    ///@param imba the acceptable rate of imbalance
template <typename T>
static void Partition_ScaPSRS(vector<T>& local_data, uint64_t local_n, uint64_t *sdispls, MPI_Comm COMM_SHARED, double imba = 0.1) {
    commID my_;
    ProcessConfig(my_, COMM_SHARED);
/* Init Lower&Upper Bounds */
    vector<uint64_t> lower_bounds( my_.global_size_, 0);
    vector<uint64_t> upper_bounds( my_.global_size_, 0);
    InitBounds(lower_bounds, upper_bounds, local_n, imba, COMM_SHARED);
        
/* Regular Sampling */
    vector<T> samples( my_.global_size_, 0);
    vector<T> pivots( my_.global_size_, UINT64_MAX);
    int quotient = local_n /  my_.global_size_;
    RegularSampling(local_n,  my_.global_size_, quotient, local_data.data(), samples.data());
        
/* Alltoall - Samples */
    AlltoallMsg(samples, 1, COMM_SHARED);
    sort(samples.begin(), samples.end());
        
/* Select Pivots */
    uint64_t* candi_s = new uint64_t[ my_.global_size_]{0}; //candi_splitting(candi_sdispls)
    uint64_t* b = new uint64_t[ my_.global_size_]{0}; //all-reduce-sum(s)
    SelectPivots(local_data, lower_bounds, upper_bounds, samples, pivots, candi_s, b, COMM_SHARED);
/* FinalRound */
    memcpy(sdispls, candi_s,  my_.global_size_ * sizeof(uint64_t));
    FinalRound_range(lower_bounds, upper_bounds, pivots, candi_s, b, sdispls, COMM_SHARED);

    FreeUpCompletely(lower_bounds);
    FreeUpCompletely(upper_bounds);
    FreeUpCompletely(samples);
    FreeUpCompletely(pivots);
    delete [] candi_s;
    delete [] b;
}

    ///@param loca_data local raw data to be sorted; (output) the sorted data
    ///@param local_n the number of local raw data
    ///@param COMM_SHARED the communicator
    ///@param imba the acceptable rate of imbalance
template <typename T>
static void ScaPSRS(vector<T>& local_data, uint64_t local_n, MPI_Comm COMM_SHARED, double imba = 0.05) {
    commID my_;
    ProcessConfig(my_, COMM_SHARED);
/* Local Sort */
    MpiTimer mtimer;
    mtimer.reset();
    mtimer.start();
    sort(local_data.begin(), local_data.end());
    mtimer.stop();
    Log::Info_mpi(0, "Time is", mtimer.time_diff, "s. (LocalSort-ScaPSRS-Part)");
    if (my_.global_size_ == 1) return;

/* Partitioning */
    uint64_t *candi_sdispls = new uint64_t[my_.global_size_]{0};
    mtimer.start();
    Partition_ScaPSRS(local_data, candi_sdispls, local_n, COMM_SHARED, imba);
    mtimer.stop();
    Log::Info_mpi(0, "Time is", mtimer.time_diff, "s(Partition-ScaPSRS-Part).");

/* Alltoallv - data */
    mtimer.start();
    // bool u_limit = false;
    int *sdispls = new int[my_.global_size_]{0};
    for (int i = 0; i < my_.global_size_; i++) {
        sdispls[i] = candi_sdispls[i];
        // if (sdispls[i] < 0) u_limit = true;
    }    
    // bool u_all_limit = false;
    // MPI_Allreduce(&u_limit, &u_all_limit, 1, MPI_UINT8_T, MPI_BOR, COMM_SHARED);
    // if (u_all_limit) New_Ialltoallv_displs(local_data, candi_sdispls, COMM_SHARED);
    // else 
    AlltoallvMsg(local_data, sdispls, COMM_SHARED);
    mtimer.stop();
    Log::Info_mpi(0, "Time is", mtimer.time_diff, "s(All2allData-ScaPSRS-Part).");

/* Last Local Sort */
    mtimer.start();
    sort(local_data.begin(), local_data.end());
    mtimer.stop();
    Log::Warn("Time is", mtimer.time_diff, "s. (LastLocalSort-ScaPSRS-Part)");

    delete [] candi_sdispls;
    delete [] sdispls;
}


#endif /* ScaPSRS_h */
