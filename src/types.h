#ifndef __PARTIX_TASK_ARGS_H__
#define __PARTIX_TASK_ARGS_H__

typedef struct {
  int num_tasks;
  int num_threads;
  int num_partitions;
  int num_partlength;
} partix_config_t;

/* Default task args */
typedef struct {
  int taskId;
  void * user_task_args;
} partix_task_args_t;

#endif /* __PARTIX_TASK_ARGS_H__*/
