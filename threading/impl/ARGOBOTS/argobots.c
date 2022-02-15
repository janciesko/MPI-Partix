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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <abt.h>
#include <thread.h>

typedef struct abt_global_t {
  char pad1[64];
  int num_xstreams;
  int num_vcis;
  ABT_xstream *xstreams;
  ABT_pool *shared_pools, *priv_pools;
  ABT_sched *scheds;
  ABT_xstream_barrier xstream_barrier;
  int first_init;
  char pad2[64];
} abt_global_t;
abt_global_t g_abt_global;

struct thread_handle_t {
  ABT_thread thread;
};

struct barrier_handle_t {
  ABT_barrier barrier;
};

inline uint32_t xorshift_rand32(uint32_t *p_seed) {
  /* George Marsaglia, "Xorshift RNGs", Journal of Statistical Software,
   * Articles, 2003 */
  uint32_t seed = *p_seed;
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  *p_seed = seed;
  return seed;
}

int sched_init(ABT_sched sched, ABT_sched_config config) { return ABT_SUCCESS; }

void sched_run(ABT_sched sched) {
  const int work_count_mask_local = 16 - 1;
  const int work_count_mask_remote = 256 - 1;
  const int work_count_mask_event = 8192 - 1;
  int rank;
  ABT_self_get_xstream_rank(&rank);

  uint64_t my_vcimask = 0;
  if (g_abt_global.first_init == 0) {
    for (int i = rank; i < 64; i += g_abt_global.num_xstreams) {
      my_vcimask += ((uint64_t)1) << ((uint64_t)i);
    }
    MPIX_Set_exp_info(MPIX_INFO_TYPE_VCIMASK, &my_vcimask, 0);
    ABT_xstream_barrier_wait(g_abt_global.xstream_barrier);
    g_abt_global.first_init = 1;
  }
  int num_pools;
  ABT_sched_get_num_pools(sched, &num_pools);
  ABT_pool *all_pools = (ABT_pool *)malloc(num_pools * sizeof(ABT_pool));
  ABT_sched_get_pools(sched, num_pools, 0, all_pools);
  ABT_pool my_shared_pool = all_pools[0];
  ABT_pool my_priv_pool = all_pools[1];
  int num_shared_pools = num_pools - 2;
  ABT_pool *shared_pools = all_pools + 2;

  uint32_t seed = (uint32_t)((intptr_t)all_pools);

  int work_count = 0;
  while (1) {
    int local_work_count = 0;
    ABT_unit unit;
    /* Try to pop a ULT from a local pool */
    ABT_pool_pop(my_priv_pool, &unit);
    if (unit != ABT_UNIT_NULL) {
      /* Move this unit to my_shared_pool. */
      ABT_xstream_run_unit(unit, my_priv_pool);
      local_work_count++;
      work_count++;
    }
    if (local_work_count == 0 || ((work_count & work_count_mask_local) == 0)) {
      ABT_pool_pop(my_shared_pool, &unit);
      if (unit != ABT_UNIT_NULL) {
        ABT_xstream_run_unit(unit, my_shared_pool);
        local_work_count++;
        work_count++;
      }
    }
    if (local_work_count == 0 || ((work_count & work_count_mask_remote) == 0)) {
      /* RWS */
      if (num_shared_pools > 0) {
        uint32_t rand_num = xorshift_rand32(&seed);
        ABT_pool victim_pool = shared_pools[rand_num % num_shared_pools];
        ABT_pool_pop(victim_pool, &unit);
        if (unit != ABT_UNIT_NULL) {
          ABT_unit_set_associated_pool(unit, my_shared_pool);
          ABT_xstream_run_unit(unit, my_shared_pool);
          local_work_count++;
          work_count++;
        }
      }
    }
    work_count++;
    if ((work_count & work_count_mask_event) == 0) {
      ABT_bool stop;
      ABT_xstream_check_events(sched);
      ABT_sched_has_to_stop(sched, &stop);
      if (stop == ABT_TRUE) {
        break;
      }
    }
  }
  free(all_pools);
}

int sched_free(ABT_sched sched) { return ABT_SUCCESS; }

void thread_library_init(void) {
  int ret;
  ret = ABT_init(0, 0);
  assert(ret == ABT_SUCCESS);

  int num_xstreams = 1;
  if (getenv("ABT_NUM_XSTREAMS")) {
    num_xstreams = atoi(getenv("ABT_NUM_XSTREAMS"));
    if (num_xstreams < 0)
      num_xstreams = 1;
  }
  int num_vcis = 1;
  if (getenv("MPIR_CVAR_CH4_NUM_VCIS")) {
    num_vcis = atoi(getenv("MPIR_CVAR_CH4_NUM_VCIS"));
  }

  g_abt_global.num_xstreams = num_xstreams;
  g_abt_global.num_vcis = num_vcis;
  g_abt_global.xstreams =
      (ABT_xstream *)malloc(sizeof(ABT_xstream) * num_xstreams);
  g_abt_global.shared_pools =
      (ABT_pool *)malloc(sizeof(ABT_pool) * num_xstreams);
  g_abt_global.priv_pools = (ABT_pool *)malloc(sizeof(ABT_pool) * num_xstreams);
  g_abt_global.scheds = (ABT_sched *)malloc(sizeof(ABT_sched) * num_xstreams);
  ret = ABT_xstream_barrier_create(num_xstreams, &g_abt_global.xstream_barrier);
  assert(ret == ABT_SUCCESS);
  /* Create pools. */
  for (int i = 0; i < num_xstreams; i++) {
    ret = ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, ABT_TRUE,
                                &g_abt_global.shared_pools[i]);
    assert(ret == ABT_SUCCESS);
    ret = ABT_pool_create_basic(ABT_POOL_FIFO, ABT_POOL_ACCESS_MPMC, ABT_TRUE,
                                &g_abt_global.priv_pools[i]);
    assert(ret == ABT_SUCCESS);
  }
  /* Create schedulers. */
  ABT_sched_def sched_def = {
      .type = ABT_SCHED_TYPE_ULT,
      .init = sched_init,
      .run = sched_run,
      .free = sched_free,
      .get_migr_pool = NULL,
  };
  for (int i = 0; i < num_xstreams; i++) {
    ABT_pool *tmp = (ABT_pool *)malloc(sizeof(ABT_pool) * num_xstreams + 1);
    int pool_index = 0;
    tmp[pool_index++] = g_abt_global.shared_pools[i];
    tmp[pool_index++] = g_abt_global.priv_pools[i];
    for (int j = 1; j < num_xstreams; j++) {
      tmp[pool_index++] = g_abt_global.shared_pools[(i + j) % num_xstreams];
    }
    ret = ABT_sched_create(&sched_def, num_xstreams + 1, tmp,
                           ABT_SCHED_CONFIG_NULL, &g_abt_global.scheds[i]);
    assert(ret == ABT_SUCCESS);
    free(tmp);
  }

  /* Create secondary execution streams. */
  for (int i = 1; i < num_xstreams; i++) {
    ret = ABT_xstream_create(g_abt_global.scheds[i], &g_abt_global.xstreams[i]);
    assert(ret == ABT_SUCCESS);
  }

  /* Set up a primary execution stream. */
  ret = ABT_xstream_self(&g_abt_global.xstreams[0]);
  assert(ret == ABT_SUCCESS);
  ret = ABT_xstream_set_main_sched(g_abt_global.xstreams[0],
                                   g_abt_global.scheds[0]);
  assert(ret == ABT_SUCCESS);

  /* Execute a scheduler once. */
  ret = ABT_self_yield();
  assert(ret == ABT_SUCCESS);
}

void thread_library_finalize(void) {
  int ret;
  /* Join secondary execution streams. */
  for (int i = 1; i < g_abt_global.num_xstreams; i++) {
    ret = ABT_xstream_join(g_abt_global.xstreams[i]);
    assert(ret == ABT_SUCCESS);
    ret = ABT_xstream_free(&g_abt_global.xstreams[i]);
    assert(ret == ABT_SUCCESS);
  }
  /* Free secondary execution streams' schedulers */
  for (int i = 1; i < g_abt_global.num_xstreams; i++) {
    ret = ABT_sched_free(&g_abt_global.scheds[i]);
    assert(ret == ABT_SUCCESS);
  }
  ret = ABT_xstream_barrier_free(&g_abt_global.xstream_barrier);
  assert(ret == ABT_SUCCESS);

  ret = ABT_finalize();
  assert(ret == ABT_SUCCESS);
  free(g_abt_global.xstreams);
  g_abt_global.xstreams = NULL;
  free(g_abt_global.shared_pools);
  g_abt_global.shared_pools = NULL;
  free(g_abt_global.priv_pools);
  g_abt_global.priv_pools = NULL;
  free(g_abt_global.scheds);
  g_abt_global.scheds = NULL;
}

void thread_barrier_init(int num_waiters, barrier_handle_t *p_barrier) {
  int ret;
  ret = ABT_barrier_create(num_waiters, &p_barrier->barrier);
  assert(ret == ABT_SUCCESS);
}

void thread_barrier_wait(barrier_handle_t *p_barrier) {
  int ret;
  ret = ABT_barrier_wait(p_barrier->barrier);
  assert(ret == ABT_SUCCESS);
}

void thread_barrier_destroy(barrier_handle_t *p_barrier) {
  int ret;
  ret = ABT_barrier_free(&p_barrier->barrier);
  assert(ret == ABT_SUCCESS);
}

void thread_create(void (*f)(void *), void *arg, thread_handle_t *p_thread) {
  int ret, rank;
  ret = ABT_self_get_xstream_rank(&rank);
  assert(ret == ABT_SUCCESS);
  ABT_pool pool = g_abt_global.shared_pools[rank];
  ret =
      ABT_thread_create(pool, f, arg, ABT_THREAD_ATTR_NULL, &p_thread->thread);
  assert(ret == ABT_SUCCESS);
}

void thread_join(thread_handle_t *p_thread) {
  int ret;
  ret = ABT_thread_free(&p_thread->thread);
  assert(ret == ABT_SUCCESS);
}