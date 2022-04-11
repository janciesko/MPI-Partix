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
  int iters;
} task_args_t;

void task_inner(partix_task_args_t *args) {
  task_args_t *task_args = (task_args_t *)args->user_task_args;
  printf("APITest2: Printing: %i on thread %u.\n", task_args->some_data,
         partix_executor_id());
  partix_mutex_enter();
  reduction_var += task_args->some_data;
  partix_mutex_exit();
}

void task_outer(partix_task_args_t *args) {
  task_args_t *task_args = (task_args_t *)args->user_task_args;

  // set context
  partix_context_t ctx;

  for (int i = 0; i < task_args->iters; ++i) {
    partix_task(&task_inner /*functor*/, task_args /*capture by ref*/, &ctx);
  }
  partix_taskwait(&ctx);
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();
  task_args_t task_args;
  task_args.some_data = DEFAULT_VALUE;
  task_args.iters = 4;

  // set context
  partix_context_t ctx;

#if defined(OMP)
#pragma omp parallel num_threads(conf.num_threads)
#pragma omp single
#endif

  for (int i = 0; i < conf.num_tasks; ++i) {
    partix_task(&task_outer /*functor*/, &task_args /*capture by ref*/, &ctx);
  }

  partix_taskwait(&ctx);

  assert(reduction_var == DEFAULT_VALUE * conf.num_tasks * task_args.iters);

  partix_library_finalize();
  return 0;
}
