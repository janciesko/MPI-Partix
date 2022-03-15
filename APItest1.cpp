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

#include <partix.h>

#define DEFAULT_VALUE 123

int reduction_var = 0;

/* My task args */
typedef struct {
  int some_data;
} task_args_t;

void task(partix_task_args_t *args) {
  task_args_t *task_args = (task_args_t *)args->user_task_args;
  printf("APITest1: Printing: %i on thread %u.\n", task_args->some_data,
         partix_executor_id());
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

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif
  for (int i = 0; i < conf.num_tasks; ++i) {
    partix_task(&task /*functor*/, &task_args /*capture by ref*/);
  }
  partix_taskwait();

  assert(reduction_var == DEFAULT_VALUE * conf.num_tasks);

  partix_library_finalize();
  return 0;
}
