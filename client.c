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

key_t cartsSmphrKey, catalogSmphrKey, controlKey;
semaphore cartsSmphr, catalogSmphr;
int cId;
cart clientCart;
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
    puts("RECEIVING");
    msgrcv(msgid, &message, sizeof(message), 2, 0);
    printf("Data Received is : %d \n", message.mesg_body.login);
    logged = message.mesg_body.login;
    if (logged)
    {
        cId = message.mesg_body.id;
        catalogSmphrKey = message.mesg_body.catalogKey;
        cartsSmphrKey = message.mesg_body.cartsKey;
    }
    msgctl(msgid, IPC_RMID, NULL);
}

void showCatalog()
{
    unsigned short i;
    down(catalogSmphr);
    key_t catalogKey = ftok("CatalogKey", 'b');
    int shmid = shmget(catalogKey, sizeof(productArray), IPC_CREAT | 0600);
    productArray *catalog = (productArray *)shmat(shmid, 0, 0);
    if (catalog->length == 0)
    {
        printf("\nCatálogo de productos vacío, vuelva pronto\n");
        up(catalogSmphr)
        exit(0);
    }
    printf("\nCatálogo de productos:\n");
    for (i = 0; i < catalog->length; i++)
        if (catalog->array[i].stock > 0)
            printf("Número: %d\tNombre del producto: %s\n",
                    catalog->array[i].id, catalog->array[i].name);
    shmdt(catalog);
    up(catalogSmphr);
}

void updateCarts()
{
}

void addToCart(unsigned short pId, unsigned short quantity)
{

    updateCarts();
}

void getCart()
{
    down(cartsSmphr);
    unsigned short i;
    key_t cartsKey = ftok("CartsKey", 'a');
    int shmid = shmget(cartsKey, sizeof(cart *), IPC_CREAT | 0600);
    cart *carts = (cart *)shmat(shmid, 0, 0);
    clientCart = carts[cId];
    printf("\nSu carrito de compras:\n");
    if (clientCart.products.length)
    {
        printf("Su carrito de compras está vacío\n");
        return;
    }
    for (i = 0; i < clientCart.products.length; i++)
    {
        printf("Nombre del producto: %s\nCantidad: %d\n",
                clientCart.products.array[i].name, clientCart.products.array[i].stock);
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