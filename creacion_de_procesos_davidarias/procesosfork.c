/**
 * @file
 * @brief Taller de creacion de procesos - Crea procesos hijos con fork(),
 *        reemplaza su imagen con exec() y el padre espera su terminacion
 *        con waitpid(), informando con que codigo salio cada hijo.
 *
 * Taller: creacion de procesos - Laboratorio de Sistemas Operativos.
 *
 * @author David Santiago Arias Narvaez <davidarias@unicauca.edu.co>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * @brief Punto de entrada del programa.
 *
 * Crea dos procesos hijos con fork(). En cada llamada se distinguen las
 * tres ramas posibles segun el valor de retorno: pid < 0 es error (no se
 * pudo crear el proceso), pid == 0 es el hijo, y pid > 0 es el padre,
 * donde ese valor es el PID del hijo recien creado.
 *
 * El hijo 1 reemplaza su imagen con execlp() para correr "ls -l"; el
 * hijo 2 hace lo mismo para correr "sleep 2". En ambos casos la lista de
 * argumentos de exec termina en (char *) NULL y no en el entero 0, ya
 * que exec espera un apuntador del tipo correcto: usar 0 sin el cast
 * puede funcionar por casualidad en una maquina y fallar en otra si int
 * y char * no ocupan lo mismo.
 *
 * Despues de cada execlp se deja codigo a proposito (ver comentario en
 * el cuerpo) para comprobar que, si exec tuvo exito, ese codigo nunca se
 * ejecuta.
 *
 * El padre espera a ambos hijos con waitpid(-1, ...), que recoge a
 * cualquier hijo que termine, sin importar el orden, e informa cual
 * termino primero y con que codigo de salida termino cada uno.
 *
 * @return Termina el proceso con exit(EXIT_SUCCESS) en el caso exitoso,
 *         o con exit(EXIT_FAILURE) si  fallan fork o waitpid.
 */
int main(void) {
    pid_t pid1, pid2;

    
    pid1 = fork();

    if (pid1 < 0) {
        perror("fork (hijo 1)");
        exit(EXIT_FAILURE);

    } else if (pid1 == 0) {
        printf("Soy el hijo 1, mi pid es %d, el pid de mi padre es %d\n",
               getpid(), getppid());

        execlp("ls", "ls", "-l", (char *) NULL);
        perror("execlp (hijo 1)");
        exit(EXIT_FAILURE);
    }

    pid2 = fork();

    if (pid2 < 0) {
        perror("fork (hijo 2)");
        exit(EXIT_FAILURE);

    } else if (pid2 == 0) {
        printf("Soy el hijo 2, mi pid es %d, el pid de mi padre es %d\n",
               getpid(), getppid());

        execlp("sleep", "sleep", "2", (char *) NULL);
        perror("execlp (hijo 2)");
        exit(EXIT_FAILURE);
    }

    printf("Soy el padre, mi pid es %d. Cree al hijo 1 (pid %d) y al hijo 2 (pid %d)\n",
           getpid(), pid1, pid2);

    int hijos_restantes = 2;
    int primero_informado = 0;

    while (hijos_restantes > 0) {
        int status;
        pid_t terminado = waitpid(-1, &status, 0);

        if (terminado == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (!primero_informado) {
            printf("El primero en terminar fue el pid %d\n", terminado);
            primero_informado = 1;
        }

        if (WIFEXITED(status)) {
            printf("El proceso pid %d termino normalmente con codigo %d\n",
                   terminado, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("El proceso pid %d termino por la senal %d\n",
                   terminado, WTERMSIG(status));
        }

        hijos_restantes--;
    }

    printf("El padre recogio a sus dos hijos, no quedan procesos zombis\n");

    exit(EXIT_SUCCESS);
}