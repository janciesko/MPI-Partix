#ifndef __PARTIX_TYPES_H__
#define __PARTIX_TYPES_H__

#define MAX_THREADS 1024
#define MAX_NESTING 64

#include <stdbool.h>
#if defined (PTHREADS)
#include <pthread.h>
#endif

typedef struct {
  int num_tasks;
  int num_threads;
  int num_partitions;
  int num_partlength;
  bool add_noise;
} partix_config_t;
/*
typedef struct
{
  #if defined (PTHREADS)
  pthread_t threadHandle[MAX_THREADS];
  int counter;
  barrier_handle_t *p_barrier;
  #endif
} partix_threadsHandle_t;
*/

/* Internal task object */
typedef struct {
  void *user_task_args;
  partix_config_t *conf;
} partix_task_args_t;

enum Options {
  partix_noise_on,
  partix_noise_off,
};

#endif /* __PARTIX_TYPES_H__ */
