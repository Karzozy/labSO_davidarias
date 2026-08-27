/**
 * @file 
 * @brief prog1 - Obtiene PID y PPID del proceso e informa por salida estandar
 *        usando la llamada al sistema write(), verificando sus retornos.
 *
 * Taller: llamadas al sistema - Laboratorio de Sistemas Operativos.
 *
 * @author David Santiago Arias Narvaez <davidarias@unicauca.edu.co>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * @brief Punto de entrada del programa.
 *
 * Obtiene el identificador del proceso actual (PID) y el de su proceso
 * padre (PPID), arma un mensaje con esos datos y lo escribe en la salida
 * estandar usando write(2), sin pasar por printf(3).
 *
 * Verifica el valor de retorno de write(): si devuelve -1 informa la
 * causa con perror(3); si devuelve una cantidad de bytes menor a la
 * solicitada, lo reporta como error.
 *
 * @return Termina el proceso con exit(7) en el caso exitoso (codigo de
 *         salida distinto de cero, elegido a proposito), o con
 *         exit(1) si ocurre algun error.
 */
int main(void) {
    pid_t pid = getpid();
    pid_t ppid = getppid();

    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer),
                        "Soy el proceso %d, mi padre es %d\n", pid, ppid);
    if (len < 0 || (size_t)len >= sizeof(buffer)) {
        fprintf(stderr, "error armando el mensaje\n");
        exit(1);
    }

    /* Llamada al sistema: escribe directo al descriptor de salida estandar,
     * sin pasar por el buffer de stdio como hace printf(). */
    ssize_t escritos = write(STDOUT_FILENO, buffer, (size_t)len);
    if (escritos == -1) {
        perror("write");
        exit(1);
    }
    if (escritos != len) {
        /* write puede escribir menos bytes de los pedidos */
        fprintf(stderr, "write escribio solo %zd de %d bytes\n", escritos, len);
        exit(1);
    }

    exit(7); /* codigo de salida distinto de cero, elegido a proposito */
}