#include <map>
#include <string>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include "fakedisk.h"

std::map<std::string, std::function<void(std::string)>> opt =
{

};

void usage()
{
	printf(
		"usage: disk <action> <path>\n"
		"action:"
		"\tfind\n"
		"\tcreate\n"
		"\tdelete\n"
		);
}

int main(int argc, char** argv)
{
	FakeDisk fd("test.img");
	FakeDisk::File* pf = fd.open("/dir1/dir11/file111");

	if (pf)
	{
		printf("found\n");
		delete pf;
	}
	else
	{
		printf("file not found\n");
	}

	return 0; 
}