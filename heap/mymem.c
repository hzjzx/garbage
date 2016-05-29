#include "mymem.h"
#include "sys_alloc.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define ALIGN8(exp) (exp = (exp + 0x7) & 0xFFFFFFF8)
#define ALIGN4096(exp) (exp = (exp + 0xFFF) & 0xFFFFF000)

static heap_t* heap;

static size_t indsiz[MEM_SIZ_LARGE] = { 8,16,32,64,128,256,512,1024,2048 };

static node_t* nodepage_init(page_t* p)
{
	memset(p, 0, sizeof(page_t));
	for (size_t i = 0;i < NODEPERPAGE - 1;i++)
	{
		p->node[i].next = &p->node[i + 1];
	}
	return &p->node[0];
}

static void append_node()
{
	page_t* pp = heap->ppage;
	while (pp->next)
	{
		pp = pp->next;
	}
	pp->next = (page_t*)sys_alloc(NULL, sizeof(page_t));

	node_t* pn = heap->free_node;
	while (pn->next)
	{
		pn = pn->next;
	}
	pn->next = nodepage_init(pp->next);
}

static node_t* get_free_node()
{
	if (!heap->free_node)
	{
		append_node();
	}
	node_t* ret = heap->free_node;
	heap->free_node = ret->next;
	return ret;
}

static size_t ind(size_t len)
{
	if (len <= 8)
	{
		return MEM_SIZ_8;
	}
	else if (len <= 16)
	{
		return MEM_SIZ_16;
	}
	else if (len <= 32)
	{
		return MEM_SIZ_32;
	}
	else if (len <= 64)
	{
		return MEM_SIZ_64;
	}
	else if (len <= 128)
	{
		return MEM_SIZ_128;
	}
	else if (len <= 256)
	{
		return MEM_SIZ_256;
	}
	else if (len <= 512)
	{
		return MEM_SIZ_512;
	}
	else if (len <= 1024)
	{
		return MEM_SIZ_1024;
	}
	else if (len <= 2048)
	{
		return MEM_SIZ_2048;
	}
	else
	{
		return MEM_SIZ_LARGE;
	}
}

static void expand_heap(size_t i)
{
	void* mem = sys_alloc(NULL, 4096);
	node_t** ppn = &heap->index[i];

	while (*ppn)
	{
		if ((*ppn)->addr > mem)
		{
			break;
		}
		ppn = &(*ppn)->next;
	}

	node_t* tmp = get_free_node();
	tmp->addr = mem;
	tmp->len = 4096;
	tmp->next = *ppn;
	tmp->stat = MEM_STAT_FREE;
	*ppn = tmp;
}

static void merge(node_t* pn)
{
	while (pn && pn->next)
	{
		if ((pn->stat == pn->next->stat) &&
		   ((size_t)pn->addr + pn->len) == (size_t)pn->next->addr)
		{
			node_t* tmp = pn->next;
			pn->len += pn->next->len;
			pn->next = tmp->next;
			tmp->next = heap->free_node;
			heap->free_node = tmp;
		}
		else
		{
			pn = pn->next;
		}
	}
}

static void* do_alloc(size_t i)
{
	node_t* pn = heap->index[i];
	node_t* prev = NULL;

	while (pn)
	{
		if (pn->stat == MEM_STAT_FREE)
		{
			if (prev)
			{
				void* ret = pn->addr;
				prev->len += indsiz[i];
				pn->addr = (void*)((size_t)pn->addr + indsiz[i]);
				pn->len -= indsiz[i];
				if (pn->len == 0)
				{
					prev->next = pn->next;
					pn->next = heap->free_node;
					heap->free_node = pn;
					merge(prev);
				}
				return ret;
			}
			else
			{
				node_t* tmp = get_free_node();
				tmp->addr = pn->addr;
				tmp->len = indsiz[i];
				tmp->next = pn;
				tmp->stat = MEM_STAT_USED;
				pn->addr = (void*)((size_t)pn->addr + indsiz[i]);
				pn->len -= indsiz[i];
				heap->index[i] = tmp;
				if (pn->len == 0)
				{
					tmp->next = pn->next;
					pn->next = heap->free_node;
					heap->free_node = pn;
					merge(tmp);
				}
				return tmp->addr;
			}
		}
		else if (pn->next)
		{
			prev = pn;
			pn = pn->next;
		}
		else
		{
			expand_heap(i);
			return do_alloc(i);
		}
	}
	expand_heap(i);
	return do_alloc(i);
}

static void* do_alloc_large(size_t len)
{
	node_t** ppn = &heap->index[MEM_SIZ_LARGE];
	ALIGN4096(len);
	void* mem = sys_alloc(NULL, len);
	
	while (*ppn)
	{
		if ((*ppn)->next && ((*ppn)->next->addr > mem))
		{
			node_t* tmp = get_free_node();
			tmp->next = (*ppn)->next;
			tmp->addr = mem;
			tmp->len = len;
			tmp->stat = MEM_STAT_USED;
			(*ppn)->next = tmp;
			return mem;
		}
		ppn = &(*ppn)->next;
	}
	*ppn = get_free_node();
	(*ppn)->addr = mem;
	(*ppn)->len = len;
	(*ppn)->next = NULL;
	(*ppn)->stat = MEM_STAT_USED;
	return mem;
}

void heap_init(heap_t* heap)
{
	assert(sizeof(void*) == 8);
	assert(sizeof(page_t) == 4096);

	memset(heap, 0, sizeof(heap_t));
	heap->ppage = (page_t*)sys_alloc(NULL, sizeof(page_t));
	heap->free_node = nodepage_init(heap->ppage);
}

void set_heap(heap_t* _heap)
{
	heap = _heap;
}

static int addr_find(void* addr, node_t** _prev, node_t** _pn, size_t* _i)
{
	size_t i = 0;
	for (i = 0;i < MEM_SIZ_LARGE;i++)
	{
		if (heap->index[i] && (heap->index[i]->addr <= addr))
		{
			node_t* prev = NULL;
			node_t* pn = heap->index[i];
			while (pn)
			{
				if ((pn->addr <= addr) && ((void*)((size_t)pn->addr + pn->len) >= addr))
				{
					if ((size_t)addr % indsiz[i] == 0)
					{
						*_prev = prev;
						*_pn = pn;
						*_i = i;
						return 1;
					}
					else
					{
						return 0;
					}
				}

				prev = pn;
				pn = pn->next;
			}
		}
	}

	if (heap->index[i] && (heap->index[i]->addr <= addr))
	{
		node_t* prev = NULL;
		node_t* pn = heap->index[i];
		while (pn)
		{
			if (pn->addr == addr)
			{
				*_prev = prev;
				*_pn = pn;
				*_i = MEM_SIZ_LARGE;
				return 1;
			}

			prev = pn;
			pn = pn->next;
		}
	}

	return 0;
}

void* myalloc(size_t len)
{
	ALIGN8(len);
	size_t i = ind(len);
	if (i != MEM_SIZ_LARGE)
	{
		return do_alloc(i);
	}
	else
	{
		return do_alloc_large(len);
	}
}

void* myrealloc(void* addr, size_t len)
{
	size_t i1 = ind(len);
	size_t i2 = MEM_SIZ_LARGE;
	node_t* prev = NULL;
	node_t* pn = NULL;
	if (addr_find(addr, &prev, &pn, &i2))
	{
		if (i2 != MEM_SIZ_LARGE)
		{
			if (i1 == i2)
			{
				return addr;
			}
			else
			{
				void* ret = myalloc(len);
				memcpy(ret, addr, (pn->len < len) ? pn->len : len);
				myfree(addr);
				return ret;
			}
		}
		else
		{
			if (i1 == i2)
			{
				ALIGN4096(len);
				if (len <= pn->len)
				{
					return pn->addr;
				}
				else
				{
					size_t delta = len - pn->len;
					void* p = sys_try_alloc((void*)((size_t)pn->addr + pn->len), delta);
					if (p)
					{
						pn->len = len;
						return pn->addr;
					}
					else
					{
						void* src = pn->addr;
						pn->addr = sys_alloc(NULL, len);
						memcpy(pn->addr, src, pn->len);
						sys_free(src, pn->len);
						pn->len = len;
						return pn->addr;
					}
				}
			}
			else
			{
				void* ret = myalloc(len);
				memcpy(ret, addr, len);
				myfree(addr);
				return ret;
			}
		}
	}
	return NULL;
}

void myfree(void* addr)
{
	size_t i = MEM_SIZ_LARGE;
	node_t* prev = NULL;
	node_t* pn = NULL;
	if (addr_find(addr, &prev, &pn, &i))
	{
		if (i != MEM_SIZ_LARGE)
		{
			node_t* free = get_free_node();
			node_t* used = get_free_node();
			used->len = pn->len - ((size_t)addr - (size_t)pn->addr) - indsiz[i];
			pn->len = (size_t)addr - (size_t)pn->addr;
			used->next = pn->next;
			free->next = used;
			pn->next = free;

			free->addr = addr;
			free->len = indsiz[i];
			free->stat = MEM_STAT_FREE;

			used->addr = (void*)((size_t)free->addr + indsiz[i]);
			used->stat = MEM_STAT_USED;

			if (pn->len == 0)
			{
				if (prev)
				{
					prev->next = pn->next;
					pn->next = heap->free_node;
					heap->free_node = pn;
					merge(prev);
				}
				else
				{
					heap->index[i] = pn->next;
					pn->next = heap->free_node;
					heap->free_node = pn;
				}
			}

			if (used->len == 0)
			{
				free->next = used->next;
				used->next = heap->free_node;
				heap->free_node = used;
				merge(free);
			}
		}
		else if(prev)
		{
			prev->next = pn->next;
			sys_free(pn->addr, pn->len);
			pn->next = heap->free_node;
			heap->free_node = pn;
		}
		else
		{
			heap->index[MEM_SIZ_LARGE];
			sys_free(pn->addr, pn->len);
			pn->next = heap->free_node;
			heap->free_node = pn;
		}
	}
	else
	{
		fprintf(stderr, "illegal free\n");
	}
}