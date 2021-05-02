#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "store.h"

FILE* file;
char* filename;
key_t clientsKey;
key_t cartsKey;
key_t catalogKey;

void initClients(client *c){ 
	filename = "Clients";
    if((file = fopen(filename, "a")) == NULL) fprintf(stderr, "Error al crear el archivo");
    c[0].id = 0;
    strcpy(c[0].name, "Carlos/Fuentes/Hernandez");
    strcpy(c[0].pswd, "LobitoVeloz777");
    c[1].id = 1;
    strcpy(c[1].name, "Pedro/Flores/Gutierrez");
    strcpy(c[1].pswd, "31234327jjfj23");
    c[2].id = 2;
    strcpy(c[2].name, "Patricia/Mercedez/Saucedo");
    strcpy(c[2].pswd, "contraseña");
    c[3].id = 3;
    strcpy(c[3].name, "Jesus/Ricardo/Suarez/Perez");
    strcpy(c[3].pswd, "LobitoVeloz777");
    c[4].id = 4;
    strcpy(c[4].name, "Alfredo/Ricardo/Dominguez");
    strcpy(c[4].pswd, "Alfredofeo"); 
    c[5].id = 5;
    strcpy(c[5].name, "Jose/Alfredo/Martinez/Sanchez");
    strcpy(c[5].pswd, "JefFErzon666");
    unsigned short i = 0;
    for (i = 0; i < 6; i++){
        fputs(c[i].name, file);
        fputs("\n", file);
		fputs(c[i].pswd, file);
        fputs("\n", file);
        fprintf(file, "%hd", c[i].id);
        if(i != 5) fputs("\n", file);
    }
	fclose(file);
}

void loadClients(){
	filename = "Clients";
	client c[6];
	if((file = fopen(filename, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
    unsigned short i = 0;
	while(!feof(file)){
		fscanf(file, "%s", c[i].name);
        fscanf(file, "%s", c[i].pswd);
        fscanf(file, "%hd", &c[i].id);
		++i;
	}
    fclose(file);
    if(i == 1) {
        initClients(c);
    }
    clientsKey = ftok("ClientsKey", 'c');
    int shmid1 = shmget(clientsKey, sizeof(client) * 6, IPC_CREAT | 0600);
    client *clients = (client*)shmat(shmid1, 0, 0);
    for(i = 0;i <= 5; i++) clients[i] = c[i];
    pthread_exit(NULL);
}

// void *loadCarts() {

// }

// void *loadCatalog() {

// }

int main(){
    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, (void*)loadClients, NULL);
    pthread_join(thread1, NULL);
    // pthread_create(&thread2, NULL, loadCarts, NULL);
    // pthread_join(thread2, NULL);
    // pthread_create(&thread3, NULL, loadCatalog, NULL);
    // pthread_join(thread3, NULL);
    return 0;
}
