#include "buddy.h"
#include <stdio.h>
#include <stdlib.h>

static int mem_size[] = { 8,16,32,64,128,256,512 };

static int size_parse(size_t size)
{
	switch (size)
	{
	case 8:		return MEM_SIZ_8;
	case 16:	return MEM_SIZ_16;
	case 32:	return MEM_SIZ_32;
	case 64:	return MEM_SIZ_64;
	case 128:	return MEM_SIZ_128;
	case 256:	return MEM_SIZ_256;
	case 512:	return MEM_SIZ_512;
	default:	return -1;
	}
}

static void list_insert(buddy_t* pb, size_t n, node_t* pn)
{
	node_t** ppn = &pb->node[n];
	while (*ppn)
	{
		if ((*ppn)->addr > pn->addr)
		{
			pn->next = *ppn;
			*ppn = pn;
			return;
		}
	}
}

static void check_merge(buddy_t* pb, size_t n)
{
	if (n > MEM_SIZ_512)
	{
		return;
	}

	node_t** ppn = &pb->node[n];
	while (*ppn)
	{
		node_t* next = (*ppn)->next;
		if (next)
		{
			size_t addr1 = (size_t)(*ppn)->addr;
			size_t addr2 = (size_t)next->addr;
			if ((addr1 + mem_size[n] == addr2) &&
				((*ppn)->stat == MEM_FREE) &&
				(next->stat == MEM_FREE) &&
				(addr1 % mem_size[n + 1] == 0)
				)
			{
				node_t* nxt = next->next;
				free((*ppn)->next);
				node_t** ppn2 = &pb->node[n + 1];
				while (*ppn2)
				{
					if ((*ppn2)->addr > (*ppn)->addr)
					{
						(*ppn)->next = *ppn2;
						(*ppn2) = *ppn;
						break;
					}
					ppn2 = &(*ppn2)->next;
				}
				if (!(*ppn2))
				{
					*ppn2 = *ppn;
					(*ppn2)->next = NULL;
					*ppn = NULL;
				}
				*ppn = nxt;
			}
			else
			{
				ppn = &(*ppn)->next;
			}
		}
		else
		{
			break;
		}
	}
	check_merge(pb, n + 1);
}

static node_t** do_alloc(buddy_t* pb, size_t n)
{
	if (n > MEM_SIZ_512)
	{
		return NULL;
	}
	node_t** ppn = &pb->node[n];
	while (*ppn)
	{
		if ((*ppn)->stat == MEM_FREE)
		{
			(*ppn)->stat = MEM_USED;
			return ppn;
		}
		ppn = &(*ppn)->next;
	}

	node_t** fm = do_alloc(pb, n + 1);
	if (fm == NULL)
	{
		return NULL;
	}
	else
	{
		node_t* newnode1 = *fm;
		(*fm) = (*fm)->next;
		ppn = &pb->node[n];
		while (*ppn)
		{
			if ((*ppn)->addr > newnode1->addr)
			{
				newnode1->next = *ppn;
				*ppn = (node_t*)malloc(sizeof(node_t));
				(*ppn)->next = newnode1;
				(*ppn)->addr = newnode1->addr;
				(*ppn)->stat = MEM_USED;
				newnode1->addr = (void*)((size_t)newnode1->addr + mem_size[n]);
				return ppn;
			}
			ppn = &(*ppn)->next;
		}
		if (!(*ppn))
		{
			newnode1->next = *ppn;
			*ppn = (node_t*)malloc(sizeof(node_t));
			(*ppn)->next = newnode1;
			(*ppn)->addr = newnode1->addr;
			(*ppn)->stat = MEM_USED;
			(*ppn)->next->stat = MEM_FREE;
			newnode1->addr = (void*)((size_t)newnode1->addr + mem_size[n]);
			return ppn;
		}
	}
	return NULL;
}

void buddy_init(buddy_t* pb)
{
	for (int i = 0;i < 7;i++)
	{
		pb->node[i] = NULL;
	}
	pb->node[MEM_SIZ_512] = (node_t*)malloc(sizeof(node_t));
	pb->node[MEM_SIZ_512]->addr = 0;
	pb->node[MEM_SIZ_512]->next = NULL;
	pb->node[MEM_SIZ_512]->stat = MEM_FREE;
}

void* buddy_alloc(buddy_t* pb, size_t size)
{
	int ind = size_parse(size);
	if (ind == -1)
	{
		fprintf(stderr, "wrong alloc size: %d\n", size);
		return NULL;
	}

	node_t** ppn = do_alloc(pb, ind);
	if (!ppn)
	{
		fprintf(stderr, "not enough free space\n");
		return NULL;
	}
	else
	{
		return (*ppn)->addr;
	}
}

void buddy_free(buddy_t* pb, void* addr)
{
	for (int i = 0;i < 7;i++)
	{
		node_t** ppn = &pb->node[i];
		while (*ppn)
		{
			if ((*ppn)->addr == addr)
			{
				(*ppn)->stat = MEM_FREE;
				check_merge(pb, i);
				return;
			}
			ppn = &(*ppn)->next;
		}
	}
	fprintf(stderr, "buddy_free error, invalid address: %0#8x\n", (size_t)addr);
}

void buddy_print(buddy_t* pb)
{
	printf("********\n");
	for (int i = 0;i < 7;i++)
	{
		printf("[%3d] ", mem_size[i]);
		node_t* pn = pb->node[i];
		while (pn)
		{
			char* stat = (pn->stat == MEM_FREE) ? "free" : "used";
			printf("-> %0#8x(%s) ", (size_t)pn->addr, stat);
			pn = pn->next;
		}
		printf("\n");
	}
}