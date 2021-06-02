#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "store.h"

int *len, shmid;
key_t catalogSmphrKey, catalogKey;
semaphore catalogSmphr;
bool isEmptyCatalog;
product *catalog, *aux;

void getCatalog() {
    unsigned short i;
    down(catalogSmphr);
    key_t providerKey = ftok("ProviderKey", 'p');
    int shmid2 = shmget(providerKey, sizeof(int), IPC_CREAT | 0600);
    len = (int*)shmat(shmid2, NULL, 0);
    catalogKey = ftok("CatalogKey", 'a');
    shmid = shmget(catalogKey, (*len) * sizeof(product), IPC_CREAT | 0600);
    catalog = (product*)shmat(shmid, NULL, 0);
    aux = initProductArray(*len);
    for(i = 0; i < *len; i++) {
        aux[i].id = catalog[i].id;
        aux[i].stock = catalog[i].stock;
        strcpy(aux[i].name, catalog[i].name);
    }
    up(catalogSmphr);
    isEmptyCatalog = *len == 0;
}

void updateCatalog() {
    down(catalogSmphr);
    int i;
    shmctl(shmid, IPC_RMID, NULL);
    product *temp, *new;
    int shmid3 = shmget(catalogKey, (*len) * sizeof(product), IPC_CREAT | 0600);
    catalog = (product*)shmat(shmid3, NULL, 0);
    for(i = 0; i < *len; i++) {
        catalog[i].id = aux[i].id;
        catalog[i].stock = aux[i].stock;
        strcpy(catalog[i].name, aux[i].name);
    }
    //aqui se actualiza la memoria compartida (tal vez)
    FILE *file;
    const char *fileName = "Catalog";
    if ((file = fopen(fileName, "w")) == NULL) fprintf(stderr, "Error al crear el archivo");
    fprintf(file, "%d", *len);
    fputs("\n", file);
    for (i = 0; i < *len; i++){
        fprintf(file, "%hd", catalog[i].id);
        fputs("\n", file);
        fprintf(file, "%hd", catalog[i].stock);
        fputs("\n", file);
        fprintf(file, "%s", catalog[i].name);
        if (i != *len - 1)
            fputs("\n", file);
    }
    fclose(file);
    up(catalogSmphr);
}

void addProduct(unsigned short sku, const char *name, unsigned short stock) {
    product p;
    p.id = sku;
    strcpy(p.name, name);
    p.stock = stock;
    pushProduct(aux, len, p);
    if(isEmptyCatalog) isEmptyCatalog = false;
    updateCatalog();
}

void getProduct(unsigned short sku) {
    unsigned short i;
    bool found = false;
    down(catalogSmphr);
    for(i = 0; i < *len; i++)
        if(catalog[i].id == sku){
            printf("\nProducto encontrado:\nNúmero: %d\tNombre del producto: %s\tExistencia: %d\n",
                    catalog[i].id, catalog[i].name, catalog[i].stock);
            found = true;
            up(catalogSmphr);
            return;
        }
    if(!found){
        printf("\nEl producto no se encuentra en el catálogo\n");
        up(catalogSmphr);
    }
}

void addStock(unsigned short sku, unsigned short stock){
    unsigned short i;
    bool found = false;
    down(catalogSmphr);
    for(i = 0; i < *len; i++)
        if(catalog[i].id == sku){
            catalog[i].stock = stock;
            printf("\nSe han agregado las existencias:\nNúmero: %d\tNombre del producto: %s\tExistencia: %d\n",
                    catalog[i].id, catalog[i].name, catalog[i].stock);
            found = true;
            up(catalogSmphr);
            updateCatalog();
            return;
        }
    if(!found) {
        printf("\nEl producto no se encuentra en el catálogo\n");
        up(catalogSmphr);
    }
}

int main() {
    unsigned short opc, stock, sku;
    char cont;
    char pName[NAMELENGTH];
    catalogSmphrKey = ftok("CatalogSmphr", 'n');
    catalogSmphr = semget(catalogSmphrKey, 1, IPC_CREAT | 0644);
    getCatalog();
    printf("\nBienvenido estimado proveedor\n");
    do{
        printf("\nLas operaciones disponibles son:\n0 Buscar producto\n1 Agregar producto\n2 Agregar existencia\nIngrese el número de la operación deseada: ");
        scanf("%hd", &opc);
        switch(opc){
            case 0:
                if(isEmptyCatalog) printf("\nEl catálogo de productos está vacío.");
                else {
                    printf("\nIngresa el sku del producto: ");
                    scanf("%hd", &sku);
                    getProduct(sku);
                }
                break;
            case 1:
                    printf("\nIngresa el sku del producto: ");
                    scanf("%hd", &sku);
                    printf("\nIngresa el nombre del producto: ");
                    scanf("%s", pName);
                    printf("\nIngresa la cantidad de existencia del producto: ");
                    scanf("%hd", &stock);
                    addProduct(sku, pName, stock);
                break;
            case 2:
                if(isEmptyCatalog) printf("\nEl catálogo de productos está vacío.");
                else {
                    printf("\nIngresa el sku del producto: ");
                    scanf("%hd", &sku);
                    printf("\nIngresa la nueva cantidad de existencia del producto: ");
                    scanf("%hd", &stock);
                    addStock(sku, stock);
                }
                break;
            default:
                printf("\nEl número de opción ingresado no es válido (0-2)\n");
                break;
        }
        printf("\n¿Desea realizar otra operación? (S/N)\n");
        fflush(stdin);
        scanf("%c", &cont);
    } while(cont == 's' || cont == 'S');
    shmdt(catalog);
    return 0;
}