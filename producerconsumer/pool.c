#include "pool.h"
#include <stddef.h>
#include <stdlib.h>

void pool_init(pool_t* pp)
{
    pp->head = NULL;
    pthread_mutex_init(&pp->mtx_p, NULL);
    pthread_mutex_init(&pp->mtx_c, NULL);
    sem_init(&pp->sem_empty, 0, 0);
    pp->count = 0;
}


void pool_put(pool_t* pp, data_t* pd)
{
     pthread_mutex_lock(&pp->mtx_p);
     
     node_t* tmp = (node_t*)malloc(sizeof(node_t));
     tmp->next = pp->head;
     tmp->data = pd;
     tmp->data->id = pp->count++;
     pp->head = tmp;
     
     pthread_mutex_unlock(&pp->mtx_p);
          
     sem_post(&pp->sem_empty);
}

data_t* pool_take(pool_t* pp)
{
    node_t* tmp = NULL;
    data_t* ret = NULL;
    pthread_mutex_lock(&pp->mtx_c);
    
    while(pp->head == NULL)
    {
        sem_wait(&pp->sem_empty);
    }    
    pthread_mutex_lock(&pp->mtx_p);
    
    tmp = pp->head;
    pp->head = tmp->next;
    
    pthread_mutex_unlock(&pp->mtx_p);
    pthread_mutex_unlock(&pp->mtx_c);  
    
    ret = tmp->data;
    free(tmp);
    return ret;
}
