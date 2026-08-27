#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
int main(void) {
    const char *msg = "hola mundo";
    printf("%s", msg);
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
 
    exit(0);
}
