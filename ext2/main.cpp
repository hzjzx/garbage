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
	FakeDisk::File* pf = fd.open("/file2");

	if (pf)
	{
		printf("found\n");
		delete pf;
	}
	else
	{
		printf("file not found\n");
	}

	FakeDisk::File* p = fd.create("/file2", ext2::DirectoryEntry::Type::RegularFile);
	FakeDisk::File* d = fd.create("/dir2", ext2::DirectoryEntry::Type::Directory);
	auto a = fd.remove("/file2");
	return 0;                                                
}