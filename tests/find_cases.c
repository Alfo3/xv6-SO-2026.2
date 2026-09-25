// Solo se integra en la imagen temporal de tests/test_find.py.
// Generado con IA: ChatGPT-6 Modelo Astra; Prompt con enfoque de detección de casos bordes.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

static int passed;
static char output[2048], errors[2048];

static void
fail(char *message)
{
  fprintf(2, "FAIL: %s\n", message);
  exit(1);
}

static void
make_dir(char *path)
{
  if (mkdir(path) < 0)
    fail(path);
}

static void
make_file(char *path)
{
  int fd = open(path, O_CREATE | O_WRONLY | O_TRUNC);
  if (fd < 0)
    fail(path);
  close(fd);
}

static void
load(char *path, char *buf)
{
  int fd = open(path, O_RDONLY), total = 0, n;
  if (fd < 0)
    fail("abrir captura");
  while ((n = read(fd, buf + total, 2047 - total)) > 0) {
    total += n;
    if (total == 2047)
      fail("captura demasiado grande");
  }
  if (n < 0)
    fail("leer captura");
  buf[total] = 0;
  close(fd);
}

static int
contains(char *text, char *part)
{
  uint len = strlen(part);
  for (; strlen(text) >= len; text++) {
    if (memcmp(text, part, len) == 0)
      return 1;
  }
  return 0;
}

static void
run(char *label, char **args, int expected_status, char *expected_output,
    char *alternative, char *expected_error, int extra_fds)
{
  int out = open("tstdout", O_CREATE | O_WRONLY | O_TRUNC);
  int err = open("tstderr", O_CREATE | O_WRONLY | O_TRUNC);
  if (out < 0 || err < 0)
    fail("crear capturas");
  int pid = fork();
  if (pid < 0)
    fail("fork");
  if (pid == 0) {
    close(1);
    if (dup(out) != 1)
      exit(98);
    close(2);
    if (dup(err) != 2)
      exit(98);
    close(out);
    close(err);
    for (int i = 0; i < extra_fds; i++) {
      if (open("/", O_RDONLY) < 0)
        exit(98);
    }
    exec("/find", args);
    exit(99);
  }
  close(out);
  close(err);
  int status = -1;
  if (wait(&status) != pid)
    fail("wait");
  load("tstdout", output);
  load("tstderr", errors);
  if (status != expected_status ||
      (strcmp(output, expected_output) != 0 &&
       (!alternative || strcmp(output, alternative) != 0)) ||
      (expected_error ? !contains(errors, expected_error) : errors[0] != 0)) {
    fprintf(2, "Caso %s: estado %d (esperado %d)\nstdout: %s\nstderr: %s\n",
            label, status, expected_status, output, errors);
    fail(label);
  }
  passed++;
  printf("PASS: %s\n", label);
}

static void
check(char *label, char *path, char *name, int status, char *out, char *error)
{
  char *args[] = {"find", path, name, 0};
  run(label, args, status, out, 0, error, 0);
}

int
main(void)
{
  make_dir("tfind");
  make_dir("tfind/sub");
  make_dir("empty");
  make_file("tfind/notas.txt");
  make_file("tfind/sub/notas.txt");
  make_file("tfind/sub/otro.txt");
  make_file("tfind/abcdefghijklmn");
  make_file("tfind/borrado");
  if (unlink("tfind/borrado") < 0)
    fail("unlink");
  make_dir("tfind/solo_dir");
  if (mknod("tfind/device", 1, 0) < 0)
    fail("mknod");

  char *basic[] = {"find", "tfind", "notas.txt", 0};
  run("coincidencias recursivas", basic, 0,
      "tfind/notas.txt\ntfind/sub/notas.txt\n",
      "tfind/sub/notas.txt\ntfind/notas.txt\n", 0, 0);
  check("sin coincidencias", "tfind", "ausente", 0, "", 0);
  check("coincidencia parcial", "tfind", "notas", 0, "", 0);
  check("mayusculas distintas", "tfind", "NOTAS.TXT", 0, "", 0);
  check("carpeta vacia", "empty", "notas.txt", 0, "", 0);
  check("entrada libre", "tfind", "borrado", 0, "", 0);
  check("directorio no es archivo", "tfind", "solo_dir", 0, "", 0);
  check("dispositivo no es archivo", "tfind", "device", 0, "", 0);
  check("nombre de 14 bytes", "tfind", "abcdefghijklmn", 0,
        "tfind/abcdefghijklmn\n", 0);
  check("ruta absoluta", "/tfind/sub", "notas.txt", 0, "/tfind/sub/notas.txt\n",
        0);
  check("barra final", "tfind/sub/", "notas.txt", 0, "tfind/sub/notas.txt\n",
        0);
  check("barras repetidas", "tfind//sub//", "notas.txt", 0,
        "tfind//sub//notas.txt\n", 0);
  check("ruta con punto", "./tfind/sub", "notas.txt", 0,
        "./tfind/sub/notas.txt\n", 0);
  check("ruta con padre", "tfind/sub/..", "abcdefghijklmn", 0,
        "tfind/sub/../abcdefghijklmn\n", 0);
  check("directorio raiz", "/", "find", 0, "/find\n", 0);
  check("ruta inexistente", "inexistente", "x", 1, "", "no se pudo abrir");
  check("archivo inicial", "tfind/notas.txt", "notas.txt", 1, "",
        "no es un directorio");
  check("dispositivo inicial", "tfind/device", "device", 1, "",
        "no es un directorio");
  check("ruta vacia", "", "x", 1, "", "ruta vacia");
  check("nombre vacio", "tfind", "", 1, "", "nombre invalido");
  check("nombre largo", "tfind", "abcdefghijklmno", 1, "", "nombre invalido");
  check("nombre con barra", "tfind", "sub/notas.txt", 1, "", "nombre invalido");
  check("nombre punto", "tfind", ".", 1, "", "nombre invalido");
  check("nombre padre", "tfind", "..", 1, "", "nombre invalido");
  check("componente largo", "tfind/abcdefghijklmno", "x", 1, "",
        "componente de ruta");
  char *missing[] = {"find", 0};
  char *one[] = {"find", "tfind", 0};
  char *extra[] = {"find", "tfind", "notas.txt", "extra", 0};
  run("sin argumentos", missing, 1, "", 0, "Uso:", 0);
  run("un argumento", one, 1, "", 0, "Uso:", 0);
  run("argumento extra", extra, 1, "", 0, "Uso:", 0);

  // exec directo evita el limite del buffer de entrada del shell.
  char longpath[MAXPATH + 1];
  strcpy(longpath, "empty");
  memset(longpath + 5, '/', MAXPATH - 5);
  longpath[MAXPATH - 1] = 0;
  check("ruta inicial de 127", longpath, "x", 0, "", 0);
  longpath[MAXPATH - 1] = '/';
  longpath[MAXPATH] = 0;
  check("ruta inicial de 128", longpath, "x", 1, "",
        "ruta vacia o demasiado larga");

  make_dir("depth");
  char path[MAXPATH], expected[MAXPATH + 2];
  strcpy(path, "depth");
  for (int level = 2; level <= 8; level++) {
    strcpy(path + strlen(path), "/d");
    make_dir(path);
  }
  strcpy(expected, path);
  strcpy(expected + strlen(expected), "/hit");
  make_file(expected);
  strcpy(expected + strlen(expected), "\n");
  check("nivel 8 permitido", "depth", "hit", 0, expected, 0);
  strcpy(path + strlen(path), "/d");
  make_dir(path);
  check("nivel 9 rechazado", "depth", "hit", 1, expected,
        "limite de 8 niveles");
  make_dir("depth/later");
  make_file("depth/later/ok");
  check("continuar tras profundidad", "depth", "ok", 1, "depth/later/ok\n",
        "limite de 8 niveles");

  // Ocho componentes de 14 bytes: directorio de longitud 119.
  strcpy(path, "aaaaaaaaaaaaaa");
  make_dir(path);
  for (int i = 1; i < 8; i++) {
    strcpy(path + strlen(path), "/aaaaaaaaaaaaaa");
    make_dir(path);
  }
  if (chdir(path) < 0)
    fail("chdir ruta larga");
  make_file("1234567");
  if (chdir("/") < 0)
    fail("chdir raiz");
  strcpy(expected, path);
  strcpy(expected + strlen(expected), "/1234567\n");
  check("resultado de 127", "aaaaaaaaaaaaaa", "1234567", 0, expected, 0);
  if (chdir(path) < 0)
    fail("chdir ruta larga");
  make_file("12345678");
  make_file("ok");
  if (chdir("/") < 0)
    fail("chdir raiz");
  check("resultado de 128 rechazado", "aaaaaaaaaaaaaa", "1234567", 1, expected,
        "ruta demasiado larga");
  strcpy(expected, path);
  strcpy(expected + strlen(expected), "/ok\n");
  check("continuar tras ruta larga", "aaaaaaaaaaaaaa", "ok", 1, expected,
        "ruta demasiado larga");

  run("sin FD para open", basic, 1, "", 0, "no se pudo abrir", NOFILE - 3);
  run("sin FD para stat", basic, 1, "", 0, "no se pudo obtener el estado",
      NOFILE - 4);
  for (int i = 0; i < 12; i++)
    check("ejecucion repetida", "tfind/sub", "notas.txt", 0,
          "tfind/sub/notas.txt\n", 0);
  printf("FIND TESTS PASSED: %d\n", passed);
  exit(0);
}
