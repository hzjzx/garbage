#pragma once
#ifndef MYMEM_H
#define MYMEM_H

#include <stddef.h>
#include "heap.h"

void heap_init(heap_t* heap);

void set_heap(heap_t* _heap);

void* myalloc(size_t len);
void* myrealloc(void* addr, size_t len);
void myfree(void* addr);
void print_heap(heap_t* heap);

#endif