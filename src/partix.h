#ifndef __PARTIX_H__
#define __PARTIX_H__

#include <stdlib.h>
#include <assert.h>

/* Number of workers in ULT libraries and OpenMP */
#define NUM_THREADS_DEFAULT 2

/* Number of tasks in ULT libraries and OpenMP
   Number of threads with Pthreads */
#define NUM_TASKS_DEFAULT NUM_THREADS_DEFAULT * 8

/* Per default, numer of partitions is the number of tasks */
#define PARTITIONS_DEFAULT NUM_TASKS_DEFAULT

/* Num elements of MPI_TYPE
   For MPI_DOUBLE, this creates a partition of 128 bytes */
#define PARTLENGTH_DEFAULT 16

/* Used to simulate computation and communication overlap */
#define OVERLAP_IN_MSEC_DEFAULT 100

/* Used add task duration divergence as a % of OVERLAP_IN_MSEC_DEFAULT */
#define NOISE_IN_PERCENTAGE_OF_OVERLAP 80

#include <thread.h>

#define DEFAULT_CONF_VAL 1

extern partix_config_t *global_conf;

void partix_init(int argc, char **argv, partix_config_t *conf) {
  conf->num_tasks = argc > 1 ? atoi(argv[1]) : NUM_TASKS_DEFAULT;
  conf->num_threads = argc > 2 ? atoi(argv[2]) : NUM_THREADS_DEFAULT;
  conf->num_partitions = argc > 3 ? atoi(argv[3]) : PARTITIONS_DEFAULT;
  conf->num_partlength = argc > 4 ? atoi(argv[4]) : PARTLENGTH_DEFAULT;
  conf->overlap_duration = argc > 5 ? atoi(argv[5]) : OVERLAP_IN_MSEC_DEFAULT;
  conf->noise_spread = argc > 6 ? atoi(argv[6]) : NOISE_IN_PERCENTAGE_OF_OVERLAP;
  
  assert(conf->num_partitions>=conf->num_tasks);
  assert(conf->num_partlength>0);

  /* conf object duration should be valid until partix_finalize() */
  global_conf = conf;

  /* Propate process args */
  conf->argc = argc;
  conf->argv = argv;
}

void partix_finalize() { ; /* Empty. */ }

void partix_add_noise() {}

#endif /* __PARTIX_H__*/
