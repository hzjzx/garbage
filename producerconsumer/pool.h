#pragma once
#ifndef POOL_H
#define POOL_H

#include <pthread.h>

typedef struct node_struct
{
    struct queue_struct* next;
    int data;
} node_t;

typedef struct pool_struct
{
    node_t* head;
    pthread_mutex_t mtxp;
    pthread_mutex_t mtxc;
    pthread_cond_t cond;
} pool_t;

void pool_init(pool_t* pp);
void pool_put(pool_t* pp, int data);
void pool_take(pool_t* pp);

#endif