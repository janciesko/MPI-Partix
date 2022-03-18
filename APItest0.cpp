#include <partix.h>

void task(partix_task_args_t *args) {
  printf("Hello World\n");
}

int main(int argc, char *argv[]) {
  partix_config_t conf;
  partix_init(argc, argv, &conf);
  partix_library_init();

  for (int i = 0; i < conf.num_tasks; ++i) {
    partix_task(&task /*functor*/, NULL);
  }
  partix_taskwait();
  partix_library_finalize();
  return 0;
}

