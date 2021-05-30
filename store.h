#define NAMELENGTH 32
#define MAILLENGTH 32
#define PSWDLENGTH 32

// ----Store core----
typedef struct client
{
    unsigned short id;
    char mail[NAMELENGTH];
    char pswd[PSWDLENGTH]; /* Password */
} client;

typedef struct product
{
    unsigned short id;
    unsigned short stock;
    char name[NAMELENGTH];
} product;

// -----Arrays structs------
typedef struct productArray
{
    unsigned short length;
    product *array;
} productArray;

typedef struct cart
{
    unsigned short clientId;
    productArray products;
} cart;

void createProductArray(productArray *pArray, unsigned short length)
{
    pArray->array = (length == 0) ? (product *)malloc(sizeof(product)) : (product *)calloc(length, sizeof(product));
    pArray->length = length;
}

void pushProduct(productArray *pArray, product element)
{
    if ((pArray->array = (product *)realloc(pArray->array, ++pArray->length * sizeof(product))))
    {
        pArray->array[pArray->length - 1] = element;
    }
    else
    {
        free(pArray);
        fprintf(stderr, "Error (re)allocating memory");
        exit(1);
    }
}

productArray *createCatalog()
{
    productArray *catalog = (productArray *)malloc(sizeof(productArray));
    if (!catalog)
        exit(1);
    catalog->length = 0;
    catalog->array = NULL;
    createProductArray(catalog, 0);
    return catalog;
}

// ----Semaphores----
typedef int semaphore;

int semaphore_init(key_t key, int value) {
    int semid = semget(key, 1, IPC_CREAT | 0644);
    if(semid == -1){
        fprintf(stderr, "Error al crear semáforo");
        exit(1);
    }
    semctl(semid, 0, SETVAL, value);
    return semid;
}

void down(int semid) {
    struct sembuf op_p[] = {0, -1, 0};
    semop(semid, op_p, 1);
}

void up(int semid) {
    struct sembuf op_v[] = {0, +1, 0};
    semop(semid,op_v, 1);
}

// -----DTS structs-----
typedef struct controlDTS
{
    bool login;
    unsigned short id;
    key_t cartsKey;
    key_t catalogKey;
} controlDTS;

typedef struct clientDTS
{
    client credentials;
} clientDTS;

typedef struct mesg_buffer
{
    long mesg_type;
    controlDTS mesg_body;
} mesg_buffer;

typedef struct mesg_buffer2
{
    long mesg_type;
    clientDTS mesg_body;
} mesg_buffer2;

/*
// -----------------------inicia productsList------------------------------
typedef struct productListElement {
    unsigned short index;
    product element;
    struct productListElement* next;
    struct productListElement* prev;
} productListElement;

typedef struct productList {
    struct productListElement* head;
    unsigned short size;
} productList;


productList* createProductList() {
    productList* list = (productList*)malloc(sizeof(productList));
    if(!list)
        exit(1);
    list->head = NULL;
    list->size = 0;
    return list;
}

unsigned short productListSize(productList* list){
    return list->size;
}

bool isEmptyProductList(productList* list){
    return !list->head;
}

productListElement* createProductListElement(unsigned short id, unsigned short stock, const char *name) {
    productListElement* listElement = (productListElement*)malloc(sizeof(productListElement));
    listElement->element.id = id;
    listElement->element.stock = stock;
    strcpy(listElement->element.name, name);
    listElement->prev = listElement->next = NULL;
    return listElement;
}

void appendProductListElement(productList* list, unsigned short id, unsigned short stock, const char *name) {
    productListElement* listElement = createProductListElement(id, stock, name), *last = list->head;
    if(isEmptyProductList(list)){
        listElement->prev = NULL;
        list->head = listElement;
        listElement->index = 0;
        ++list->size;
        return;
    }
    while(last->next)
        last = last->next;
    last->next = listElement;
    listElement->prev = last;
    listElement->index = list->size;
    ++list->size;
}

productListElement* getProductListElement(productList* list, unsigned short index){
    if(index < 0 || index >= list->size || isEmptyProductList(list)) {
        puts("No existe el elemento");
        return NULL;
    }
    productListElement* listElement = list->head;
    unsigned short i;
    for(i = 0; i < index; i++)
        listElement = listElement->next;
    return listElement;
}

void setProductListElement(productList* list, unsigned short index, unsigned short id, unsigned short stock, const char *name) {
    productListElement* aux;
    if((aux = getProductListElement(list, index))) {
        aux->element.id = id;
        strcpy(aux->element.name, name);
        aux->element.stock = stock;
    } else 
        puts("Pocisión inválida");
}

void removeProductListElement(productList* list, productListElement* listElement) {
    if(!listElement || isEmptyProductList(list)){
        puts("No existe el elemento");
        return;
    }
    if(listElement->next) listElement->next->prev = listElement->prev;
    if(listElement->prev) listElement->prev->next = listElement->next;
    free(listElement);
}

void removeProductListElementAt(productList* list, unsigned short index){
    if(index >= 0 && index < list->size && !isEmptyProductList(list)) {
        puts("No existe el elemento");
        return;
    }
    productListElement* listElement = list->head;
    unsigned short i;
    for(i = 1; listElement && i < index; i++)
        listElement = listElement->next;
    if(!listElement){
        puts("No existe el elemento");
        return;
    }
    removeProductListElement(list, listElement);
}

void printProductList(productList* list){
    if(isEmptyProductList(list)){
        puts("");
        return;
    }
    productListElement* listElement = list->head;
    printf("\nProductos:\n|Índice\t|ID\t|Nombre\t|Cantidad\t|\n");
    while (listElement->next) {
        printf("|%d\t|%d\t|%s\t|%d\t|\n", listElement->index, listElement->element.id, listElement->element.name, listElement->element.stock);
        listElement = listElement->next;
    }
    printf("|%d\t|%d\t|%s\t|%d\t|\n", listElement->index, listElement->element.id, listElement->element.name, listElement->element.stock);
}

void clearProductList(productList* list) {
    if(!isEmptyProductList(list)){
        productListElement* listElement;
        while ((listElement = list->head)) {
            list->head = list->head->next;
            free(listElement);
        }
        list->head = NULL;
        list->size = 0;
    }
}
*/
