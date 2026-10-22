#include "kernel/types.h"
#include "user/user.h"
#include "user/thread.h"

static void
worker_a(void)
{
}

static void
worker_b(void)
{
}

static void
worker_c(void)
{
}

int
main(void)
{
  int i;
  int id;
  int id_a;
  int id_b;
  int id_c;

  thread_init();

  id_a = thread_create(worker_a);
  id_b = thread_create(worker_b);
  id_c = thread_create(worker_c);
  if (id_a != 1 || id_b != 2 || id_c != 3) {
    printf("thread_test: unexpected initial IDs %d %d %d\n", id_a, id_b, id_c);
    exit(1);
  }

  for (i = 4; i < THREAD_MAX; i++) {
    id = thread_create(worker_a);
    if (id != i) {
      printf("thread_test: unexpected thread ID\n");
      exit(1);
    }
  }

  if (thread_create(worker_a) != -1) {
    printf("thread_test: table was not full\n");
    exit(1);
  }

  printf("thread_test: creation checks passed\n");
  exit(0);
}
