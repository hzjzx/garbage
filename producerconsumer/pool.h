#include <pthread.h>
#include <semaphore.h>

typedef struct data_struct
{
    size_t producer;
    size_t id;
} data_t;

typedef struct node_struct
{
    data_t* data;
    struct node_struct* next;
} node_t;

typedef struct pool_struct
{
    size_t count;
    sem_t sem_empty;
    pthread_mutex_t mtx_p;
    pthread_mutex_t mtx_c;
    node_t* head;
} pool_t;

void pool_init(pool_t* pp);
void pool_put(pool_t* pp, data_t* pd);
data_t* pool_take(pool_t* pp);
