#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "store.h"

key_t cartsSmphrKey, catalogSmphrKey;
bool isEmptyCatalog;
productArray *catalog;

void getCatalog() {
    unsigned short i;
    key_t catalogKey = ftok("CatalogKey", 'b');
    int shmid = shmget(catalogKey, sizeof(productArray), IPC_CREAT | 0600);
    catalog = (productArray*)shmat(shmid, 0, 0);
    isEmptyCatalog = catalog->length == 0;
}

void addProduct(unsigned short sku, const char *name, unsigned short stock) {
    product p;
    pushProduct(catalog, p);
}

void getProduct(unsigned short sku) {

}

void addStock(unsigned short sku, unsigned short stock){

}

int main() {
    unsigned short opc, stock, sku;
    char cont;
    char pName[NAMELENGTH];
    cartsSmphrKey = ftok("CartsSmphr", 'm');
    catalogSmphrKey = ftok("CatalogSmphr", 'n');
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
                    printf("\nIngresa la cantidad de existencia del producto: ");
                    scanf("%hd", &stock);
                    addStock(sku, stock);
                }
                break;
            default:
                printf("\nEl número de opción ingresado no es válido (0-2)\n");
                break;
        }
        printf("\n¿Desea realizar otra operación? (S/N)\n");
    } while(cont == 's' || cont == 'S');
    return 0;
}
