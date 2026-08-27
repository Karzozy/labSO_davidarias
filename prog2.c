/**
 * @file 
 * @brief prog2 - Compara el orden de salida entre printf() (biblioteca) y write()
 *        (llamada al sistema) al imprimir el mismo texto sin flush ni
 *        salto de linea.
 *
 * Taller: llamadas al sistema - Laboratorio de Sistemas Operativos.
 *
 * @author David Santiago Arias Narvaez <davidarias@unicauca.edu.co>
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
/**
 * @brief Punto de entrada del programa.
 *
 * Imprime el mismo mensaje dos veces: primero con printf(3) (funcion de
 * biblioteca, que acumula el texto en el buffer de stdio) y luego con
 * write(2). No se llama a fflush() ni se agrega '\n' al mensaje, para poder observar en
 * que orden aparece realmente cada texto en pantalla.
 *
 * Verifica el valor de retorno de write(): si devuelve -1 informa la
 * causa con perror(3); si devuelve menos bytes de los solicitados,
 * lo reporta como error.
 *
 * @return Termina el proceso con exit(0) en el caso exitoso, o con
 *         exit(1) si ocurre algun error en write().
 */
int main(void) {
    const char *msg = "hola mundo";
 
    /* Funcion de biblioteca: el texto queda en el buffer de stdio,
     * no sale de inmediato a la pantalla. */
    printf("%s", msg);
 
    /* Llamada al sistema: el texto se manda directo al kernel,
     * sin buffer intermedio de nuestro programa. */
    ssize_t escritos = write(STDOUT_FILENO, msg, strlen(msg));
    if (escritos == -1) {
        perror("write");
        exit(1);
    }
    if ((size_t)escritos != strlen(msg)) {
        fprintf(stderr, "write escribio solo %zd de %zu bytes\n",
                escritos, strlen(msg));
        exit(1);
    }
 
    /* Sin fflush y sin '\n' a proposito: se quiere ver el orden real. */
    exit(0);
}
