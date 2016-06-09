#include "fakedisk.h"
#include <cmath>
#include <cstring>

std::vector<std::string> FakeDisk::parse_path(const std::string& path)
{
	std::vector<std::string> ret;
	std::string::const_iterator iter1 = path.begin(), iter2 = path.begin();

	while (iter2 != path.end())
	{
		if (*iter2 == '/')
		{
			if (iter1 != iter2)
			{
				ret.emplace_back(iter1, iter2);
			}
			iter1 = iter2 + 1;
		}
		iter2++;
	}
	if (iter1 != iter2)
	{
		ret.emplace_back(iter1, iter2);
	}
	return std::move(ret);
}

inline void FakeDisk::read_inode(size_t inode, ext2::InodeData* pInode)
{
	size_t gid = (inode - 1) / m_superBlock.inodePerGroup;
	size_t iiding = (inode - 1) % m_superBlock.inodePerGroup;
	fseek(m_pFile, m_groupDescriptor[gid].inodeTableAddr * m_blockSize, SEEK_SET);
	fseek(m_pFile, iiding * sizeof(ext2::InodeData), SEEK_CUR);
	fread(pInode, sizeof(ext2::InodeData), 1, m_pFile);
}

void FakeDisk::read_block_singly(uint32_t* dst, size_t blkid, size_t total)
{
	read_block(blkid, dst, 0, total*sizeof(uint32_t));
}

void FakeDisk::read_block_doubly(uint32_t* dst, size_t blkid, size_t total)
{
	std::vector<uint32_t> block;
	size_t ppblk = m_blockSize / sizeof(uint32_t);
	block.resize(total * sizeof(uint32_t) / m_blockSize);
	read_block(blkid, &block[0], 0, block.size() * sizeof(uint32_t));
	for (uint32_t i : block)
	{
		size_t toread = (total > ppblk) ? ppblk : total;
		read_block_singly(dst, i, toread);
		dst = reinterpret_cast<uint32_t*>(reinterpret_cast<size_t>(dst) + toread * sizeof(uint32_t));
		total -= toread;
	}
}

void FakeDisk::read_block_triply(uint32_t * dst, size_t blkid, size_t total)
{
	std::vector<uint32_t> block;
	size_t ppblk = m_blockSize / sizeof(uint32_t);
	block.resize(total * sizeof(uint32_t) / m_blockSize);
	read_block(blkid, &block[0], 0, block.size() * sizeof(uint32_t));
	for (uint32_t i : block)
	{
		size_t toread = (total > ppblk * ppblk) ? ppblk * ppblk : total;
		read_block_doubly(dst, i, toread);
		dst = reinterpret_cast<uint32_t*>(reinterpret_cast<size_t>(dst) + toread * sizeof(uint32_t));
		total -= toread;
	}
}

FakeDisk::FakeDisk(char* path)
{
	m_pFile = fopen(path, "rb+");
	if (m_pFile == nullptr)
	{
		perror("open disk error");
		exit(-1);
	}

	fseek(m_pFile, 1024, SEEK_SET);
	fread(&m_superBlock, sizeof(m_superBlock), 1, m_pFile);
	if (m_superBlock.magic != 0xef53)
	{
		fprintf(stderr, "can not find ext2 file system\n");
		exit(-1);
	}
	m_blockSize = static_cast<size_t>(pow(2, m_superBlock.blockSize + 10));
	size_t ng = m_superBlock.blockTotal / m_superBlock.blockPerGroup;
	m_groupDescriptor.resize(ng);
 
	if (m_superBlock.blockSize == 0)
	{
		fseek(m_pFile, 2048, SEEK_SET);
	}
	else
	{
		fseek(m_pFile, m_blockSize, SEEK_SET);
	}
	fread(&m_groupDescriptor[0], sizeof(ext2::BlockGroupDescriptor), ng, m_pFile);
	
}

FakeDisk::~FakeDisk()
{
	fclose(m_pFile);
}

FakeDisk::File* FakeDisk::open(const std::string& path)
{
	std::vector<std::string> vec = parse_path(path);
	std::vector<std::string>::iterator iterName = vec.begin();
	File* root = open_inode(2);
	if (iterName == vec.end())
	{
		return root;
	}
	else
	{
		File* ret = open(root, iterName, vec);
		delete root;
		return ret;
	}

	return nullptr;
}

FakeDisk::File* FakeDisk::open(File* pFile, std::vector<std::string>::const_iterator iter, const std::vector<std::string>& vec)
{
	if ((pFile->inode.mode & 0xF000) != 0x4000)
	{
		fprintf(stderr, "%s is not a directory\n", iter->c_str());
		return nullptr;
	}

	while (true)
	{
		ext2::DirectoryEntry dir;
		size_t off = pFile->offset;
		size_t a = read_file(pFile, &dir, 8);
		if (dir.inode == 0)
		{
			return nullptr;
		}
		else
		{
			dir.name.resize(dir.nameLen);
			read_file(pFile, &dir.name[0], dir.nameLen);
			if (dir.name == *iter)
			{
				iter++;
				if (iter == vec.end())
				{
					File* ret = open_inode(dir.inode);
					ret->name = *(--iter);
					return ret;
				}
				else
				{
					File* pdir = open_inode(dir.inode);
					File* ret = open(pdir, iter, vec);
					delete pdir;
					return ret;
				}
			}
			else
			{
				pFile->offset = off + dir.size;
			}
		}
	}
	return nullptr;
}

FakeDisk::File* FakeDisk::open_inode(size_t inode)
{
	File* ret = new File;
	read_inode(inode, &ret->inode);
	ret->block.resize(ret->inode.sizeLow / m_blockSize + (((ret->inode.sizeLow % m_blockSize) > 0) ? 1 : 0));
	ret->offset = 0;
	ret->size = ret->inode.sizeLow;
	
	if (ret->block.size() != 0)
	{
		size_t len = sizeof(uint32_t)*((ret->block.size() < 12) ? ret->block.size() : 12);
		memcpy(&ret->block[0], ret->inode.blockDirect, len);
	}

	size_t bppb = m_blockSize / sizeof(uint32_t);

	if (ret->block.size() > 12)
	{
		read_block_singly(&ret->block[12], ret->inode.blockIndirectSingly, (ret->block.size() > bppb + 12) ? bppb : ret->block.size() - 12);
	}

	if (ret->block.size() >  bppb + 12)
	{
		read_block_doubly(&ret->block[bppb + 12], ret->inode.blockIndirectDoubly, (ret->block.size() > bppb * bppb + bppb + 12) ? bppb : ret->block.size() - bppb - 12);
	}

	if (ret->block.size() >  bppb * bppb + bppb + 12)
	{
		read_block_triply(&ret->block[bppb * bppb + bppb + 12], ret->inode.blockIndirectTriply, ret->block.size() - bppb * bppb - bppb - 12);
	}

	return ret;
}

size_t FakeDisk::read_file(File* pFile, void* buf, size_t len)
{
	size_t ret;
	if (pFile->offset + len > pFile->inode.sizeLow)
	{
		len = pFile->inode.sizeLow - pFile->offset;
	}
	ret = len;
	while (len > 0)
	{
		uint32_t blkid = pFile->block[(pFile->offset / m_blockSize)];
		uint32_t blkoff = pFile->offset % m_blockSize;
		uint32_t blklen = (blkoff + len > m_blockSize) ? m_blockSize - blkoff : len;
		read_block(blkid, buf, blkoff, blklen);
		pFile->offset += blklen;
		len -= blklen;
	}
	return ret;
}

void FakeDisk::read_block(size_t block, void* buf, size_t offset, size_t len)
{
	fseek(m_pFile, block * m_blockSize + offset, SEEK_SET);
	fread(buf, len, 1, m_pFile);
}

void FakeDisk::file_seek(File * pFile, size_t pos, int from)
{

}
