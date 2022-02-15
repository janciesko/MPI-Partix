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
#include <pthread.h>

struct thread_handle_t {
    pthread_t thread;
    void (*f)(void *);
    void *arg;
};

struct barrier_handle_t {
    pthread_barrier_t barrier;
};

void thread_library_init(void)
{
    ; /* Empty. */
}

void thread_library_finalize(void)
{
    ; /* Empty. */
}

void thread_barrier_init(int num_waiters, barrier_handle_t *p_barrier)
{
    pthread_barrier_init(&p_barrier->barrier, NULL, num_waiters);
}

void thread_barrier_wait(barrier_handle_t *p_barrier)
{
    pthread_barrier_wait(&p_barrier->barrier);
}

void thread_barrier_destroy(barrier_handle_t *p_barrier)
{
    pthread_barrier_destroy(&p_barrier->barrier);
}

void *pthread_func(void *arg)
{
    thread_handle_t *p_thread = (thread_handle_t *)arg;
    p_thread->f(p_thread->arg);
    return NULL;
}

void thread_create(void (*f)(void *), void *arg,
                          thread_handle_t *p_thread)
{
    p_thread->f = f;
    p_thread->arg = arg;
    pthread_create(&p_thread->thread, NULL, pthread_func, p_thread);
}

void thread_join(thread_handle_t *p_thread)
{
    pthread_join(p_thread->thread, NULL);
}
