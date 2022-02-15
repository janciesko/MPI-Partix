#ifndef __PARTIX_H__
#define __PARTIX_H__

typedef struct {
    int tid;
    MPI_Comm comm;
    double elapsed_time;
    thread_handle_t * thread;
} thread_arg_t;


#endif /* __PARTIX_H__*/
