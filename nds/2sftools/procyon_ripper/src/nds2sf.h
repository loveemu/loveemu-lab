
#pragma once

#ifndef NDS2SF_H_INCLUDED
#define NDS2SF_H_INCLUDED

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#define PSF_SIGNATURE	"PSF"
#define PSF_TAG_SIGNATURE	"[TAG]"

#define NDS2SF_PSF_VERSION	0x24
#define NDS2SF_EXE_HEADER_SIZE	8

#define MAX_NDS_ROM_SIZE	0x020000000
#define MAX_NDS2SF_EXE_SIZE	(MAX_NDS_ROM_SIZE + NDS2SF_EXE_HEADER_SIZE)

class NDS2SF
{
public:
	NDS2SF()
	{
	}

	~NDS2SF()
	{
	}

	static void put_2sf_exe_header(uint8_t *exe, uint32_t load_offset, uint32_t rom_size);
	static bool exe2sf(const std::string& nds2sf_path, uint8_t *rom, size_t rom_size);
	static bool exe2sf(const std::string& nds2sf_path, uint8_t *rom, size_t rom_size, std::map<std::string, std::string>& tags);
	static bool exe2sf_file(const std::string& nds_path, const std::string& nds2sf_path);
	static bool make_mini2sf(const std::string& nds2sf_path, uint32_t address, size_t size, uint32_t num, std::map<std::string, std::string>& tags);
};

#endif
