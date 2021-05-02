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
short i = 0;
char* filename;
key_t clientsKey;
key_t cartsKey;
key_t catalogKey;

void initClients(client c[6]){ 
    FILE* file;
	char* filename = "Clients";
    if((file = fopen(filename, "w")) == NULL){
		fprintf(stderr, "Error al crear el archivo");
	}
    c[0].id = 0;
    strcpy(c[0].name, "Carlos Fuentes Hernandez");
    strcpy(c[0].pswd, "LobitoVeloz777");
    c[1].id = 1;
    strcpy(c[1].name, "Pedro Flores Gutierrez");
    strcpy(c[1].pswd, "31234327jjfj23");
    c[2].id = 2;
    strcpy(c[2].name, "Patricia Mercedez Saucedo");
    strcpy(c[2].pswd, "contraseña");
    c[3].id = 3;
    strcpy(c[3].name, "Jesus Ricardo Suarez Perez");
    strcpy(c[3].pswd, "LobitoVeloz777");
    c[4].id = 4;
    strcpy(c[4].name, "Alfredo Ricardo Dominguez");
    strcpy(c[4].pswd, "Alfredofeo"); 
    c[5].id = 5;
    strcpy(c[5].name, "Jose Alfredo Martinez Sanchez");
    strcpy(c[5].pswd, "JefFErzon666");
    for (i = 0; i < 6; i++){
        fprintf(file, "%hd\n", c[i].id);
        fprintf(file, "%s\n", c[i].name);
        fprintf(file, "%s\n", c[i].pswd);
    }
	fclose(file);
    pthread_exit(NULL);
}

void *loadClients(){
	filename = "Clients";
	client c[6];
	if((file = fopen(filename, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
	while(!feof(file)){
		fscanf(file,"%hd",&c[i].id);
		fgets(c[i].name, NAMELENGTH, file);
        fgets(c[i].pswd, PSWDLENGTH, file);
		++i;
	}
    puts("He llegado");
	fclose(file);
    //meter lo del arreglo a la compartida
    clientsKey = ftok("ClientsKey", 'c');
    int shmid1 = shmget(clientsKey, sizeof(client)*6, IPC_CREAT | 0600);
    client *arreglo = (client*)shmat(shmid1,0,0);
    for(i = 0;i <= 5; i++) arreglo[i] = c[i];
}

// void *loadCarts() {

// }

// void *loadCatalog() {

// }

int main(){
    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, loadClients, NULL);
    pthread_join(thread1, NULL);
    // pthread_create(&thread2, NULL, loadCarts, NULL);
    // pthread_join(thread2, NULL);
    // pthread_create(&thread3, NULL, loadCatalog, NULL);
    // pthread_join(thread3, NULL);
    wait(NULL);
    return 0;
}
