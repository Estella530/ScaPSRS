//
//  utils.h
//  ScaPSRS
//
//  Created by Estella Wong on 2020/5/29.
//  Copyright © 2020 Estella Wong. All rights reserved.
//

#ifndef utils_h
#define utils_h

#pragma once
#include <string.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>
#include <map>

#include "CountsDispls.h"
#include "Log.hpp"
#include "mpi_utils.h"
#include "ProcessConfig.hpp"
#include "Timer.h"

using namespace std;

template <typename T>
static void FreeUpCompletely(vector<T>& vec) {
    vec.clear();
    vec.shrink_to_fit();
};

/*---------------------------------------*/
#include <sys/stat.h>
#include <unistd.h>
    /// Create if the directory does not exist
    /// @param dir_name the name of the directory
static void may_mkdir(string dir_name) {
    if (access(dir_name.c_str(), 0) == -1) {
        Log::Info("creating dir_name:", dir_name.c_str());
        mkdir(dir_name.c_str(), 00755);
    }
};
static void may_mkdir_mpi(int master_rank, string dir_name) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == master_rank) {
        may_mkdir(dir_name);
    }
};

static long get_file_size(const char * filename) {
    struct stat statbuf;
    int ret = stat(filename, &statbuf);
    if (ret != 0) return -1;
    return statbuf.st_size;
};
static long get_file_size(string filename) {
    return get_file_size(filename.c_str());
};

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

/*---------------------------------------*/
/* binary search */
template <typename T1, typename T2>
static T1 BinarySearch_min_left(T2 target, T1 arrlen, T2 *arr) {
    if (arrlen == 0) return -1;
    T1 left = 0;
    T1 right = arrlen - 1;
    T2 mid = 0;
    while (left <= right) {
        mid = left + (T1)((right - left) / 2);
        if (arr[mid] == target) {
            while (mid > 0) {
                if (arr[mid-1] < target) return mid;
                else mid--;
            }
            return mid;
        }
        else if (arr[mid] > target) {
            if (mid == 0) return mid;
            else if (arr[mid-1] < target) {
                if (arr[mid] == target) return mid;
                return mid-1;
            }
            else right = mid - 1;
        }
        else left = mid + 1;
    }
    return mid;
};
template <typename T1, typename T2>
static T1 BinarySearch(T2 target, T1 arrlen, T2 *arr) {
    if (arrlen == 0) return -1;
    T1 left = 0;
    T1 right = arrlen - 1;
    T2 mid = 0;
    while (left <= right) {
        mid = left + (T1)((right - left) / 2);
        if (arr[mid] <= target) {
            if (mid == arrlen - 1) return mid;
            else if (arr[mid+1] > target) return mid;
            else left = mid +1;
        }
        else right = mid -1;
    }
    return mid;
};

template <typename T1, typename T2>
static T1 BinarySearch(T2 target, T1 arrlen, T1 left, T1 right, T2 *arr) {
    if (arrlen == 0) return -1;
    T1 mid = 0;
    while (left <= right) {
        mid = left + (T1)((right - left) / 2);
        if (arr[mid] <= target) {
            if (mid == arrlen - 1) return mid;
            else if (arr[mid + 1] > target) return mid;
            else left = mid + 1;
        }
        else right = mid -1;
    }
    return mid;
};

template <typename T1, typename T2, typename T3>
static T1 BinarySearch(T2 target, T3 tnum, T1 arrlen, T1 left, T1 right, T2 *arr, T3 *num_arr) {
    if (arrlen == 0) return -1;
    T1 mid = 0;
    while (left <= right) {
        mid = left + (T1)((right - left) / 2);
        if ((arr[mid]>>2) == (target>>2)) {
            if (num_arr[mid] == tnum) {
                if (arr[mid] <= target) {
                    if (mid == arrlen - 1) return mid;
                    else if (arr[mid + 1] > target) return mid;
                    else left = mid + 1;
                }
                else right = mid - 1;
            }
            else if (num_arr[mid] > tnum) {
                if (mid == arrlen - 1) return mid;// else if (arr[mid + 1] > target) return mid;
                else left = mid + 1;
            }
            else right = mid - 1;
        }
        else if (arr[mid] < target) {
            if (mid == arrlen - 1) return mid;
            else if (arr[mid + 1] > target) return mid;
            else left = mid + 1;
        }
        else right = mid - 1;
    }
    return mid;
};

template <typename T1, typename T2>
static T1 find_max_right(T2 *arr, T1 len, T2 target) {
    T1 left = 0;
    T1 right = len-1;
    T1 max_right = 0;
    while (left <= right) { //binary search
        max_right = left + (T1)((right-left)/2);
        if (arr[max_right] <= target) {
            if (max_right == len-1) break;
            else if (arr[max_right+1] > target) break;
            else left = max_right + 1;
        }
        else right = max_right - 1;
    }
    return max_right;
};

///arr is increasing sorted
template <typename T1, typename T2>
static T1 scan_fwd(T2 target, T1 lpos, T1 rpos, T2 *arr) {
    while (lpos <= rpos) {
        if (arr[lpos] < target) lpos++;
        else return lpos;
    }
    return rpos;
};
///arr is increasing sorted
template <typename T1, typename T2>
static T1 scan_bwd(T2 target, T1 lpos, T1 rpos, T2 *arr) {
    while (lpos <= rpos) {
        if (arr[rpos] > target) rpos--;
        else return rpos;
    }
    return lpos;
};


#endif /* utils_h */
