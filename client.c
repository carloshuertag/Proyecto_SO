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
#include<ctype.h>
#include "store.h"

const char *clientFIFOPath = "ClientFIFO";
key_t cartsKey;
key_t catalogKey;
int loginPipe;
bool logged;

void login(const char *mail, const char *pswd) {
    mkfifo(clientFIFOPath, 0666);
    loginPipe = open(clientFIFOPath, O_WRONLY);
    write(loginPipe, mail, NAMELENGTH);
    write(loginPipe, pswd, PSWDLENGTH);
    close(loginPipe);
    loginPipe = open(clientFIFOPath, O_RDONLY);
    read(loginPipe, &logged, 1);
    read(loginPipe, &cartsKey, sizeof(key_t));
    read(loginPipe, &catalogKey, sizeof(key_t));
    close(loginPipe);
}

int main() {
    unsigned short i;
    char mail[MAILLENGTH], pswd[PSWDLENGTH];
    printf("Introduzca su correo electrónico: ");
    fflush(stdin);
    scanf("%s", mail);
    printf("Introduzcaa su contraseña: ");
    fflush(stdin);
    scanf("%s", pswd);
    printf("LOGING IN %s, %s", mail, pswd);
    login(mail, pswd);
    do{
        printf("Usuario o contraseña incorrectos.\n\nIntroduzca su correo electrónico:");
        fflush(stdin);
        scanf("%s", mail);
        printf("Introduzcaa su contraseña:");
        fflush(stdin);
        scanf("%s", pswd);
        printf("LOGING IN %s, %s", mail, pswd);
        login(mail, pswd);
    } while(!logged);
    return 0;
}