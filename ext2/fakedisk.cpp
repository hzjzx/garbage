#include "fakedisk.h"
#include <cmath>
#include <cstring>
#include <ctime>
#include "bitmap.h"

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

inline void FakeDisk::write_inode(size_t inode, ext2::InodeData * pInode)
{
	size_t gid = (inode - 1) / m_superBlock.inodePerGroup;
	size_t iiding = (inode - 1) % m_superBlock.inodePerGroup;
	fseek(m_pFile, m_groupDescriptor[gid].inodeTableAddr * m_blockSize, SEEK_SET);
	fseek(m_pFile, iiding * sizeof(ext2::InodeData), SEEK_CUR);
	fwrite(pInode, sizeof(ext2::InodeData), 1, m_pFile);
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

std::vector<uint32_t> FakeDisk::alloc_block(size_t n)
{
	std::vector<uint32_t> ret;
	if (n == 0)
	{
		return ret;
	}
	size_t blockbase = 0;
	for (const ext2::BlockGroupDescriptor& group : m_groupDescriptor)
	{
		if (group.blockFree == 0)
		{
			continue;
		}
		Bitmap blkmap(m_superBlock.blockPerGroup);
		read_block(group.blockBitmapAddr, blkmap.c_ptr(), 0, m_blockSize);
		for (size_t i = 0;i < m_superBlock.blockPerGroup;i++)
		{
			if (blkmap.get(i) == 0)
			{
				blkmap.set(i, 1);
				ret.push_back(blockbase + i);
			}
			if (ret.size() == n)
			{
				return ret;
			}
		}
		write_block(group.blockBitmapAddr, blkmap.c_ptr(), 0, m_blockSize);
		blockbase += m_blockSize * m_superBlock.blockPerGroup;
	}
	return ret;
}

void FakeDisk::write_block(size_t block, void * buf, size_t offset, size_t len)
{
	fseek(m_pFile, block * m_blockSize + offset, SEEK_SET);
	fwrite(buf, len, 1, m_pFile);
}

size_t FakeDisk::alloc_inode(ext2::DirectoryEntry::Type type)
{
/*
0x1000  FIFO  
0x2000  Character device  
0x4000  Directory  
0x6000  Block device  
0x8000  Regular file  
0xA000  Symbolic link  
0xC000  Unix socket  
*/
	static uint32_t mode_type[8] = { 0x0000,0x8000,0x4000,0x2000,0x6000,0x1000,0xC000,0xA000 };
	size_t ret = 0;
	time_t time = ::time(0);
	ext2::InodeData inode;
	memset(&inode, 0, sizeof(inode));
	inode.timeAccess = static_cast<uint32_t>(time);
	inode.timeCreate = static_cast<uint32_t>(time);
	inode.timeModify = static_cast<uint32_t>(time);
	inode.mode |= mode_type[static_cast<uint8_t>(type)];
	inode.mode |= 0x01A4;
	inode.gengeration = 0x271F35CB;
	inode.osspec = 1;
	inode.hardLink = 1;
	size_t gid = 0;
	for (auto& group : m_groupDescriptor)
	{
		if (group.inodeFree == 0)
		{
			continue;
		}

		Bitmap bitmap(m_superBlock.inodePerGroup);
		read_block(group.inodeBitmapAddr, bitmap.c_ptr(), 0, m_superBlock.inodePerGroup / 8);
		for (size_t i = 0;i < bitmap.size();i++)
		{
			if (bitmap.get(i) == 0)
			{
				bitmap.set(i, 1);
				ret = i + gid*m_superBlock.inodePerGroup + 1;
				write_inode(ret, &inode);
				group.inodeFree--;
				write_block(group.inodeBitmapAddr, bitmap.c_ptr(), 0, m_superBlock.inodePerGroup / 8);
				if (m_superBlock.blockSize == 0)
				{
					fseek(m_pFile, 2048, SEEK_SET);
				}
				else
				{
					fseek(m_pFile, m_blockSize, SEEK_SET);
				}
				fwrite(&m_groupDescriptor[0], sizeof(ext2::BlockGroupDescriptor), m_groupDescriptor.size(), m_pFile);
				m_superBlock.inodeFree--;
				m_superBlock.timeWrite = static_cast<uint32_t>(time);
				if (fseek(m_pFile, 1024, SEEK_SET) == -1)
				{
					perror("fseek error");
				}
				fwrite(&m_superBlock, sizeof(m_superBlock), 1, m_pFile);
				return ret;
			}
		}
		gid++;
	}
	return 0;
}

void FakeDisk::delete_inode(size_t inode)
{
	size_t gid = inode / m_superBlock.inodePerGroup;
	ext2::InodeData inodeData;
	read_inode(inode, &inodeData);
	
	Bitmap bitmap(m_superBlock.inodePerGroup);
	read_block(m_groupDescriptor[gid].inodeBitmapAddr, bitmap.c_ptr(), 0, m_superBlock.inodePerGroup / 8);
	bitmap.set(inode % m_superBlock.inodePerGroup, 0);
	write_block(m_groupDescriptor[gid].inodeBitmapAddr, bitmap.c_ptr(), 0, m_superBlock.inodePerGroup / 8);

	//TODO: big file
	for (uint32_t blk : inodeData.blockDirect)
	{
		delete_block_singly(blk);
	}
}

void FakeDisk::delete_block_singly(size_t blkid)
{
	size_t gid = blkid / m_superBlock.blockPerGroup;
	size_t bing = blkid % m_superBlock.blockPerGroup;

	Bitmap bitmap(m_superBlock.blockPerGroup);
	read_block(m_groupDescriptor[gid].blockBitmapAddr, bitmap.c_ptr(), 0, m_superBlock.blockPerGroup / 8);
	bitmap.set(bing, 0);
	write_block(m_groupDescriptor[gid].blockBitmapAddr, bitmap.c_ptr(), 0, m_superBlock.blockPerGroup / 8);
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
		size_t readlen = read_file(pFile, &dir, 8);
		if (dir.inode == 0 || readlen < 8)
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
					ret->dirEntry = dir;
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

FakeDisk::File* FakeDisk::create(const std::string& path, ext2::DirectoryEntry::Type type)
{
	auto vec = parse_path(path);
	std::string filename = vec.back();
	vec.pop_back();
	std::vector<std::string>::iterator iterName = vec.begin();
	File* root = open_inode(2);
	root->dirEntry.inode = 2;
	File* parent;
	if (iterName == vec.end())
	{
		parent = root;
	}
	else
	{
		parent = open(root, iterName, vec);
		delete root;
	}
	if (parent == nullptr)
	{
		return nullptr;
	}
	else
	{
		vec.push_back(filename);
		iterName = vec.end() - 1;
		File* tmp = open(parent, iterName, vec);
		if (tmp != nullptr)
		{
			delete tmp;
			fprintf(stderr, "file already exist\n");
			return nullptr;
		}
		else
		{
			parent->offset = 0;
			size_t need = 8 + filename.size();
			while (true)
			{
				ext2::DirectoryEntry dir;
				size_t beg = parent->offset;
				size_t readlen = read_file(parent, &dir, 8);
				size_t sizeFormer = dir.size;
				if (readlen == 0)
				{
					file_seek(parent, 0, FilePos::End);
				}
				else if(static_cast<size_t>(dir.size - dir.nameLen) >= need)
				{
					dir.size = (8 + dir.nameLen + 3) & 0xFFFC;
					parent->offset = beg;
					write_file(parent, &dir, 8);
					parent->offset = beg;
					file_seek(parent, dir.size, FilePos::Current);
				}
				else
				{
					parent->offset = beg;
					file_seek(parent, dir.size, FilePos::Current);
					continue;
				}
				ext2::DirectoryEntry fileDir;
				size_t inodeind = alloc_inode(type);
				fileDir.inode = inodeind;
				fileDir.name = filename;
				fileDir.nameLen = static_cast<uint8_t>(filename.length());
				fileDir.size = static_cast<uint16_t>((readlen == 0) ? m_blockSize : (sizeFormer - dir.size));
				fileDir.type = type;
				write_file(parent, &fileDir, 8);
				write_file(parent, &fileDir.name[0], fileDir.nameLen);
				File* pFile = open_inode(inodeind);
				pFile->dirEntry = fileDir;
				pFile->name = filename;
				if (type == ext2::DirectoryEntry::Type::Directory)
				{
					ext2::DirectoryEntry dir1;
					dir1.inode = fileDir.inode;
					dir1.name = ".";
					dir1.nameLen = static_cast<uint8_t>(dir1.name.length());
					dir1.size = 12;
					dir1.type = ext2::DirectoryEntry::Type::Directory;
					write_file(pFile, &dir1, 8);
					write_file(pFile, &dir1.name[0], dir1.name.length());
					file_seek(pFile, 12, FilePos::Begin);
					ext2::DirectoryEntry dir2;
					dir2.inode = fileDir.inode;
					dir2.name = "..";
					dir2.nameLen = static_cast<uint8_t>(dir2.name.length());
					dir2.size = static_cast<uint16_t>(m_blockSize - 12);
					dir2.type = ext2::DirectoryEntry::Type::Directory;
					write_file(pFile, &dir2, 8);
					write_file(pFile, &dir2.name[0], dir2.name.length());

					//TODO: dir, big file
					if (m_superBlock.blockSize == 0)
					{
						fseek(m_pFile, 2048, SEEK_SET);
					}
					else
					{
						fseek(m_pFile, m_blockSize, SEEK_SET);
					}
					m_groupDescriptor[parent->dirEntry.inode / m_superBlock.inodePerGroup].cntDir++;
					fwrite(&m_groupDescriptor[0], sizeof(ext2::BlockGroupDescriptor), m_groupDescriptor.size(), m_pFile);
				}
				delete parent;
				return pFile;
			}
		}
	}
}

int32_t FakeDisk::file_seek(File* pFile, int32_t offset, FilePos from)
{
	switch (from)
	{
	case FakeDisk::FilePos::Begin:
		if (offset < 0)
		{
			return -1;
		}
		else
		{
			pFile->offset = offset;
			return 1;
		}
		break;
	case FakeDisk::FilePos::Current:
		if (offset < 0 && static_cast<size_t>(abs(offset)) > pFile->size)
		{
			return -2;
		}
		else if (pFile->offset + offset <= pFile->size)
		{
			pFile->offset = pFile->offset + offset;
			return 2;
		}
		else
		{
			pFile->offset = pFile->size;
			size_t padding = pFile->offset + offset - pFile->size;
			void* tmp = malloc(padding);
			memset(tmp, 0, padding);
			write_file(pFile, tmp, padding);
			free(tmp);
			return 2;
		}
		break;
	case FakeDisk::FilePos::End:
		if (offset < 0 && static_cast<size_t>(abs(offset)) > pFile->size)
		{
			return -3;
		}
		else if(offset < 0 && static_cast<size_t>(abs(offset)) < pFile->size)
		{
			pFile->offset = offset + pFile->offset;
		}
		else
		{
			pFile->offset = pFile->size;
			size_t padding = offset;
			void* tmp = malloc(padding);
			memset(tmp, 0, padding);
			write_file(pFile, tmp, padding);
			free(tmp);
			return 3;
		}
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

size_t FakeDisk::write_file(File* pFile, void* buf, size_t len)
{
	if (pFile->offset + len > pFile->size)
	{
		pFile->size = pFile->offset + len;
	}
	if (pFile->size > pFile->block.size() * m_blockSize)
	{
		size_t delta = pFile->size - pFile->block.size() * m_blockSize;
		size_t blockNeed = delta / m_blockSize + ((delta % m_blockSize) > 0) ? 1 : 0;
		size_t origin = pFile->block.size();
		auto dvec = alloc_block(blockNeed);
		pFile->block.insert(pFile->block.end(), dvec.begin(), dvec.end());
		//TODO: big file
		memcpy(pFile->inode.blockDirect, &pFile->block[0], sizeof(uint32_t)*((pFile->block.size() > 12) ? 12 : pFile->block.size()));

		
	}
	
	while (len > 0)
	{
		uint32_t blkid = pFile->block[(pFile->offset / m_blockSize)];
		uint32_t blkoff = pFile->offset % m_blockSize;
		uint32_t blklen = (blkoff + len > m_blockSize) ? m_blockSize - blkoff : len;
		write_block(blkid, buf, blkoff, blklen);
		pFile->offset += blklen;
		len -= blklen;
	}

	return len;
}

int32_t FakeDisk::remove(char* path)
{
	File* target = open(path);
	if (target == nullptr)
	{
		fprintf(stderr, "file \"%s\" does not exist\n", path);
		return -1;
	}
	auto vec = parse_path(path);
	std::string name = vec.back();
	vec.pop_back();
	File* root = open_inode(2);
	File* parent;
	if ((vec.size() == 0))
	{
		parent = root;
	}
	else
	{
		parent = open(root, vec.begin(), vec);
		delete root;
	}

	ext2::DirectoryEntry* prev = nullptr;
	ext2::DirectoryEntry* cur = nullptr;
	prev = new ext2::DirectoryEntry;
	size_t prevStart = parent->offset;
	read_file(parent, prev, 8);
	prev->name.resize(prev->nameLen);
	read_file(parent, &prev->name[0], prev->nameLen);
	while (parent)
	{
		cur = new ext2::DirectoryEntry;
		parent->offset = prevStart + prev->size;
		read_file(parent, cur, 8);
		if (cur->inode == 0)
		{
			return -1;
		}
		cur->name.resize(cur->nameLen);
		read_file(parent, &cur->name[0], cur->nameLen);

		if (cur->name == name)
		{
			delete_inode(cur->inode);
			void* buf = malloc(cur->size);
			memset(buf, 0, cur->size);
			write_file(parent, prev, 8);
			parent->offset = prevStart + prev->size;
			write_file(parent, buf, cur->size);
			prev->size += cur->size;
			free(buf);
			delete cur;
			return 0;
		}
		
		prevStart += prev->size;
		delete prev;
		prev = cur;
		cur = nullptr;
	}
	return -1;
}

