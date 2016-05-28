#pragma once
#ifndef FILE_H
#define FILE_H

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys\stat.h>

#define READONLY "r"
#define RWCLS	 "w"

inline FILE* file_open(const char* path, const char* attr)
{
	FILE* ret = fopen(path, attr);
	if (ret)
	{
		return ret;
	}
	else
	{
		fprintf(stderr, "can't open file %s:%s\n", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

inline size_t file_size(const char* path)
{
	struct stat st;
	if (::stat(path, &st) != 0)
	{
		fprintf(stderr, "can't open file %s:%s\n", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return st.st_size;
}

#endif