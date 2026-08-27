#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
 
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
 
    ssize_t escritos = write(STDOUT_FILENO, buffer, (size_t)len);
    if (escritos == -1) {
    perror("write");
    exit(1);
    }
    if (escritos != len) {
        fprintf(stderr, "write escribio solo %zd de %d bytes\n", escritos, len);
        exit(1);
    }
 
    exit(7); 
}
