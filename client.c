#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "store.h"

const char *clientFIFOPath = "ClientFIFO";
key_t cartsKey;
key_t catalogKey;

int main() {
    unsigned short i;
    char *name, *pswd;
    int pipe;
    bool login;
    puts("Introduzca su nombre de usuario:");
    fflush(stdin);
    gets(name);
    puts("Introduzcaa su contraseña");
    fflush(stdin);
    gets(pswd);
    mkfifo(clientFIFOPath, 0666);
    pipe = open(clientFIFOPath, O_WRONLY);
    write(pipe, name, NAMELENGTH);
    write(pipe, pswd, PSWDLENGTH);
    close(pipe);
    pipe = open(clientFIFOPath, O_RDONLY);
    read(pipe, &login, 1);
    read(pipe, &cartsKey, sizeof(key_t));
    read(pipe, &catalogKey, sizeof(key_t));
    close(pipe);
    return 0;
}