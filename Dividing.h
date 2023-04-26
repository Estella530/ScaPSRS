//
//  Dividing.h
//  ScaPSRS
//
//  Created by Estella Wong on 2022/11/1.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef Dividing_h
#define Dividing_h

#include <stdio.h>
#include <vector>
#include <string>
#include "utils/Log.hpp"

///这是最简单的划分 (二分查找)
///没有重复pivots
template <typename T1, typename T2>
static void DivideByPivots(T1 *data, int begin, int end, T1 *pivots, int pl, int pr, T2 *displs, int displs_size) {
    if (end == 0) return;
    int left = begin;
    int right = end;
    int mid = left + (right - left) / 2;
    int pm = pl + (pr - pl) / 2;

    while (left <= right) {
        mid = left + (right - left) / 2;
        if (data[mid] > pivots[pm]) {
            if (mid == 0) break;
            else if (data[mid - 1] <= pivots[pm]) break;
            else right = mid - 1;
        }
        else left = mid + 1;
    }
    if (pm + 1 < displs_size) displs[1 + pm] = mid;

    if (pl < pm)
        DivideByPivots(data, begin, mid, pivots, pl, pm - 1, displs, displs_size);
    if (pm < pr)
        DivideByPivots(data, mid, end, pivots, pm + 1, pr, displs, displs_size);
};

    ///@param data sorted
    ///@return the last pos, which key is equal to that at pos
template <typename T>
static int FindDuplicated_Fwd(T* data, int size, int pos) {
    T key = data[pos];
    int mid = pos + 1;
    if (mid >= size) return pos;
    if (data[mid] != key) return pos;
    int left = pos;
    int right = size - 1;
    while (left <= right) {
        mid = left + (right - left) / 2;
        //data[mid] must be >= key
        if (data[mid] > key) {
            right = mid - 1;
            continue;
        }
        else left = mid + 1;
    }
    if (mid >= size) return mid - 1;
    if (data[mid] == key) return mid;
    else return mid - 1;
};
template <typename T1>
static int FindDuplicated_Fwd(pair<T1, T1> *data, int size, int pos) {
    T1 key = data[pos].first;
    int mid = pos + 1;
    if (mid >= size) return pos;
    if (data[mid].first != key) return pos;
    int left = pos;
    int right = size - 1;
    while (left <= right) {
        mid = left + (right - left) / 2;
        //data[mid] must be >= key
        if (data[mid].first > key) {
            right = mid - 1;
            continue;
        }
        else left = mid + 1;
    }
    if (mid >= size) return mid - 1;
    if (data[mid].first == key) return mid;
    else return mid - 1;
};
    ///@param data sorted
    ///@return the first pos, which key is equal to that at pos
template <typename T>
static int FindDuplicated_Bwd(T* data, int pos) {
    T key = data[pos];
    int mid = pos - 1;
    if (mid < 0) return pos;
    if (data[mid] != key) return pos;
    int left = 0;
    int right = pos;
    while (left <= right) {
        mid = left + (right - left) / 2;
        //data[mid] must be <= key
        if (data[mid] < key) {
            left = mid + 1;
            continue;
        }
        else right = mid - 1;
    }
    if (mid < 0) return 0;
    if (data[mid] == key) return mid;
    else return mid + 1;
};
template <typename T1>
static int FindDuplicated_Bwd(pair<T1, T1> *data, int pos) {
    T1 key = data[pos].first;
    int mid = pos - 1;
    if (mid < 0) return pos;
    if (data[mid].first != key) return pos;
    int left = 0;
    int right = pos;
    while (left <= right) {
        mid = left + (right - left) / 2;
        //data[mid] must be <= key
        if (data[mid].first < key) {
            left = mid + 1;
            continue;
        }
        else right = mid - 1;
    }
    if (mid < 0) return 0;
    if (data[mid].first == key) return mid;
    else return mid + 1;
};
    ///@param data sorted
    ///@return the number of same keys, which are equal to that at pos
template <typename T>
static int FindDuplicated(T* data, int size, int pos, int &lpos, int &hpos) {
    lpos = FindDuplicated_Bwd(data, pos);
    hpos = FindDuplicated_Fwd(data, size, pos);
    return hpos - lpos + 1;
};

///二分
template <typename T1, typename T2>
static void DivideByPivots_duplicated_2(T1 *data, int data_num, int begin, int end, T1 *pivots, int pivots_num, int pl, int pr, T2 *displs, int displs_size) {
    if (end < 0) return;
    if (displs_size == 0) return;
    int left = begin;
    int right = end;
    int mid = left;
    int left_same = left;
    int pm = pl + (pr - pl) / 2;

    while (left <= right) {
        mid = left + (right - left) / 2;
        if (data[mid] > pivots[pm]) {
            if (mid == 0) break;
            else if (data[mid - 1] <= pivots[pm]) break;
            else right = mid - 1;
        }
        else {//<=
            if (data[left_same] != data[mid]) left_same = mid;
            left = mid + 1;
        }
    }
    if (data[mid] <= pivots[pm]) {
        mid++;
    }
    if (data[left_same] < data[mid-1]) left_same = mid-1;
    left_same = FindDuplicated_Bwd(data, left_same);
    int ldp = 0; //left duplicated pivot
    int rdp = 0; //right duplicated pivot
    int dup_num = FindDuplicated(pivots, pivots_num, pm, ldp, rdp);
    int mid_per = 0;
    if (dup_num == 1) {
        if (pm + 1 < displs_size) {
            displs[pm + 1] = mid;
        }
        if (pl < pm) {
            if (mid > end) mid = end;
            DivideByPivots_duplicated_2(data, data_num, begin, mid, pivots, pivots_num, pl, pm - 1, displs, displs_size);
        }
        if (pm < pr) {
            DivideByPivots_duplicated_2(data, data_num, mid, end, pivots, pivots_num, pm + 1, pr, displs, displs_size);
        }
    }
    else { //duplicated pivots do exist
        displs[ldp] = left_same;
        if (mid + 1 >= data_num) dup_num++;
        mid_per = (mid - left_same) / dup_num;
        for (int i = 1; i < dup_num; i++) {
            displs[ldp + i] = mid_per * i + left_same;
        }
        if (mid + 1 < data_num) {
            if (rdp + 1 < displs_size) {
                displs[rdp + 1] = mid;
            }
        }
        if (pl < ldp) {
            DivideByPivots_duplicated_2(data, data_num, begin, left_same, pivots, pivots_num, pl, ldp - 1, displs, displs_size);
        }
        if (pr > rdp) {
            DivideByPivots_duplicated_2(data, data_num, mid, end, pivots, pivots_num, rdp + 1, pr, displs, displs_size);
        }
    }
};

template <typename T1, typename T2, typename T3>
static void DivideByPivots_duplicated_2(pair<T1, T1> *data, T3 data_num, T3 begin, T3 end, T1 *pivots, int pivots_num, int pl, int pr, T2 *displs, int displs_size) {
    if (end < 0) return;
    if (displs_size == 0) return;
    T3 left = begin;
    T3 right = end;
    T3 mid = left;
    T3 left_same = left;
    int pm = pl + (pr - pl) / 2;

    while (left <= right) {
        mid = left + (right - left) / 2;
        if (data[mid].first > pivots[pm]) {
            if (mid == 0) break;
            else if (data[mid - 1].first <= pivots[pm]) break;
            else right = mid - 1;
        }
        else {//<=
            if (data[left_same].first != data[mid].first) left_same = mid;
            left = mid + 1;
        }
    }
    if (data[mid].first <= pivots[pm]) {
        mid++;
    }
    if (data[left_same].first < data[mid-1].first) left_same = mid-1;
    left_same = FindDuplicated_Bwd(data, left_same);
    int ldp = 0; //left duplicated pivot
    int rdp = 0; //right duplicated pivot
    int dup_num = FindDuplicated(pivots, pivots_num, pm, ldp, rdp);
    T3 mid_per = 0;
    if (dup_num == 1) {
        if (pm + 1 < displs_size) {
            displs[pm + 1] = mid;
        }
        if (pl < pm) {
            if (mid > end) mid = end;
            DivideByPivots_duplicated_2(data, data_num, begin, mid, pivots, pivots_num, pl, pm - 1, displs, displs_size);
        }
        if (pm < pr) {
            DivideByPivots_duplicated_2(data, data_num, mid, end, pivots, pivots_num, pm + 1, pr, displs, displs_size);
        }
    }
    else { //duplicated pivots do exist
        displs[ldp] = left_same;
        if (mid + 1 >= data_num) dup_num++;
        mid_per = (mid - left_same) / dup_num;
        for (int i = 1; i < dup_num; i++) {
            displs[ldp + i] = mid_per * i + left_same;
        }
        if (mid + 1 < data_num) {
            if (rdp + 1 < displs_size) {
                displs[rdp + 1] = mid;
            }
        }
        if (pl < ldp) {
            DivideByPivots_duplicated_2(data, data_num, begin, left_same, pivots, pivots_num, pl, ldp - 1, displs, displs_size);
        }
        if (pr > rdp) {
            DivideByPivots_duplicated_2(data, data_num, mid, end, pivots, pivots_num, rdp + 1, pr, displs, displs_size);
        }
    }
}


#endif /* Dividing_h */
