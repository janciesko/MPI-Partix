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

#ifndef __PARTIX_H__
#define __PARTIX_H__

/* Number of workers in ULT libraries and OpenMP*/
#define NUM_THREADS_DEFAULT 2

/* Number of tasks in ULT libraries and OpenMP
   Number of threads with Pthreads */
#define NUM_TASKS_DEFAULT NUM_THREADS_DEFAULT * 8

/* Per default, numer of partitions is the number of tasks*/
#define PARTITIONS_DEFAULT NUM_TASKS_DEFAULT

/* Num elements of MPI_TYPE
   For MPI_DOUBLE, this creates a partition of 128 bytes */
#define PARTLENGTH_DEFAULT 16

#include <thread.h>

#define DEFAULT_CONF_VAL 1

extern partix_config_t *global_conf;

void partix_init(int argc, char *argv[], partix_config_t *conf) {
  conf->num_tasks = argc > 1 ? atoi(argv[1]) : NUM_TASKS_DEFAULT;
  conf->num_threads = argc > 2 ? atoi(argv[2]) : NUM_THREADS_DEFAULT;
  conf->num_partitions = argc > 3 ? atoi(argv[3]) : PARTITIONS_DEFAULT;
  conf->num_partlength = argc > 4 ? atoi(argv[4]) : PARTLENGTH_DEFAULT;
  conf->add_noise = partix_noise_off;

  /* conf object duration should be valid until partix_finalize() */
  global_conf = conf;
}

void partix_finalize() { ; /* Empty. */ }

void partix_add_noise() {}

#endif /* __PARTIX_H__*/
