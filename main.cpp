//
//  main.cpp
//  ScaPSRS
//
//  Created by Estella Wong on 2023/4/25.
//  Copyright © 2023 Estella Wong. All rights reserved.
//
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mpi.h>
#include <random>

#pragma once
#include "utils/utils.h"
#include "utils/mpi_utils.h"
#include "utils/Log.hpp"
#include "utils/ProcessConfig.hpp"
#include "ScaPSRS.hpp"
#include "ScaPSRS.cpp"

using namespace std;

static uint64_t data_max = 1ul<<63;
static uint32_t normal_base = 1ul<<31;

struct Options {
int local_n{1};
int test_data_version{0};
double imba{0.1};
string gen{"rand"}; //creating data
string sort_method{"psrs"};
string out_dir;
}opt;

void Option(int argc, const char * argv[]) {
    if (argc <= 1) return;
    
    int i = 1;
    while(i < argc) {
        if (strcmp(argv[i], "-n") == 0) {
            opt.local_n = atoi(argv[++i]);
            Log::Info_mpi(0, "local_n:", opt.local_n);
        }
        else if (strcmp(argv[i], "-a") == 0) {
            opt.test_data_version = atoi(argv[++i]);
            Log::Info_mpi(0, "test_data_version:", opt.test_data_version);
        }
        else if (strcmp(argv[i], "-i") == 0) {
            opt.imba = atof(argv[++i]);
            Log::Info_mpi(0, "imba:", opt.imba);
        }
        else if (strcmp(argv[i], "-g") == 0) {//the name of creating data, default: random
            opt.gen = argv[++i];
            Log::Info_mpi(0, "gen_data:", opt.gen);
        }
        else if (strcmp(argv[i], "-sm") == 0) {
            opt.sort_method = argv[++i];
            Log::Info_mpi(0, "sort_method:", opt.sort_method);
        }
        else if (strcmp(argv[i], "-o") == 0) {//the output directory
            opt.out_dir = argv[++i];
            if (opt.out_dir.back() != '/') opt.out_dir += '/';
            Log::Info_mpi(0, "out_dir:", opt.out_dir);
            may_mkdir_mpi(0, opt.out_dir);      //如果out_dir不存在，则创建out_dir目录
        }
        else if (strcmp(argv[i], "-debugmode") == 0) {
            Log::DebugModeOn();
            Log::Info_mpi(0, "debug mode:", "ON");
        }
        i++;
    }
};

string InitOutPutSubDir() {
    static char str_time[20];
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    strftime(str_time, sizeof(str_time), "%Y%m%d_%H%M%S", now);
    stringstream ss;
    ss << str_time;
        //初始化输出路径
    string out_subdir = opt.out_dir + ss.str() + "/"; //给out_subdir加上时间后缀
    Log::Info("out_subdir:", out_subdir);
    may_mkdir(out_subdir);   //如果out_subdir不存在，则创建
        //初始化 log_file
    string log_file = out_subdir + "log.md";
    Log::NewFile(log_file); //新建log文件，打印到文件
    Log::Info("[TestToFile]", "log_file:", log_file);
    return out_subdir;
};

template <typename T>
void CreateData_Random(vector<T>& data_buf, int offset, int num, uint32_t seed = time(0)) {
    srand(seed);
    uint64_t data_max = INT64_MAX;
    for (int i = 0; i < num; i++) {
        data_buf[i + offset] = rand() % data_max;
    }
};
template <typename T>
void CreateData_Uniform_Random(vector<T>& data_buf, int offset, int num, T base, uint32_t seed = time(0)) {
    default_random_engine gen(seed);
    uniform_int_distribution<T> dis(0, base);
    for (int i = 0; i < num; i++) {
        double it = dis(gen);
        data_buf[i + offset] = it;
    }
};
template <typename T>
void CreateData_Normal_Random(vector<T>& data_buf, int offset, int num, T base, uint32_t seed = time(0)) {
    default_random_engine gen(seed);
    normal_distribution<double> dis(0, 1);
    for (int i = 0; i < num; i++) {
        double it = dis(gen);
        data_buf[i + offset] = it * base;
    }
};
template <typename T>
void CreateData_Exponential_Random(vector<T>& data_buf, int offset, int num, T base, uint32_t seed = time(0)) {
    default_random_engine gen(seed);
    exponential_distribution<double> dis(1.2);
    for (int i = 0; i < num; i++) {
        double it = dis(gen);
        data_buf[i + offset] = base * it;
    }
};

template <typename T>
void DataInput(vector<T>& data_buf, uint32_t seed = time(0)) {
    int j = 0;
    if (j >= opt.local_n) return;
    Log::Info_mpi(0, "Creating data, by", opt.gen);
    if (opt.gen == "uniform") CreateData_Uniform_Random(data_buf, j, opt.local_n - j, data_max, seed);
    if (opt.gen == "normal") CreateData_Normal_Random(data_buf, j, opt.local_n - j, data_max, seed);
    if (opt.gen == "exp") CreateData_Exponential_Random(data_buf, j, opt.local_n - j, data_max, seed);
    else CreateData_Random(data_buf, j, opt.local_n - j, seed);
};

int main(int argc, const char * argv[]) {
    MPI_Init(NULL, NULL);
    myID my_;
    ProcessConfig(my_);
    Option(argc, argv); //获取option

    //初始化变量
    SimpleTimer timer; //初始化时间变量
    timer.reset();
    timer.start();
    vector<uint64_t> local_data;
    local_data.resize(opt.local_n, 0ul);
    Log::Info_mpi(0, "local_size:", local_data.size());
    uint32_t seed = time(0) + my_.global_rank_;
    DataInput(local_data, seed); //获取待排序的数据
    timer.stop();
    MPI_Barrier(MPI_COMM_WORLD);
    Log::Info_mpi(0, "Time is", timer.time_diff, "s. (InputPart)");
    
    timer.start();
    ScaPSRS<uint64_t> scapsrs(local_data, MPI_COMM_WORLD, opt.imba);
    timer.stop();
    Log::Info_mpi(0, "Time is", timer.time_diff, "s. (PS-Part)");
    timer.end();
    Log::Info_mpi(0, "time is", timer.time_elapsed, "s(wholePart).");
    FreeUpCompletely(local_data);
    MPI_Finalize();
    return 0;
}