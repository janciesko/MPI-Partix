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

#include <thread.h>
#include <partix.h>

#if defined (IS_OPENMP)
void task ( task_args_t * task_args ){
  #pragma omp single
  {
    /* single thread creates 64 tasks to be executed by 8 threads */
    for (int partition_num = 0; partition_num < 64;
          partition_num++) {
      #pragma omp task firstprivate(partition_num)
      {
        /* compute and fill partition #partition_num, then mark
        ready: */
        /* buffer is filled in arbitrary order from each task */
        MPI_Pready(partition_num, request);
      } /*end task*/
    }   /* end for */
  }     /* end single */
};
#else
void task ( task_args_t * task_args ){
  MPI_Pready(task_args->i, task_args->request);
};
#endif

int main(int argc, char *argv[]) /* send-side partitioning */
{
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_thread_library_init();
  
  MPI_Count partitions = conf.num_partitions;
  MPI_Count partlength = conf.num_partlength;
  double message[partitions * partlength];

  int send_partitions = partitions, send_partlength = partlength,
      recv_partitions = 1, recv_partlength = partitions * partlength;
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
  if (myrank == 0) {
    MPI_Psend_init(message, send_partitions, send_partlength, send_type, dest, tag, info,
                   MPI_COMM_WORLD, &request);
    MPI_Start(&request);
    partix_parallel_for(&task /*functor*/, partitions /*iters*/, partix_noise_on /*config options*/ );
    partix_thread_barrier_wait();
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