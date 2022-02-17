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

#ifndef __PARTIX_THREAD_H__
#define __PARTIX_THREAD_H__

#include <types.h>

typedef struct thread_handle_t thread_handle_t;
typedef struct barrier_handle_t barrier_handle_t;

//barrier_handle_t *g_barrier;

void partix_parallel_for(void (*)(partix_task_args_t *), void *);
void partix_parallel_for(void (*)(partix_task_args_t *), void *, partix_config_t *);
void partix_task(void (*)(partix_task_args_t *), void *);
void partix_task(void (*)(partix_task_args_t *), void *, partix_config_t *);
void partix_critical_enter(void);
void partix_critical_exit(void);
void partix_taskwait(void);
int partix_executor_id(void);
void partix_barrier();
void partix_thread_library_init(void);
void partix_thread_library_finalize(void);
void partix_thread_barrier_init(int, barrier_handle_t *);
void partix_thread_barrier_wait(void);
void partix_thread_barrier_destroy(barrier_handle_t *);
void *partix_pthread_func(void *);
void partix_thread_create(void (*f)(void *), void *, thread_handle_t *);
void partix_thread_join(thread_handle_t *);

#endif /* __PARTIX_THREAD_H__ */
