// ZlibReader - simple zlib wrapper for C++
// This library is released into the public domain

#ifndef ZLIBREADER_H_INCLUDED
#define ZLIBREADER_H_INCLUDED

#include <stdint.h>

#include <zlib.h>
#include <zconf.h>

#include <string>
#include <vector>

class ZlibReader
{
public:
	ZlibReader();
	ZlibReader(const void * buf, size_t size);
	virtual ~ZlibReader();

	void assign(const void * buf, size_t size);
	int read(const void * buf, size_t size);

	inline bool readByte(uint8_t& value)
	{
		return (read(&value, 1) == 1);
	}

	inline bool readShort(uint16_t& value)
	{
		uint8_t data[2];

		if (read(data, 2) == 2)
		{
			value = data[0] | (data[1] << 8);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool readInt(uint32_t& value)
	{
		uint8_t data[4];

		if (read(data, 4) == 4)
		{
			value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline void rewind()
	{
		reset_zlib();
	}

	inline size_t position() const
	{
		return pos;
	}

	inline const uint8_t * compressed_data() const
	{
		if (zbuf.size() != 0)
		{
			return &zbuf[0];
		}
		else
		{
			static uint8_t empty_byte[1];
			return (const uint8_t *) &empty_byte;
		}
	}

	inline size_t compressed_size() const
	{
		return zbuf.size();
	}

	static inline uint32_t crc32(const void * buf, size_t size)
	{
		uLong crc = ::crc32(0L, (const Bytef *) buf, (uInt) size);
		return (uint32_t) crc;
	}

	inline uint32_t compressed_crc32() const
	{
		return zbuf_crc;
	}

	inline uint32_t crc32() const
	{
		return (uint32_t) crc;
	}

	inline void reset_crc32()
	{
		crc = ::crc32(0L, Z_NULL, 0);
	}

	inline std::string message() const
	{
		if (z.msg == NULL)
		{
			return "";
		}
		else
		{
			return z.msg;
		}
	}

private:
	std::vector<uint8_t> zbuf;
	uLong zbuf_crc;
	size_t zpos;
	size_t pos;
	uLong crc;
	z_stream z;
	bool initialized;

	bool reset_zlib();

private:
	ZlibReader(const ZlibReader&);
	ZlibReader& operator=(const ZlibReader&);
};

#endif /* !ZLIBREADER_H_INCLUDED */
