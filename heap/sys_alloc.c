#include "sys_alloc.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
void* sys_alloc(void* base, size_t len)
{
	assert(!(len & 0xFFF));
	void* ret = VirtualAlloc(base, len, MEM_COMMIT, PAGE_READWRITE);
	if (base && (ret == NULL))
	{
		fprintf(stderr, "VirtualAlloc fail\n");
		exit(-1);
	}
	return ret;
}

void* sys_try_alloc(void* addr, size_t len)
{
	assert(!(len & 0xFFF));
	return VirtualAlloc(addr, len, MEM_COMMIT, PAGE_READWRITE);
}

void sys_free(void* base, size_t len)
{
	assert(!(len & 0xFFF));
	if (!VirtualFree(base, 0, MEM_RELEASE))
	{
		fprintf(stderr, "VirtualFree fail\n");
		exit(-1);
	}
}

#elif linux

#include <sys/mman.h>

void* sys_alloc(void* base, size_t len)
{
	assert(!(len & 0xFFF));
	void * ret = mmap(base, len, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
	if (base && (ret == NULL))
	{
		fprintf(stderr, "mmap fail\n");
		exit(-1);
	}
	return ret;
}

void* sys_try_alloc(void* addr, size_t len)
{
	assert(!(len & 0xFFF));
	return mmap(base, len, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
}

void sys_free(void* base, size_t len)
{
	assert(!(len & 0xFFF));
	if (munmap(base, len) == -1)
	{
		fprintf(stderr, "munmap fail\n");
		exit(-1);
	}
}

#endif