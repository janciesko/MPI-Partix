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

#include <cassert>
#include <map>
#include <thread.h>
#include <malloc.h>

#define SUCCEED(val) assert(val == 0)
#define MAX_THREADS 1048576

partix_mutex_t global_mutex;
partix_mutex_t context_mutex;
partix_config_t *global_conf;

typedef struct {
  aligned_t thread;
  partix_task_args_t args;
} thread_handle_t;

/* We need this to implement taskwait*/
typedef struct {
  thread_handle_t threadHandle[MAX_THREADS];
  int context_task_counter;
} partix_handle_t;

typedef std::map<size_t, partix_handle_t *> partix_context_map_t;
partix_context_map_t context_map;

__attribute__((noinline)) size_t get_context() {
  debug("get_context_outer");
  const unsigned int level = 2;
  size_t addr =
      (size_t)__builtin_extract_return_addr(__builtin_frame_address(level));
  return addr;
}

thread_handle_t *register_task(size_t context) {
  partix_context_map_t::iterator it;
  partix_mutex_enter(&context_mutex);
  it = context_map.find(context);
  if (it != context_map.end()) {
    partix_handle_t *context_handle = it->second;
    debug("register_task, it != context_map.end()");
    partix_mutex_exit(&context_mutex);
    return &context_handle
                ->threadHandle[context_handle->context_task_counter++];
  } else {
    partix_handle_t *context_handle =
        (partix_handle_t *)calloc(1, sizeof(partix_handle_t));
    const auto it_insert = context_map.insert(
        std::pair<size_t, partix_handle_t *>(context, context_handle));
    if (it_insert.second) { /*Insert successful*/
    }
    debug("register_task, it == context_map.end()");
    partix_mutex_exit(&context_mutex);
    return &context_handle
                ->threadHandle[context_handle->context_task_counter++];
  }
  assert(false); // DO NOT REACH HERE
}

void partix_mutex_enter() {
  debug("partix_mutex_enter");
  int ret = qthread_lock(&global_mutex);
  SUCCEED(ret);
}

void partix_mutex_exit() {
  debug("partix_mutex_exit");
  int ret = qthread_unlock(&global_mutex);
  SUCCEED(ret);
}

void partix_mutex_enter(partix_mutex_t *m) {
  debug("partix_mutex_enter");
  int ret = qthread_lock(m);
  SUCCEED(ret);
}

void partix_mutex_exit(partix_mutex_t *m) {
  debug("partix_mutex_exit");
  int ret = qthread_unlock(m);
  SUCCEED(ret);
}

void partix_mutex_init(partix_mutex_t *m) {
  /*EMPTY*/
}

void partix_mutex_destroy(partix_mutex_t *m) {
  /*EMPTY*/
}

int partix_executor_id(void) {
  debug("partix_executor_id");
  return qthread_id();
};

void partix_library_init(void) {
  debug("partix_library_init");
  qthread_initialize();
  partix_mutex_init(&global_mutex);
  partix_mutex_init(&context_mutex);
}

void partix_library_finalize(void) {
  debug("partix_library_finalize");
  partix_mutex_destroy(&global_mutex);
  partix_mutex_destroy(&context_mutex);
  qthread_finalize();
}

void partix_thread_create(void (*f)(partix_task_args_t *), void *args,
                     aligned_t *handle) {
  debug("partix_thread_create");
  int ret = qthread_fork((long unsigned int (*)(void *))f, args, handle);
  SUCCEED(ret);
}

void partix_thread_join(aligned_t * handle) {
  debug("partix_thread_join");
  int ret = qthread_readFF(NULL, handle);
  SUCCEED(ret);
}

__attribute__((noinline)) void partix_task(void (*f)(partix_task_args_t *),
                                           void *user_args) {
  size_t context = get_context();
  thread_handle_t *threadhandle = register_task(context);
  partix_task_args_t *partix_args = &threadhandle->args;
  partix_args->user_task_args = user_args;
  partix_args->conf = global_conf;
  debug("partix_task");
  partix_thread_create(f, partix_args, &threadhandle->thread);
}

__attribute__((noinline)) void partix_taskwait() {
  size_t context = get_context();
  partix_context_map_t::iterator it;
  it = context_map.find(context);
  if (it == context_map.end()) {
    debug("partix_taskwait, it == context_map.end()");
    return;
  }
  debug("partix_taskwait, it != context_map.end()");

  partix_handle_t *context_handle = it->second;
  for (int i = 0; i < context_handle->context_task_counter; ++i) {
    debug("partix_taskwait, partix_thread_join");
    partix_thread_join(&context_handle->threadHandle[i].thread);
  }
  free(context_handle);
  partix_mutex_enter(&context_mutex);
  context_map.erase(it);
  partix_mutex_exit(&context_mutex);
}
