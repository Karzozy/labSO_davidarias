/**
 * @file
 * @brief Servidor TCP concurrente con un recurso compartido entre lectores y escritores.
 *
 * El servidor mantiene un tablero de texto en memoria. Atiende cada conexion en un hilo
 * independiente, de modo que varios clientes operan sobre el tablero al mismo tiempo. La
 * politica de sincronizacion se elige al arrancar:
 *
 *   sin     - ninguna proteccion sobre el tablero
 *   mutex   - exclusion mutua total: un solo cliente a la vez, lea o escriba
 *   rwlock  - lectores y escritores: varias lecturas simultaneas, escritura exclusiva
 *
 * Protocolo de texto, una orden por linea:
 *
 *   LEER              -> OK <version> <contenido del tablero>
 *   ESCRIBIR <car>    -> OK <version>
 *   SALIR             -> cierra la conexion
 *
 * Senales:
 *
 *   SIGINT  - terminacion ordenada: cierra el socket de escucha y presenta los contadores
 *   SIGALRM - cada PERIODICIDAD_ALARMA segundos se presenta el estado del tablero
 *
 * Los manejadores solo activan una bandera; el trabajo ocurre en el ciclo principal.
 *
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

/** @brief Puerto en el que el servidor recibe conexiones. */
#define PUERTO 5500

/** @brief Numero de casillas del tablero compartido. */
#define TAM_TABLERO 64

/** @brief Tamano del buffer de ordenes y respuestas. */
#define TAM_BUFFER 256

/** @brief Microsegundos de espera entre casilla y casilla al escribir. */
#define RETARDO_ESCRITURA 2000

/** @brief Microsegundos de espera entre casilla y casilla al leer. */
#define RETARDO_LECTURA 200

/** @brief Periodo de la alarma del servidor en segundos. */
#define PERIODICIDAD_ALARMA 5

/** @brief Politicas de sincronizacion disponibles. */
typedef enum {
    SIN_PROTECCION, /**< El tablero queda expuesto a accesos simultaneos. */
    CON_MUTEX,      /**< Un solo cliente accede al tablero, lea o escriba. */
    CON_RWLOCK      /**< Varios lectores a la vez, un escritor en exclusiva. */
} politica;

/* Recurso compartido: todas las casillas del tablero contienen el mismo caracter */
static char tablero[TAM_TABLERO];

/* Numero de la ultima escritura completada */
static int version_tablero;

/* Mecanismos de sincronizacion sobre el tablero */
static pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t candado_rw = PTHREAD_RWLOCK_INITIALIZER;

/* Contadores de ordenes recibidas. Comparten un mismo candado para poder leerse juntos
   de forma consistente: el informe final toma ambos valores en el mismo instante. */
static pthread_mutex_t candado_contadores = PTHREAD_MUTEX_INITIALIZER;
static unsigned long lecturas_atendidas = 0;
static unsigned long escrituras_atendidas = 0;

/* Politica en uso durante esta ejecucion */
static politica politica_actual = SIN_PROTECCION;

/* Banderas que los manejadores activan y el ciclo principal consulta */
static volatile sig_atomic_t terminar = 0;
static volatile sig_atomic_t alarma_activada = 0;

/**
 * @brief Manejador de SIGINT: solo registra el hecho.
 * @param senal Numero de la senal recibida.
 */
static void al_interrumpir(int senal);

/**
 * @brief Manejador de SIGALRM: solo registra el hecho.
 * @param senal Numero de la senal recibida.
 */
static void al_alarma(int senal);

/**
 * @brief Instala un manejador sin SA_RESTART, para que accept devuelva EINTR.
 * @param senal Senal que se captura.
 * @param manejador Funcion que atiende la senal.
 * @return 0 si se instalo, -1 si ocurre un error.
 */
static int instalar_manejador(int senal, void (*manejador)(int));

/**
 * @brief Cuenta una orden LEER recibida.
 */
static void registrar_lectura(void);

/**
 * @brief Cuenta una orden ESCRIBIR recibida.
 */
static void registrar_escritura(void);

/**
 * @brief Obtiene ambos contadores en un mismo instante, bajo un solo candado.
 * @param lecturas Recibe el numero de lecturas atendidas.
 * @param escrituras Recibe el numero de escrituras atendidas.
 */
static void leer_contadores(unsigned long *lecturas, unsigned long *escrituras);

/**
 * @brief Presenta la version y el contenido del tablero, leido con la politica vigente.
 */
static void imprimir_informe_tablero(void);

/**
 * @brief Crea un hilo de cliente con SIGINT y SIGALRM bloqueadas.
 *
 * El hilo hereda la mascara del creador, de modo que solo el hilo principal recibe esas
 * senales. Si las recibiera un hilo de cliente, su read fallaria con EINTR y la conexion
 * se cerraria sin motivo.
 *
 * @param hilo Recibe el identificador del hilo creado.
 * @param descriptor Descriptor del cliente, que el hilo libera al iniciar.
 * @return 0 si se creo el hilo, o el codigo de error devuelto por pthreads.
 */
static int crear_hilo_cliente(pthread_t *hilo, int *descriptor);

/**
 * @brief Traduce el nombre de la politica recibido por linea de comandos.
 * @param nombre Cadena con el nombre de la politica.
 * @param destino Apuntador a la variable que recibe la politica reconocida.
 * @return 1 si el nombre corresponde a una politica conocida, 0 en caso contrario.
 */
int reconocer_politica(const char *nombre, politica *destino);

/**
 * @brief Crea el socket de escucha del servidor.
 * @param puerto Puerto en el que se reciben las conexiones.
 * @return Descriptor del socket de escucha, o -1 si ocurre un error.
 */
int crear_socket_escucha(int puerto);

/**
 * @brief Atiende las ordenes de un cliente hasta que cierra la conexion.
 * @param arg Apuntador al descriptor del socket del cliente, reservado por el llamador.
 * @return NULL siempre.
 */
void *atender_cliente(void *arg);

/**
 * @brief Lee una linea terminada en salto de linea desde un socket.
 * @param socket_cliente Descriptor del socket.
 * @param destino Buffer que recibe la linea, sin el salto de linea final.
 * @param tam Tamano del buffer de destino.
 * @return Numero de caracteres leidos, o -1 si el cliente cerro la conexion o hubo error.
 */
int leer_linea(int socket_cliente, char *destino, int tam);

/**
 * @brief Copia el contenido del tablero aplicando la politica de sincronizacion vigente.
 * @param destino Buffer que recibe la copia, con al menos TAM_TABLERO + 1 caracteres.
 * @return Version del tablero observada durante la lectura.
 */
int leer_tablero(char *destino);

/**
 * @brief Llena el tablero con un caracter aplicando la politica de sincronizacion vigente.
 * @param caracter Caracter que se escribe en todas las casillas.
 * @return Version del tablero despues de la escritura.
 */
int escribir_tablero(char caracter);

int main(int argc, char *argv[]) {
    int socket_escucha;
    int socket_cliente;
    int *descriptor;
    int codigo;
    int estado = EXIT_SUCCESS;
    unsigned long lecturas;
    unsigned long escrituras;
    pthread_t hilo;

    if (argc != 2 || !reconocer_politica(argv[1], &politica_actual)) {
        fprintf(stderr, "Uso: %s sin|mutex|rwlock\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Un cliente que cierra de golpe no debe terminar el servidor */
    signal(SIGPIPE, SIG_IGN);

    /* Los manejadores se instalan antes de aceptar la primera conexion */
    if (instalar_manejador(SIGINT, al_interrumpir) == -1) {
        perror("sigaction SIGINT");
        return EXIT_FAILURE;
    }
    if (instalar_manejador(SIGALRM, al_alarma) == -1) {
        perror("sigaction SIGALRM");
        return EXIT_FAILURE;
    }

    memset(tablero, '.', TAM_TABLERO);
    version_tablero = 0;

    socket_escucha = crear_socket_escucha(PUERTO);
    if (socket_escucha == -1) {
        return EXIT_FAILURE;
    }

    printf("Tablero de %d casillas, politica %s, escuchando en el puerto %d\n",
           TAM_TABLERO, argv[1], PUERTO);
    fflush(stdout);

    alarm(PERIODICIDAD_ALARMA);

    /* Si SIGINT llega justo entre la condicion y accept, la salida espera como maximo
       hasta la siguiente alarma, que interrumpe accept. */
    while (!terminar) {
        /* La bandera se limpia antes del informe: una alarma que llegue durante el
           informe queda registrada y no se pierde. */
        if (alarma_activada) {
            alarma_activada = 0;
            imprimir_informe_tablero();
            alarm(PERIODICIDAD_ALARMA); /* alarm no es periodica: se reprograma */
        }

        socket_cliente = accept(socket_escucha, NULL, NULL);
        if (socket_cliente == -1) {
            if (errno == EINTR) {
                continue; /* Una senal interrumpio la espera: no es un error */
            }
            /* Un error pasajero (por ejemplo ECONNABORTED) no debe tumbar el servidor */
            perror("accept");
            continue;
        }

        descriptor = malloc(sizeof(int));
        if (descriptor == NULL) {
            perror("malloc");
            close(socket_cliente);
            continue;
        }
        *descriptor = socket_cliente;

        codigo = crear_hilo_cliente(&hilo, descriptor);
        if (codigo != 0) {
            /* Las funciones pthread devuelven el codigo de error y no usan errno */
            fprintf(stderr, "pthread_create: %s\n", strerror(codigo));
            free(descriptor);
            close(socket_cliente);
            continue;
        }

        /* Nadie espera el resultado del hilo: sus recursos se liberan al terminar */
        codigo = pthread_detach(hilo);
        if (codigo != 0) {
            fprintf(stderr, "pthread_detach: %s\n", strerror(codigo));
        }
    }

    alarm(0);
    if (alarma_activada) {
        alarma_activada = 0;
        imprimir_informe_tablero();
    }

    if (close(socket_escucha) == -1) {
        perror("close");
        estado = EXIT_FAILURE;
    }

    leer_contadores(&lecturas, &escrituras);
    printf("Servidor terminado. Lecturas atendidas: %lu, escrituras atendidas: %lu\n",
           lecturas, escrituras);
    fflush(stdout);
    return estado;
}

static void al_interrumpir(int senal) {
    (void) senal;
    terminar = 1;
}

static void al_alarma(int senal) {
    (void) senal;
    alarma_activada = 1;
}

static int instalar_manejador(int senal, void (*manejador)(int)) {
    struct sigaction accion;

    memset(&accion, 0, sizeof(accion));
    accion.sa_handler = manejador;
    sigemptyset(&accion.sa_mask);
    accion.sa_flags = 0; /* Sin SA_RESTART: accept debe devolver el control al ciclo */
    return sigaction(senal, &accion, NULL);
}

static void registrar_lectura(void) {
    pthread_mutex_lock(&candado_contadores);
    lecturas_atendidas++;
    pthread_mutex_unlock(&candado_contadores);
}

static void registrar_escritura(void) {
    pthread_mutex_lock(&candado_contadores);
    escrituras_atendidas++;
    pthread_mutex_unlock(&candado_contadores);
}

static void leer_contadores(unsigned long *lecturas, unsigned long *escrituras) {
    pthread_mutex_lock(&candado_contadores);
    *lecturas = lecturas_atendidas;
    *escrituras = escrituras_atendidas;
    pthread_mutex_unlock(&candado_contadores);
}

static void imprimir_informe_tablero(void) {
    char contenido[TAM_TABLERO + 1];
    int version = leer_tablero(contenido);

    printf("Informe periodico: version %d, tablero %s\n", version, contenido);
    fflush(stdout);
}

static int crear_hilo_cliente(pthread_t *hilo, int *descriptor) {
    sigset_t senales;
    sigset_t anteriores;
    int codigo;
    int resultado;

    sigemptyset(&senales);
    sigaddset(&senales, SIGINT);
    sigaddset(&senales, SIGALRM);

    codigo = pthread_sigmask(SIG_BLOCK, &senales, &anteriores);
    if (codigo != 0) {
        return codigo;
    }

    resultado = pthread_create(hilo, NULL, atender_cliente, descriptor);

    /* El hilo principal vuelve a recibir las senales; las pendientes se entregan aqui */
    codigo = pthread_sigmask(SIG_SETMASK, &anteriores, NULL);
    if (resultado == 0 && codigo != 0) {
        fprintf(stderr, "pthread_sigmask: %s\n", strerror(codigo));
    }

    return resultado;
}

int reconocer_politica(const char *nombre, politica *destino) {
    if (strcmp(nombre, "sin") == 0) {
        *destino = SIN_PROTECCION;
        return 1;
    }
    if (strcmp(nombre, "mutex") == 0) {
        *destino = CON_MUTEX;
        return 1;
    }
    if (strcmp(nombre, "rwlock") == 0) {
        *destino = CON_RWLOCK;
        return 1;
    }
    return 0;
}

int crear_socket_escucha(int puerto) {
    int socket_escucha;
    int reutilizar = 1;
    struct sockaddr_in direccion;

    socket_escucha = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_escucha == -1) {
        perror("socket");
        return -1;
    }

    /* El puerto queda disponible de inmediato al terminar el servidor */
    if (setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEADDR, &reutilizar,
                   sizeof(reutilizar)) == -1) {
        perror("setsockopt");
        close(socket_escucha);
        return -1;
    }

    memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = htonl(INADDR_ANY);
    direccion.sin_port = htons(puerto);

    if (bind(socket_escucha, (struct sockaddr *) &direccion, sizeof(direccion)) == -1) {
        perror("bind");
        close(socket_escucha);
        return -1;
    }

    if (listen(socket_escucha, 16) == -1) {
        perror("listen");
        close(socket_escucha);
        return -1;
    }

    return socket_escucha;
}

void *atender_cliente(void *arg) {
    int socket_cliente = *((int *) arg);
    char orden[TAM_BUFFER];
    char copia[TAM_TABLERO + 1];
    char respuesta[TAM_BUFFER];
    int version;
    int longitud;

    free(arg);

    while (1) {
        longitud = leer_linea(socket_cliente, orden, TAM_BUFFER);
        if (longitud == -1) {
            break;
        }

        /* Los contadores registran ordenes recibidas, antes de ejecutarlas */
        if (strcmp(orden, "LEER") == 0) {
            registrar_lectura();
            version = leer_tablero(copia);
            snprintf(respuesta, TAM_BUFFER, "OK %d %s\n", version, copia);
        } else if (strncmp(orden, "ESCRIBIR ", 9) == 0 && orden[9] != '\0') {
            registrar_escritura();
            version = escribir_tablero(orden[9]);
            snprintf(respuesta, TAM_BUFFER, "OK %d\n", version);
        } else if (strcmp(orden, "SALIR") == 0) {
            break;
        } else {
            snprintf(respuesta, TAM_BUFFER, "ERROR orden no reconocida\n");
        }

        if (write(socket_cliente, respuesta, strlen(respuesta)) == -1) {
            if (errno != EPIPE) { /* EPIPE: el cliente se fue, es esperable */
                perror("write");
            }
            break;
        }
    }

    close(socket_cliente);
    return NULL;
}

int leer_linea(int socket_cliente, char *destino, int tam) {
    int total = 0;
    char caracter;
    ssize_t leidos;

    while (total < tam - 1) {
        leidos = read(socket_cliente, &caracter, 1);

        /* El cliente cerro la conexion: una linea a medias no se procesa */
        if (leidos <= 0) {
            return -1;
        }
        if (caracter == '\n') {
            break;
        }
        if (caracter != '\r') {
            destino[total] = caracter;
            total++;
        }
    }

    destino[total] = '\0';
    return total;
}

int leer_tablero(char *destino) {
    int i;
    int version;

    if (politica_actual == CON_MUTEX) {
        pthread_mutex_lock(&candado);
    } else if (politica_actual == CON_RWLOCK) {
        pthread_rwlock_rdlock(&candado_rw);
    }

    for (i = 0; i < TAM_TABLERO; i++) {
        destino[i] = tablero[i];
        usleep(RETARDO_LECTURA);
    }
    destino[TAM_TABLERO] = '\0';
    version = version_tablero;

    if (politica_actual == CON_MUTEX) {
        pthread_mutex_unlock(&candado);
    } else if (politica_actual == CON_RWLOCK) {
        pthread_rwlock_unlock(&candado_rw);
    }

    return version;
}

int escribir_tablero(char caracter) {
    int i;
    int version;

    if (politica_actual == CON_MUTEX) {
        pthread_mutex_lock(&candado);
    } else if (politica_actual == CON_RWLOCK) {
        pthread_rwlock_wrlock(&candado_rw);
    }

    for (i = 0; i < TAM_TABLERO; i++) {
        tablero[i] = caracter;
        usleep(RETARDO_ESCRITURA);
    }
    version_tablero++;
    version = version_tablero;

    if (politica_actual == CON_MUTEX) {
        pthread_mutex_unlock(&candado);
    } else if (politica_actual == CON_RWLOCK) {
        pthread_rwlock_unlock(&candado_rw);
    }

    return version;
}