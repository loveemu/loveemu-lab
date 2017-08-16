// ZlibWriter - simple zlib wrapper for C++
// This library is released into the public domain

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include <zlib.h>
#include <zconf.h>

#include "ZlibWriter.h"

#define ZLIB_CHUNK_SIZE 16384

ZlibWriter::ZlibWriter() :
	zbuf_changed(false)
{
	reset_zlib(Z_DEFAULT_COMPRESSION);
}

ZlibWriter::ZlibWriter(int compression_level) :
	zbuf_changed(false)
{
	reset_zlib(compression_level);
}

ZlibWriter::~ZlibWriter()
{
	deflateEnd(&z);
}

bool ZlibWriter::reset_zlib(int compression_level)
{
	int zresult;

	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	zresult = deflateInit(&z, Z_BEST_COMPRESSION);

	return (zresult == Z_OK);
}

int ZlibWriter::write(const void * buf, size_t size)
{
	int zresult;
	uint8_t zchunk[ZLIB_CHUNK_SIZE];

	zbuf_changed = true;

	z.next_in = (Bytef *) buf;
	z.avail_in = (uInt) size;
	do
	{
		z.next_out = zchunk;
		z.avail_out = ZLIB_CHUNK_SIZE;

		zresult = deflate(&z, Z_NO_FLUSH);
		if (zresult != Z_OK && zresult != Z_STREAM_END)
		{
			return (int) (size - z.avail_in);
		}

		size_t bytes_written = ZLIB_CHUNK_SIZE - z.avail_out;
		if (bytes_written != 0)
		{
			zbuf.reserve(zbuf.size() + bytes_written);
			for (size_t i = 0; i < bytes_written; i++)
			{
				zbuf.push_back(zchunk[i]);
			}
		}
	} while (z.avail_in != 0);

	return (int) size;
}

bool ZlibWriter::flush() const
{
	if (!zbuf_changed)
	{
		return true;
	}

	int zresult;
	uint8_t zchunk[ZLIB_CHUNK_SIZE];

	z.next_in = Z_NULL;
	z.avail_in = 0;
	do
	{
		z.next_out = zchunk;
		z.avail_out = ZLIB_CHUNK_SIZE;

		zresult = deflate(&z, Z_FINISH);
		if (zresult != Z_OK && zresult != Z_STREAM_END)
		{
			return false;
		}

		size_t bytes_written = ZLIB_CHUNK_SIZE - z.avail_out;
		if (bytes_written != 0)
		{
			zbuf.reserve(zbuf.size() + bytes_written);
			for (size_t i = 0; i < bytes_written; i++)
			{
				zbuf.push_back(zchunk[i]);
			}
		}
	} while (zresult != Z_STREAM_END);

	zbuf_changed = false;
	return true;
}
