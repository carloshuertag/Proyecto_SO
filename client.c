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
key_t cartsKey, catalogKey, controlKey;
int loginPipe;
bool logged = false;

void login(const char *mail, const char *pswd) {
    controlKey = ftok("ControlKey", 'o');
    int shmid = shmget(controlKey, sizeof(loginDTS), IPC_CREAT | 0600);
    loginDTS *loginBuffer = (loginDTS*)shmat(shmid, 0, 0);
    strcpy(loginBuffer->credentials.mail, mail);
    strcpy(loginBuffer->credentials.pswd, pswd);
    sleep(2);
    logged = loginBuffer->login;
    cartsKey = loginBuffer->cartsKey;
    catalogKey = loginBuffer->catalogKey;
}

int main() {
    char mail[MAILLENGTH], pswd[PSWDLENGTH];
    printf("Introduzca su correo electrónico: ");
    fflush(stdin);
    scanf("%s", mail);
    printf("Introduzcaa su contraseña: ");
    fflush(stdin);
    scanf("%s", pswd);
    printf("LOGING IN %s, %s", mail, pswd);
    login(mail, pswd);
    while(!logged){
        printf("Usuario o contraseña incorrectos.\n\nIntroduzca su correo electrónico:");
        fflush(stdin);
        scanf("%s", mail);
        printf("Introduzcaa su contraseña:");
        fflush(stdin);
        scanf("%s", pswd);
        printf("LOGING IN %s, %s", mail, pswd);
        login(mail, pswd);
    }
    return 0;
}