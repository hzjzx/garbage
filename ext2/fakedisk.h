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
		size_t size;
		ext2::DirectoryEntry dirEntry;
	};

	enum class FilePos
	{
		Begin,
		Current,
		End
	};

private:
	FILE* m_pFile;
	ext2::SuperBlock m_superBlock;
	std::vector<ext2::BlockGroupDescriptor> m_groupDescriptor;
	size_t m_blockSize;

private:
	static std::vector<std::string> parse_path(const std::string& path);
	inline void read_inode(size_t inode, ext2::InodeData* pInode);
	inline void write_inode(size_t inode, ext2::InodeData* pInode);
	void read_block_singly(uint32_t* dst, size_t blkid, size_t total);
	void read_block_doubly(uint32_t* dst, size_t blkid, size_t total);
	void read_block_triply(uint32_t* dst, size_t blkid, size_t total);
	std::vector<uint32_t> alloc_block(size_t n);
	void write_block(size_t block, void* buf, size_t offset, size_t len);
	size_t alloc_inode(ext2::DirectoryEntry::Type type);
	void delete_inode(size_t inode);
	void delete_block_singly(size_t blkid);

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
	File* create(const std::string& path, ext2::DirectoryEntry::Type type);
	int32_t file_seek(File* pFile, int32_t offset, FilePos from);
	size_t write_file(File* pFile, void* buf, size_t len);
	int32_t remove(char* path);
};

#endif