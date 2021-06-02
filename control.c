#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
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
unsigned short catalogLength = 0;
char *mails[MAILLENGTH] = {"cfuentesh@gmail.com", "pfloresg@hotmail.com", "pmercedezs@outlook.com",
                            "jsuarezp@gmail.com", "aricardod@live.com", "jmartinezs@yahoo.com"},
     *pswds[PSWDLENGTH] = {"LobitoVeloz777", "31234327jjfj23", "contraseña", "12345678", "Alfredofeo",
                            "JefFErzon666"};

void initClients(client *clients)
{
    FILE *file;
    const char *fileName = "Clients";
    if ((file = fopen(fileName, "a")) == NULL)
        fprintf(stderr, "Error al crear el archivo");
    clients[0].id = 0;
    strcpy(clients[0].mail, mails[0]);
    strcpy(clients[0].pswd, pswds[0]);
    clients[1].id = 1;
    strcpy(clients[1].mail, mails[1]);
    strcpy(clients[1].pswd, pswds[1]);
    clients[2].id = 2;
    strcpy(clients[2].mail, mails[2]);
    strcpy(clients[2].pswd, pswds[2]);
    clients[3].id = 3;
    strcpy(clients[3].mail, mails[3]);
    strcpy(clients[3].pswd, pswds[3]);
    clients[4].id = 4;
    strcpy(clients[4].mail, mails[4]);
    strcpy(clients[4].pswd, pswds[4]);
    clients[5].id = 5;
    strcpy(clients[5].mail, mails[5]);
    strcpy(clients[5].pswd, pswds[5]);
    unsigned short i = 0;
    for (i = 0; i < 6; i++)
    {
        fprintf(file, "%hd", clients[i].id);
        fputs("\n", file);
        fputs(clients[i].mail, file);
        fputs("\n", file);
        fputs(clients[i].pswd, file);
        if (i != 5)
            fputs("\n", file);
    }
    fclose(file);
}

void initCarts(cart *carts)
{
    FILE *file;
    const char *fileName = "Carts";
    if ((file = fopen(fileName, "a")) == NULL)
        fprintf(stderr, "Error al crear el archivo");
    carts[0].clientId = 0;
    carts[0].pArray = initProductArray(0);
    carts[1].clientId = 1;
    carts[1].pArray = initProductArray(0);
    carts[2].clientId = 2;
    carts[2].pArray = initProductArray(0);
    carts[3].clientId = 3;
    carts[3].pArray = initProductArray(0);
    carts[4].clientId = 4;
    carts[4].pArray = initProductArray(0);
    carts[5].clientId = 5;
    carts[5].pArray = initProductArray(0);
    unsigned short i = 0;
    for (i = 0; i < 6; i++)
    {
        fprintf(file, "%hd", carts[i].clientId);
        fputs("\n", file);
        fprintf(file, "%hd", (unsigned short)0);
        if (i != 5)
            fputs("\n", file);
    }
    fclose(file);
}

void initCatalog(product *catalog, unsigned short len)
{
    FILE *file;
    const char *fileName = "Catalog";
    if ((file = fopen(fileName, "w")) == NULL)
        fprintf(stderr, "Error al crear el archivo");
    fprintf(file, "%hd", len);
    fputs("\n", file);
    fclose(file);
}

void loadClients()
{
    puts("LOADING CLIENTS");
    FILE *file;
    const char *fileName = "Clients";
    client clients[6];
    if ((file = fopen(fileName, "r")) == NULL)
        fprintf(stderr, "Error al leer el archivo");
    unsigned short i;
    for (i = 0; !feof(file); i++)
    {
        fscanf(file, "%hd", &clients[i].id);
        fscanf(file, "%s", clients[i].mail);
        fscanf(file, "%s", clients[i].pswd);
    }
    fclose(file);
    if (i <= 1)
        initClients(clients);
    key_t clientsKey = ftok("ClientsKey", 'c');
    int shmid = shmget(clientsKey, sizeof(client) * 6, IPC_CREAT | 0600);
    client *shmClients = (client *)shmat(shmid, 0, 0);
    for (i = 0; i < 6; i++)
        shmClients[i] = clients[i];
    shmdt(shmClients);
    puts("LOADED CLIENTS");
    pthread_exit(NULL);
}

void loadCatalog()
{
    puts("LOADING CATALOG");
    FILE *file;
    const char *fileName = "Catalog";
    product *catalog, *shmCatalog;
    if ((file = fopen(fileName, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
    unsigned short i;
    fscanf(file, "%hd", &catalogLength);
    catalog = initProductArray(catalogLength);
    if (catalogLength > 0)
        while (!feof(file))
            for (i = 0; i < catalogLength; i++)
            {
                fscanf(file, "%hd", &catalog[i].id);
                fscanf(file, "%hd", &catalog[i].stock);
                fscanf(file, "%s", catalog[i].name);
            }
    fclose(file);
    if (!catalogLength) initCatalog(catalog, 0);
    key_t catalogKey = ftok("CatalogKey", 'a');
    int shmid1 = shmget(catalogKey, catalogLength * sizeof(product), IPC_CREAT | 0600);
    shmCatalog = (product *)shmat(shmid1, NULL, 0);
    product *aux;
    for (i = 0; i < catalogLength; i++) {
        shmCatalog[i].id = catalog[i].id;
        shmCatalog[i].stock = catalog[i].stock;
        strcpy(shmCatalog[i].name, catalog[i].name);
    }
    shmdt(shmCatalog);
    key_t providerKey = ftok("ProviderKey", 'p');
    int shmid2 = shmget(providerKey, sizeof(int), IPC_CREAT | 0600), *len;
    len = (int*)shmat(shmid2, NULL, 0);
    *len = catalogLength;
    shmdt(len);
    puts("CATALOG LOADED");
    pthread_exit(NULL);
}

void loadCarts()
{
    puts("LOADING CARTS");
    FILE *file;
    const char *fileName = "Carts";
    product* pArray;
    cart* carts;
    unsigned short *cLengths, *cIds;
    if ((file = fopen(fileName, "r")) == NULL) fprintf(stderr, "Error al leer el archivo");
    if(!(carts = (cart *)malloc(sizeof(cart) * 6))) fprintf(stderr, "Error en malloc");;
    unsigned short i, j, k, len = 0;
    for (i = 0; !feof(file); i++)
    {
        fscanf(file, "%hd", &carts[i].clientId);
        fscanf(file, "%hd", &carts[i].length);
        carts[i].pArray = initProductArray(carts[i].length);
        len += carts[i].length;
        for (j = 0; j < carts[i].length; j++)
        {
            fscanf(file, "%hd", &carts[i].pArray[j].id);
            fscanf(file, "%hd", &carts[i].pArray[j].stock);
            fscanf(file, "%s", carts[i].pArray[j].name);
        }
    }
    fclose(file);
    if (i <= 1)
        initCarts(carts);
    key_t cartsKey = ftok("CartsKey", 'a');
    int shmid1 = shmget(cartsKey, sizeof(unsigned short) * 6, IPC_CREAT | 0600), shmid2, shmid3;
    cLengths = (unsigned short*)shmat(shmid1, NULL, 0);
    cartsKey = ftok("CartsKey", 'b');
    shmid2 = shmget(cartsKey, sizeof(unsigned short) * 6, IPC_CREAT | 0600);
    cIds = (unsigned short*)shmat(shmid2, NULL, 0);
    cartsKey = ftok("CartsKey", 'c');
    shmid3 = shmget(cartsKey, sizeof(product) * len, IPC_CREAT | 0600);
    pArray = (product*)shmat(shmid3, NULL, 0);
    i = 0;
    for (j = 0; j < 6; j++){
        cIds[j] = carts[j].clientId;
        cLengths[j] = carts[j].length;
        for (k = 0; k < cLengths[j]; k++, i++) {
            pArray[i].id = carts[j].pArray[k].id;
            pArray[i].stock = carts[j].pArray[k].stock;
            strcpy(pArray[i].name, carts[j].pArray[k].name);
        }
    }
    shmdt(cIds);
    shmdt(cLengths);
    shmdt(pArray);
    puts("LOADED CARTS");
    pthread_exit(NULL);
}

void clientLogin()
{
    mesg_buffer message;
    mesg_buffer2 message2;
    key_t controlKey = ftok("ControlKey", 65), controlKey2 = ftok("ControlKey", 'p');
    int msgid, msgid2;
    unsigned char i;
    while (1)
    {
        msgid2 = msgget(controlKey2, 0666 | IPC_CREAT);
        puts("RECEIVING");
        msgrcv(msgid2, &message2, sizeof(message2), 1, 0);
        printf("Data Received is : %s,%s \n", message2.mesg_body.credentials.mail, message2.mesg_body.credentials.pswd);
        for (i = 0; i < 6; i++)
        {
            if (!strcmp(message2.mesg_body.credentials.mail, mails[i]) && !strcmp(message2.mesg_body.credentials.pswd, pswds[i]))
            {
                message.mesg_body.login = true;
                message.mesg_body.id = i;
                message.mesg_body.cartsKey = cartsSmphrKey;
                message.mesg_body.catalogKey = catalogSmphrKey;
                message.mesg_body.catalogLength = catalogLength;
                break;
            }
            else
                message.mesg_body.login = message.mesg_body.id = 0;
        }
        msgid = msgget(controlKey, 0666 | IPC_CREAT);
        message.mesg_type = 2;
        msgsnd(msgid, &message, sizeof(message), 0);
        printf("Data send is : %d \n", message.mesg_body.login);
        msgctl(msgid2, IPC_RMID, NULL);
    }
}

int main()
{
    pthread_t clientsThread, catalogThread, cartsThread, controlThread;
    cartsSmphrKey = ftok("CartsSmphr", 'm');
    cartsSmphr = semaphore_init(&cartsSmphrKey, 1);
    catalogSmphrKey = ftok("CatalogSmphr", 'n');
    catalogSmphr = semaphore_init(&catalogSmphrKey, 1);
    pthread_create(&clientsThread, NULL, (void *)loadClients, NULL);
    pthread_join(clientsThread, NULL);
    pthread_create(&catalogThread, NULL, (void *)loadCatalog, NULL);
    pthread_join(catalogThread, NULL);
    pthread_create(&cartsThread, NULL, (void *)loadCarts, NULL);
    pthread_join(cartsThread, NULL);
    pthread_create(&controlThread, NULL, (void *)clientLogin, NULL);
    pthread_join(controlThread, NULL);
    return 0;
}
