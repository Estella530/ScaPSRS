//
//  FinalRound.h
//  ScaPSRS
//
//  Created by Estella Wong on 2022/11/1.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef FinalRound_h
#define FinalRound_h

#include <stdio.h>
#include <vector>
#include <string>
#include "utils/Log.hpp"
#include "utils/ProcessConfig.hpp"


static void FinalRound_range(vector<uint64_t>& lower_bounds, vector<uint64_t>& upper_bounds, vector<uint64_t>& candi_pivots,  uint64_t* candi_s2, uint64_t* b, uint64_t* sdispls, MPI_Comm COMM_SHARED) {
    commID my_;
    ProcessConfig(my_, COMM_SHARED);
    if (Log::LogToFile()) {
        Log::Line('+');
        Log::Info("[FinalRound_range]", my_.global_size_);
    }
    uint64_t my_pre_sum = 0;
    uint64_t pre_rank_sum = 0;
    uint64_t mid_bound = 0;
    double rate = 0.0;
    uint64_t* prefix_sum_s2 = new uint64_t[my_.global_size_]{0};
    MPI_Scan(candi_s2, prefix_sum_s2, my_.global_size_, MPI_UINT64_T, MPI_SUM, COMM_SHARED);

    int j = 1;
    for (j = 1; j < my_.global_size_-1; j++) {
        if (b[j] > upper_bounds[j]) {
            if (candi_pivots[j - 1] + 1 < candi_pivots[j]) {
                sdispls[j] = candi_s2[j];
                continue;
            }
            my_pre_sum = prefix_sum_s2[j];
            mid_bound = lower_bounds[j] + (upper_bounds[j] - lower_bounds[j]) / 2;
            rate = (double)mid_bound / (double)b[j];
            sdispls[j] = candi_s2[j] * rate;
            if (sdispls[j] < sdispls[j - 1]) sdispls[j] = sdispls[j - 1];
        }
        else if (b[j] < lower_bounds[j]) {
            my_pre_sum = prefix_sum_s2[j];
            if (candi_pivots[j] + 1 < candi_pivots[j + 1]) {
                sdispls[j] = candi_s2[j];
                continue;
            }
            mid_bound = lower_bounds[j] + (upper_bounds[j] - lower_bounds[j]) / 2;
            rate = (double)mid_bound / (double)b[j];
            sdispls[j] = candi_s2[j] * rate;
            if (sdispls[j] > sdispls[j + 1]) sdispls[j] = sdispls[j + 1];
        }
        else continue;
    }
    if (j == my_.global_size_ - 1) {
        if (b[j] > upper_bounds[j]) {
            if (candi_pivots[j - 1] + 1 < candi_pivots[j]) {
                sdispls[j] = candi_s2[j];
            }
            else {
                my_pre_sum = prefix_sum_s2[j];
                pre_rank_sum = my_pre_sum - candi_s2[j];
                if (pre_rank_sum < lower_bounds[j]) {
                    mid_bound = lower_bounds[j] + (upper_bounds[j] - lower_bounds[j]) / 2;
                    rate = (double)mid_bound / (double)b[j];
                    sdispls[j] = candi_s2[j] * rate;
                    if (sdispls[j] < sdispls[j - 1]) sdispls[j] = sdispls[j - 1];
                    if (candi_pivots[j - 1] < candi_pivots[j]) {
                        if (sdispls[j] < candi_s2[j - 1]) {
                            sdispls[j] = candi_s2[j - 1];
                        }
                    }
                    else {//==
                        if (sdispls[j] < sdispls[j - 1]) {
                            sdispls[j] = sdispls[j - 1];
                        }
                    }
                }
                else { //pre_rank_sum >= lower
                    mid_bound = lower_bounds[j] + (upper_bounds[j] - lower_bounds[j]) / 2;
                    rate = (double)mid_bound / (double)b[j];
                    sdispls[j] = candi_s2[j] * rate;
                    if (sdispls[j] < sdispls[j - 1]) sdispls[j] = sdispls[j - 1];
                }
            }
        }
    }
};


#endif /* FinalRound_h */