#ifndef __PARTIX_TASK_ARGS_H__
#define __PARTIX_TASK_ARGS_H__

#include <stdbool.h> 

typedef struct {
  int num_tasks;
  int num_threads;
  int num_partitions;
  int num_partlength;
  bool add_noise;
} partix_config_t;

/* Default task args */
typedef struct {
  int taskId;
  void * user_task_args;
  partix_config_t * conf;
} partix_task_args_t;


enum Options {
  partix_noise_on,
  partix_noise_off,
};

#endif /* __PARTIX_TASK_ARGS_H__*/
