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
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#if defined(DEBUG)
#include <pthread.h>
#endif

#include <partix.h>

#define COMM_TYPE_PT2PT 0
#define COMM_TYPE_PUTGET 1 /*TBD*/

#define DEFAULT_VALUE 123
#define comm MPI_COMM_WORLD

partix_mutex_t mutex;
int reduction_var = 0;

/* My task args */
typedef struct {
  int some_data;
  int target;
} task_args_t;

void task_send(partix_task_args_t *args) {
  int ret;
  MPI_Request request;

  task_args_t *task_args = (task_args_t *)args->user_task_args;

  ret = MPI_Isend(&task_args->some_data, 1, MPI_INT, task_args->target, 0, comm,
                  &request);

  assert(ret == 0);

#if defined(DEBUG)
  printf("ULTcorrectness1: Sending: %i in task %u, pthread %lu to rank %i.\n",
         task_args->some_data, partix_executor_id(), pthread_self(),
         task_args->target);

#else
  printf("ULTcorrectness1: Sending: %i in task %u to rank %i.\n",
         task_args->some_data, partix_executor_id(), task_args->target);
#endif
  ret = MPI_Wait(&request, MPI_STATUS_IGNORE);

#if defined(DEBUG)
  // Additional output to see the resuming pthread
  printf(
      "ULTcorrectness1: Sending(B): %i in task %u, pthread %lu to rank %i.\n",
      task_args->some_data, partix_executor_id(), pthread_self(),
      task_args->target);
#endif

  assert(ret == 0);
  partix_mutex_enter(&mutex);
  reduction_var += task_args->some_data;
  partix_mutex_exit(&mutex);
}

void task_recv(partix_task_args_t *args) {
  int ret, tmp;
  MPI_Request request;

  task_args_t *task_args = (task_args_t *)args->user_task_args;

  ret = MPI_Irecv(&tmp, 1, MPI_INT, task_args->target, 0, comm, &request);
  assert(ret == 0);

#if defined(DEBUG)
  printf(
      "ULTcorrectness1: Received: %i in task %u, pthread %lu from rank %i.\n",
      task_args->some_data, partix_executor_id(), pthread_self(),
      task_args->target);
#else
  printf("ULTcorrectness1: Received: %i in task %u from rank %i.\n",
         task_args->some_data, partix_executor_id(), task_args->target);
#endif

  ret = MPI_Wait(&request, MPI_STATUS_IGNORE);

#if defined(DEBUG)
  // Additional output to see the resuming pthread
  printf("ULTcorrectness1: Received(B): %i in task %u, pthread %lu from rank "
         "%i.\n",
         task_args->some_data, partix_executor_id(), pthread_self(),
         task_args->target);
#endif

  assert(ret == 0);
  partix_mutex_enter(&mutex);
  reduction_var += tmp;
  partix_mutex_exit(&mutex);
}

int main(int argc, char *argv[]) {
  partix_config_t conf;

  int provided;
  partix_init(argc, argv, &conf);
  partix_library_init();
  task_args_t task_args;
  task_args.some_data = DEFAULT_VALUE;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  assert(provided == MPI_THREAD_MULTIPLE);

  int comm_rank;
  int comm_size;

  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  // Requires two ranks
  assert(comm_size == 2);
  // Requires even number of tasks
  assert(conf.num_tasks % 2 == 0);
  // Requires at least 4 tasks
  assert(conf.num_tasks >= 4);

  task_args.some_data = DEFAULT_VALUE;
  task_args.target = comm_rank ^ 1;

  partix_mutex_init(&mutex);

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif

  // Create in this sequence that starts and ends with recv tasks
  // This tests correct ULT functionality with ULT libs with
  // FIFO or LIFO schedulers

  for (int i = 0; i < conf.num_tasks; i += 2) {
    if (i < 2) {
      partix_task(&task_recv /*functor*/, &task_args /*capture by ref*/);
      partix_task(&task_send /*functor*/, &task_args /*capture by ref*/);
    } else {
      partix_task(&task_send /*functor*/, &task_args /*capture by ref*/);
      partix_task(&task_recv /*functor*/, &task_args /*capture by ref*/);
    }
  }

  partix_taskwait();

  assert(reduction_var == DEFAULT_VALUE * conf.num_tasks);
  partix_mutex_destroy(&mutex);
  partix_library_finalize();
  MPI_Finalize();
  return 0;
}
