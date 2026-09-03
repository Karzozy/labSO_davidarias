/**
 * @file 
 * @brief infoproc - Informa el estado de un proceso y su cadena de padres
 *        leyendo /proc exclusivamente con llamadas al sistema.
 *
 * El programa recibe un PID como argumento (o usa el propio PID si
 * no recibe ninguno), lee la ruta /proc/PID/status usando open, read y close,
 * y muestra Name, State, PPid y Threads, repitiendo el proceso para
 * cada padre en la jerarquia hasta llegar al proceso 1 donde el PPid sea 0.
 * @author David Santiago Arias Narvaez <davidarias@unicauca.edu.co>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 8192

/**
 * @brief Lee el contenido completo de /proc/pid/status hacia un buffer.
 *
 * Abre el archivo con open, lo lee con un ciclo de llamadas a read
 * y lo cierra con close, si el contenido no cabe en el buffer reservado,
 * se informa el error.
 *
 * @param pid Identificador del proceso.
 * @param buf Buffer donde se almacena el contenido leido.
 * @param bufsize Tamano del buffer, incluyendo espacio para el
 *        terminador nulo.
 * @return Cantidad de bytes leidos, o -1 si ocurrio un error.
 */
ssize_t leer_status(int pid, char *buf, size_t bufsize) {
    char ruta[64];
    snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid);

    int fd = open(ruta, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    size_t total = 0;
    ssize_t leidos;

    while (total < bufsize - 1) {
        leidos = read(fd, buf + total, bufsize - 1 - total);
        if (leidos == -1) {
            perror("read");
            close(fd);
            return -1;
        }
        if (leidos == 0) {
            break;
        }
        total += (size_t) leidos;
    }

    if (total == bufsize - 1) {
        char extra;
        ssize_t r = read(fd, &extra, 1);
        if (r > 0) {
            fprintf(stderr, "Error: el contenido de %s no cabe en el buffer reservado\n", ruta);
            close(fd);
            return -1;
        }
    }

    if (close(fd) == -1) {
        perror("close");
        return -1;
    }

    buf[total] = '\0';
    return (ssize_t) total;
}

/**
 * @brief Extrae el valor de un campo con formato "Etiqueta: valor".
 *
 * Busca la etiqueta dentro del contenido ya leido, salta los espacios
 * y copia el valor hasta el fin de linea. Este analisis se hace sobre el arreglo ya cerrado, usando funciones
 * de string.h, no llamadas al sistema.
 *
 * @param contenido Texto completo de /proc/pid/status.
 * @param etiqueta Etiqueta a buscar, por ejemplo "Name:".
 * @param out Buffer donde se copia el valor encontrado.
 * @param outsize Tamano del buffer de salida.
 */
void extraer_campo(const char *contenido, const char *etiqueta, char *out, size_t outsize) {
    const char *p = strstr(contenido, etiqueta);
    out[0] = '\0';
    if (p == NULL) {
        return;
    }
    p += strlen(etiqueta);
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    const char *fin = strchr(p, '\n');
    size_t len = fin ? (size_t)(fin - p) : strlen(p);
    if (len >= outsize) {
        len = outsize - 1;
    }
    memcpy(out, p, len);
    out[len] = '\0';
}

/**
 * @brief Muestra Name, State, PPid y Threads de un proceso.
 *
 * @param pid Identificador del proceso que se va a ver.
 * @return El PPid del proceso como entero, o -1 si hubo un error
 *         al leer su informacion.
 */
int mostrar_info(int pid) {
    char buf[BUF_SIZE];

    if (leer_status(pid, buf, sizeof(buf)) < 0) {
        return -1;
    }

    char nombre[256], estado[256], ppid_str[256], hilos[256];
    extraer_campo(buf, "Name:", nombre, sizeof(nombre));
    extraer_campo(buf, "State:", estado, sizeof(estado));
    extraer_campo(buf, "PPid:", ppid_str, sizeof(ppid_str));
    extraer_campo(buf, "Threads:", hilos, sizeof(hilos));

    printf("PID: %d | Name: %s | State: %s | PPid: %s | Threads: %s\n",
           pid, nombre, estado, ppid_str, hilos);

    return atoi(ppid_str);
}

/**
 * @brief Punto de entrada del programa.
 *
 * Valida el argumento recibido, comprueba que el proceso exista, y recorre la cadena
 * de padres mostrando la informacion de cada uno hasta llegar al
 * proceso 1.
 *
 * @param argc Cantidad de argumentos recibidos.
 * @param argv Vector de argumentos; argv[1] es el PID a consultar.
 * @return 0 si el recorrido se completo, distinto de cero si hubo error.
 */
int main(int argc, char *argv[]) {
    int pid;

    if (argc > 1) {
        pid = atoi(argv[1]);
        if (pid <= 0) {
            fprintf(stderr, "Error: argumento invalido '%s'\n", argv[1]);
            exit(1);
        }
    } else {
        pid = getpid();
    }

    char ruta[64];
    snprintf(ruta, sizeof(ruta), "/proc/%d", pid);
    if (access(ruta, F_OK) == -1) {
        fprintf(stderr, "Error: el proceso %d no existe\n", pid);
        exit(1);
    }

    printf("infoproc: pid = %d, ppid = %d\n\n", getpid(), getppid());

    int actual = pid;
    while (actual != 1) {
        int padre = mostrar_info(actual);
        if (padre == -1) {
            fprintf(stderr, "Error: no se pudo leer el proceso %d\n", actual);
            exit(1);
        }
        if (padre == 0) {
            break;
        }
        actual = padre;
    }

    return 0;
}