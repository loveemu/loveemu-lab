// ZlibWriter - simple zlib wrapper for C++
// This library is released into the public domain

#ifndef ZLIBWRITER_H_INCLUDED
#define ZLIBWRITER_H_INCLUDED

#include <stdint.h>

#include <zlib.h>
#include <zconf.h>

#include <string>
#include <vector>

class ZlibWriter
{
public:
	ZlibWriter();
	ZlibWriter(int compression_level);
	virtual ~ZlibWriter();

	int write(const void * buf, size_t size);

	inline bool writeByte(uint8_t value)
	{
		return write(&value, 1) == 1;
	}

	inline bool writeShort(uint16_t value)
	{
		uint8_t data[2] = {
			value & 0xff,
			(value >> 8) & 0xff,
		};
		return write(data, 2) == 2;
	}

	inline bool writeInt(uint32_t value)
	{
		uint8_t data[4] = {
			value & 0xff,
			(value >> 8) & 0xff,
			(value >> 16) & 0xff,
			(value >> 24) & 0xff,
		};
		return write(data, 4) == 4;
	}

	static inline uint32_t crc32(const void * buf, size_t size)
	{
		uLong crc = ::crc32(0L, (const Bytef *) buf, (uInt) size);
		return (uint32_t) crc;
	}

	inline uint32_t crc32() const
	{
		flush();
		return crc32(&zbuf[0], zbuf.size());
	}

	inline const uint8_t * data() const
	{
		flush();
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

	inline size_t size() const
	{
		flush();
		return zbuf.size();
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
	// mutable is attached because of flush()
	mutable std::vector<uint8_t> zbuf;
	mutable z_stream z;
	mutable bool zbuf_changed;

	bool reset_zlib(int compression_level);
	bool flush() const;

private:
	ZlibWriter(const ZlibWriter&);
	ZlibWriter& operator=(const ZlibWriter&);
};

#endif /* !ZLIBWRITER_H_INCLUDED */
