//
//  Sampling.h
//  ScaPSRS
//
//  Created by Estella Wong on 2022/11/1.
//  Copyright © 2022 Estella Wong. All rights reserved.
//

#ifndef Sampling_h
#define Sampling_h

#include <stdio.h>
#include <vector>
#include <string>
#include "utils/Log.hpp"

/* ---------- RegularSampling ---------- */
template <typename T>
static void RegularSamplingWithOffset(int data_num, int samples_num, int quotient, int offset, T *data, T *samples) {
    for (int i = 0; i < samples_num; i++) {
        if (offset < data_num) samples[i] = data[offset];
        else samples[i] = data[data_num-1];
        offset += quotient;
    }
};
template <typename T>
static void RegularSamplingWithOffset(int data_num, int samples_num, int quotient, int offset, pair<T, T> *data, T *samples) {
    for (int i = 0; i < samples_num; i++) {
        if (offset < data_num) samples[i] = data[offset].first;
        else samples[i] = data[data_num-1].first;
        offset += quotient;
    }
};
template <typename T>
static void RegularSamplingWithOffset(int data_num, int samples_num, int quotient, int offset, vector< pair<T, T> >& data, vector<T>& samples) {
    for (int i = 0; i < samples_num; i++) {
        if (offset < data_num) samples.at(i) = data.at(offset).first;
        else samples.at(i) = data.back().first;
        offset += quotient;
    }
};
template <typename T>
static void RegularSampling(int data_num, int samples_num, int quotient, T *data, T *samples) {
    RegularSamplingWithOffset(data_num, samples_num, quotient, 0, data, samples);
};
template <typename T>
static void RegularSampling(int data_num, int samples_num, int quotient, pair<T, T> *data, T *samples) {
    RegularSamplingWithOffset(data_num, samples_num, quotient, 0, data, samples);
};
template <typename T>
static void RegularSampling(int data_num, int samples_num, int quotient, vector< pair<T, T> >& data, vector<T>& samples) {
    RegularSamplingWithOffset(data_num, samples_num, quotient, 0, data.data(), samples);
};

#endif /* Sampling_h */
