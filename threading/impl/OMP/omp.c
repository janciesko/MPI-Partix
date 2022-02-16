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

#include <omp.h>
#include <thread.h>

struct thread_handle_t {
  pthread_t thread;
  void (*f)(void *);
  void *arg;
};

void partix_parallel_for(void (*f)(void), task_args_t & task_args, int num_tasks )
{
    #pragma omp parallel for shared(request) num_threads(NUM_THREADS)
    {
        for (int taskId = 0; taskId < num_tasks; taskId++) {
            f(&task_args);
        }
    }
}


void partix_thread_library_init(void) { ; /* Empty. */ }

void partix_thread_library_finalize(void) { ; /* Empty. */ }

void partix_thread_barrier_init(int num_waiters, barrier_handle_t *p_barrier) {
  #pragma omp barrier
}

void partix_thread_barrier_wait(barrier_handle_t *p_barrier) { ; /* Empty. */ }

void partix_thread_barrier_destroy(barrier_handle_t *p_barrier) {
  ; /* Empty. */
}

void *partix_pthread_func(void *arg) { ; /* Empty. */ }

void partix_thread_create(void (*f)(void *), void *arg,
                          thread_handle_t *p_thread) {
  
}

void partix_thread_join(thread_handle_t *p_thread) { ; /* Empty. */ }
