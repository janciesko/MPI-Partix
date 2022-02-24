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

#include <thread.h>

partix_config_t *global_conf;
partix_parallel_for_thead_handle_t global_parallel_for_thread_handle;

void partix_parallel_for(void (*f)(partix_task_args_t *), void *args,
                         partix_config_t *conf) {
  global_conf = conf;
  partix_threadHandle_t * threadsHandle = (partix_threadHandle_t *) calloc (0, sizeof(partix_threadHandle_t));
  partix_thread_barrier_init(threadsHandle.p_barrier, conf->num_theads);
  global_parallel_for_handle_level.handles[nesting_level++] = threadsHandle;
  
  for (int task = 0; task < conf->num_threads; task++) {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    partix_args.conf = conf;
    partix_task(f, args);
  }
}

void partix_task(void (*f)(partix_task_args_t *), void *args,
                 partix_config_t *conf) {
  global_conf = conf;
  {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    partix_task(f, args);
  }
}

void partix_parallel_for(void (*f)(partix_task_args_t *), void *args) {
  partix_critical_enter();
  {
    if (global_conf == NULL) {
      partix_init(global_conf);
      // Optionally error out and require the first call into partix_parallel_for
      // to be of the other overload.
    }
  }
  partix_critical_exit();
  for (int task = 0; task < global_conf->num_threads; task++) {
    partix_task_args_t partix_args;
    partix_args.user_task_args = args;
    partix_args.conf = global_conf;
    memset(&partix_args.threadHandle.used, 0, sizeof(partix_args.threadHandle.used));
    partix_task(f, args);
  }
}

void partix_task(void (*f)(partix_task_args_t *), void *args) {
  partix_critical_enter();
  {
    if (global_conf == NULL) {
      partix_init(global_conf);
      // Optionally error out and require the first call into partix_parallel_for
      // to be of the other overload
    }
  }
  partix_critical_exit(); 
  partix_task_args_t partix_args;
  partix_args.user_task_args = args;
  partix_args.conf = global_conf;
  partix_thread_create(f, args, patix_get_unique_id(&partix_args));
  
}

int patix_get_unique_id(partix_task_args_t *args)
{
  partix_critical_enter(); //barrier on theads in the current nesting level
  int level = global_parallel_for_thread_handle.nesting_level;
  global_parallel_for_thread_handle.handles[level]->counter++;
  partix_critical_exit();
  if(counter >= MAX_THREADS)
  {/*Error out*/}
  
  return counter;
}

int partix_executor_id(void) { return pthread_getthreadid_np(); };

void partix_thread_library_init(void) { ; /* Empty. */ }

void partix_thread_library_finalize(void) { ; /* Empty. */ }

void partix_thread_barrier_init(int num_waiters, barrier_handle_t *p_barrier) {
  pthread_barrier_init(&p_barrier->barrier, NULL, num_waiters);
}

void partix_thread_barrier_wait(barrier_handle_t *p_barrier) {
  pthread_barrier_wait(&p_barrier->barrier);
}

void partix_thread_barrier_destroy(barrier_handle_t *p_barrier) {
  pthread_barrier_destroy(&p_barrier->barrier);
}

void partix_thread_create(void (*f)(void *), void *args, int theadId) {  
  partix_task_args_t * partix_args = (partix_task_args_t*) args;
  pthread_create(&partix_args->threadHandle[theadId], NULL, pthread_func, p_thread);
}

void partix_thread_join(void *args, int threadId) {
  partix_task_args_t * partix_args = (partix_task_args_t*) args;
  pthread_join(partix_args->threadHandle[threadId], NULL);
}
