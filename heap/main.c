#include "mymem.h"
#include <stdlib.h>
#include <stdio.h>
#include "heap.h"

int rand89() 
{
	return rand() % 890000;
}

void check(heap_t* heap)
{
	for (int i = 0;i < MEM_SIZ_LARGE;i++)
	{
		node_t* pn = heap->index[i];
		
		while (pn)
		{
			if (pn->len == 0)
			{
				print_heap(heap);
				printf("err\n");
			}
			pn = pn->next;
		}

	}
}

void test1(heap_t* heap)
{
	#define N 1000

	void* vps[N];
	//void **vps = malloc(sizeof(void *) * N);

	for (int i = 0; i < N; i++)
	{
		vps[i] = myalloc(rand89());
		check(heap);
	}

	for (int i = 0; i < N; i++)
	{
		if (vps[i] == NULL) {
			printf("error\n");
		}
		myfree(vps[i]);

		for (int j = 0;j < MEM_SIZ_LARGE;j++)
		{
			node_t* pn = heap->index[j];

			while (pn)
			{
				if (pn->len == 0)
				{
					print_heap(heap);
					printf("err\n");
				}
				pn = pn->next;
			}

		}

		check(heap);
	}
}

void test2(heap_t* heap)
{
	size_t size = 2048;
	#define N2 4
	void* p[N2];
	for (int i = 0;i < N2;i++)
	{
		p[i] = myalloc(size);
		print_heap(heap);
		//printf("%16zx\n", p[i]);
	}
	for (int i = 0;i < N2; i++)
	{
		myfree(p[i]);
		print_heap(heap);
		//printf("%16zx\n", p[i]);
	}
}

void test0() {
	int Y = 100;
	void **vps = malloc(sizeof(void *) * Y);
	for (int i = 0; i < Y; i++) {
		vps[i] = myalloc(rand89() + 20000000);
		char *p = vps[i];
		for (int i = 0; i < 20; i++) {
			p[i] = 90;
		}
	}
	for (int i = 0; i < Y; i++) {
		myfree(vps[i]);
	}
}

int main()
{
	heap_t heap;
	heap_init(&heap);
	set_heap(&heap);

	//test1(&heap);

	//test0();

	test2(&heap);

	system("pause");

	return 0;
}