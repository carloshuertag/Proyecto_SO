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

key_t cartsSmphrKey, catalogSmphrKey, controlKey;
int cId;
cart clientCart;
bool logged = false;

void login(const char *mail, const char *pswd) {
    controlKey = ftok("ControlKey", 'o');
    int shmid = shmget(controlKey, sizeof(loginDTS), IPC_CREAT | 0600);
    loginDTS *loginBuffer = (loginDTS*)shmat(shmid, 0, 0);
    strcpy(loginBuffer->credentials.mail, mail);
    strcpy(loginBuffer->credentials.pswd, pswd);
    sleep(1);
    logged = loginBuffer->login;
    cId = loginBuffer->credentials.id;
    cartsSmphrKey = loginBuffer->cartsKey;
    catalogSmphrKey = loginBuffer->catalogKey;
}

void showCatalog() {
    unsigned short i;
    key_t catalogKey = ftok("CatalogKey", 'b');
    int shmid = shmget(catalogKey, sizeof(productArray), IPC_CREAT | 0600);
    productArray *catalog = (productArray*)shmat(shmid, 0, 0);
    if(catalog->length == 0){
        printf("\nCatálogo de productos vacío, vuelva pronto\n");
        exit(0);
    }
    printf("\nCatálogo de productos:\n");
    for(i = 0; i < catalog->length; i++)
        if(catalog->array[i].stock > 0)
            printf("Número: %d\tNombre del producto: %s\n",
                    catalog->array[i].id, catalog->array[i].name);
}

void addToCart(unsigned short pId, unsigned short quantity){
    
}

void getCart() {
    unsigned short i;
    key_t cartsKey = ftok("CartsKey", 'a');
    int shmid = shmget(cartsKey, sizeof(cart*), IPC_CREAT | 0600);
    cart *carts = (cart*)shmat(shmid, 0, 0);
    clientCart = carts[cId];
    printf("\nSu carrito de compras:\n");
    if(clientCart.products.length) {
        printf("Su carrito de compras está vacío\n");
        return;
    }
    for(i = 0; i < clientCart.products.length; i++){
        printf("Nombre del producto: %s\nCantidad: %d\n",
                clientCart.products.array[i].name, clientCart.products.array[i].stock);
    }
}

int main() {
    unsigned short pId, quantity;
    unsigned char opc;
    char pushProd;
    char mail[MAILLENGTH], pswd[PSWDLENGTH];
    printf("Introduzca su correo electrónico: ");
    fflush(stdin);
    scanf("%s", mail);
    printf("Introduzca su contraseña: ");
    fflush(stdin);
    scanf("%s", pswd);
    login(mail, pswd);
    while(!logged){
        printf("\nUsuario o contraseña incorrectos.\n\nIntroduzca su correo electrónico: ");
        fflush(stdin);
        scanf("%s", mail);
        printf("Introduzca su contraseña: ");
        fflush(stdin);
        scanf("%s", pswd);
        login(mail, pswd);
    }
    showCatalog();
    getCart();
    printf("¿Desea agregar productos a su carrito? (S/N)\n");
    fflush(stdin);
    scanf("%c", &pushProd);
    while(pushProd == 'S' || pushProd == 's'){
        printf("\nIngresa el número del productp a añadir al carrito: ");
        scanf("%hd", &pId);
        printf("\nIngresa la cantidad del producto a añadir al carrito: ");
        scanf("%hd", &quantity);
        addToCart(pId, quantity);
        printf("¿Desea agregar más productos a su carrito? (S/N)\n");
        fflush(stdin);
        scanf("%c", &pushProd);
    }
    return 0;
}