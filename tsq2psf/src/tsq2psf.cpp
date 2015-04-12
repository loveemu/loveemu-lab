
#define NOMINMAX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <limits>
#include <algorithm>

#include "tsq2psf.h"
#include "PSFFile.h"
#include "cpath.h"

#ifdef WIN32
#include <direct.h>
#include <float.h>
#define getcwd _getcwd
#define chdir _chdir
#define isnan _isnan
#define strcasecmp _stricmp
#else
#include <unistd.h>
#endif

#define APP_NAME    "tsq2psf"
#define APP_VER     "[2015-04-12]"
#define APP_URL     ""

#define PSX_MEMORY_SIZE         0x200000
#define PSX_MEMORY_MASK         0xFFFFFF

#define PSF1_PSF_VERSION        0x01
#define PSF1_EXE_HEADER_SIZE    0x800
#define PSF1_EXE_ALIGN_SIZE     0x800

#define DRIVER_PSFLIB_NAME      "tsq2psf.psflib"

/*
** Define the location of the PSF driver stub.
** You should define this to somewhere safe where there's no useful data and
** which will not get overwritten by the BSS clear loop.
*/
#define MINIPSF_PARAM       (0x800EF800)

/*
** You can also define locations of game-specific data here.
*/
#define MY_SEQ          (0x800F0000)
#define MY_SEQ_SIZE     (0x00018000)
#define MY_INSTR        (0x8001D800)
#define MY_INSTR_SIZE   (0x00080000)

/*
** Parameters - you can make up any parameters you want within the
** PSFDRV_PARAM block.
** In this example, I'm including the sequence volume, reverb type and depth.
*/
#define MPARAM_SONGINDEX    (MINIPSF_PARAM + 0x0000)
#define MPARAM_RESERVED     (MINIPSF_PARAM + 0x0002)
#define MPARAM_RUSE_SUB     (MINIPSF_PARAM + 0x0004)
#define MPARAM_RTYPE_SUB    (MINIPSF_PARAM + 0x0005)
#define MPARAM_RDEPTH_SUB   (MINIPSF_PARAM + 0x0006)

static uint8_t readByte(uint8_t * buf)
{
	return buf[0];
}

static uint16_t readShort(uint8_t * buf)
{
	return buf[0] | (buf[1] << 8);
}

static uint32_t readInt(uint8_t * buf)
{
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

static void writeByte(uint8_t * buf, uint8_t value)
{
	buf[0] = value;
}

static void writeShort(uint8_t * buf, uint16_t value)
{
	buf[0] = value & 0xff;
	buf[1] = (value >> 8) & 0xff;
}

static void writeInt(uint8_t * buf, uint32_t value)
{
	buf[0] = value & 0xff;
	buf[1] = (value >> 8) & 0xff;
	buf[2] = (value >> 16) & 0xff;
	buf[3] = (value >> 24) & 0xff;
}

static size_t align(size_t size, size_t alignment)
{
	return (size + alignment - 1) / size * size;
}

bool save_psf1(const char * psf_path, const uint8_t * rom, uint32_t load_address, size_t rom_size, uint32_t initial_pc, uint32_t initial_sp, const char * region_name, std::map<std::string, std::string> tags)
{
	uint32_t load_offset = load_address & 0xffffff;

	if ((load_address >> 24) == 0) {
		load_address |= 0x80000000;
	}

	if ((load_address >> 24) != 0x80 || load_offset >= PSX_MEMORY_SIZE) {
		fprintf(stderr, "Error: Invalid load address 0x%08X\n", load_address);
		return false;
	}

	size_t load_size = align(rom_size, PSF1_EXE_ALIGN_SIZE);
	if (load_offset + load_size > PSX_MEMORY_SIZE) {
		fprintf(stderr, "Error: Address out of range (0x%08X + 0x%X)\n", load_address, load_size);
		return false;
	}

	uint8_t * exe = new uint8_t[PSF1_EXE_HEADER_SIZE + load_size];
	if (exe == NULL) {
		fprintf(stderr, "Error: Unable to allocate memory\n");
		return false;
	}
	memset(exe, 0, PSF1_EXE_HEADER_SIZE);
	memcpy(&exe[PSF1_EXE_HEADER_SIZE], rom, rom_size);
	memset(&exe[PSF1_EXE_HEADER_SIZE + rom_size], 0, load_size - rom_size);

	// Construct PSX-EXE header
	char region_marker[256];
	if (region_name == NULL || region_name[0] == '\0') {
		region_name = "North America";
	}
	sprintf(region_marker, "Sony Computer Entertainment Inc. for %s area", region_name);

	memcpy(exe, "PS-X EXE", 8);
	writeInt(&exe[0x10], initial_pc);
	writeInt(&exe[0x18], load_address);
	writeInt(&exe[0x1c], load_size);
	writeInt(&exe[0x30], initial_sp);
	strcpy((char *)(&exe[0x4c]), region_marker);

	// Write to PSF1 file
	ZlibWriter exe_z(Z_BEST_COMPRESSION);
	if (exe_z.write(exe, PSF1_EXE_HEADER_SIZE + load_size) != PSF1_EXE_HEADER_SIZE + load_size) {
		fprintf(stderr, "Error: Zlib compress error \"%s\"\n", psf_path);
		delete[] exe;
		return false;
	}

	if (!PSFFile::save(psf_path, PSF1_PSF_VERSION, NULL, 0, exe_z, tags)) {
		fprintf(stderr, "Error: File write error \"%s\"\n", psf_path);
		delete[] exe;
		return false;
	}

	delete[] exe;
	return true;
}

bool build_choroq_psf(const char * psf_path, const uint8_t * tsq, size_t tsq_size, const uint8_t * tvb, size_t tvb_size, bool write_driver, const char * driver_psflib_path, bool write_param, int song_index, std::map<std::string, std::string> tags)
{
	uint8_t * mem = new uint8_t[PSX_MEMORY_SIZE];
	if (mem == NULL) {
		fprintf(stderr, "Error: Memory allocation error\n");
		return false;
	}
	memset(mem, 0, PSX_MEMORY_SIZE);

	uint32_t load_address = 0x80000000 | PSX_MEMORY_SIZE;
	uint32_t load_end_address = 0x80000000 | 0;

	PSFFile * driver_psflib = PSFFile::load(driver_psflib_path);
	if (driver_psflib == NULL) {
		fprintf(stderr, "Error: Unable to read \"%s\"\n", driver_psflib_path);
		delete[] mem;
		return false;
	}

	if (driver_psflib->version != PSF1_PSF_VERSION) {
		fprintf(stderr, "Error: Invalid PSF version \"%s\"\n", driver_psflib_path);
		delete driver_psflib;
		delete[] mem;
		return false;
	}

	uint8_t psx_exe_header[PSF1_EXE_HEADER_SIZE];
	if (driver_psflib->compressed_exe.read(psx_exe_header, PSF1_EXE_HEADER_SIZE) != PSF1_EXE_HEADER_SIZE) {
		fprintf(stderr, "Error: Unable to read PS-X EXE header \"%s\"\n", driver_psflib_path);
		delete driver_psflib;
		delete[] mem;
		return false;
	}

	uint32_t driver_load_address = readInt(&psx_exe_header[0x18]);
	uint32_t driver_load_size = readInt(&psx_exe_header[0x1c]);
	uint32_t initial_pc = readInt(&psx_exe_header[0x10]);
	uint32_t initial_sp = readInt(&psx_exe_header[0x30]);

	if (memcmp(psx_exe_header, "PS-X EXE", 8) != 0 || (driver_load_address & PSX_MEMORY_MASK) + driver_load_size >= PSX_MEMORY_SIZE) {
		fprintf(stderr, "Error: Invalid PS-X EXE header \"%s\"\n", driver_psflib_path);
		delete driver_psflib;
		delete[] mem;
		return false;
	}

	char * region_name;
	if (strcmp((const char *)&psx_exe_header[0x4c], "Sony Computer Entertainment Inc. for Japan area") == 0) {
		region_name = "Japan";
	}
	else if (strcmp((const char *)&psx_exe_header[0x4c], "Sony Computer Entertainment Inc. for North America area") == 0) {
		region_name = "North America";
	}
	else if (strcmp((const char *)&psx_exe_header[0x4c], "Sony Computer Entertainment Inc. for Europe area") == 0) {
		region_name = "Europe";
	}
	else {
		fprintf(stderr, "Warning: Unknown region name \"%s\"\n", driver_psflib_path);
		region_name = "North America";
	}

	if (write_driver) {
		if (driver_psflib->compressed_exe.read(&mem[driver_load_address & PSX_MEMORY_MASK], driver_load_size) != driver_load_size) {
			fprintf(stderr, "Error: Unable to read driver block \"%s\"\n", driver_psflib_path);
			delete driver_psflib;
			delete[] mem;
			return false;
		}

		load_address = std::min<uint32_t>(load_address, driver_load_address);
		load_end_address = std::max<uint32_t>(load_end_address, driver_load_address + driver_load_size);
	}
	delete driver_psflib;

	if (tsq != NULL && tsq_size != 0) {
		memcpy(&mem[MY_SEQ & PSX_MEMORY_MASK], tsq, tsq_size);
		load_address = std::min<uint32_t>(load_address, MY_SEQ);
		load_end_address = std::max<uint32_t>(load_end_address, MY_SEQ + tsq_size);
	}

	if (tvb != NULL && tvb_size != 0) {
		memcpy(&mem[MY_INSTR & PSX_MEMORY_MASK], tvb, tvb_size);
		load_address = std::min<uint32_t>(load_address, MY_INSTR);
		load_end_address = std::max<uint32_t>(load_end_address, MY_INSTR + tvb_size);
	}

	if (write_param) {
		writeByte(&mem[MPARAM_SONGINDEX & PSX_MEMORY_MASK], song_index);
		load_address = std::min<uint32_t>(load_address, MINIPSF_PARAM);
		load_end_address = std::max<uint32_t>(load_end_address, MINIPSF_PARAM + 8);
	}

	uint32_t rom_size = load_end_address - load_address;

	if (!save_psf1(psf_path, &mem[load_address & PSX_MEMORY_MASK], load_address, rom_size, initial_pc, initial_sp, region_name, tags)) {
		delete[] mem;
		return false;
	}

	delete[] mem;
	return true;
}

bool read_file_all(const char * filename, uint8_t *& file_buf, size_t & file_size)
{
	off_t size_off = path_getfilesize(filename);
	if (size_off == -1) {
		fprintf(stderr, "Error: File not exist \"%s\"\n", filename);
		return false;
	}

	uint8_t * buf = new uint8_t[size_off];
	if (buf == NULL) {
		fprintf(stderr, "Error: Memory allocation error\n");
		return false;
	}

	FILE * fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: File open error \"%s\"\n", filename);
		delete[] buf;
		return false;
	}

	if (fread(buf, 1, size_off, fp) != size_off) {
		fprintf(stderr, "Error: File read error \"%s\"\n", filename);
		fclose(fp);
		delete[] buf;
		return false;
	}

	fclose(fp);

	file_buf = buf;
	file_size = (size_t)size_off;
	return true;
}

void delete_file_memory(uint8_t * file_buf)
{
	if (file_buf != NULL) {
		delete[] file_buf;
	}
}

static void usage(const char * progname)
{
	printf("%s %s\n", APP_NAME, APP_VER);
	if (strcmp(APP_URL, "") != 0) {
		printf("<%s>\n", APP_URL);
	}
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s (options) outfile.psf`\n", progname);
	printf("\n");
	printf("Note: Please put %s to the directory where %s is installed\n", DRIVER_PSFLIB_NAME, APP_NAME);
	printf("\n");

	printf("### Options\n");
	printf("\n");
	printf("`--help`\n");
	printf("  : Show help\n");
	printf("\n");
	printf("`--standalone [TSQ path] [song index] [TVB path] `\n");
	printf("  : Create a standalone PSF file\n");
	printf("\n");
	printf("`--install-driver`\n");
	printf("  : Install driver block for playback (for psflib creation)\n");
	printf("\n");
	printf("`--tsq [TSQ path]`\n");
	printf("  : Save TSQ file into output PSF file\n");
	printf("\n");
	printf("`--tvb [TVB path]`\n");
	printf("  : Save TVB file into output PSF file\n");
	printf("\n");
	printf("`--song [song index]`\n");
	printf("  : Specify song index for TSQ playback (often used with `--tsq` option)\n");
	printf("\n");
	printf("`--lib [psflib name]`\n");
	printf("  : Set the name to the `_lib` tag of output PSF file\n");
	printf("\n");
	printf("`--psfby [name]`\n");
	printf("  : Set the name to the `psfby` tag of output PSF file\n");
	printf("\n");

	printf("### Example of Use\n");
	printf("\n");
	printf("```\n");
	printf("%s --standalone BGM_01.TSQ 1 BGM.TVB BGM_01.psf\n", progname);
	printf("```\n");
	printf("\n");
	printf("```\n");
	printf("%s --install-driver --tvb BGM.TVB BGM.psflib\n", progname);
	printf("%s --lib BGM.psflib --tsq BGM_01.TSQ --song 1 BGM_01.minipsf\n", progname);
	printf("```\n");
	printf("\n");
	printf("```\n");
	printf("%s --install-driver --tsq BGM_01.TSQ --tvb BGM.TVB BGM.psflib\n", progname);
	printf("%s --lib BGM.psflib --song 1 BGM_01.minipsf\n", progname);
	printf("```\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	bool write_driver = false;
	bool write_param = false;
	bool write_tsq = false;
	bool write_tvb = false;
	char tsq_path[PATH_MAX] = { '\0' };
	char tvb_path[PATH_MAX] = { '\0' };
	uint8_t song_index = 1;

	std::map<std::string, std::string> tags;

	long longval;
	char *endptr = NULL;

	char module_path[PATH_MAX];
	char module_dir[PATH_MAX];
	path_modulepath(module_path);
	strcpy(module_dir, module_path);
	path_dirname(module_dir);

	char driver_psflib_path[PATH_MAX];
	sprintf(driver_psflib_path, "%s%s%s", module_dir, PATH_SEPARATOR_STR, DRIVER_PSFLIB_NAME);

	int argi = 1;
	while (argi < argc && argv[argi][0] == '-') {
		if (strcmp(argv[argi], "--help") == 0) {
			usage(argv[0]);
			return EXIT_FAILURE;
		}
		else if (strcmp(argv[argi], "--standalone") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			strcpy(tsq_path, argv[argi + 1]);

			longval = strtol(argv[argi + 2], &endptr, 10);
			if (*endptr != '\0' || errno == ERANGE) {
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 3]);
				return EXIT_FAILURE;
			}
			if (longval < 0 || longval > 255) {
				fprintf(stderr, "Error: Number out of range %ld\n", longval);
				return EXIT_FAILURE;
			}
			song_index = (uint8_t)longval;

			strcpy(tvb_path, argv[argi + 3]);

			write_driver = true;
			write_tsq = true;
			write_tvb = true;
			write_param = true;
			argi += 3;
		}
		else if (strcmp(argv[argi], "--install-driver") == 0) {
			write_driver = true;
		}
		else if (strcmp(argv[argi], "--tsq") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			strcpy(tsq_path, argv[argi + 1]);

			write_tsq = true;
			argi++;
		}
		else if (strcmp(argv[argi], "--tvb") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			strcpy(tvb_path, argv[argi + 1]);

			write_tvb = true;
			argi++;
		}
		else if (strcmp(argv[argi], "--song") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			longval = strtol(argv[argi + 1], &endptr, 10);
			if (*endptr != '\0' || errno == ERANGE) {
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 1]);
				return EXIT_FAILURE;
			}
			if (longval < 0 || longval > 255) {
				fprintf(stderr, "Error: Number out of range %ld\n", longval);
				return EXIT_FAILURE;
			}
			song_index = (uint8_t)longval;

			write_param = true;
			argi++;
		}
		else if (strcmp(argv[argi], "--lib") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			tags["_lib"] = argv[argi + 1];

			argi++;
		}
		else if (strcmp(argv[argi], "--lib2") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			tags["_lib2"] = argv[argi + 1];

			argi++;
		}
		else if (strcmp(argv[argi], "--lib3") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			tags["_lib3"] = argv[argi + 1];

			argi++;
		}
		else if (strcmp(argv[argi], "--psfby") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			tags["psfby"] = argv[argi + 1];

			argi++;
		}
		else {
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	int argnum = argc - argi;
	if (argnum != 1) {
		fprintf(stderr, "Error: Too many arguments\n");
		return EXIT_FAILURE;
	}

	if (!write_tsq && !write_tvb && !write_driver && !write_param) {
		fprintf(stderr, "Error: No output contents\n");
		return EXIT_FAILURE;
	}

	char psf_path[PATH_MAX] = { '\0' };
	strcpy(psf_path, argv[argi]);

	uint8_t * tsq = NULL;
	uint8_t * tvb = NULL;
	size_t tsq_size = 0;
	size_t tvb_size = 0;

	if (write_tsq) {
		if (!read_file_all(tsq_path, tsq, tsq_size)) {
			return EXIT_FAILURE;
		}
	}

	if (write_tvb) {
		if (!read_file_all(tvb_path, tvb, tvb_size)) {
			delete_file_memory(tsq);
			return EXIT_FAILURE;
		}
	}

	if (!build_choroq_psf(psf_path, tsq, tsq_size, tvb, tvb_size, write_driver, driver_psflib_path, write_param, song_index, tags)) {
		delete_file_memory(tsq);
		delete_file_memory(tvb);
		return EXIT_FAILURE;
	}

	delete_file_memory(tsq);
	delete_file_memory(tvb);

	return EXIT_SUCCESS;
}
