
#pragma once

#ifndef PROCYON_RIPPER_H_INCLUDED
#define PROCYON_RIPPER_H_INCLUDED

#include <vector>

struct Region
{
	Region() :
		offset(0),
		size(0)
	{
	}

	Region(off_t offset, size_t size) :
		offset(offset),
		size(size)
	{
	}

	off_t offset;
	size_t size;
};

class procyon_ripper
{
public:
	procyon_ripper();
	~procyon_ripper();

	static bool is_valid_nds_rom(const std::string & nds_filename);
	bool load_rom(const std::string & nds_filename);
	bool scan();
	bool save_2sfs(const std::string & basename);

public:
	bool verbose;

private:
	std::string nds_path;
	uint32_t rom_size;
	uint32_t arm9_rom_offset;
	uint32_t arm9_load_address;
	uint32_t arm9_size;
	uint8_t * exe;
	uint8_t * rom;
	uint8_t * arm9;

	std::vector<Region> nopRegions;
	uint32_t driver_offset;
	uint32_t bgm_play_function_offset;
};

#endif
