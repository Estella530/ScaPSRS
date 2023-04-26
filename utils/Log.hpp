//
//  Log.hpp
//  ScaPSRS
//
//  Created by Estella Wong on 2022/3/14.
//  Copyright © 2022 Estella Wong. All rights reserved.
//

#ifndef Log_hpp
#define Log_hpp

#include <stdio.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <mutex>
#include <stdarg.h>

#include "mpi.h"

using namespace std;

class Log {
public:
    Log();
    ~Log();

        /// 获取当前日期
    string get_date();
        /// 获取当前时间
    string get_time();
    
private:
    static ostream *output;
    static ofstream ofs;

public:
    static void NewFile(string filename);
    static void ToFile(string filename);
    static void ToScreen();
    static void DebugModeOn();
    static void DebugModeOff();
    static void EndlModeOn();
    static void EndlModeOff();
    static void BlankModeOn();
    static void BlankModeOff();

    static void Line(int flag = 0) {
        string line(50, '-');
        if (flag == 1) line.assign(50, '=');
        else if (flag == 2) line.assign(50, '@');
        else if (flag == 3) line.assign(50, '#');
        else if (flag == 4) line.assign(50, '$');
        else if (flag == 5) line.assign(50, '~');
        else if (flag == 6) line.assign(50, '^');
        else if (flag == 7) line.assign(50, '+');
        else if (flag == 8) line.assign(50, '*');
        else if (flag == 9) line.assign(50, '.');
        *output << line << endl;
    }
    static void Line(char x) {
        string line(50, x);
        *output << line << endl;
    }
    static void Line_mpi(int masterId, int x = 0) {
        int myid;
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);
        if (myid == masterId) {
            Line(x);
        }
    }
    static void Line_mpi(int masterId, char x) {
        int myid;
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);
        if (myid == masterId) {
            Line(x);
        }
    }
    static void Line_title(string x) {
        int len = x.length();
        if (len >= 50) *output << x << endl;
        else {
            int emp1 = (50 - len) / 2;
            int emp2 = 50 - len - emp1;
            string line1(emp1, '-');
            string line2(emp2, '-');
            *output << line1 << x << line2 << endl;
        }
    }

    template <class T>
    static void Vector2Detail(vector<vector<T>> array, bool without_endl = true) {
        for (int i = 0; i < array.size(); i++) {
            if (without_endl) Log::EndlModeOff();
            Log::Info(" ");
            for (int j = 0; j < array[i].size(); j++) {
                Log::Info(array[i][j], ",");
            }
            if (without_endl) {
                Log::EndlModeOn();
                Log::Info();
            }
        }
    }
    template <class T>
    static void Vector1Detail(string title, vector<T> array, bool need_comma = true, bool without_endl = true) {
        if (without_endl) Log::EndlModeOff();
        Log::Info(title, ":");
        for (auto it : array) {
            Log::Info(it);
            if (need_comma) Log::Info(",");
        }
        if (without_endl) {
            Log::EndlModeOn();
            Log::Info();
        }
    }
    template <class T>
    static void Vector1Detail(vector<T> array, bool need_comma = true, bool without_endl = true) {
        if (without_endl) Log::EndlModeOff();
        Log::Info(" ");
        for (auto it : array) {
            Log::Info(it);
            if (need_comma) Log::Info(",");
        }
        if (without_endl) {
            Log::EndlModeOn();
            Log::Info();
        }
    }
    template <class T>
    static void Array1Detail(string title, int len, T *array, bool without_endl = true) {
        if (without_endl) Log::EndlModeOff();
        Log::Info(title, ":");
        for (int i = 0; i < len; i++) {
            Log::Info(array[i], ",");
        }
        if (without_endl) {
            Log::EndlModeOn();
            Log::Info();
        }
    }
    template <class T>
    static void Array1Detail(int len, T *array, bool without_endl = true) {
        if (without_endl) Log::EndlModeOff();
        Log::Info(" ");
        for (int i = 0; i < len; i++) {
            Log::Info(array[i], ",");
        }
        if (without_endl) {
            Log::EndlModeOn();
            Log::Info();
        }
    }
    template <class T>
    static void Array2Detail(int row, int col, T array, bool without_endl = true) {
        if (without_endl) Log::EndlModeOff();
        // Log::Info(" ");
        for (int i = 0; i < row; i++) {
            if (without_endl) Log::EndlModeOff();
            for (int j = 0; j < col; j++) {
                Log::Info(array[i][j], ",");
            }
            if (without_endl) {
                Log::EndlModeOn();
                Log::Info();
            }
        }
    }
    template <class T>
    static void Set1Detail(set<T> array, bool without_endl = true) {
        if (without_endl) Log::EndlModeOff();
        Log::Info(" ");
        for (auto it : array) {
            Log::Info(it, ",");
        }
        if (without_endl) Log::EndlModeOn();
        Log::Info();
    }
    template <class T1, class T2>
    static void VectorPairDetail(vector<pair<T1, T2>> array, bool without_endl = true) {
        for (int i = 0; i < array.size(); i++) {
            if (without_endl) Log::EndlModeOff();
            Log::Info(" ");
            Log::Info(array[i].first, ",", array[i].second);
            Log::Info(";");
        }
        if (without_endl) Log::EndlModeOn();
        Log::Info();
    }
    template <class T>
    static void VectorPairDetail(vector<vector<T>> array, bool without_endl = true) {
        for (int i = 0; i < array.size(); i++) {
            if (without_endl) Log::EndlModeOff();
            Log::Info(" ");
            for (int j = 0; j < array[i].size(); j++) {
                Log::Info(array[i][j], ",");
            }
            Log::Info(";");
        }
        if (without_endl) Log::EndlModeOn();
        Log::Info();
    }
    template <class T>
    static void Map1Detail(map<T,T> map, bool without_endl = true) {
        for (auto it : map) {
            if (without_endl) Log::EndlModeOff();
            Log::Info(" ");
            Log::Info(it.first, "->", it.second);
            Log::Info(";");
        }
        if (without_endl) Log::EndlModeOn();
        Log::Info();
    }
    template <class T>
    static void Map2Detail(map<vector<T>, T> map, bool without_endl = true) {
        for (auto it : map) {
            if (without_endl) Log::EndlModeOff();
            Log::Info(" ");
            for (int j = 0; j < it.first.size(); j++) {
                Log::Info(it.first[j], ",");
            }
            Log::Info("->", it.second);
            Log::Info(";");
        }
        if (without_endl) Log::EndlModeOn();
        Log::Info();
    }
    template <class T>
    static void Vector1Detail_mpi(int masterId, vector<T> array, bool need_comma = true, bool without_endl = true) {
        int myid;
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);
        if (myid == masterId) {
            if (without_endl) Log::EndlModeOff();
            Log::Info(" ");
            for (auto it : array) {
                Log::Info(it);
                if (need_comma) Log::Info(",");
            }
            if (without_endl) {
                Log::EndlModeOn();
                Log::Info();
            }
        }
    }
    

    template<class FIRST, class ...PACK>
    static void Info(FIRST first, PACK... params) {
        if (init_output_) {
            *output << "\033[32m[Info]:\033[0m" << " ";
            init_output_ = false;
        }
        *output << first;
        if (blank_mode_) *output << " ";
        Info(params...);
    }
    template <class T>
    static void Info(T msg) {
        init_output_ = true;
        if (!endl_mode_) init_output_ = false;
        *output << msg;
        if (endl_mode_) *output << endl;
        else if (blank_mode_) *output << " ";
    }
    static void Info() {
        init_output_ = true;
        if (!endl_mode_) init_output_ = false;
        if (endl_mode_) *output << endl;
        else if (blank_mode_) *output << " ";
    }
    
    template<class FIRST, class ...PACK>
    static void Debug(FIRST first, PACK... params) {
        if (!debug_mode_) return;
        if (init_output_) {
            *output << "\033[34m[Debug]:\033[0m" << " ";
            init_output_ = false;
        }
        *output << first;
        if (blank_mode_) *output << " ";
        Info(params...);
    }
    template <class T>
    static void Debug(T msg) {
        if (!debug_mode_) return;
        init_output_ = true;
        *output << msg;
        if (endl_mode_) *output << endl;
        else if (blank_mode_) *output << " ";
    }
    static void Debug() {
        if (!debug_mode_) return;
        init_output_ = true;
        if (!endl_mode_) init_output_ = false;
        if (endl_mode_) *output << endl;
        else if (blank_mode_) *output << " ";
    }
    
    template<class FIRST, class ...PACK>
    static void Warn(FIRST first, PACK... params) {
        if (init_output_) {
            *output << "\033[33m[Warning]:\033[0m" << " ";   // print yellow style
            init_output_ = false;
        }
        *output << first;
        if (blank_mode_) *output << " ";
        Info(params...);
    }
    template <class T>
    static void Warn(T msg) {
        init_output_ = true;
        if (!endl_mode_) init_output_ = false;
        *output << msg;
        if (endl_mode_) *output << endl;
        else if (blank_mode_) *output << " ";
    }

    template<class FIRST, class ...PACK>
    static void Info_mpi(int masterId, FIRST first, PACK... params) {
        int myid;
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);
        if (myid == masterId) {
            Info(first, params...);
        }
    }
    
private:
    static bool log_to_file_;
    static bool init_output_;
    static bool debug_mode_;
    static bool endl_mode_;
    static bool blank_mode_;

public:
    static bool DebugMode();
    static bool LogToFile();
};

//date       time     - rank  path         : line - detail
//2020-04-24 12:00:00 - INFO  test/log.cpp : 100  - Logging....

#endif /* Log_hpp */
