/*
//@HEADER
// ************************************************************************
//
//                        Partix 1.0
//              Copyright Type Date and Name (2022)
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
  int partition_id;
} send_task_args_t;

/* My task args */
typedef struct {
  MPI_Request *request;
  int recv_partitions;
  int partition_id;
} recv_task_args_t;

void recv_task(partix_task_args_t *args) {
  recv_task_args_t *task_args = (recv_task_args_t *)args->user_task_args;

  int flag1 = 0;
  int flag2 = 0;

  int partition_id = task_args->partition_id;

  while (flag1 == 0 || flag2 == 0) {
    /* test partition_id #partition_id and #partition_id+1 */
    MPI_Parrived(*task_args->request, partition_id, &flag1);
    if (partition_id + 1 < task_args->recv_partitions) {
      MPI_Parrived(*task_args->request, partition_id + 1, &flag2);
    } else {
      flag2++;
    }
  }
}

void send_task(partix_task_args_t *args) {
  send_task_args_t *task_args = (send_task_args_t *)args->user_task_args;
  MPI_Pready(task_args->partition_id, *task_args->request);
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  MPI_Count send_partitions = conf.num_partitions;
  MPI_Count send_partlength = conf.num_partlength;
  MPI_Count recv_partitions = send_partitions * 2;
  MPI_Count recv_partlength = send_partlength / 2;

  double * message = new double[send_partitions * send_partlength];

  int count = 1, source = 0, dest = 1, tag = 1, flag = 0;
  int myrank;
  int provided;

  MPI_Request request;
  MPI_Info info = MPI_INFO_NULL;
  MPI_Datatype send_xfer_type;
  MPI_Datatype recv_xfer_type;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Type_contiguous(send_partlength, MPI_DOUBLE, &send_xfer_type);
  MPI_Type_contiguous(recv_partlength, MPI_DOUBLE, &recv_xfer_type);
  MPI_Type_commit(&send_xfer_type);
  MPI_Type_commit(&recv_xfer_type);

  send_task_args_t *send_args =
      (send_task_args_t *)calloc(send_partitions, sizeof(send_task_args_t));
  recv_task_args_t *recv_args =
      (recv_task_args_t *)calloc(recv_partitions, sizeof(recv_task_args_t));

  if (myrank == 0) {
    MPI_Psend_init(message, send_partitions, count, send_xfer_type, dest, tag,
                   MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);
#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < send_partitions; ++i) {
      send_args[i].request = &request;
      send_args[i].partition_id = i;
      partix_task(&send_task /*functor*/, &send_args[i] /*capture*/);
    }
    partix_taskwait();
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, recv_partitions, count, recv_xfer_type, source, tag,
                   MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);
#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < recv_partitions; i += 2) {
      recv_args[i].request = &request;
      recv_args[i].partition_id = i;
      partix_task(&recv_task /*functor*/, &recv_args[i] /*capture*/);
    }
    partix_taskwait();

    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  }

  free(send_args);
  free(recv_args);
  delete[] message;
  MPI_Finalize();
  partix_library_finalize();

  return 0;
}
