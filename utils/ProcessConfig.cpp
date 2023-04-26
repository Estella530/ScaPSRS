//
//  ProcessConfig.cpp
//  ScaPSRS
//
//  Created by Estella Wong on 2021/6/7.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#include "ProcessConfig.hpp"


void ProcessConfig(myID& my_) {
    MPI_Comm_rank(MPI_COMM_WORLD ,&my_.global_rank_);
    MPI_Comm_size(MPI_COMM_WORLD, &my_.global_size_);
}

void ProcessConfig(myID& my_, MPI_Comm& COMM_SHARED) {
    ProcessConfig(my_);
        // create the per-node communicator
    COMM_SHARED = MPI_COMM_NULL;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &COMM_SHARED);
    MPI_Comm_rank(COMM_SHARED, &my_.local_rank_);
    MPI_Comm_size(COMM_SHARED, &my_.local_size_);
    MPI_Allreduce(&my_.local_size_, &my_.max_local_size_, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    
    my_.node_id_ = my_.global_rank_ / my_.local_size_;
    my_.node_num_ = (my_.global_size_ + my_.max_local_size_ - 1) / my_.max_local_size_;
}

void ProcessConfig(myID& my_, MPI_Comm& COMM_SHARED, MPI_Comm& NODE_FIRST) {
    ProcessConfig(my_, COMM_SHARED);
        //create the node-first-process communicator
    NODE_FIRST = MPI_COMM_NULL;
    MPI_Comm_split(MPI_COMM_WORLD, my_.local_rank_, my_.node_id_, &NODE_FIRST);
}

void ProcessConfig(commID& my_, MPI_Comm& COMM_SHARED) {
    MPI_Comm_rank(COMM_SHARED ,&my_.global_rank_);
    MPI_Comm_size(COMM_SHARED, &my_.global_size_);
}

void ProcessConfig_group(int group_num, myID_group& my_, MPI_Comm& COMM_GROUP, MPI_Comm& COMM_ALIGNED) {
    MPI_Comm_rank(MPI_COMM_WORLD ,&my_.global_rank_);
    MPI_Comm_size(MPI_COMM_WORLD, &my_.global_size_);

    int *color_t; //group_id
    int *key_t; //id_in_group
    color_t = new int[my_.global_size_]{0};
    key_t = new int[my_.global_size_]{0};
    init_color_key(color_t, key_t, my_.global_size_, group_num);
    my_.group_id_ = color_t[my_.global_rank_];
    my_.group_num_ = group_num;
    my_.local_rank_ = key_t[my_.global_rank_];
    my_.local_size_ = my_.global_size_ / my_.group_num_;
    int remainder = my_.global_size_ % my_.group_num_;
    if (my_.group_id_ < remainder) my_.global_size_ += 1;

    MPI_Comm_split(MPI_COMM_WORLD, my_.group_id_, my_.local_rank_, &COMM_GROUP);
    MPI_Comm_split(MPI_COMM_WORLD, my_.local_rank_, my_.group_id_, &COMM_ALIGNED);

    delete [] color_t;
    delete [] key_t;
}
