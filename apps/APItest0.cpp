/*
//@HEADER
// ************************************************************************
//
//                        Partix 1.0
//              Copyright Type Date and Name (2022)
//
// Questions? Contact Jan Ciesko (jciesko@sandia.gov)
//
// ************************************************************************
//@HEADER
*/


#include <partix.h>
#include <cstdio>

void task(partix_task_args_t *args) { printf("Hello World\n"); }

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  partix_context_t ctx;

  for (int i = 0; i < conf.num_tasks; ++i) {
    partix_task(&task /*functor*/, NULL, &ctx);
  }
  partix_taskwait(&ctx);
  partix_library_finalize();
  return 0;
}