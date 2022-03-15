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
} task_args_t;

void task(partix_task_args_t *args) {
  task_args_t *task_args = (task_args_t *)args->user_task_args;
  MPI_Pready(task_args->partition_id, *task_args->request);
};

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  MPI_Count partitions = conf.num_partitions;
  MPI_Count partlength = conf.num_partlength;

  double message[partitions * partlength];

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
  MPI_Type_contiguous(partlength, MPI_DOUBLE, &xfer_type);
  MPI_Type_commit(&xfer_type);

  task_args_t *args =
      (task_args_t *)calloc(conf.num_partitions, sizeof(task_args_t));

  if (myrank == 0) {
    MPI_Psend_init(message, partitions, 1, xfer_type, dest, tag, MPI_COMM_WORLD,
                   info, &request);
    MPI_Start(&request);
#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
    for (int i = 0; i < partitions; ++i) {
      args[i].request = &request;
      args[i].partition_id = i;
      partix_task(&task /*functor*/, &args[i] /*capture*/);
    }
    partix_taskwait();
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, partitions, 1, xfer_type, source, tag,
                   MPI_COMM_WORLD, info, &request);
    MPI_Start(&request);
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  }

  free(args);
  MPI_Finalize();
  partix_library_finalize();
  return 0;
}
