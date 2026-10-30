#include "kernel/types.h"
#include "user/user.h"
#include "user/thread.h"

static struct thread threads[THREAD_MAX];
static struct thread *current_thread;

static void
thread_entry(void)
{
  current_thread->fn();

  current_thread->state = THREAD_FREE;

  thread_schedule();
}

void
thread_init(void)
{
  memset(threads, 0, sizeof(threads));
  threads[0].state = THREAD_RUNNING;
  current_thread = &threads[0];
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
  thread->fn = fn;
  thread->context.ra = (uint64)thread_entry;
  thread->context.sp = (uint64)(thread->stack + THREAD_STACK_SIZE);
  thread->state = THREAD_READY;

  return i;
}

void
thread_schedule(void)
{
  struct thread *t, *next_thread;
  int i, current_idx;

  current_idx = current_thread - threads;
  next_thread = 0;

  for (i = 1; i <= THREAD_MAX; i++) {
    int idx = (current_idx + i) % THREAD_MAX;
    if (idx == 0)
      continue;
    t = &threads[idx];
    if (t->state == THREAD_READY) {
      next_thread = t;
      break;
    }
  }

  if (next_thread == 0) {
    if (current_thread != &threads[0] &&
        current_thread->state == THREAD_READY) {
      next_thread = current_thread;
    } else if (threads[0].state == THREAD_READY) {
      next_thread = &threads[0];
    } else {
      current_thread->state = THREAD_RUNNING;
      return;
    }
  }

  if (current_thread->state == THREAD_RUNNING) {
    current_thread->state = THREAD_READY;
  }

  next_thread->state = THREAD_RUNNING;

  if (current_thread != next_thread) {
    struct thread *prev_thread = current_thread;
    current_thread = next_thread;
    thread_switch(&prev_thread->context, &next_thread->context);
  }
}

void
thread_yield(void)
{
  current_thread->state = THREAD_READY;
  thread_schedule();
}
