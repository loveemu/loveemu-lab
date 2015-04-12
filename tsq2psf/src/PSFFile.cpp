
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include "PSFFile.h"
#include "ZlibReader.h"
#include "ZlibWriter.h"
#include "cpath.h"

PSFFile::PSFFile()
{
}

PSFFile::~PSFFile()
{
}

bool PSFFile::IsPSFFile(const std::string& filename)
{
	FILE *fp = NULL;
	bool isPSF = false;
	uint8_t sig[3];

	fp = fopen(filename.c_str(), "rb");
	if (fp == NULL)
	{
		return false;
	}

	if (fread(sig, 1, PSF_SIGNATURE_SIZE, fp) == PSF_SIGNATURE_SIZE)
	{
		if (memcmp(sig, PSF_SIGNATURE, PSF_SIGNATURE_SIZE) == 0)
		{
			isPSF = true;
		}
	}

	fclose(fp);
	return isPSF;
}

PSFFile * PSFFile::load(const std::string& filename)
{
	uint8_t data[4];

	off_t off_psf_size = path_getfilesize(filename.c_str());
	if (off_psf_size < 0)
	{
		return NULL;
	}
	size_t psf_size = (size_t) off_psf_size;

	FILE * fp = fopen(filename.c_str(), "rb");
	if (fp == NULL)
	{
		return NULL;
	}

	// signature
	uint8_t sig[PSF_SIGNATURE_SIZE];
	if (fread(sig, 1, PSF_SIGNATURE_SIZE, fp) != PSF_SIGNATURE_SIZE)
	{
		fclose(fp);
		return NULL;
	}
	if (memcmp(sig, PSF_SIGNATURE, PSF_SIGNATURE_SIZE) != 0)
	{
		fclose(fp);
		return NULL;
	}

	// version number
	if (fread(data, 1, 1, fp) != 1)
	{
		fclose(fp);
		return NULL;
	}
	uint8_t version = data[0];

	// size of reserved area
	if (fread(data, 1, 4, fp) != 4)
	{
		fclose(fp);
		return NULL;
	}
	uint32_t reserved_size = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

	// size of compressed program
	if (fread(data, 1, 4, fp) != 4)
	{
		fclose(fp);
		return NULL;
	}
	uint32_t compressed_exe_size = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

	// crc32 of compressed program
	if (fread(data, 1, 4, fp) != 4)
	{
		fclose(fp);
		return NULL;
	}
	uint32_t compressed_exe_crc_expected = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

	// check the size consistency beforehand
	if (0x10 + reserved_size + compressed_exe_size > psf_size)
	{
		fclose(fp);
		return NULL;
	}

	// create new PSF object
	PSFFile * psf = new PSFFile();
	psf->version = version;

	// reserved area
	uint8_t * reserved_data = new uint8_t[reserved_size];
	if (fread(reserved_data, 1, reserved_size, fp) != reserved_size)
	{
		delete [] reserved_data;
		delete psf;
		fclose(fp);
		return NULL;
	}
	// set to vector
	for (size_t i = 0; i < reserved_size; i++)
	{
		psf->reserved.push_back(reserved_data[i]);
	}
	delete [] reserved_data;

	// compressed exe
	uint8_t * compressed_exe_data = new uint8_t[compressed_exe_size];
	if (fread(compressed_exe_data, 1, compressed_exe_size, fp) != compressed_exe_size)
	{
		delete [] compressed_exe_data;
		delete psf;
		fclose(fp);
		return NULL;
	}
	// test crc32
	uint32_t compressed_exe_crc = ZlibReader::crc32(compressed_exe_data, compressed_exe_size);
	if (compressed_exe_crc != compressed_exe_crc_expected)
	{
		delete [] compressed_exe_data;
		delete psf;
		fclose(fp);
		return NULL;
	}
	// set to ZlibReader
	psf->compressed_exe.assign(compressed_exe_data, compressed_exe_size);
	delete [] compressed_exe_data;

	// check tag marker (optional)
	uint8_t tag_marker[PSF_TAG_MARKER_SIZE];
	if (fread(tag_marker, 1, PSF_TAG_MARKER_SIZE, fp) != PSF_TAG_MARKER_SIZE)
	{
		// no tags
		fclose(fp);
		return psf;
	}
	if (memcmp(tag_marker, PSF_TAG_MARKER, PSF_TAG_MARKER_SIZE) != 0)
	{
		// no tags
		fclose(fp);
		return psf;
	}

	// read entire tag area
	size_t tag_size = psf_size - (0x10 + reserved_size + compressed_exe_size + PSF_TAG_MARKER_SIZE);
	char * tag_chrs = new char[tag_size + 1];
	if (fread(tag_chrs, 1, tag_size, fp) != tag_size)
	{
		// I/O error
		delete [] tag_chrs;
		delete psf;
		fclose(fp);
		return NULL;
	}
	tag_chrs[tag_size] = '\0';

	// Parse tag section. Details are available here:
	// http://wiki.neillcorlett.com/PSFTagFormat
	size_t off_curtag = 0;
	while (off_curtag < tag_size)
	{
		// Search the end position of the current line.
		char* ptr_newline = strchr(&tag_chrs[off_curtag], 0x0a);
		if (ptr_newline == NULL)
		{
			// Tag section must end with a newline.
			// Read the all remaining bytes if a newline lacks though.
			ptr_newline = tag_chrs + tag_size;
		}

		// Replace the newline with NUL,
		// for better C string function compatibility.
		*ptr_newline = '\0';

		// Search the variable=value separator.
		char* ptr_separator = strchr(&tag_chrs[off_curtag], '=');
		if (ptr_separator == NULL)
		{
			// Blank lines, or lines not of the form "variable=value", are ignored.
			off_curtag = ptr_newline + 1 - tag_chrs;
			continue;
		}

		// Determine the start/end position of variable.
		char* ptr_name = &tag_chrs[off_curtag];
		char* ptr_name_end = ptr_separator;
		char* ptr_value = ptr_separator + 1;
		char* ptr_value_end = ptr_newline;

		// Whitespace at the beginning/end of the line and before/after the = are ignored.
		// All characters 0x01-0x20 are considered whitespace.
		// (There must be no null (0x00) characters.)
		// Trim them.
		while (ptr_name_end > ptr_name && *(unsigned char*)(ptr_name_end - 1) <= 0x20)
			ptr_name_end--;
		while (ptr_value_end > ptr_value && *(unsigned char*)(ptr_value_end - 1) <= 0x20)
			ptr_value_end--;
		while (ptr_name < ptr_name_end && *(unsigned char*)ptr_name <= 0x20)
			ptr_name++;
		while (ptr_value < ptr_value_end && *(unsigned char*)ptr_value <= 0x20)
			ptr_value++;

		// Read variable=value as string.
		std::string tag_var_name(ptr_name, ptr_name_end - ptr_name);
		std::string tag_var_value(ptr_value, ptr_value_end - ptr_value);

		// Multiple-line variables must appear as consecutive lines using the same variable name.
		// For instance:
		//   comment=This is a
		//   comment=multiple-line
		//   comment=comment.
		// Therefore, check if the variable had already appeared.
		std::map<std::string, std::string>::iterator it = psf->tags.lower_bound(tag_var_name);
		if (it != psf->tags.end() && it->first == tag_var_name)
		{
			it->second += "\n";
			it->second += tag_var_value;
		}
		else
		{
			psf->tags.insert(it, make_pair(tag_var_name, tag_var_value));
		}

		off_curtag = ptr_newline + 1 - tag_chrs;
	}
	delete[] tag_chrs;
	fclose(fp);

	return psf;
}

bool PSFFile::save(const std::string& filename)
{
	return save(filename, version, reserved.data(), (uint32_t)reserved.size(), compressed_exe.compressed_data(), (uint32_t)compressed_exe.compressed_size(), tags);
}

bool PSFFile::save(const std::string& filename, uint8_t version, const uint8_t * reserved, uint32_t reserved_size, const ZlibWriter& exe, std::map<std::string, std::string> tags)
{
	return save(filename, version, reserved, reserved_size, exe.data(), (uint32_t)exe.size(), tags);
}

bool PSFFile::save(const std::string& filename, uint8_t version, const uint8_t * reserved, uint32_t reserved_size, const uint8_t * compressed_exe, uint32_t compressed_exe_size, std::map<std::string, std::string> tags)
{
	uint8_t data[4];

	FILE * fp = fopen(filename.c_str(), "wb");
	if (fp == NULL)
	{
		return false;
	}

	// signature
	if (fwrite(PSF_SIGNATURE, 1, PSF_SIGNATURE_SIZE, fp) != PSF_SIGNATURE_SIZE)
	{
		fclose(fp);
		return false;
	}

	// version
	if (fputc(version, fp) == EOF)
	{
		fclose(fp);
		return false;
	}

	// size of reserved area
	data[0] = reserved_size & 0xff;
	data[1] = (reserved_size >> 8) & 0xff;
	data[2] = (reserved_size >> 16) & 0xff;
	data[3] = (reserved_size >> 24) & 0xff;
	if (fwrite(data, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}

	// size of program area
	data[0] = compressed_exe_size & 0xff;
	data[1] = (compressed_exe_size >> 8) & 0xff;
	data[2] = (compressed_exe_size >> 16) & 0xff;
	data[3] = (compressed_exe_size >> 24) & 0xff;
	if (fwrite(data, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}

	// crc32 of program area
	uint32_t exe_crc = (compressed_exe != NULL) ? crc32(0L, compressed_exe, compressed_exe_size) : 0;
	data[0] = exe_crc & 0xff;
	data[1] = (exe_crc >> 8) & 0xff;
	data[2] = (exe_crc >> 16) & 0xff;
	data[3] = (exe_crc >> 24) & 0xff;
	if (fwrite(data, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}

	// reserved area
	if (reserved != NULL && reserved_size != 0)
	{
		if (fwrite(reserved, 1, reserved_size, fp) != reserved_size)
		{
			fclose(fp);
			return false;
		}
	}

	// program area
	if (compressed_exe != NULL && compressed_exe_size != 0)
	{
		if (fwrite(compressed_exe, 1, compressed_exe_size, fp) != compressed_exe_size)
		{
			fclose(fp);
			return false;
		}
	}

	// tags
	if (!tags.empty())
	{
		if (fwrite(PSF_TAG_MARKER, 1, PSF_TAG_MARKER_SIZE, fp) != PSF_TAG_MARKER_SIZE)
		{
			fclose(fp);
			return false;
		}

		for (std::map<std::string, std::string>::iterator it = tags.begin(); it != tags.end(); ++it)
		{
			const std::string& key = it->first;
			const std::string& value = it->second;
			std::istringstream value_reader(value);
			std::string line;

			// process for each lines
			while (std::getline(value_reader, line))
			{
				if (fprintf(fp, "%s=%s\n", key.c_str(), line.c_str()) < 0)
				{
					fclose(fp);
					return false;
				}
			}
		}
	}

	fclose(fp);
	return true;
}
