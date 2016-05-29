#pragma once
#ifndef SYSALLOC_H
#define SYSALLOC_H

#include <stddef.h>

void* sys_alloc(void* addr, size_t len);
void* sys_try_alloc(void* addr, size_t len);
void sys_free(void* addr, size_t len);

#endif