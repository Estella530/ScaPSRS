//
//  ProcessConfig.hpp
//  ScaPSRS
//
//  Created by Estella Wong on 2021/6/7.
//  Copyright © 2021 Estella Wong. All rights reserved.
//

#ifndef ProcessConfig_hpp
#define ProcessConfig_hpp

#pragma once
#include <stdio.h>
#include <iostream>
#include "mpi.h"
#include "CountsDispls.h"

struct myID {
    int node_id_{0}; //the id of this node
    int node_num_{0}; //the number of nodes
    
    int global_rank_{0}; //the mpi rank id in MPI_COMM_WORLD
    int global_size_{0}; //the number of mpi process in MPI_COMM_WORLD
    int local_rank_{0}; //the mpi ank id in this node (COMM_SHARED)
    int local_size_{16}; //the number of mpi process in this node (COMM_SHARED)
    int max_local_size_{1}; //the max number of mpi processes in one node, is used to compute the number of nodes
};

    /// Set the node id, local process id of the caller process.
void ProcessConfig(myID& my_);
void ProcessConfig(myID& my_, MPI_Comm& COMM_SHARED);
void ProcessConfig(myID& my_, MPI_Comm& COMM_SHARED, MPI_Comm& NODE_FIRST);

struct commID {
    int global_rank_{0}; //the mpi rank id in MPI_COMM_WORLD
    int global_size_{0}; //the number of mpi process in MPI_COMM_WORLD
};
void ProcessConfig(commID& my_, MPI_Comm& COMM_SHARED);

struct myID_group {
    int group_id_{0}; //the id of the group which this process in
    int group_num_{0}; //the number of groups
    
    int global_rank_{0}; //the mpi rank id in MPI_COMM_WORLD
    int global_size_{0}; //the number of mpi process in MPI_COMM_WORLD

    int local_rank_{0}; //the mpi rank id in this group (COMM_GROUP)
    int local_size_{1}; //the number of mpi process in this group (COMM_GROUP)
};

void ProcessConfig_group(int group_num, myID_group& my_, MPI_Comm& COMM_GROUP, MPI_Comm& COMM_ALIGNED);


#endif /* ProcessConfig_hpp */
