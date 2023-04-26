//
//  Timer.h
//  ScaPSRS
//
//  Created by Estella Wong on 2021/6/7.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef Timer_h
#define Timer_h

#include <sys/time.h>
#include <time.h>
struct SimpleTimer {
    timespec reset_time, start_time, finish_time;
    double time_elapsed, time_diff;
    
    void reset() {clock_gettime(CLOCK_REALTIME, &reset_time);time_elapsed=0;}
    void start() {clock_gettime(CLOCK_REALTIME, &start_time);}
    void stop() {
        clock_gettime(CLOCK_REALTIME, &finish_time);
        time_diff = finish_time.tv_sec - start_time.tv_sec + (double)(finish_time.tv_nsec-start_time.tv_nsec)/1000000000;
    }
    void end() {
        clock_gettime(CLOCK_REALTIME, &finish_time);
        time_elapsed = finish_time.tv_sec - reset_time.tv_sec + (double)(finish_time.tv_nsec-reset_time.tv_nsec)/1000000000;
    }
};
/*---------------------------------------*/
#include "mpi.h"
struct MpiTimer {
    double reset_time, start_time, finish_time;
    double time_elapsed, time_diff;
    
    void reset() {reset_time = MPI_Wtime(); time_elapsed=0;}
    void start() {start_time = MPI_Wtime();}
    void stop() {
        finish_time = MPI_Wtime();
        time_diff = finish_time - start_time;
    }
    void end() {
        finish_time = MPI_Wtime();
        time_elapsed = finish_time - reset_time;
    }
};
/*---------------------------------------*/
struct Timer_OneProcess {
    int work_rank{0};
    timespec reset_time, start_time, finish_time;
    double time_elapsed, time_diff;

    void set_work_rank(int rank) {
        work_rank = rank;
    }
    void reset() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        clock_gettime(CLOCK_REALTIME, &reset_time);
        time_elapsed=0;
    }
    void start() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        clock_gettime(CLOCK_REALTIME, &start_time);
    }
    void stop() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        clock_gettime(CLOCK_REALTIME, &finish_time);
        time_diff = finish_time.tv_sec - start_time.tv_sec + (double)(finish_time.tv_nsec-start_time.tv_nsec)/1000000000;
    }
    void end() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        clock_gettime(CLOCK_REALTIME, &finish_time);
        time_elapsed = finish_time.tv_sec - reset_time.tv_sec + (double)(finish_time.tv_nsec-reset_time.tv_nsec)/1000000000;
    }
};
/*---------------------------------------*/
struct MpiTimer_OneProcess {
    int work_rank{0};
    double reset_time, start_time, finish_time;
    double time_elapsed, time_diff;

    void set_work_rank(int rank) {
        work_rank = rank;
    }
    void reset() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        reset_time = MPI_Wtime();
        time_elapsed=0;
    }
    void start() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        start_time = MPI_Wtime();
    }
    void stop() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        finish_time = MPI_Wtime();
        time_diff = finish_time - start_time;
    }
    void end() {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if (my_rank != work_rank) return;
        finish_time = MPI_Wtime();
        time_elapsed = finish_time - reset_time;
    }
};
#endif /* Timer_h */
