#ifndef __PARTIX_GENERIC_TYPES_H__
#define __PARTIX_GENERIC_TYPES_H__

typedef struct {
  int num_tasks;
  int num_threads;
  int num_partitions;
  int num_partlength;
  bool add_noise;
} partix_config_t;

typedef size_t partix_context_t;

/* Internal task object */
typedef struct {
  void *user_task_args;
  partix_config_t *conf;
} partix_task_args_t;

enum Options {
  partix_noise_on,
  partix_noise_off,
};

#endif /* __PARTIX_GENERIC_TYPES_H__ */
