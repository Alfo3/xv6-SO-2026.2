#ifndef XV6_USER_THREAD_H
#define XV6_USER_THREAD_H

#include "kernel/types.h"

#define THREAD_MAX 16
#define THREAD_STACK_SIZE 4096

enum thread_state {
  THREAD_FREE,
  THREAD_READY,
  THREAD_RUNNING,
};

// Registers preserved across RISC-V function calls and context switches.
struct thread_context {
  uint64 ra;
  uint64 sp;
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct thread {
  struct thread_context context;
  uchar stack[THREAD_STACK_SIZE] __attribute__((aligned(16)));
  enum thread_state state;
};

// Prepare the table and register the program's main context as slot 0.
void thread_init(void);

// Create a ready thread. Returns its slot ID, or -1 when the table is full.
int thread_create(void (*fn)(void));

#endif
