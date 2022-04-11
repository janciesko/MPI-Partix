#ifndef __PARTIX_THREAD_H__
#define __PARTIX_THREAD_H__

#include <generic_types.h>
#include <types.h>

#if defined(DEBUG)
#define debug(m) printf("%s\n", m)
#else
#define debug(m)
#endif

__attribute__((noinline)) void partix_task(void (*)(partix_task_args_t *),
                                           void *, partix_context_t *);
__attribute__((noinline)) void partix_taskwait(partix_context_t *);

void partix_library_init(void);
void partix_library_finalize(void);

void partix_mutex_enter(void);
void partix_mutex_exit(void);

void partix_mutex_enter(partix_mutex_t *);
void partix_mutex_exit(partix_mutex_t *);

void partix_mutex_init(partix_mutex_t *);
void partix_mutex_destroy(partix_mutex_t *);

int partix_executor_id(void);

/*Non-conexted functions are deprecated*/
__attribute__((noinline)) void partix_task(void (*)(partix_task_args_t *),
                                           void *);
__attribute__((noinline)) void partix_taskwait(void);

#endif /* __PARTIX_THREAD_H__ */
