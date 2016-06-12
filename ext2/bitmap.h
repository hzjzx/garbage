#pragma once
#ifndef FDBITMAP_H
#define FDBITMAP_H

#include <cinttypes>
#include <cstdlib>

class Bitmap
{
private:
	uint8_t* m_data;
	size_t m_len;

public:
	explicit Bitmap(size_t n)
	{
		if (n % 8 != 0)
		{
			exit(-1);
		}
		m_data = new uint8_t[n / 8];
		m_len = n;
	}

	bool get(size_t n) const
	{
		if (n < m_len)
		{
			uint8_t container = m_data[n / 8];
			return static_cast<bool>(container >> (n % 8));
		}
		else
		{
			exit(-1);
		}
	}

	void set(size_t n, bool d)
	{
		if (n < m_len)
		{
			uint8_t container = m_data[n / 8];
			container |= static_cast<uint8_t>(d) << n % 8;
			m_data[n / 8] = container;
		}
		else
		{
			exit(-1);
		}
	}

	size_t size() const
	{
		return m_len;
	}

	uint8_t* c_ptr() const
	{
		return m_data;
	}

	~Bitmap()
	{
		delete[] m_data;
	}
};

#endif