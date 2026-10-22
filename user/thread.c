#include "kernel/types.h"
#include "user/user.h"
#include "user/thread.h"

static struct thread threads[THREAD_MAX];

void
thread_init(void)
{
  memset(threads, 0, sizeof(threads));
  threads[0].state = THREAD_RUNNING;
}

int
thread_create(void (*fn)(void))
{
  struct thread *thread;
  int i;

  for (i = 1; i < THREAD_MAX; i++) {
    if (threads[i].state == THREAD_FREE)
      break;
  }
  if (i == THREAD_MAX)
    return -1;

  thread = &threads[i];
  memset(&thread->context, 0, sizeof(thread->context));
  thread->context.ra = (uint64)fn;
  thread->context.sp = (uint64)(thread->stack + THREAD_STACK_SIZE);
  thread->state = THREAD_READY;

  return i;
}
