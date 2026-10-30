# Hito 2: Hebras de usuario

## Avance: Requerimientos A y B

La biblioteca de hebras vive por completo en espacio de usuario; no modifica el
kernel. Sus estructuras y su interfaz se encuentran en
[`user/thread.h`](../user/thread.h), mientras que la implementación está en
[`user/thread.c`](../user/thread.c).

`thread_init()` limpia la tabla y reserva el slot 0 para el contexto inicial del
programa. `thread_create(void (*fn)(void))` busca un slot libre, prepara el
contexto inicial para que `ra` apunte a `fn`, ubica `sp` en el tope alineado de
su pila privada y deja la hebra en estado `THREAD_READY`. Retorna el ID asignado
o `-1` si no hay espacio. En xv6 una rutina válida puede estar enlazada en la
dirección 0, por lo que la función no usa ese valor como centinela.

La tabla admite 16 contextos: el principal y hasta 15 hebras creadas. Cada
hebra posee una pila privada de 4096 bytes. El contexto conserva `ra`, `sp` y
los registros callee-saved `s0` a `s11`, que son los que utilizará el cambio de
contexto de los requerimientos posteriores.

Todavía no hay planificador ni `thread_yield`; por tanto, las rutinas creadas
no se ejecutan en este avance y no deben retornar. El tratamiento de retorno y
la conmutación de contexto se incorporarán con los requerimientos C y D.

## Prueba parcial

Compilar con `make qemu` y, en la consola de xv6, ejecutar:

```text
thread_test
```

El programa inicializa la biblioteca, crea tres hebras y llena la tabla para
verificar el caso sin espacio. La salida esperada es:

```text
thread_test: creation checks passed
```
