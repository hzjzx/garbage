#pragma once
#ifndef FAKEDISK_H
#define FAKEDISK_H

#include <cstdio>
#include <vector>
#include <string>
#include "ext2.h"

class FakeDisk
{
public:
	struct File
	{
		std::string name;
		size_t offset;
		ext2::InodeData inode;
		std::vector<uint32_t> block;
		uint64_t size;
	};

private:
	FILE* m_pFile;
	ext2::SuperBlock m_superBlock;
	std::vector<ext2::BlockGroupDescriptor> m_groupDescriptor;
	size_t m_blockSize;

private:
	static std::vector<std::string> parse_path(const std::string& path);
	inline void read_inode(size_t inode, ext2::InodeData* pInode);
	void read_block_singly(uint32_t* dst, size_t blkid, size_t total);
	void read_block_doubly(uint32_t* dst, size_t blkid, size_t total);
	void read_block_triply(uint32_t* dst, size_t blkid, size_t total);

public:
	FakeDisk() = delete;
	FakeDisk(const FakeDisk& fd) = delete;
	explicit FakeDisk(char* path);
	~FakeDisk();

	File* open(const std::string& path);
	File* open(File* pFile, std::vector<std::string>::const_iterator iter, const std::vector<std::string>& vec);
	File* open_inode(size_t inode);
	size_t read_file(File* pFile, void* buf, size_t len);
	void read_block(size_t block, void* buf, size_t offset, size_t len);
	void file_seek(File* pFile, size_t pos, int from);
	
};

#endif