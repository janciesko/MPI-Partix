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

#include <stdlib.h>
#include <thread.h>

partix_mutex_t global_mutex;
partix_config_t *global_conf;

void partix_task(void (*f)(partix_task_args_t *), void *args,
                 partix_context_t *ctx) {
#pragma omp task
  {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    f(&partix_args);
  }
}

void partix_taskwait(partix_context_t *ctx) {
#pragma omp taskwait
}

void partix_mutex_init(partix_mutex_t *m) { omp_init_lock(m); };
void partix_mutex_destroy(partix_mutex_t *m) { omp_destroy_lock(m); };

void partix_mutex_enter(partix_mutex_t *m) { omp_set_lock(m); };

void partix_mutex_exit(partix_mutex_t *m) { omp_unset_lock(m); };

void partix_mutex_init() { omp_init_lock(&global_mutex); };
void partix_mutex_destroy() { omp_destroy_lock(&global_mutex); };

void partix_mutex_enter(void) { omp_set_lock(&global_mutex); };

void partix_mutex_exit(void) { omp_unset_lock(&global_mutex); };

int partix_executor_id(void) { return omp_get_thread_num(); };

void partix_library_init(void) { ; /* Empty. */ }

void partix_library_finalize(void) { ; /* Empty. */ }
