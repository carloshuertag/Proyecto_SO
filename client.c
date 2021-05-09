#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include "store.h"

int msgId;
key_t cartsKey, catalogKey, controlKey;
bool logged = false;

void initElements() {
    printf("LOGGED");
}

void login(msgBuffer msgB) {
    msgB.msgType = 1;
    //msgB.msgLogin = false;
    //msgB.msgCartsKey = 0;
    //msgB.msgCatalogKey = 0;
    msgsnd(msgId, &msgB, sizeof(msgB), 0);
    msgrcv(msgId, &msgB, sizeof(msgB), 2, 0);
    printf("Message: %s", msgB.msgMail);
    //logged = msgB.msgLogin;
    if(logged) {
        //cartsKey = msgB.msgCartsKey;
        //catalogKey = msgB.msgCatalogKey;
        initElements();
    }
}

int main() {
    int msgId;
    msgBuffer msgB;
    char mail[MAILLENGTH], pswd[PSWDLENGTH];
    printf("Introduzca su correo electrónico: ");
    fflush(stdin);
    scanf("%s", msgB.msgMail);
    printf("Introduzca su contraseña: ");
    fflush(stdin);
    //scanf("%s", msgB.msgPswd);
    controlKey = ftok("ControlKey", 'o');
    msgId = msgget(controlKey, 0666 | IPC_CREAT);
    login(msgB);
    do{
        printf("Usuario o contraseña incorrectos.\n\nIntroduzca su correo electrónico: ");
        fflush(stdin);
        scanf("%s", msgB.msgMail);
        printf("Introduzca su contraseña: ");
        fflush(stdin);
        //scanf("%s", msgB.msgPswd);
        login(msgB);
    } while(!logged);
    return 0;
}