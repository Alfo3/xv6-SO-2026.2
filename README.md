# Proyecto de Sistemas Operativos — xv6

Repositorio del proyecto de Sistemas Operativos sobre xv6 para RISC-V.
El desarrollo se organiza por hitos, con su implementación y documentación
técnica en este repositorio.

- **Profesor:** Viktor Tapia.
- **Integrantes:** Héctor Chanampe, Alfonso Pavez e Isidora Villegas.

## Desarrollo realizado

| Hito | Implementación |
| --- | --- |
| Hito 1: Ficheros, Pipes y Procesos (Forks) | `find`: búsqueda recursiva de archivos, integrada en xv6 y con pruebas básicas realizadas. |

Se verificaron la compilación desde cero en una copia temporal y el arranque
de la versión actual con `make qemu`.

<!--
Pendientes del Hito 1: implementar primes e integrarlo en UPROGS; completar
las pruebas de límites y de la entrega completa descritas en docs/hito1.md.
Añadir las siguientes entregas a la tabla y a la documentación cuando tengan
desarrollo realizado. La tabla actual no certifica que el Hito 1 esté completo.
-->

## Compilar y ejecutar

Se necesitan GNU Make, GCC/binutils para RISC-V, GCC anfitrión, Perl, `bc` y
`qemu-system-riscv64` (versión 7.2 o superior, según el Makefile). El Makefile
detecta prefijos habituales de la herramienta RISC-V; en la revisión se utilizó
`riscv64-linux-gnu-`.

Desde la terminal del equipo, en la raíz del repositorio:

```sh
make qemu
```

Cuando aparezca el indicador `$` de xv6, ejecutar, por ejemplo:

```text
ls
```

Este comando lista el contenido del directorio actual. Para salir de QEMU,
pulsar `Ctrl+a` y luego `x`.

## Documentación

- [Guía técnica y pruebas del Hito 1](docs/hito1.md)
