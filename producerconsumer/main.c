#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "pool.h"

#define NUM_PRODUCER 1
#define NUM_CONSUMER 10

static _Bool go;
static _Bool finish;
static pool_t pool;
pthread_t producer[NUM_PRODUCER];
pthread_t consumer[NUM_CONSUMER];

void* producer_runner(void* i);
void* thread_runner(void* i);

int main(void)
{
    go = false;
    finish = false;
    pool_init(&pool);
    
    for(int i = 0;i < NUM_PRODUCER;i++)
    {
        pthread_create(&producer[i], NULL, &producer_runner, (void*)(size_t)i);
    }
    
    for(int i = 0;i < NUM_CONSUMER;i++)
    {
        pthread_create(&producer[i], NULL, &thread_runner, (void*)(size_t)i);
    }
    
    go = true;
    
    sleep(1);
    
    finish = true;
    
    for(int i = 0;i < NUM_PRODUCER;i++)
    {
        pthread_join(producer[i], NULL);
    }
    
    for(int i = 0;i < NUM_CONSUMER;i++)
    {
        pthread_join(consumer[i], NULL);
    }
    return 0;
}

void* producer_runner(void* i)
{
    size_t pid = (size_t)i;
    while(!go);
    while(!finish)
    {
        data_t* pd = (data_t*)malloc(sizeof(data_t));
        pd->producer = pid;
        pool_put(&pool, pd);
        printf("produce by %ld\n", pid);
    }
    return NULL;
}

void* thread_runner(void* i)
{
    size_t cid = (size_t)i;
    while(!go);
    while(!finish)
    {
        data_t* pd = pool_take(&pool);
        printf("producer: %ld, consumer: %ld, product id: %ld\n", pd->producer, cid, pd->id);
    }
    return NULL;
}