#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "store.h"

key_t cartsSmphrKey, catalogSmphrKey, cartsKey;
semaphore cartsSmphr, catalogSmphr;
unsigned short *cIds, *cLengths;
int cId, catalogLength, shmid1, shmid2, shmid3, cartsLen = 0;
cart clientCart, *clientsCarts;
product *catalog, *aux, *pArray;
bool logged = false;

void login(const char *mail, const char *pswd)
{
    mesg_buffer message = {1, {false, 0, 0, 0}};
    mesg_buffer2 message2;
    key_t controlKey2 = ftok("ControlKey", 'p'), controlKey;
    int msgid, msgid2;
    msgid2 = msgget(controlKey2, 0666 | IPC_CREAT);
    message2.mesg_type = 1;
    strcpy(message2.mesg_body.credentials.mail, mail);
    strcpy(message2.mesg_body.credentials.pswd, pswd);
    msgsnd(msgid2, &message2, sizeof(message2), 0);
    controlKey = ftok("ControlKey", 65);
    msgid = msgget(controlKey, 0666 | IPC_CREAT);
    msgrcv(msgid, &message, sizeof(message), 2, 0);
    logged = message.mesg_body.login;
    if (logged)
    {
        cId = message.mesg_body.id;
        catalogSmphrKey = message.mesg_body.catalogKey;
        cartsSmphrKey = message.mesg_body.cartsKey;
        catalogLength = message.mesg_body.catalogLength;
    }
    msgctl(msgid, IPC_RMID, NULL);
}

void showCatalog()
{
    unsigned short i;
    down(catalogSmphr);
    key_t catalogKey = ftok("CatalogKey", 'a');
    int shmid = shmget(catalogKey, sizeof(product) * catalogLength, IPC_CREAT | 0600);
    catalog = (product *)shmat(shmid, NULL, 0);
    if (catalogLength == 0)
    {
        printf("\nCatálogo de productos vacío, vuelva pronto\n");
        up(catalogSmphr);
        exit(0);
    }
    printf("\nCatálogo de productos:\n");
    for (i = 0; i < catalogLength; i++)
        if (catalog[i].stock > 0) printf("Número: %d\tNombre del producto: %s\n",
                                            catalog[i].id, catalog[i].name);
    up(catalogSmphr);
}

void updateCarts()
{
    down(cartsSmphr);
    unsigned short i, j, k = 0;
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid3, IPC_RMID, NULL);
    cartsKey = ftok("CartsKey", 'a');
    int shmid1 = shmget(cartsKey, sizeof(unsigned short) * 6, IPC_CREAT | 0600), shmid2, shmid3;
    cLengths = (unsigned short*)shmat(shmid1, NULL, 0);
    cartsKey = ftok("CartsKey", 'b');
    shmid2 = shmget(cartsKey, sizeof(unsigned short) * 6, IPC_CREAT | 0600);
    cIds = (unsigned short*)shmat(shmid2, NULL, 0);
    cartsKey = ftok("CartsKey", 'c');
    shmid3 = shmget(cartsKey, sizeof(product) * cartsLen, IPC_CREAT | 0600);
    pArray = (product*)shmat(shmid3, NULL, 0);
    for(i = 0; i < 6; i++) {
        cIds[i] = clientsCarts[i].clientId;
        cLengths[i] = clientsCarts[i].length;
        for(j = 0; j < cLengths[i]; j++, k++) {
            pArray[k].id = clientsCarts[i].pArray[j].id;
            pArray[k].stock = clientsCarts[i].pArray[j].stock;
            strcpy(pArray[k].name, clientsCarts[i].pArray[j].name);
        }
    }
    FILE *file;
    const char *fileName = "Carts";
    if ((file = fopen(fileName, "w")) == NULL) fprintf(stderr, "Error al crear el archivo");
    for (i = 0; i < 6; i++){
        fprintf(file, "%hd", cIds[i]);
        fputs("\n", file);
        fprintf(file, "%hd", cLengths[i]);
        if(cLengths[i] != 0 && i != 5) fputs("\n", file);
        for(j = 0; j < cLengths[i]; j++, k++) {
            fprintf(file, "%hd", pArray[k].id);
            fputs("\n", file);
            fprintf(file, "%hd", pArray[k].stock);
            fputs("\n", file);
            fprintf(file, "%s", pArray[k].name);
            if (i != 5) fputs("\n", file);
        }
    }
    fclose(file);
    up(cartsSmphr);
}

void addToCart(unsigned short pId, unsigned short quantity)
{
    unsigned short i;
    bool found = false;
    down(catalogSmphr);
    for(i = 0; i < catalogLength; i++)
        if(catalog[i].id == pId && catalog[i].stock != 0){
            --catalog[i].stock;
            product prod;
            prod.id = catalog[i].id;
            prod.stock = quantity;
            strcpy(prod.name, catalog[i].name);
            pushProduct(clientCart.pArray, &clientCart.length, prod);
            clientsCarts[cId].length = clientCart.length;
            clientsCarts[cId].pArray = clientCart.pArray;
            updateCarts();
            printf("\nEl producto ha sido agregado a su carrito:\nNúmero: %d\tNombre del producto: %s\nCantidad: %d\n",
                    clientCart.pArray[clientCart.length - 1].id, clientCart.pArray[clientCart.length - 1].name,
                    clientCart.pArray[clientCart.length - 1].stock);
            found = true;
            up(catalogSmphr);
            return;
        }
    if(!found){
        printf("\nEl producto no se encuentra en el catálogo\n");
        up(catalogSmphr);
    }
}

void getCart()
{
    down(cartsSmphr);
    unsigned short i, j, k, pLength = 0;
    cartsKey = ftok("CartsKey", 'a');
    int shmid1 = shmget(cartsKey, sizeof(unsigned short) * 6, IPC_CREAT | 0600), shmid2, shmid3;
    cLengths = (unsigned short*)shmat(shmid1, NULL, 0);
    cartsKey = ftok("CartsKey", 'b');
    shmid2 = shmget(cartsKey, sizeof(unsigned short) * 6, IPC_CREAT | 0600);
    cIds = (unsigned short*)shmat(shmid2, NULL, 0);
    cartsKey = ftok("CartsKey", 'c');
    for(i = 0; i < 6; i++) pLength += cLengths[i];
    cartsLen = pLength;
    shmid3 = shmget(cartsKey, sizeof(product) * pLength, IPC_CREAT | 0600);
    pArray = (product*)shmat(shmid3, NULL, 0);
    if(!(clientsCarts = (cart*)malloc(sizeof(cart) * 6))) perror("Error en malloc");
    k = 0;
    for(i = 0; i < 6; i++) {
        clientsCarts[i].clientId = cIds[i];
        clientsCarts[i].length = cLengths[i];
        clientsCarts[i].pArray = initProductArray(clientsCarts[i].length);
        for(j = 0; j < clientsCarts[i].length; j++, k++) {
            clientsCarts[i].pArray[j].id = pArray[k].id;
            clientsCarts[i].pArray[j].stock = pArray[k].stock;
            strcpy(clientsCarts[i].pArray[j].name, pArray[k].name);
        }
    }
    clientCart = clientsCarts[cId];
    printf("\nSu carrito de compras:\n");
    if (!clientCart.length)
    {
        printf("Su carrito de compras está vacío\n");
        up(cartsSmphr);
        return;
    }
    for (i = 0; i < clientCart.length; i++)
    {
        printf("Nombre del producto: %s\nCantidad: %d\n",
                clientCart.pArray[i].name, clientCart.pArray[i].stock);
    }
    up(cartsSmphr);
}

int main()
{
    unsigned short pId, quantity, menu = 0;
    unsigned char opc;
    char pushProd;
    char mail[MAILLENGTH], pswd[PSWDLENGTH];
    printf("\nBienvenido estimado cliente\n\nIntroduzca su correo electrónico: ");
    fflush(stdin);
    scanf("%s", mail);
    printf("Introduzca su contraseña: ");
    fflush(stdin);
    scanf("%s", pswd);
    login(mail, pswd);
    while (!logged)
    {
        printf("\nUsuario o contraseña incorrectos.\n\nIntroduzca su correo electrónico: ");
        fflush(stdin);
        scanf("%s", mail);
        printf("Introduzca su contraseña: ");
        fflush(stdin);
        scanf("%s", pswd);
        login(mail, pswd);
    }
    cartsSmphr = semget(cartsSmphrKey, 1, IPC_CREAT | 0644);
    up(cartsSmphr);
    catalogSmphr = semget(catalogSmphrKey, 1, IPC_CREAT | 0644);
    showCatalog();
    getCart();
    printf("¿Desea agregar productos a su carrito? (S/N)\n");
    fflush(stdin);
    scanf("%c", &pushProd);
    while (pushProd == 'S' || pushProd == 's')
    {
        if (menu != 0)
        {
            showCatalog();
            getCart();
        }
        printf("\nIngresa el número del producto a añadir al carrito: ");
        scanf("%hd", &pId);
        printf("\nIngresa la cantidad del producto a añadir al carrito: ");
        scanf("%hd", &quantity);
        addToCart(pId, quantity);
        printf("¿Desea agregar más productos a su carrito? (S/N)\n");
        fflush(stdin);
        scanf("%c", &pushProd);
        menu++;
    }
    return 0;
}