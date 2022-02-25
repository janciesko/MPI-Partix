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
#include <map>
#include <cassert>

#define SUCCEED(f) assert((f)!= 0)
#define MAX_THREADS 128

partix_mutex_t global_mutex;
partix_mutex_t context_mutex;
partix_config_t * global_conf;

typedef struct 
{
  pthread_t thread;
  partix_task_args_t args;
} thread_handle_t;

/* We need this to implement taskwait*/
typedef struct
{
  thread_handle_t threadHandle[MAX_THREADS];
  int context_task_counter;
} partix_handle_t;

typedef std::map<size_t, partix_handle_t*> partix_context_map_t;
partix_context_map_t context_map; 

__attribute__ ((noinline))
size_t get_context(size_t context)
{
  (void) context;
  const unsigned int ctx = 1;
  size_t addr = (size_t) __builtin_extract_return_addr (__builtin_return_address (1));
  return addr;
}

thread_handle_t * register_task(size_t context)
{
  partix_context_map_t::iterator it;
  partix_mutex_enter(&context_mutex);
  it = context_map.find(context);

  if (it != context_map.end())
  {
    partix_handle_t * context_handle = it->second;
    context_handle->context_task_counter++;
    partix_mutex_exit(&context_mutex); 
    return &context_handle->threadHandle[context_handle->context_task_counter];
  }else 
  {
    partix_handle_t * context_handle = (partix_handle_t *) calloc (1, sizeof(partix_handle_t));
    const auto it_insert = context_map.insert(std::pair<size_t,partix_handle_t*>(context,context_handle));
    if(it_insert.second){/*Insert successful*/}
    context_handle->context_task_counter++;
    partix_mutex_exit(&context_mutex); 
    return &context_handle->threadHandle[context_handle->context_task_counter];
  }
  assert(false); //DO NOT REACH HERE
}


void partix_mutex_enter()
{
  SUCCEED(pthread_mutex_lock(&global_mutex));
}

void partix_mutex_exit()
{
  SUCCEED(pthread_mutex_unlock(&global_mutex));
}

void partix_mutex_enter(partix_mutex_t  * m)
{
  SUCCEED(pthread_mutex_lock(m));
}

void partix_mutex_exit(partix_mutex_t  * m)
{
  SUCCEED(pthread_mutex_unlock(m));
}

void partix_mutex_init(partix_mutex_t  * m)
{
  SUCCEED(pthread_mutex_init(m, NULL));
}

void partix_mutex_destroy(partix_mutex_t * m)
{
  SUCCEED(pthread_mutex_destroy(m));
}

int partix_executor_id(void) { return pthread_self(); };

void partix_library_init(void) { 
  partix_mutex_init(&global_mutex);
  partix_mutex_init(&context_mutex);
  
 }

void partix_library_finalize(void) { 
  partix_mutex_destroy(&global_mutex);
  partix_mutex_destroy(&context_mutex);
 }

void partix_thread_create(void (*f)(partix_task_args_t *), void *args, pthread_t * handle ){
  SUCCEED(pthread_create(handle, NULL, (void* (*)(void*))f, args));
}

void partix_thread_join(pthread_t handle) {
  SUCCEED(pthread_join(handle, NULL));
}

__attribute__ ((noinline))
void partix_task(void (*f)(partix_task_args_t *), void *user_args) {
  size_t context = get_context(0);
  thread_handle_t * threadhandle = register_task(context);
  partix_task_args_t * partix_args = & threadhandle->args;
  partix_args->user_task_args = user_args;
  partix_args->conf = global_conf;
  partix_thread_create(f, partix_args, &threadhandle->thread); 
}

void partix_taskwait()
{
  size_t context = get_context(0);
  partix_context_map_t::iterator it;
  it = context_map.find(context);
  if (it == context_map.end())
  return;

  partix_handle_t * context_handle = it->second;
  for(int i = 0; i < context_handle->context_task_counter; ++i)
  {
    partix_thread_join(context_handle->threadHandle[i].thread);
  }
  free(context_handle);
  partix_mutex_enter(&context_mutex);
  context_map.erase(it);
  partix_mutex_exit(&context_mutex); 
  return;
}

