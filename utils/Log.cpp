//
//  Log.cpp
//  ScaPSRS
//
//  Created by Estella Wong on 2022/3/14.
//  Copyright © 2022 Estella Wong. All rights reserved.
//

#include "Log.hpp"

Log::Log() {
}
Log::~Log() {
    if (log_to_file_) ofs.close();
}

string Log::get_date() {
    static char str_time[20];
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    strftime(str_time, sizeof(str_time), "%Y-%m-%d", now);
    stringstream ss;
    ss << str_time;
    return ss.str();
}
string Log::get_time() {
    static char str_time[10];
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    strftime(str_time, sizeof(str_time), "%H:%M:%S", now);
    stringstream ss;
    ss << str_time;
    return ss.str();
}

void Log::NewFile(string filename) {
    ofs.open(filename, ios::trunc); //如果文件已存在则先删除该文件
    output = (ostream*)&(ofs);
    log_to_file_ = true;
}
void Log::ToFile(string filename) {
    ofs.open(filename, ios::app);
    output = (ostream*)&(ofs);
    log_to_file_ = true;
}
void Log::ToScreen() {
    ofs.close();
    output = (ostream*)&(cout);
    log_to_file_ = false;
}
void Log::DebugModeOn() {
    debug_mode_ = true;
}
void Log::DebugModeOff() {
    debug_mode_ = false;
}
void Log::EndlModeOn() {
    endl_mode_ = true;
}
void Log::EndlModeOff() {
    endl_mode_ = false;
    init_output_ = true;
}
void Log::BlankModeOn() {
    blank_mode_ = true;
}
void Log::BlankModeOff() {
    blank_mode_ = false;
}

bool Log::log_to_file_ = false;
bool Log::init_output_ = true;
bool Log::debug_mode_ = false;
bool Log::endl_mode_ = true;
bool Log::blank_mode_ = true;
ostream* Log::output = (ostream*)&(cout);
ofstream Log::ofs;

bool Log::DebugMode() {
    return debug_mode_;
}
bool Log::LogToFile() {
    return log_to_file_;
}