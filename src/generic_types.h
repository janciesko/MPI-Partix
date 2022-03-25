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

#ifndef __PARTIX_GENERIC_TYPES_H__
#define __PARTIX_GENERIC_TYPES_H__

#include <cstddef>

typedef struct {
  int num_tasks;
  int num_threads;
  int num_partitions;
  int num_partlength;
  int overlap_duration;
  int noise;
  int argc;
  char ** argv;
} partix_config_t;

typedef std::size_t partix_context_t;

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
