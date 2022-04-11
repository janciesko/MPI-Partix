#include "mpi.h"
#include <stdlib.h>

#include <partix.h>

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  MPI_Count partitions = conf.num_partitions;
  MPI_Count partlength = conf.num_partlength;

  double *message = new double[partitions * partlength];
  int source = 0, dest = 1, tag = 1, flag = 0;
  int myrank, i;
  int provided;
  MPI_Request request;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
  if (provided < MPI_THREAD_SERIALIZED)
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);

  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if (myrank == 0) {
    MPI_Psend_init(message, partitions, partlength, MPI_DOUBLE, dest, tag,
                   MPI_COMM_WORLD, MPI_INFO_NULL, &request);
    MPI_Start(&request);
    for (i = 0; i < partitions; ++i) {
      /* compute and fill partition #i, then mark ready: */
      partix_add_noise();
      MPI_Pready(i, request);
    }
    while (!flag) {
      /* do useful work #1 */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* do useful work #2 */
    }
    MPI_Request_free(&request);
  } else if (myrank == 1) {
    MPI_Precv_init(message, partitions, partlength, MPI_DOUBLE, source, tag,
                   MPI_COMM_WORLD, MPI_INFO_NULL, &request);
    MPI_Start(&request);
    while (!flag) {
      /* do useful work #1 */
      MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
      /* do useful work #2 */
    }
    MPI_Request_free(&request);
  }

  delete[] message;
  MPI_Finalize();
  partix_library_finalize();
  return 0;
}