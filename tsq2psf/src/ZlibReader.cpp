// ZlibReader - simple zlib wrapper for C++
// This library is released into the public domain

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include <zlib.h>
#include <zconf.h>

#include "ZlibReader.h"

ZlibReader::ZlibReader() :
	initialized(false),
	zbuf_crc(0)
{
	reset_zlib();
	initialized = true;
}

ZlibReader::ZlibReader(const void * buf, size_t size) :
	initialized(false)
{
	assign(buf, size);
	initialized = true;
}

ZlibReader::~ZlibReader()
{
	inflateEnd(&z);
}

bool ZlibReader::reset_zlib()
{
	int zresult;

	if (initialized)
	{
		inflateEnd(&z);
	}

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	zresult = inflateInit(&z);

	zpos = 0;
	pos = 0;

	reset_crc32();

	return (zresult == Z_OK);
}

void ZlibReader::assign(const void * buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		zbuf.push_back(((uint8_t *)buf)[i]);
	}
	zbuf_crc = ::crc32(0L, (const Bytef *) buf, (uInt) size);

	reset_zlib();
}

int ZlibReader::read(const void * buf, size_t size)
{
	int zresult;

	if (zpos >= zbuf.size())
	{
		return 0;
	}

	uInt z_avail_in_old = (uInt) (zbuf.size() - zpos);

	z.next_in = ((Bytef *) &zbuf[0]) + zpos;
	z.avail_in = z_avail_in_old;
	z.next_out = (Bytef *) buf;
	z.avail_out = (uInt) size;
	zresult = inflate(&z, Z_SYNC_FLUSH);
	if (zresult != Z_OK && zresult != Z_STREAM_END)
	{
		return -1;
	}

	zpos += (z_avail_in_old - z.avail_in);

	size_t bytes_read = (size - z.avail_out);
	pos += bytes_read;

	crc = ::crc32(crc, (const Bytef *) buf, (uInt) bytes_read);

	return (int) bytes_read;
}
