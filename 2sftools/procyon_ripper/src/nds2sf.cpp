/*
** NDS 2SF utility class.
*/

#ifdef _WIN32
#define ZLIB_WINAPI
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <map>

#include <zlib.h>

#include "nds2sf.h"
#include "cbyteio.h"
#include "cpath.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <sys/stat.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define _chdir(s)	chdir((s))
#define _mkdir(s)	mkdir((s), 0777)
#define _rmdir(s)	rmdir((s))
#endif

void NDS2SF::put_2sf_exe_header(uint8_t *exe, uint32_t load_offset, uint32_t rom_size)
{
	mput4l(load_offset, &exe[0]);
	mput4l(rom_size, &exe[4]);
}

bool NDS2SF::exe2sf(const std::string& nds2sf_path, uint8_t *exe, size_t exe_size)
{
	std::map<std::string, std::string> tags;
	return exe2sf(nds2sf_path, exe, exe_size, tags);
}

#define CHUNK 16384
bool NDS2SF::exe2sf(const std::string& nds2sf_path, uint8_t *exe, size_t exe_size, std::map<std::string, std::string>& tags)
{
	FILE *nds2sf_file = NULL;

	z_stream z;
	uint8_t zbuf[CHUNK];
	uLong zcrc;
	uLong zlen;
	int zflush;
	int zret;

	// check exe size
	if (exe_size > MAX_NDS2SF_EXE_SIZE)
	{
		return false;
	}

	// open output file
	nds2sf_file = fopen(nds2sf_path.c_str(), "wb");
	if (nds2sf_file == NULL)
	{
		return false;
	}

	// write PSF header
	// (EXE length and CRC will be set later)
	fwrite(PSF_SIGNATURE, strlen(PSF_SIGNATURE), 1, nds2sf_file);
	fputc(NDS2SF_PSF_VERSION, nds2sf_file);
	fput4l(0, nds2sf_file);
	fput4l(0, nds2sf_file);
	fput4l(0, nds2sf_file);

	// init compression
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	if (deflateInit(&z, Z_BEST_COMPRESSION) != Z_OK)
	{
		return false;
	}

	// compress exe
	z.next_in = exe;
	z.avail_in = (uInt) exe_size;
	z.next_out = zbuf;
	z.avail_out = CHUNK;
	zcrc = crc32(0L, Z_NULL, 0);
	do
	{
		if (z.avail_in == 0)
		{
			zflush = Z_FINISH;
		}
		else
		{
			zflush = Z_NO_FLUSH;
		}

		// compress
		zret = deflate(&z, zflush);
		if (zret != Z_STREAM_END && zret != Z_OK)
		{
			deflateEnd(&z);
			fclose(nds2sf_file);
			return false;
		}

		// write compressed data
		zlen = CHUNK - z.avail_out;
		if (zlen != 0)
		{
			if (fwrite(zbuf, zlen, 1, nds2sf_file) != 1)
			{
				deflateEnd(&z);
				fclose(nds2sf_file);
				return false;
			}
			zcrc = crc32(zcrc, zbuf, zlen);
		}

		// give space for next chunk
		z.next_out = zbuf;
		z.avail_out = CHUNK;
	} while (zret != Z_STREAM_END);

	// set EXE info to PSF header
	fseek(nds2sf_file, 8, SEEK_SET);
	fput4l(z.total_out, nds2sf_file);
	fput4l(zcrc, nds2sf_file);
	fseek(nds2sf_file, 0, SEEK_END);

	// end compression
	deflateEnd(&z);

	// write tags
	if (!tags.empty())
	{
		fwrite(PSF_TAG_SIGNATURE, strlen(PSF_TAG_SIGNATURE), 1, nds2sf_file);

		for (std::map<std::string, std::string>::iterator it = tags.begin(); it != tags.end(); ++it)
		{
			const std::string& key = it->first;
			const std::string& value = it->second;
			std::istringstream value_reader(value);
			std::string line;

			// process for each lines
			while (std::getline(value_reader, line))
			{
				if (fprintf(nds2sf_file, "%s=%s\n", key.c_str(), line.c_str()) < 0)
				{
					fclose(nds2sf_file);
					return false;
				}
			}
		}
	}

	fclose(nds2sf_file);
	return true;
}

bool NDS2SF::exe2sf_file(const std::string& nds_path, const std::string& nds2sf_path)
{
	off_t rom_size_off = path_getfilesize(nds_path.c_str());
	if (rom_size_off == -1) {
		return false;
	}

	if (rom_size_off > MAX_NDS_ROM_SIZE) {
		return false;
	}

	uint32_t rom_size = (uint32_t)rom_size_off;

	FILE * rom_file = fopen(nds_path.c_str(), "rb");
	if (rom_file == NULL) {
		return false;
	}

	uint8_t * exe = new uint8_t[NDS2SF_EXE_HEADER_SIZE + rom_size];
	if (exe == NULL) {
		fclose(rom_file);
		return false;
	}

	put_2sf_exe_header(exe, 0, rom_size);
	if (fread(&exe[NDS2SF_EXE_HEADER_SIZE], 1, rom_size, rom_file) != rom_size) {
		delete[] exe;
		fclose(rom_file);
		return false;
	}

	if (!exe2sf(nds2sf_path, exe, NDS2SF_EXE_HEADER_SIZE + rom_size)) {
		delete[] exe;
		fclose(rom_file);
		return false;
	}

	delete[] exe;
	fclose(rom_file);
	return true;
}

bool NDS2SF::make_mini2sf(const std::string& nds2sf_path, uint32_t address, size_t size, uint32_t num, std::map<std::string, std::string>& tags)
{
	uint8_t exe[NDS2SF_EXE_HEADER_SIZE + 4];

	// limit size
	if (size > 4)
	{
		return false;
	}

	// make exe
	put_2sf_exe_header(exe, address, (uint32_t)size);
	mput4l(num, &exe[NDS2SF_EXE_HEADER_SIZE]);

	// write mini2sf file
	return exe2sf(nds2sf_path, exe, NDS2SF_EXE_HEADER_SIZE + size, tags);
}
