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

/* My task args */
typedef struct {
  MPI_Request *request;
  int partition;
} send_task_args_t;

/* My task args */
typedef struct {
  MPI_Request *request;
  int partition;
  int recv_partitions;
} recv_task_args_t;

void recv_task(partix_task_args_t *args) {
  recv_task_args_t *task_args = (recv_task_args_t *)args->user_task_args;

  int flag1 = 0;
  int flag2 = 0;

  int partition = task_args->partition;

  while (flag1 == 0 || flag1 == 0) {
    /* test partition #partition and #partition+1 */
    MPI_Parrived(*task_args->request, partition, &flag1);
    if (partition + 1 < task_args->recv_partitions) {
      MPI_Parrived(*task_args->request, partition + 1, &flag2);
    } else {
      flag2++;
    }
  }
};

void send_task(partix_task_args_t *args) {
  send_task_args_t *task_args = (send_task_args_t *)args->user_task_args;
  MPI_Pready(task_args->partition, *task_args->request);
};

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_thread_library_init();

  MPI_Count send_partitions = conf.num_partitions;
  MPI_Count send_partlength = conf.num_partlength;
  send_partlength += send_partlength % 2;
  MPI_Count recv_partitions = send_partitions * 2;
  MPI_Count recv_partlength = send_partlength / 2;

  double message[send_partitions * send_partlength];

  int count = 1, source = 0, dest = 1, tag = 1, flag = 0;
  int myrank;
  int provided;

  MPI_Request request;
  MPI_Info info = MPI_INFO_NULL;
  MPI_Datatype xfer_type;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Type_contiguous(send_partlength, MPI_DOUBLE, &xfer_type);
  MPI_Type_commit(&xfer_type);

  send_task_args_t send_args;
  recv_task_args_t recv_args;
  
  if (myrank == 0) {
    MPI_Psend_init(message, send_partitions, send_partlength, xfer_type, dest,
                   tag, MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);

    send_args.request = &request;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < send_partitions; ++i) {
      send_args.partition = i;
      partix_task(&send_task /*functor*/, &send_args /*capture*/);
    }
    partix_taskwait();
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, recv_partitions, recv_partlength, xfer_type, source,
                   tag, MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);

    recv_args.recv_partitions = recv_partitions;
    recv_args.request = &request;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < recv_partitions; i += 2) {
      recv_args.partition = i;
      partix_task(&recv_task /*functor*/, &recv_args /*capture*/);
    }
    partix_taskwait();

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
