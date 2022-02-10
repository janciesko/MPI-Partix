#include "mpi.h"
#include <stdlib.h>
#include <qthread.h>

#define NUM_THREADS 64
#define PARTITIONS NUM_THREADS
#define PARTLENGTH 16
#define MESSAGE_LENGTH PARTITIONS *PARTLENGTH

int main(int argc, char *argv[]) /* send-side partitioning */
{

  qthread_initialize();
  double message[MESSAGE_LENGTH];
  int send_partitions = PARTITIONS, send_partlength = PARTLENGTH,
      recv_partitions = PARTITIONS * 2, recv_partlength = PARTLENGTH / 2;
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
    MPI_Psend_init(message, send_partitions, 1, send_type, dest, tag, info,
                   MPI_COMM_WORLD, &request);
    MPI_Start(&request);
#pragma omp parallel for shared(request) num_threads(NUM_THREADS)
    for (int i = 0; i < send_partitions; i++) {
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
    MPI_Precv_init(message, recv_partitions, recv_partlength, MPI_DOUBLE,
                   source, tag, info, MPI_COMM_WORLD, &request);
    MPI_Start(&request);
#pragma omp parallel for shared(request) num_threads(NUM_THREADS)
    for (int j = 0; j < recv_partitions; j += 2) {
      int part1_complete = 0;
      int part2_complete = 0;

      while (part1_complete == 0 || part2_complete == 0) {
        /* test partition #j and #j+1 */
        MPI_Parrived(request, j, &flag);
        if (flag && part1_complete == 0) {
          part1_complete++;
          /* Do work using partition j data */
        }
        if (j + 1 < recv_partitions) {
          MPI_Parrived(request, j + 1, &flag);
          if (flag && part2_complete == 0) {
            part2_complete++;
            /* Do work using partition j+1 */
          }
        } else {
          part2_complete++;
        }
      }
    }
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