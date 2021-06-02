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

key_t cartsSmphrKey, catalogSmphrKey;
semaphore cartsSmphr, catalogSmphr;
int cId, catalogLength, shmid, cartsLen = 0;
cart clientCart, *clientsCarts, *carts;
product *catalog, *aux;
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
    unsigned short i;
    int j;
    shmctl(shmid, IPC_RMID, NULL);
    key_t cartsKey = ftok("CartsKey", 'a');
    int shmid3 = shmget(cartsKey, 6 * sizeof(cart) + cartsLen * sizeof(product), IPC_CREAT | 0600);
    carts = (cart*)shmat(shmid3, NULL, 0);
    for(i = 0; i < 6; i++) {
        carts[i].clientId = clientsCarts[i].clientId;
        carts[i].length = clientsCarts[i].length;
        for(j = 0; j < carts[i].length; j++) {
            carts[i].pArray[j].id = clientsCarts[i].pArray[j].id;
            carts[i].pArray[j].stock = clientsCarts[i].pArray[j].stock;
            strcpy(carts[i].pArray[j].name, clientsCarts[i].pArray[j].name);
        }
    }
    FILE *file;
    const char *fileName = "Carts";
    if ((file = fopen(fileName, "w")) == NULL) fprintf(stderr, "Error al crear el archivo");
    for (i = 0; i < 6; i++){
        fprintf(file, "%hd", carts[i].clientId);
        fputs("\n", file);
        fprintf(file, "%hd", carts[i].length);
        fputs("\n", file);
        for(j = 0; j < carts[i].length; j++) {
            fprintf(file, "%hd", carts[i].pArray[j].id);
            fputs("\n", file);
            fprintf(file, "%hd", carts[i].pArray[j].stock);
            fputs("\n", file);
            fprintf(file, "%s", carts[i].pArray[j].name);
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
            ++clientCart.length;
            product prod;
            prod.id = catalog[i].id;
            prod.stock = quantity;
            strcpy(prod.name, catalog[i].name);
            pushProduct(clientCart.pArray, &clientCart.length, prod);
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
    unsigned short i, j;
    key_t cartsKey = ftok("CartsKey", 'a');
    shmid = shmget(cartsKey, sizeof(cart) * 6 + catalogLength * sizeof(product), IPC_CREAT | 0600);
    carts = (cart *)shmat(shmid, 0, 0);
    if(!(clientsCarts = (cart*)malloc(sizeof(cart) * 6))) perror("Error en malloc");
    for(i = 0; i < 6; i++) {
        printf("%hd\n", carts[i].clientId);
        clientsCarts[i].clientId = carts[i].clientId;
        clientsCarts[i].length = carts[i].length;
        cartsLen += carts[i].length;
        puts("CCCC");
        clientsCarts[i].pArray = initProductArray(clientsCarts[i].length);
        puts("DDDD");
        for(j = 0; j < clientsCarts[i].length; j++) {
            clientsCarts[i].pArray[j].id = carts[i].pArray[j].id;
            clientsCarts[i].pArray[j].stock = carts[i].pArray[j].stock;
            strcpy(clientsCarts[i].pArray[j].name, carts[i].pArray[j].name);
        }
    }
    clientCart = clientsCarts[cId];
    printf("\nSu carrito de compras:\n");
    if (clientCart.length)
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
        printf("\nIngresa el número del productp a añadir al carrito: ");
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