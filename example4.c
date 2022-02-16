/*
//@HEADER
// ************************************************************************
//
//                        Partix 1.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY NTESS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NTESS OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Jan Ciesko (jciesko@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#include "mpi.h"
#include <stdlib.h>

#include <partix.h>
#include <thread.h>

/* My task args */
typedef struct {
  MPI_Request * request;
  int recv_partitions;
} task_args_t;

void send_task(partix_task_args_t *args) {
  task_args_t * task_args = args->user_task_args;
  MPI_Pready(args->taskId, task_args->request);
};

void recv_task(partix_task_args_t *args) {
  task_args_t * task_args = args->user_task_args;
  
  int part1_complete = 0;
  int part2_complete = 0;
  int flag = 0;

  while (part1_complete == 0 || part2_complete == 0) {
    /* test partition #j and #j+1 */
    MPI_Parrived(task_args->request, args->taskId, &flag);
    if (flag && part1_complete == 0) {
      part1_complete++;
      /* Do work using partition j data */
    }

    if (args->taskId + 1 < task_args->recv_partitions) {
      MPI_Parrived(task_args->request, args->taskId + 1, &flag);
      if (flag && part2_complete == 0) {
        part2_complete++;
        /* Do work using partition j+1 */
      }
    } else {
      part2_complete++;
    }
  }
}

int main(int argc, char *argv[]) /* send-side partitioning */
{
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_thread_library_init();

  MPI_Count partitions = conf.num_partitions;
  MPI_Count partlength = conf.num_partlength;
  double message[partitions * partlength];

  int send_partitions = partitions, send_partlength = partlength,
      recv_partitions = partitions * 2, recv_partlength = partlength / 2;
  int source = 0, dest = 1, tag = 1, flag = 0;
  int myrank;
  int provided;

  MPI_Request request;
  MPI_Info info = MPI_INFO_NULL;
  MPI_Datatype send_type;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Type_contiguous(send_partlength, MPI_DOUBLE, &send_type);
  MPI_Type_commit(&send_type);

  task_args_t args;
  args.request = &request;
  args.recv_partitions = recv_partitions;

  if (myrank == 0) {
    MPI_Psend_init(message, send_partitions, 1, send_type, dest, tag, info,
                   MPI_COMM_WORLD, &request);
    MPI_Start(&request);
    #if defined (OMP)
    #pragma omp parallel num_threads(conf.num_threads)
    #pragma omp single
    #endif
    for(int i = 0; i < conf.num_tasks; ++i)
    {
      partix_task(&send_task /*functor*/, &args /*capture*/, &conf /*iters*/,
                  partix_noise_off /*config options*/);
    }
    partix_barrier();
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, recv_partitions, recv_partlength, MPI_DOUBLE,
                   source, tag, info, MPI_COMM_WORLD, &request);
    MPI_Start(&request);
    #if defined (OMP)
    #pragma omp parallel num_threads(conf.num_threads)
    #pragma omp single
    #endif
    for(int i = 0; i < recv_partitions; ++i)
    {
      partix_task(&recv_task /*functor*/, &args /*capture*/, &conf /*iters*/,
                  partix_noise_off /*config options*/);
    }
    partix_barrier();

    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  }
  MPI_Finalize();

  partix_thread_library_finalize();

  return 0;
}