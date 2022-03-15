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
#include <limits>

#include <partix.h>

#define DEFAULT_VALUE 123

int reduction_var = 0;

/* My task args */
typedef struct {
  int some_data;
} task_args_t;

void task(partix_task_args_t *args) {
  // Do nothing
  task_args_t *task_args = (task_args_t *)args->user_task_args;
  partix_mutex_enter();
  reduction_var += task_args->some_data;
  partix_mutex_exit();
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();
  task_args_t task_args;
  task_args.some_data = DEFAULT_VALUE;

  int num_tasks = conf.num_tasks;
  int next_num_tasks = num_tasks;

  double time;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
  do {
    reduction_var = 0;
    num_tasks = next_num_tasks;
    double start_time = MPI_Wtime();
    for (int i = 0; i < num_tasks; ++i) {
      partix_task(&task /*functor*/, &task_args /*capture by ref*/);
    }
    partix_taskwait();
    double end_time = MPI_Wtime();

    if (next_num_tasks * 2 > std::numeric_limits<int>::max() / 4)
      break;
    next_num_tasks *= 2;
    time = (end_time - start_time) * 1.0e3 /*msec*/;
  } while (time <= 200.0);

  printf("%f[ms] per task (%i tasks executed)\n", time / num_tasks, num_tasks);
  /*Perceived BW*/
  assert(reduction_var == DEFAULT_VALUE * num_tasks);
  partix_library_finalize();
  return 0;
}
