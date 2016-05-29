#include "pool.h"
#include <stdlib.h>

void pool_init(pool_t* pp)
{
    pp->head = NULL;
    pthread_mutex_init(&pp->mtxc, NULL);  
    pthread_mutex_init(&pp->mtxp, NULL); 
    pp->cond = PTHREAD_COND_INITIALIZER;
}

void pool_put(pool_t* pp, int data)
{
    pthread_mutex_lock(&pp->mtxp);
    node_t** ppn = &pp->head;
    while(*ppn)
    {
        ppn = &(*ppn)->next;
    }
    *ppn = (node_t*)malloc(sizeof(node_t));
    (*ppn)->next = NULL;
    (*ppn)->data = data;
    pthread_mutex_unlock(&pp->mtxp);
    pthread_cond_signal(&pp->cond);
}

void pool_take(pool_t* pp)
{
    node_t* pn = NULL;
    
    pthread_mutex_lock(&pp->mtxc);
    while(!pp->head)
    {
        pthread_cond_wait(&pp->cond, &pp->mtxc);
    }
    pthread_mutex_lock(&pp->mtxp);
    
    pn = pp->head;
    pp->head = pp->head->next;
    
    pthread_mutex_unlock(&pp->mtxp);
    pthread_mutex_unlock(&pp->mtxc);
    
    printf("take %d\n", pn->data);
    free(pn);
}