#include "kernel/types.h"
#include "user/user.h"
#include "user/thread.h"

static void
worker_a(void)
{
  for (int i = 0; i < 5; i++) {
    printf("Hebra A: iteracion %d\n", i);
    thread_yield();
  }
}

static void
worker_b(void)
{
  for (int i = 0; i < 5; i++) {
    printf("Hebra B: iteracion %d\n", i);
    thread_yield();
  }
}

static void
worker_c(void)
{
  for (int i = 0; i < 5; i++) {
    printf("Hebra C: iteracion %d\n", i);
    thread_yield();
  }
}

int
main(void)
{
  int id_a, id_b, id_c;

  // 1. Inicialización
  thread_init();

  // 2. Creación de al menos 3 hebras
  id_a = thread_create(worker_a);
  id_b = thread_create(worker_b);
  id_c = thread_create(worker_c);
  if (id_a != 1 || id_b != 2 || id_c != 3) {
    printf("thread_test: unexpected initial IDs %d %d %d\n", id_a, id_b, id_c);
    exit(1);
  }

  // 4. Ejecución: encendemos el planificador
  printf("Iniciando planificador de hebras...\n");
  thread_schedule();
  printf("Todas las hebras han terminado correctamente.\n");

  exit(0);
}