#include "mpi.h"
#include <stdlib.h>
#include <qthread.h>

#define NUM_THREADS 8
#define PARTITIONS 8
#define PARTLENGTH 16
int main(int argc, char *argv[]) /* same send/recv partitioning */
{
  double message[PARTITIONS * PARTLENGTH];
  int partitions = PARTITIONS;
  int partlength = PARTLENGTH;
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
  if (myrank == 0) {
    MPI_Psend_init(message, partitions, count, xfer_type, dest, tag, info,
                   MPI_COMM_WORLD, &request);
    MPI_Start(&request);
#pragma omp parallel for shared(request) num_threads(NUM_THREADS)
    for (int i = 0; i < partitions; i++) {
      /* compute and fill partition #i, then mark ready: */
      MPI_Pready(i, request);
    }
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, partitions, count, xfer_type, source, tag, info,
                   MPI_COMM_WORLD, &request);
    MPI_Start(&request);
    while (!flag) {
      /* Do useful work */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* Do useful work */
    }
    MPI_Request_free(&request);
  }
  MPI_Finalize();
  return 0;
}
