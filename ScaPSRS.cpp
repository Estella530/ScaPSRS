//
//  ScaPSRS.cpp
//  ScaPSRS
//
//  Created by Estella Wong on 2023/2/16.
//  Copyright © 2023 Estella Wong. All rights reserved.
//

#include "ScaPSRS.hpp"

template <class T>
ScaPSRS<T>::ScaPSRS(vector<T>& local_data, MPI_Comm COMM_SHARED_, double imba_) {
    local_n = local_data.size();
    COMM_SHARED = COMM_SHARED_;
    imba = imba_;
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
    // candi_s = new uint64_t[my_.global_size_]{0};
    // b = new uint64_t[ my_.global_size_]{0};
    candi_s.resize(my_.global_size_, 0);
    b.resize(my_.global_size_, 0);
    mtimer.start();
    Partition_ScaPSRS(local_data);
    mtimer.stop();
    Log::Info_mpi(0, "Time is", mtimer.time_diff, "s(Partition-ScaPSRS-Part).");

/* Alltoallv - data */
    mtimer.start();
    int* sdispls = new int[my_.global_size_]{0};
    for (int i = 0; i < my_.global_size_; i++) {
        sdispls[i] = candi_s[i];
    }  
    AlltoallvMsg(local_data, sdispls, COMM_SHARED);
    mtimer.stop();
    Log::Info_mpi(0, "Time is", mtimer.time_diff, "s(All2allData-ScaPSRS-Part).");

/* Last Local Sort */
    mtimer.start();
    sort(local_data.begin(), local_data.end());
    mtimer.stop();
    Log::Info_mpi(0, "Time is", mtimer.time_diff, "s. (LastLocalSort-ScaPSRS-Part)");

    FreeUpCompletely(lower_bounds);
    FreeUpCompletely(upper_bounds);
    FreeUpCompletely(samples);
    FreeUpCompletely(pivots);
    FreeUpCompletely(candi_s);
    FreeUpCompletely(b);
    // delete [] b;
    // delete [] candi_s;
    delete [] sdispls;
}

template <class T>
void ScaPSRS<T>::Partition_ScaPSRS(vector<T>& local_data) {
/* Init Lower&Upper Bounds */
    InitBounds();
/* Regular Sampling */
    samples.resize(my_.global_size_, 0);
    pivots.resize(my_.global_size_, UINT64_MAX);
    int quotient = local_n / my_.global_size_;
    RegularSampling(local_n, my_.global_size_, quotient, local_data.data(), samples.data());
/* Alltoall - Samples */
    AlltoallMsg(samples, 1, COMM_SHARED);
    sort(samples.begin(), samples.end());
/* Select Pivots */
    SelectPivots(local_data);
/* FinalRound */
    // uint64_t* tmp_s = new uint64_t[ my_.global_size_]{0};
    vector<uint64_t> tmp_s; 
    tmp_s.assign(candi_s.begin(), candi_s.end());
    // memcpy(tmp_s, candi_s, my_.global_size_ * sizeof(uint64_t));
    FinalRound_range(lower_bounds, upper_bounds, pivots, tmp_s.data(), b.data(), candi_s.data(), COMM_SHARED);
    // delete [] tmp_s;
    FreeUpCompletely(tmp_s);
}

template <class T>
void ScaPSRS<T>::InitBounds() {
    lower_bounds.resize(my_.global_size_, 0);
    upper_bounds.resize(my_.global_size_, 0);
    uint64_t total_n = 0;
    MPI_Allreduce(&local_n, &total_n, 1, MPI_UINT64_T, MPI_SUM, COMM_SHARED);
    uint64_t avg_n = total_n / my_.global_size_;
    uint64_t imba_n = imba * avg_n;
    for (int i = 1; i <  my_.global_size_; i++) {
        lower_bounds.at(i) = i * avg_n - imba_n;
        upper_bounds.at(i) = i * avg_n + imba_n;
    }
}

template <class T>
void ScaPSRS<T>::SelectPivots(vector<T>& local_data) {
    uint64_t mid_bound = lower_bounds.at(my_.global_rank_) + (upper_bounds.at(my_.global_rank_) - lower_bounds.at(my_.global_rank_)) / 2;
    int rounds = log2(my_.global_size_) + 1;
    hpos = samples.size() - 1;

    T last_candi_p = 0;
    T candi_p = 0;
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
                candi_p = SelectPivot_FromSamples();
                if (lpos + 1 > hpos) {
                    from_samples = false;
                }
            }
            else {
                if (to_be_larger) {
                    UpdatePivot_to_be_larger(candi_p, last_candi_p);
                }
                else { //to_be_smaller
                    UpdatePivot_to_be_smaller(candi_p, last_candi_p);
                }
            }
        }
        AllGatherMsg(candi_p, pivots, COMM_SHARED);
        sort(pivots.begin(), pivots.end());
        if (candi_p != pivots.at(my_.global_rank_)) {
            candi_sorted = false;
        }
        else {
            candi_sorted = true;
        }
        uint64_t local_n_1 = local_n - 1;
        DivideByPivots_duplicated_2(local_data.data(), local_n, (uint64_t)0, local_n_1, pivots.data() + 1, pivots.size() - 1, 0,  my_.global_size_-2, candi_s.data(), my_.global_size_);
        
        AllReduceMsg(candi_s, b, MPI_SUM, COMM_SHARED);
        if (complete >= 1) continue;
        //重新定位lpos和hpos
        if (b[my_.global_rank_] < lower_bounds[my_.global_rank_]) {
            to_be_larger = true;
            if (candi_sorted) lpos = mpos + 1;
            else {
                LocatePos_to_be_larger(candi_p);
            }
        }
        else if (b[my_.global_rank_] > upper_bounds[my_.global_rank_]) {
            to_be_larger = false;
            if (candi_sorted) hpos = mpos - 1;
            else {
                LocatePos_to_be_smaller(candi_p);
            }
        }
        else {
            candi_p = pivots[my_.global_rank_];
            complete = 1;
        }
    }
    AllGatherMsg(candi_p, pivots, COMM_SHARED);
    sort(pivots.begin(), pivots.end());
}

template <class T>
T ScaPSRS<T>::SelectPivot_FromSamples() {
    T pivot;
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
}


template <class T>
void ScaPSRS<T>::UpdatePivot_add(T &pivot, T &last_pivot) {
    T tmp = pivot;
    pivot += (last_pivot - pivot) / 2;
    last_pivot = tmp;
}
template <class T>
void ScaPSRS<T>::UpdatePivot_minus(T &pivot, T &last_pivot) {
    T tmp = pivot;
    pivot -= (pivot - last_pivot) / 2;
    last_pivot = tmp;
}

template <class T>
void ScaPSRS<T>::UpdatePivot_to_be_larger(T &pivot, T &last_pivot) {
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
}
template <class T>
void ScaPSRS<T>::UpdatePivot_to_be_smaller(T &pivot, T &last_pivot) {
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
}

template <class T>
void ScaPSRS<T>::LocatePos_to_be_larger(T &pivot) {
    if (pivot < pivots.at(my_.global_rank_)) {
        lpos = find_max_right(samples.data(), my_.global_size_, pivots.at(my_.global_rank_));
        if (lpos <= mpos) lpos = mpos + 1;
    }
    else { //找出 pivot 的位置及其对应b，判断更新方向
        int real_rank_pos = scan_fwd(pivot, my_.global_rank_ + 1, my_.global_size_ - 1, pivots.data());
        if (real_rank_pos >=  my_.global_size_) {
            real_rank_pos =  my_.global_size_ - 1;
        }
        if (b[real_rank_pos] < lower_bounds.at(my_.global_rank_)) {
            to_be_larger = true;
            lpos = mpos + 1;
        }
        else if (b[real_rank_pos] > upper_bounds.at(my_.global_rank_)) {
            to_be_larger = false;
            hpos = mpos - 1;
            mpos = find_max_right(samples.data(), my_.global_size_, pivots.at(my_.global_rank_));
            if (lpos <= mpos) lpos = mpos + 1;
        }
        else {
            pivot = pivots.at(real_rank_pos);
            complete = 1;
        }
    }
}
template <class T>
void ScaPSRS<T>::LocatePos_to_be_smaller(T &pivot) {
    if (pivot > pivots.at(my_.global_rank_)) {
        hpos = BinarySearch_min_left(pivots.at(my_.global_rank_), my_.global_size_, samples.data());
        if (hpos >= mpos) hpos = mpos - 1;
    }
    else { //找出 pivot 的位置及其对应b，判断更新方向
        int real_rank_pos = scan_bwd(pivot, 0, my_.global_rank_ - 1, pivots.data());
        if (real_rank_pos < 0) {
            real_rank_pos = 0;
        }
        if (b[real_rank_pos] < lower_bounds.at(my_.global_rank_)) {
            to_be_larger = true;
            lpos = mpos + 1;
            mpos = BinarySearch_min_left(pivots.at(my_.global_rank_), my_.global_size_, samples.data());
            if (hpos >= mpos) hpos = mpos - 1;
        }
        else if (b[real_rank_pos] > upper_bounds.at(my_.global_rank_)) {
            to_be_larger = false;
            hpos = mpos - 1;
        }
        else {
            pivot = pivots.at(real_rank_pos);
            complete = 1;
        }
    }
}
