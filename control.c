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

key_t clientsKey;
key_t cartsKey;
key_t catalogKey;

void initClients(client *clients){ 
    FILE* file;
    char* fileName = "Clients";
    if((file = fopen(fileName, "a")) == NULL) fprintf(stderr, "Error al crear el archivo");
    clients[0].id = 0;
    strcpy(clients[0].name, "Carlos/Fuentes/Hernandez");
    strcpy(clients[0].pswd, "LobitoVeloz777");
    clients[1].id = 1;
    strcpy(clients[1].name, "Pedro/Flores/Gutierrez");
    strcpy(clients[1].pswd, "31234327jjfj23");
    clients[2].id = 2;
    strcpy(clients[2].name, "Patricia/Mercedez/Saucedo");
    strcpy(clients[2].pswd, "contraseña");
    clients[3].id = 3;
    strcpy(clients[3].name, "Jesus/Ricardo/Suarez/Perez");
    strcpy(clients[3].pswd, "LobitoVeloz777");
    clients[4].id = 4;
    strcpy(clients[4].name, "Alfredo/Ricardo/Dominguez");
    strcpy(clients[4].pswd, "Alfredofeo"); 
    clients[5].id = 5;
    strcpy(clients[5].name, "Jose/Alfredo/Martinez/Sanchez");
    strcpy(clients[5].pswd, "JefFErzon666");
    unsigned short i = 0;
    for (i = 0; i < 6; i++){
        fprintf(file, "%hd", clients[i].id);
        fputs("\n", file);
        fputs(clients[i].name, file);
        fputs("\n", file);
		fputs(clients[i].pswd, file);
        if(i != 5) fputs("\n", file);
    }
	fclose(file);
}

void loadClients(){
    FILE* file;
    const char* fileName = "Clients";
	client clients[6];
	if((file = fopen(fileName, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
    unsigned short i;
	for(i = 0; !feof(file); i++){
        fscanf(file, "%hd", &clients[i].id);
		fscanf(file, "%s", clients[i].name);
        fscanf(file, "%s", clients[i].pswd);
	}
    fclose(file);
    if(i == 1) {
        initClients(clients);
    }
    clientsKey = ftok("ClientsKey", 'c');
    int shmid1 = shmget(clientsKey, sizeof(client) * 6, IPC_CREAT | 0600);
    client *shmClients = (client*)shmat(shmid1, 0, 0);
    for(i = 0; i <= 5; i++) shmClients[i] = clients[i];
    puts("Clients loaded");
    pthread_exit(NULL);
}

void *loadCatalog() {
    puts("Loading Catalog");
    FILE *file;
    const char *fileName = "Catalog";
    productArray *catalog;
    catalog = createCatalog();
    if((file = fopen(fileName, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
    puts("Got Catalog File");
    unsigned short i, j, len;
	for(i = 0; !feof(file); i++){
        puts("Loading");
        fscanf(file, "%hd", &len);
        createProductArray(&catalog[i], len);
        printf("%hd\n", catalog[i].length);
        for(j = 0; j < catalog[i].length; j++){
            fscanf(file, "%hd", &catalog[i].array[j].id);
            fscanf(file, "%hd", &catalog[i].array[j].stock);
		    fscanf(file, "%s", catalog[i].array[j].name);
        }
	}
    fclose(file);
    puts("Got Catalog from File");
    catalogKey = ftok("CatalogKey", 'b');
    int shmid1 = shmget(catalogKey, sizeof(productArray) * i, IPC_CREAT | 0600);
    productArray *shmCatalog = (productArray*)shmat(shmid1, 0, 0);
    for(j = 0; j <= i; j++) shmCatalog[j] = catalog[j];
    puts("Catalog loaded");
    pthread_exit(NULL);
}

void loadCarts() {
    puts("Loading Carts");
    FILE* file;
    const char* fileName = "Carts";
    cart* carts = (cart*)malloc(sizeof(cart));
    if((file = fopen(fileName, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
    unsigned short i, j;
    puts("Got Carts File");
	for(i = 0; !feof(file); i++){
        fscanf(file, "%hd", &carts[i].clientId);
        fscanf(file, "%hd", &carts[i].products.length);
        for(j = 0; j < carts[i].products.length; j++){
            fscanf(file, "%hd", &carts[i].products.array[j].id);
            fscanf(file, "%hd", &carts[i].products.array[j].stock);
		    fscanf(file, "%s", carts[i].products.array[j].name);
        }
	}
    fclose(file);
    puts("Got Carts from File");
    cartsKey = ftok("CartsKey", 'a');
    int shmid1 = shmget(cartsKey, sizeof(cart) * i, IPC_CREAT | 0600);
    cart *shmCarts = (cart*)shmat(shmid1, 0, 0);
    for(j = 0; j <= i; j++) shmCarts[j] = carts[j];
    puts("Carts loaded");
    pthread_exit(NULL);
}

int main(){
    pthread_t clientsThread, catalogThread, cartsThread;
    pthread_create(&clientsThread, NULL, (void*)loadClients, NULL);
    pthread_join(clientsThread, NULL);
    pthread_create(&catalogThread, NULL, (void*)loadCatalog, NULL);
    pthread_join(catalogThread, NULL);
    pthread_create(&cartsThread, NULL, (void*)loadCarts, NULL);
    pthread_join(cartsThread, NULL);
    return 0;
}
