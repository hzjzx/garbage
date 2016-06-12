#include "buddy.h"
#include <stdlib.h>

int main()
{
	buddy_t buddy;
	buddy_init(&buddy);

	void* p1 = buddy_alloc(&buddy, 128);
	buddy_print(&buddy);
	void* p2 = buddy_alloc(&buddy, 128);
	buddy_print(&buddy);
	void* p3 = buddy_alloc(&buddy, 128);
	buddy_print(&buddy);
	void* p4 = buddy_alloc(&buddy, 128);
	buddy_print(&buddy);
	void* p5 = buddy_alloc(&buddy, 128);
	buddy_print(&buddy);

	buddy_free(&buddy, p2);
	buddy_print(&buddy);
	buddy_free(&buddy, p3);

	buddy_print(&buddy);

	buddy_free(&buddy, p4);
	buddy_print(&buddy);

	buddy_free(&buddy, p1);
	buddy_print(&buddy);

	system("pause");
	return 0;
}