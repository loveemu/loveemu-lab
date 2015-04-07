
#ifndef PSFFILE_H_INCLUDED
#define PSFFILE_H_INCLUDED

#include <stdint.h>

#include <string>
#include <vector>
#include <map>

#include "ZlibReader.h"
#include "ZlibWriter.h"

#define PSF_SIGNATURE       "PSF"
#define PSF_SIGNATURE_SIZE  3
#define PSF_TAG_MARKER      "[TAG]"
#define PSF_TAG_MARKER_SIZE 5

class PSFFile
{
public:
	PSFFile();
	virtual ~PSFFile();

	uint8_t version;
	std::vector<uint8_t> reserved;
	ZlibReader compressed_exe;
	std::map<std::string, std::string> tags;

	static PSFFile * load(const std::string& filename);
	bool save(const std::string& filename);
	static bool save(const std::string& filename, uint8_t version, const uint8_t * reserved, uint32_t reserved_size, const ZlibWriter& exe, std::map<std::string, std::string> tags);
	static bool save(const std::string& filename, uint8_t version, const uint8_t * reserved, uint32_t reserved_size, const uint8_t * compressed_exe, uint32_t compressed_exe_size, std::map<std::string, std::string> tags);
	static bool IsPSFFile(const std::string& filename);

private:
	PSFFile(const PSFFile&);
	PSFFile& operator=(const PSFFile&);
};

#endif /* !PSFFILE_H_INCLUDED */
