#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

#define MAX_DEPTH 8

static int
valid_path(char *path)
{
  int component = 0;

  if (path[0] == 0 || strlen(path) >= MAXPATH) {
    fprintf(2, "find: ruta vacia o demasiado larga (maximo %d bytes)\n",
            MAXPATH - 1);
    return 0;
  }
  for (char *p = path; *p; p++) {
    if (*p == '/') {
      component = 0;
    } else if (++component > DIRSIZ) {
      fprintf(2, "find: componente de ruta mayor que %d bytes\n", DIRSIZ);
      return 0;
    }
  }
  return 1;
}

static int
search(char *path, char *name, int depth)
{
  char buf[MAXPATH], entry[DIRSIZ + 1];
  struct dirent de;
  struct stat st;
  int fd, n, result = 0;
  uint len = strlen(path);
  int slash = path[len - 1] != '/';

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: no se pudo abrir %s\n", path);
    return 1;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: no se pudo obtener el estado de %s\n", path);
    result = 1;
    goto done;
  }
  if (st.type != T_DIR) {
    fprintf(2, "find: no es un directorio: %s\n", path);
    result = 1;
    goto done;
  }

  while ((n = read(fd, &de, sizeof(de))) != 0) {
    if (n != sizeof(de)) {
      fprintf(2, "find: %s en %s\n",
              n < 0 ? "error de lectura" : "entrada de directorio incompleta",
              path);
      result = 1;
      break;
    }
    if (de.inum == 0)
      continue;

    // Un nombre de DIRSIZ bytes en disco puede no tener terminador nulo.
    memmove(entry, de.name, DIRSIZ);
    entry[DIRSIZ] = 0;
    if (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0)
      continue;

    if (len + slash + strlen(entry) + 1 > sizeof(buf)) {
      fprintf(2, "find: ruta demasiado larga: %s%s%s\n", path, slash ? "/" : "",
              entry);
      result = 1;
      continue;
    }
    strcpy(buf, path);
    if (slash)
      buf[len] = '/';
    strcpy(buf + len + slash, entry);

    if (stat(buf, &st) < 0) {
      fprintf(2, "find: no se pudo obtener el estado de %s\n", buf);
      result = 1;
      continue;
    }
    if (st.type == T_FILE) {
      if (strcmp(entry, name) == 0)
        printf("%s\n", buf);
    } else if (st.type == T_DIR) {
      // No reservar un noveno marco de recursion: informar y seguir la rama actual.
      if (depth == MAX_DEPTH) {
        fprintf(2, "find: limite de %d niveles excedido: %s\n", MAX_DEPTH, buf);
        result = 1;
      } else if (search(buf, name, depth + 1) != 0) {
        // Acumular errores sin dejar de visitar las demas entradas.
        result = 1;
      }
    }
  }

done:
  // Toda ruta posterior a un open exitoso converge en este unico cierre.
  close(fd);
  return result;
}

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(2, "Uso: find <ruta> <nombre>\n");
    exit(1);
  }
  if (!valid_path(argv[1]))
    exit(1);
  if (argv[2][0] == 0 || strlen(argv[2]) > DIRSIZ || strchr(argv[2], '/') ||
      strcmp(argv[2], ".") == 0 || strcmp(argv[2], "..") == 0) {
    fprintf(2,
            "find: nombre invalido (1 a %d bytes, sin /, distinto de . y ..)\n",
            DIRSIZ);
    exit(1);
  }
  exit(search(argv[1], argv[2], 1));
}
