#pragma once
#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

#define MEM_SIZ_8		0
#define MEM_SIZ_16		1
#define MEM_SIZ_32		2
#define MEM_SIZ_64		3
#define MEM_SIZ_128		4
#define MEM_SIZ_256		5
#define MEM_SIZ_512		6
#define MEM_SIZ_1024	7
#define MEM_SIZ_2048	8
#define MEM_SIZ_LARGE	9

#define MEM_STAT_FREE	0	
#define MEM_STAT_USED	1

#define NODEPERPAGE		146

#pragma pack(push,1)
typedef struct node_struct
{
	struct node_struct* next;
	void* addr;
	size_t len;
	uint8_t stat;
	uint8_t padding[3];
} node_t, *p_node_t;

typedef struct page_struct
{
	struct page_struct* next;
	node_t node[NODEPERPAGE];
} page_t, *p_page_t;

typedef struct heap_struct
{
	p_node_t index[MEM_SIZ_LARGE + 1];
	p_node_t free_node;
	p_page_t ppage;
} heap_t, *p_heap_t;

#pragma pack(pop)

#endif