//
//  LocalSort.h
//  ScaPSRS
//
//  Created by Estella Wong on 2021/11/9.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef LocalSort_h
#define LocalSort_h

#pragma once
#include <stdio.h>
#include <limits.h>
#include "utils/utils.h"

static int cmp(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
};
static int cmp2(const void* a, const void* b) {
    const int64_t p1 = *(const int64_t*)a;
    const int64_t p2 = *(const int64_t*)b;
    if (p1 < p2) return -1;
    else if (p1 > p2) return 1;
    else return 0;
};
static int log_base2_(int y) {
    int x = -1;
    while (y) {
        y = y >> 1;
        x++;
    } // 2^x <= y < 2^(x+1)
    return x;
};
template <typename T>
static bool isSorted(vector<T> data) {
    return is_sorted(data.begin(), data.end());
}

/* ---------- QuickSort ---------- */
/* only data */
// qsort(data, num, bytesize, cmp)
/* data with shift */
template <typename T1, typename T2>
static void QuickSort(vector<T1>& data, vector<T2>& shift) {
    int num = data.size() <= shift.size() ? data.size() : shift.size();
    if (num <= 0) return;
    vector<int> idx(num);
    for (int i = 0; i < num; i++) {
        idx[i] = i;
    }
    sort(idx.begin(), idx.end(), [&data](size_t i1, size_t i2){return data[i1]<data[i2];});
    vector<T1> temp_data(num); /* (temp) */
    vector<T2> temp_shift(num); /* (temp) */
    for (int i = 0; i < num; i++) {
        int id = idx[i];
        temp_data[i] = data[id];
        temp_shift[i] = shift[id];
    }
    data.swap(temp_data);
    shift.swap(temp_shift);
    idx.clear();
    temp_data.clear();
    temp_shift.clear();
};

/* ---------- MergeSort ---------- */
/* only data */
template <typename T>
static void Merge(vector<T>& data, int low, int mid, int high) {
    vector<T> tmp(data.begin()+low, data.begin()+high+1);
    int i = low;
    int j = mid + 1;
    int k = low;
    for (i = low, j = mid + 1, k = i; i <= mid && j <= high; k++) {
        if (tmp[i-low] <= tmp[j-low]) {
            data[k] = tmp[i-low];
            i++;
        }
        else {
            data[k] = tmp[j-low];
            j++;
        }
    }
    while (i <= mid) {
        data[k++] = tmp[i-low];
        i++;
    }
    while (j < high) {
        data[k++] = tmp[j-low];
        j++;
    }
}
template <typename T>
static void MergeSort(vector<T>& data, int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;
        MergeSort(data, low, mid);
        MergeSort(data, mid+1, high);
        Merge(data, low, mid, high);
    }
}
template <typename T>
static void MergeSort(vector<T>& data, int *displs, int ways) {
    int len = data.size();
    int rounds = log_base2_(ways)+1;
    int jump = 1;
    int next = 1;
    int low = displs[0];
    int mid = displs[0];
    int high = displs[ways - 1];
    for (int i = 1; i <= rounds; i++) {
        jump = (1 << i);
        for (int j = 0; j < ways; j = j + jump) {
            next = j + jump / 2;
            if (next < ways) {
                low = displs[j];
                mid = displs[next] - 1;
                if ((next + jump / 2) < ways) high = displs[next + jump / 2] - 1;
                else high = len - 1;
                Merge(data, low, mid, high);
            }
        }
    }
}
/* data with shift */
template <typename T1, typename T2>
static void Merge(vector<T1>& data, vector<T2>& shift, int low, int mid, int high) {
    vector<T1> tmp_data(data.begin()+low, data.begin()+high+1);
    vector<T2> tmp_shift(shift.begin()+low, shift.begin()+high+1);
    int i = low;
    int j = mid + 1;
    int k = low;
    for (i = low, j = mid + 1, k = i; i <= mid && j <= high; k++) {
        if (tmp_data[i-low] <= tmp_data[j-low]) {
            data[k] = tmp_data[i-low];
            shift[k] = tmp_shift[i-low];
            i++;
        }
        else {
            data[k] = tmp_data[j-low];
            shift[k] = tmp_shift[j-low];
            j++;
        }
    }
    while (i <= mid) {
        data[k] = tmp_data[i-low];
        shift[k++] = tmp_shift[i-low];
        i++;
    }
    while (j < high) {
        data[k] = tmp_data[j-low];
        shift[k++] = tmp_shift[j-low];
        j++;
    }
}
template <typename T1, typename T2>
static void MergeSort(vector<T1>& data, vector<T2>& shift, int *displs, int ways) {
    int len = data.size();
    int rounds = log_base2_(ways)+1;
    int jump = 1;
    int next = 1;
    int low = displs[0];
    int mid = displs[0];
    int high = displs[ways - 1];
    for (int i = 1; i <= rounds; i++) {
        jump = (1 << i);
        for (int j = 0; j < ways; j = j + jump) {
            next = j + jump / 2;
            if (next < ways) {
                low = displs[j];
                mid = displs[next] - 1;
                if ((next + jump / 2) < ways) high = displs[next + jump / 2] - 1;
                else high = len - 1;
                Merge(data, shift, low, mid, high);
            }
        }
    }
}
/* data with id,shift */
template <typename T1, typename T2>
static void Merge(vector<T1>& data, vector<T2>& id, vector<T2>& shift, int low, int mid, int high) {
    vector<T1> tmp_data(data.begin()+low, data.begin()+high+1);
    vector<T2> tmp_id(id.begin()+low, id.begin()+high+1);
    vector<T2> tmp_shift(shift.begin()+low, shift.begin()+high+1);
    int i = low;
    int j = mid + 1;
    int k = low;
    for (i = low, j = mid + 1, k = i; i <= mid && j <= high; k++) {
        if (tmp_data[i-low] <= tmp_data[j-low]) {
            data[k] = tmp_data[i-low];
            id[k] = tmp_id[i-low];
            shift[k] = tmp_shift[i-low];
            i++;
        }
        else {
            data[k] = tmp_data[j-low];
            id[k] = tmp_id[j-low];
            shift[k] = tmp_shift[j-low];
            j++;
        }
    }
    while (i <= mid) {
        data[k] = tmp_data[i-low];
        id[k] = tmp_id[i-low];
        shift[k++] = tmp_shift[i-low];
        i++;
    }
    while (j < high) {
        data[k] = tmp_data[j-low];
        id[k] = tmp_id[j-low];
        shift[k++] = tmp_shift[j-low];
        j++;
    }
}
template <typename T1, typename T2>
static void MergeSort(vector<T1>& data, vector<T2>& id, vector<T2>& shift, int *displs, int ways) {
    int len = data.size();
    int rounds = log_base2_(ways)+1;
    int jump = 1;
    int next = 1;
    int low = displs[0];
    int mid = displs[0];
    int high = displs[ways - 1];
    for (int i = 1; i <= rounds; i++) {
        jump = (1 << i);
        for (int j = 0; j < ways; j = j + jump) {
            next = j + jump / 2;
            if (next < ways) {
                low = displs[j];
                mid = displs[next] - 1;
                if ((next + jump / 2) < ways) high = displs[next + jump / 2] - 1;
                else high = len - 1;
                Merge(data, id, shift, low, mid, high);
            }
        }
    }
}
/* data in vector<pair> */
template <typename T1, typename T2>
static void Merge(vector< pair<T1, T2> >& data, int low, int mid, int high) {
    inplace_merge(data.begin() + low, data.begin() + mid, data.begin() + high);
}
template <typename T1, typename T2>
static void MergeSort(vector< pair<T1, T2> >& data, int *displs, int ways) {
    int len = data.size();
    int rounds = log_base2_(ways)+1;
    int jump = 1;
    int next = 1;
    int low = displs[0];
    int mid = displs[0];
    int high = displs[ways - 1];
    for (int i = 1; i <= rounds; i++) {
        jump = (1 << i);
        for (int j = 0; j < ways; j = j + jump) {
            next = j + jump / 2;
            if (next < ways) {
                low = displs[j];
                mid = displs[next];
                if ((next + jump / 2) < ways) high = displs[next + jump / 2];
                else high = len;
                Merge(data, low, mid, high);
            }
        }
    }
}


/* ---------- SortUnique ---------- */
template <typename T1>
vector<T1> SortUnique(vector<T1>& cur, bool need_sort = true, bool need_unique = true, bool need_print = false) {
    if (need_sort) {//排序
        sort(cur.begin(), cur.end());
    }
    if (need_unique) {//去重
        cur.erase(unique(cur.begin(), cur.end()), cur.end());
    }
    if (need_print) {
        Log::Vector1Detail(cur);
    }
    return cur;
};

template <typename T1, typename T2>
void SortWithIndex(vector<T1>& vec1, vector<T2>& vec2) {
    vector<int> idx;
    int vec_size = vec1.size() < vec2.size() ? vec1.size() : vec2.size();
    for (int i = 0; i < vec_size; i++) { //初始化idx
        idx.push_back(i);
    }
    sort(idx.begin(), idx.end(), [&vec1, &vec2](size_t i1, size_t i2){return (vec1[i1] == vec1[i2]) ? (vec2[i1] < vec2[i2]) : (vec1[i1] < vec1[i2]);});
    vector<T1> tmp1(vec_size);
    vector<T2> tmp2(vec_size);
    for (int i = 0; i < vec_size; i++) {
        tmp2[i] = vec2[idx[i]];
    }
    vec2.swap(tmp2);
    for (int i = 0; i < vec_size; i++) {
        tmp1[i] = vec1[idx[i]];
    }
    vec1.swap(tmp1);
    tmp1.clear();
    tmp2.clear();
};


template <typename T1, typename T2, typename T3>
static void SortEdgeFlagMultiWithIndex(vector<T1>& vec1, vector<T2>& vec2, vector<T3>& vec3) {
    vector<int> idx;
    int vec_size = vec1.size() < vec2.size() ? vec1.size() : vec2.size();
    for (int i = 0; i < vec_size; i++) { //初始化idx
        idx.push_back(i);
    }

    sort(idx.begin(), idx.end(), [&vec1, &vec2](size_t i1, size_t i2){return ((vec1[i1]>>2) == (vec1[i2]>>2)) ? ((vec2[i1] == vec2[i2]) ? (vec1[i1] < vec1[i2]) : (vec2[i1] > vec2[i2])) : (vec1[i1] < vec1[i2]);});

    vector<T1> tmp1(vec_size);        
    vector<T2> tmp2(vec_size);
    vector<T3> tmp3(vec_size);
    for (int i = 0; i < vec_size; i++) {
        tmp1[i] = vec1[idx[i]];
        tmp2[i] = vec2[idx[i]];
        tmp3[i] = vec3[idx[i]];
    }
    vec1.swap(tmp1);
    vec2.swap(tmp2);
    vec3.swap(tmp3);
    tmp1.clear();
    tmp2.clear();
    tmp3.clear();
};

#endif /* LocalSort_h */
