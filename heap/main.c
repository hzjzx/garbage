#include "mymem.h"
#include "heap.h"

int main()
{
	heap_t heap;
	heap_init(&heap);
	set_heap(&heap);

	void* a;
	a = myalloc(1);
	a = myalloc(2);
	myfree(a);
	a = myalloc(8);
	void* b = myalloc(8);
	myfree(a);
	a = myalloc(13);
	a = myalloc(17);
	a = myalloc(255);
	a = myalloc(1024);
	a = myalloc(1025);
	a = myalloc(1234124214);
	myfree(a);
	a = myalloc(1234567890);
	a = myrealloc(a, 1234567891);
	myfree(a);
	getchar();

	return 0;
}