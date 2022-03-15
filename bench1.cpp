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

#define DEFAULT_ITERS 1
#define DEFAULT_SEND_RECV_PARTITION_RATIO 1
#define DATA_TYPE MPI_DOUBLE

double timer[3] = {0.0, 0.0, 0.0};

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
};

void send_task(partix_task_args_t *args) {
  send_task_args_t *task_args = (send_task_args_t *)args->user_task_args;
  MPI_Pready(task_args->partition_id, *task_args->request);
};

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);

  int myrank;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  int iterations = DEFAULT_ITERS;
  int num_partitions = conf.num_partitions;
  int num_partlength = conf.num_partlength;
  int data_size = 0;

  for (int i = 0; i < iterations; ++i) {
    // Benchmark iteration
    {
      MPI_Request request;
      MPI_Info info = MPI_INFO_NULL;
      MPI_Datatype send_xfer_type;
      MPI_Datatype recv_xfer_type;

      MPI_Count send_partitions = num_partitions;
      MPI_Count send_partlength = num_partlength;

      MPI_Count recv_partitions =
          num_partitions * DEFAULT_SEND_RECV_PARTITION_RATIO;
      MPI_Count recv_partlength =
          num_partlength / DEFAULT_SEND_RECV_PARTITION_RATIO;

      double message[num_partitions * num_partlength];

      data_size = num_partitions * num_partlength * sizeof(DATA_TYPE);

      int count = 1, source = 0, dest = 1, tag = 1, flag = 0;

      MPI_Type_contiguous(send_partlength, DATA_TYPE, &send_xfer_type);
      MPI_Type_contiguous(recv_partlength, DATA_TYPE, &recv_xfer_type);
      MPI_Type_commit(&send_xfer_type);
      MPI_Type_commit(&recv_xfer_type);

      /* Rank 0 */

      if (myrank == 0) {

        send_task_args_t *send_args = (send_task_args_t *)calloc(
            send_partitions, sizeof(send_task_args_t));

        MPI_Psend_init(message, send_partitions, 1, send_xfer_type, dest, tag,
                       MPI_COMM_WORLD, info, &request);
        MPI_Start(&request);
        double start_time = MPI_Wtime();

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
        for (int j = 0; j < send_partitions; ++j) {
          send_args[j].request = &request;
          send_args[j].partition_id = j;
          partix_task(&send_task /*functor*/, &send_args[j] /*capture*/);
        }
        partix_taskwait();
        while (!flag) {
          /* Do useful work */
          MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
          /* Do useful work */
        }

        double delta_time = MPI_Wtime() - start_time;
        timer[0] += delta_time;

        MPI_Request_free(&request);
        free(send_args);
      } else if (myrank == 1) {

        /* Rank 1 */

        recv_task_args_t *recv_args = (recv_task_args_t *)calloc(
            recv_partitions, sizeof(recv_task_args_t));

        MPI_Precv_init(message, recv_partitions, 1, recv_xfer_type, source, tag,
                       MPI_COMM_WORLD, info, &request);
        MPI_Start(&request);
        double start_time = MPI_Wtime();

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
        for (int j = 0; j < recv_partitions; j += 2) {
          recv_args[j].request = &request;
          recv_args[j].partition_id = j;
          partix_task(&recv_task /*functor*/, &recv_args[j] /*capture*/);
        }
        partix_taskwait();

        while (!flag) {
          /* Do useful work */
          MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
          /* Do useful work */
        }

        double delta_time = MPI_Wtime() - start_time;
        timer[1] += delta_time;

        MPI_Request_free(&request);
        free(recv_args);
      }
    }
  }

  timer[0] /= iterations;
  timer[1] /= iterations;
  double recv_BW = data_size / timer[1];
  printf("%d, %d, %d\n", timer[0] /*rank0*/, timer[1] /*rank1*/, recv_BW);

  MPI_Finalize();
  partix_library_finalize();
  return 0;
}
