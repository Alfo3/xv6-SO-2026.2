# Hito 1: Ficheros, Pipes y Procesos (Forks)

## Desarrollo realizado

Se implementó [find.c](../user/find.c), un programa de usuario que busca archivos por nombre y recorre subdirectorios recursivamente.

## Uso y resultados

```text
find <carpeta> <nombre>
```

La búsqueda compara el nombre completo, imprime una ruta por archivo coincidente en la salida
estándar (descriptor 1); los errores se escriben en el descriptor 2.

Los límites se cuentan en bytes; para nombres ASCII cada carácter ocupa un byte.

La ruta impresa conserva la base recibida. Por ejemplo, `find . notas.txt` puede
imprimir `./sub/notas.txt`. También acepta rutas absolutas, `.` y `..` en la ruta
inicial, barras repetidas y barras finales. El orden depende de las entradas del disco.

| Estado de salida | Significado |
| --- | --- |
| `0` | Recorrido completo, con o sin coincidencias. |
| `1` | Argumentos inválidos o algún error que impidió completar el recorrido. Puede haber coincidencias válidas impresas. |

## Explicación del código

### Validación: `main` y `valid_path`

`main` comprueba la cantidad de argumentos, la ruta y el nombre. Si alguna
validación falla, informa el motivo y termina con 1. En caso contrario, devuelve
el resultado de `search`, comenzando con profundidad 1.

`valid_path` recorre los componentes separados por `/` y comprueba sus longitudes.
La existencia y el tipo de la ruta se verifican después mediante `open` y `fstat`.

### Recorrido: `search`

Cada llamada abre un directorio y realiza estos pasos:

1. Consulta su tipo con `fstat`; rechaza archivos y dispositivos como ruta inicial.
2. Lee entradas de tipo `struct dirent` con `read`.
3. Omite entradas libres (`inum == 0`), `.` y `..`.
4. Copia el nombre a un buffer de `DIRSIZ + 1` bytes y agrega el terminador nulo.
5. Comprueba que la ruta hija, incluida su terminación nula, quepa en `MAXPATH`.
6. Consulta el tipo con `stat`. Imprime los archivos regulares coincidentes y
   recurre sobre los directorios dentro del límite de profundidad.
7. Acumula los errores de las ramas visitadas y cierra su descriptor al terminar.

## Supuestos y decisiones con su justificación

### Interpretación del enunciado y entorno de ejecución

| Supuesto o interpretación | Justificación | Consecuencia |
| --- | --- | --- |
| La ruta inicial debe ser una carpeta. | El enunciado pide la carpeta donde empezar a buscar; no solicita buscar sobre un archivo inicial. | Un archivo o dispositivo como ruta inicial produce un diagnóstico y estado 1. |
| Se buscan archivos regulares por nombre exacto. | Se interpreta el nombre solicitado como un nombre literal, y se distingue el archivo buscado de los directorios que se recorren. No se solicitan patrones. | Se usa `strcmp`, se distinguen mayúsculas y no se imprimen directorios ni dispositivos como coincidencias. |
| La ruta completa incluye la carpeta inicial y todos los subdirectorios hasta el archivo. | El ejemplo permite iniciar desde `.`; conservar esa base identifica la ubicación del resultado sin requerir convertirla a una ruta absoluta. | `find . notas.txt` puede imprimir `./sub/notas.txt`. Esta es la interpretación adoptada de «ruta completa». |

### Decisiones de implementación

| Decisión | Justificación | Comportamiento |
| --- | --- | --- |
| Limitar el recorrido a ocho directorios simultáneos. | Cada llamada recursiva conserva un descriptor y ocupa pila. El límite acordado deja margen dentro de los recursos de xv6 sin modificar el kernel. | La carpeta inicial es el nivel 1. Se inspeccionan archivos hasta el nivel 8; un subdirectorio del nivel 9 se omite con diagnóstico y estado final 1. **Es una limitación de esta implementación.** |
| Devolver 0 al completar el recorrido y 1 si hubo algún error. | Una búsqueda sin coincidencias puede completarse correctamente; una búsqueda parcial no debe comunicarse como exitosa solo porque encontró algún archivo. | Un error acumulado no se borra cuando otra rama termina correctamente. |
| Continuar con las demás ramas tras un error localizado. | Un problema en una entrada no impide necesariamente explorar las otras y obtener resultados útiles. | Se informa el error y se conservan las coincidencias válidas. Un error de lectura del directorio termina esa rama porque no se puede continuar interpretándola con seguridad. |
| No ordenar los resultados. | El enunciado exige imprimir cada coincidencia, pero no establece un orden. Mantener el orden de lectura evita almacenar y ordenar todas las rutas. | Las pruebas aceptan las coincidencias independientemente de su orden. |

### Límites propios de xv6

Los límites de longitud no son supuestos: `DIRSIZ == 14` se define en
[fs.h](../kernel/fs.h) y `MAXPATH == 128` en [param.h](../kernel/param.h).
Por eso se admiten nombres de hasta 14 bytes y rutas de hasta 127 bytes más
el terminador nulo.


## Pruebas

Desde la terminal, en la raíz del repositorio:

```sh
python3 tests/test_find.py
```

El [ejecutor Python](../tests/test_find.py) crea una copia temporal, 
compila desde cero y arranca `make qemu`.
Requiere Python 3 además de las herramientas normales de xv6.

## Ejemplo de uso

Iniciar `make qemu` desde la terminal. En el shell de xv6, ejecutar una
línea a la vez y esperar el indicador `$` antes de enviar la siguiente:

```text
mkdir hito1
mkdir hito1/sub
echo uno > hito1/notas.txt
echo dos > hito1/sub/notas.txt
find hito1 notas.txt
```

Si ya existen los directorios, omitir su creación. La salida contiene ambas rutas,
sin un orden garantizado:

```text
hito1/sub/notas.txt
hito1/notas.txt
```

Para salir de QEMU, pulsar `Ctrl+a` y luego `x`.

<!--
PENDIENTES DEL HITO 1
- Implementar primes: generador 2 a 35, filtros en cadena mediante pipe y fork,
  cierre de extremos, EOF, wait y propagacion de errores.
- Habilitar primes en UPROGS y verificar los 11 primos esperados y retorno al shell.
- Documentar primes cuando este implementado y probado.
- No se han provocado artificialmente fallos de fstat, read o entradas incompletas
  de directorio: esas rutas se revisan en el codigo y no se declaran probadas.
- Las ejecuciones repetidas y las pruebas de agotamiento de FD no demuestran por
  si solas ausencia exhaustiva de fugas o de fallos del sistema.
-->