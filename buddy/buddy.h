#pragma once
#ifndef BUDDY_H
#define BUDDY_H

#include <stddef.h>

#define MEM_SIZ_8		0
#define MEM_SIZ_16		1
#define MEM_SIZ_32		2
#define MEM_SIZ_64		3
#define MEM_SIZ_128		4
#define MEM_SIZ_256		5
#define MEM_SIZ_512		6

#define MEM_FREE 0
#define MEM_USED 1

typedef struct node_struct
{
	struct node_struct* next;
	void* addr;
	int stat;
} node_t;

typedef struct buddy_struct
{
	node_t* node[7];
} buddy_t;

void buddy_init(buddy_t* pb);
void* buddy_alloc(buddy_t* pb, size_t size);
void buddy_free(buddy_t* pb, void* addr);
void buddy_print(buddy_t* pb);

#endif