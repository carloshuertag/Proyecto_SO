#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#define NAMELENGTH 32
#define PSWDLENGTH 32

typedef struct client
{
    unsigned short id;
    char mail[NAMELENGTH];
    char pswd[PSWDLENGTH]; /* Password */
} client;

typedef struct serverDTS
{
    bool login;
    key_t cartsKey;
    key_t catalogKey;
} serverDTS;

typedef struct clientDTS
{
    client credentials;
} clientDTS;

struct mesg_buffer
{
    long mesg_type;
    serverDTS mesg_body;
} message;

struct mesg_buffer2
{
    long mesg_type;
    clientDTS mesg_body;
} message2;

int main()
{
    key_t key2;
    int msgid2;
    key2 = ftok("progfile", 70);
    msgid2 = msgget(key2, 0666 | IPC_CREAT);
    message2.mesg_type = 1;
    printf("Write Data : ");
    fflush(stdin);
    fgets(message2.mesg_body.credentials.mail, NAMELENGTH, stdin);
    printf("Write Data : ");
    fflush(stdin);
    fgets(message2.mesg_body.credentials.pswd, PSWDLENGTH, stdin);
    msgsnd(msgid2, &message2, sizeof(message2), 0);
    printf("Data send is : %s, %s \n", message2.mesg_body.credentials.mail, message2.mesg_body.credentials.pswd);
    key_t key;
    int msgid;
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);
    msgrcv(msgid, &message, sizeof(message), 1, 0);
    printf("Data Received is : %d \n", message.mesg_body.login);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}