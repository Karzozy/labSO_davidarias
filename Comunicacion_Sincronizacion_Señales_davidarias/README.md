# Taller 5 - señales en un servidor concurrente

## Objetivo

Capturar señales en un servidor concurrente, programar una alarma periodica e informar los errores de las llamadas al sistema. Al terminar, el servidor debe cerrar de forma ordenada, presentar sus contadores y publicar el estado del tablero cada cierto tiempo.

## Enunciado

### 1. Ejecutar el servidor sin modificaciones

- Ingresar al directorio src/
- Compilar con make
- Ejecutar:
  ```sh
  ./servidor_tablero rwlock
  ```
- Conectar un cliente y detener el servidor con Ctrl-C
- Registrar en la bitacora que presenta el servidor al terminar y que ocurre con el cliente que estaba conectado

### 2. Instalar un manejador para SIGINT

- Declarar una bandera de tipo `volatile sig_atomic_t`
- Instalar un manejador para `SIGINT` usando `sigaction`
- El manejador solo debe asignar 1 a la bandera y no realizar ninguna otra operacion

```c
static volatile sig_atomic_t terminar = 0;
static void al_interrumpir(int senal) {
    (void) senal;
    terminar = 1;
}
```

- La instalacion ocurre en `main`, antes de aceptar la primera conexion
- Usar `sa_flags = 0` para que no se active `SA_RESTART`

```c
struct sigaction accion;
memset(&accion, 0, sizeof(accion));
action.sa_handler = al_interrumpir;
sigemptyset(&accion.sa_mask);
action.sa_flags = 0;
if (sigaction(SIGINT, &accion, NULL) == -1) {
    perror("sigaction");
    return EXIT_FAILURE;
}
```

### 3. Convertir el ciclo de accept en un ciclo controlado

- Cambiar el ciclo principal para que termine cuando la bandera quede activa
- Una llamada interrumpida por una senal retorna -1 y deja `errno = EINTR`
- El programa debe diferenciar `EINTR` de un error real
- Al salir del ciclo, cerrar el socket de escucha y mostrar cuantas lecturas y cuantas escrituras atendio

### 4. Proteger los contadores

- Los hilos que atienden clientes deben incrementar los contadores cada vez que atienden una orden
- Los contadores son memoria compartida y necesitan proteccion
- Registrar en la bitacora que mecanismo se eligio y con que criterio

### 5. Instalar manejador para SIGALRM y alarma periodica

- Instalar un manejador para `SIGALRM`
- Programar la alarma con `alarm()`
- Cada vez que el ciclo principal observe la bandera de alarma:
  - presentar la version del tablero y su contenido
  - volver a programar la alarma
- La lectura del tablero debe aplicarse con la politica vigente en modo lectura, igual que la orden `LEER` de un cliente

### 6. Enviar las senales desde otra terminal

- Ejecutar el servidor en segundo plano
- Enviar señales con `kill` desde otra terminal
- Comprobar el efecto de cada senal
- Probar despues capturar `SIGKILL` y explicar por que el sistema no lo permite

Ejemplo:

```sh
./servidor_tablero rwlock &
kill -ALRM $!
kill -INT $!
kill -KILL $!
```

### 7. Informar errores de llamadas al sistema

- Verificar el valor de retorno de las llamadas del programa
- Informar cada error con `perror`
- Provocar un fallo a proposito, arrancando un segundo servidor mientras el primero escucha
- Registrar el mensaje que produce

### 8. Explicar la senal SIGPIPE

- Explicar el efecto de la linea:

```c
signal(SIGPIPE, SIG_IGN)
```

- Explicar que sucederia sin esta linea cuando un cliente cierra la conexion de golpe

## Consideraciones importantes

- El manejador solo debe registrar el hecho; el trabajo se realiza en el ciclo principal
- No se debe usar `printf` dentro del manejador
- No se debe tomar el candado del tablero dentro del manejador
- No se debe instalar `SA_RESTART` para que `accept()` pueda devolver el control al ciclo principal
- El servidor debe cerrar el socket de escucha y presentar sus contadores al terminar

## Entregables

- Archivo `servidor_tablero.c` modificado
- Compilacion sin advertencias con:
  ```sh
  gcc -Wall -Wextra servidor_tablero.c -o servidor_tablero -pthread
  ```
- Bitacora con las respuestas de los puntos 1, 4, 6, 7 y 8, y con la salida de un informe periodico
