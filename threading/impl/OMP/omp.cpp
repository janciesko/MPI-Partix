/*
//@HEADER
// ************************************************************************
//
//                        Partix 1.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY NTESS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NTESS OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Jan Ciesko (jciesko@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#include <stdlib.h>
#include <omp.h>

#include <thread.h>

extern void partix_init(partix_config_t *);

struct thread_handle_t {
  int thread;
  void (*f)(void *);
  void *arg;
};

partix_config_t * global_conf;

void partix_parallel_for(void (*f)(partix_task_args_t *), void * args, partix_config_t * conf){
  global_conf = conf;
  #pragma omp parallel for shared(args) num_threads(conf->num_threads)  
  for (int task = 0; task < conf->num_threads; task++) {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    partix_args.conf = conf;
    f(&partix_args);
  }
}

void partix_task(void (*f)(partix_task_args_t *), void * args, partix_config_t * conf){
  global_conf = conf;
  #pragma omp task
  {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    f(&partix_args);
  }
}

void partix_parallel_for(void (*f)(partix_task_args_t *), void * args){
  partix_critical_enter();
  if(global_conf==NULL) {
    partix_init(global_conf);
    // Optionally error out and require the first call into partix_parallel_for to be of the other overload
  }
  partix_critical_exit();
  #pragma omp parallel for num_threads(global_conf->num_threads)  
  for (int task = 0; task < global_conf->num_threads; task++) {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    partix_args.conf = global_conf; 
    f(&partix_args);
  }
}

void partix_task(void (*f)(partix_task_args_t *), void * args){
  partix_critical_enter();
  if(global_conf==NULL) {
    partix_init(global_conf);
    // Optionally error out and require the first call into partix_parallel_for to be of the other overload
  }
  partix_critical_exit();
  #pragma omp task
  {
    partix_task_args_t partix_args;   
    partix_args.user_task_args = args;
    partix_args.conf = global_conf; 
    f(&partix_args);
  }
}

void partix_barrier(void){
  #pragma omp barrier
}

void partix_taskwait(void){
  #pragma omp taskwait
}

void partix_critical_enter(void) { ; /* Empty. */ };

void partix_critical_exit(void) { ; /* Empty. */ };

int partix_executor_id(void){ return omp_get_thread_num(); };

void partix_thread_library_init(void) { ; /* Empty. */ }

void partix_thread_library_finalize(void) { ; /* Empty. */ }

void partix_thread_barrier_init(int num_waiters, barrier_handle_t *p_barrier) { ; /* Empty. */ }

void partix_thread_barrier_wait(void) { ; /* Empty. */ }

void partix_thread_barrier_destroy(barrier_handle_t *p_barrier) { ; /* Empty. */ }

void *partix_pthread_func(void *arg) { ; /* Empty. */ }

void partix_thread_create(void (*f)(void *), void *arg,
                          thread_handle_t *p_thread) { ; /* Empty. */ }

void partix_thread_join(thread_handle_t *p_thread) { ; /* Empty. */ }
