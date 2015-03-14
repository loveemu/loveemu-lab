/*
** Automated 2SF ripper for Procyon Studio engine.
*/

#define APP_NAME	"procyon_ripper"
#define APP_VER		"[2015-03-14]"
#define APP_DESC	"Automated 2SF ripper for Procyon Studio engine"
#define APP_AUTHOR	"loveemu <http://github.com/loveemu>"

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

#include "procyon_ripper.h"
#include "BytePattern.h"
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

procyon_ripper::procyon_ripper() :
	verbose(false),
	exe(NULL),
	rom(NULL),
	arm9(NULL),
	driver_offset(0),
	bgm_play_function_offset(0)
{
}

procyon_ripper::~procyon_ripper()
{
	if (exe != NULL) {
		delete[] exe;
	}
}

bool procyon_ripper::is_valid_nds_rom(const std::string & nds_filename)
{
	off_t rom_size_off = path_getfilesize(nds_filename.c_str());
	if (rom_size_off < 0x180) {
		return false;
	}
	size_t rom_size = (size_t)rom_size_off;

	FILE * fp = fopen(nds_filename.c_str(), "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: Unable to open file.\n");
		return false;
	}

	uint8_t header[0x180];
	if (fread(header, 1, 0x180, fp) != 0x180) {
		fprintf(stderr, "Error: Unable to read header.\n");
		fclose(fp);
		return false;
	}

	// game title/code must be printable
	for (int i = 0; i < 12 + 4; i++) {
		if (header[i] != 0 && !isprint(header[i])) {
			fprintf(stderr, "Error: Invalid game title.\n");
			fclose(fp);
			return false;
		}
	}

	uint32_t arm9_rom_offset = mget4l(&header[0x20]);
	uint32_t arm9_load_address = mget4l(&header[0x28]);
	uint32_t arm9_size = mget4l(&header[0x2c]);

	// ARM9 rom offset must be aligned to 4KB boundary
	if (arm9_rom_offset % 0x1000 != 0) {
		fprintf(stderr, "Error: Invalid ARM9 ROM offset.\n");
		fclose(fp);
		return false;
	}

	// ARM9 boundary check
	if (arm9_rom_offset + arm9_size > rom_size) {
		fprintf(stderr, "Error: ARM9 section out of bounds.\n");
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

bool procyon_ripper::load_rom(const std::string & nds_filename)
{
	if (verbose) {
		printf("Input NDS ROM \"%s\"\n", nds_filename.c_str());
		printf("\n");
	}

	off_t rom_size_off = path_getfilesize(nds_filename.c_str());
	if (rom_size_off < 0x180) {
		return false;
	}
	size_t rom_size = (size_t)rom_size_off;

	if (!is_valid_nds_rom(nds_filename)) {
		fprintf(stderr, "Error: Invalid NDS ROM.\n");
		return false;
	}

	FILE * fp = fopen(nds_filename.c_str(), "rb");
	if (fp == NULL) {
		fprintf(stderr, "Error: Unable to open file.\n");
		return false;
	}

	uint8_t header[0x180];
	if (fread(header, 1, 0x180, fp) != 0x180) {
		fprintf(stderr, "Error: Unable to read header.\n");
		fclose(fp);
		return false;
	}

	uint32_t arm9_rom_offset = mget4l(&header[0x20]);
	uint32_t arm9_load_address = mget4l(&header[0x28]);
	uint32_t arm9_size = mget4l(&header[0x2c]);

	if (exe != NULL) {
		delete[] exe;
		exe = NULL;
		rom = NULL;
		arm9 = NULL;
	}

	exe = new uint8_t[NDS2SF_EXE_HEADER_SIZE + rom_size];
	if (exe == NULL) {
		fprintf(stderr, "Error: Unable to allocate memory.\n");
		fclose(fp);
		return false;
	}
	NDS2SF::put_2sf_exe_header(exe, 0, rom_size);

	rewind(fp);
	if (fread(&exe[NDS2SF_EXE_HEADER_SIZE], 1, rom_size, fp) != rom_size) {
		fprintf(stderr, "Error: Unable to read ROM.\n");
		delete[] exe;
		exe = NULL;
		fclose(fp);
		return false;
	}

	this->rom = &exe[NDS2SF_EXE_HEADER_SIZE];
	this->rom_size = rom_size;
	this->arm9 = &rom[arm9_rom_offset];
	this->arm9_rom_offset = arm9_rom_offset;
	this->arm9_load_address = arm9_load_address;
	this->arm9_size = arm9_size;

	char abspath[PATH_MAX];
	path_getabspath(nds_filename.c_str(), abspath);
	this->nds_path.assign(abspath);

	nopRegions.clear();
	driver_offset = 0;
	bgm_play_function_offset = 0;

	fclose(fp);
	return true;
}

bool procyon_ripper::scan()
{
	if (arm9 == NULL) {
		return false;
	}

	// main:
	// STMFD           SP!, {R4,LR}
	// SUB             SP, SP, #8
	// BL              sub_206CA90
	// MOV             R0, #0x12            ; NOP
	// BL              sub_20762C0          ; NOP
	// LDR             R0, =unk_20023A0     ; NOP
	// BL              sub_20778E0          ; NOP
	// BL              sub_206D5A0          ; NOP
	// ...
	BytePattern ptnMainStart(
		"\x10\x40\x2d\xe9\x08\xd0\x4d\xe2"
		"\x93\xaf\x01\xeb\x12\x00\xa0\xe3"
		"\x9d\xd5\x01\xeb\x48\x01\x9f\xe5"
		"\x23\xdb\x01\xeb\x52\xb2\x01\xeb"
		,
		"?xxx??xx"
		"???xxxxx"
		"???x??xx"
		"???x???x"
		,
		32);
	size_t main_offset;
	if (!ptnMainStart.search(arm9, arm9_size, main_offset)) {
		fprintf(stderr, "Error: Unable to find main function\n");
		return false;
	}
	if (verbose) {
		printf("Address of main = 0x%08X\n", arm9_load_address + main_offset);
	}

	// BL              sub_2003138  ; never return
	// ; End of function main
	// 
	// -------------------------------------------------------------
	// BL              sub_206B784  ; NOP
	// 
	// BL              sub_206E44C  ; NOP
	// B               loc_2000D90  ; NOP
	// MOV             R0, #0xFFFFFFFF
	// BL              sub_2072528
	BytePattern ptnMainEnd(
		"\xea\x08\x00\xeb\x7c\xaa\x01\xeb"
		"\xad\xb5\x01\xeb\xfd\xff\xff\xea"
		,
		"???x???x"
		"???xxxxx"
		,
		16);
	size_t main_end_ptn_offset;
	if (!ptnMainEnd.search(arm9, main_offset + 0x200, main_end_ptn_offset, main_offset)) {
		fprintf(stderr, "Error: Unable to find the end of main function\n");
		return false;
	}
	size_t main_function_size = (main_end_ptn_offset + 4) - main_offset;
	if (verbose) {
		printf("Size of main = 0x%08X\n", main_function_size);
	}

	// MOV             R0, #0xFFFFFFFF
	// BL              sub_2072528
	BytePattern ptnMainBL_1(
		"\x00\x00\xe0\xe3\x31\xc6\x01\xeb"
		,
		"xxxx???x"
		,
		8);
	size_t main_bl_1_offset;
	if (!ptnMainBL_1.search(arm9, main_offset + main_function_size, main_bl_1_offset, main_offset)) {
		fprintf(stderr, "Error: Unable to find BL #1 in main function\n");
		return false;
	}
	if (verbose) {
		printf("Address of main BL #1 = 0x%08X\n", arm9_load_address + main_bl_1_offset);
	}

	// MLA             R0, R2, R1, R0       ; NOP
	// BL              nullsub_2            ; NOP
	// BL              sub_20023B0
	// BL              nullsub_3            ; NOP
	// BL              nullsub_4            ; NOP
	// BL              nullsub_5            ; NOP
	// BL              sub_2002AC4          ; NOP
	// BL              sub_2002D1C          ; NOP
	// BL              sub_200302C
	// BL              sub_200B0B4          ; NOP
	// BL              sub_2002DC8
	// LDR             R0, =unk_208A4B4
	BytePattern ptnMainBL_2(
		"\x92\x01\x20\xe0\xf4\x04\x00\xeb"
		"\x95\x05\x00\xeb\x88\x06\x00\xeb"
		"\xa0\x06\x00\xeb\xf2\x06\x00\xeb"
		"\x56\x07\x00\xeb\xeb\x07\x00\xeb"
		"\xae\x08\x00\xeb\xcf\x28\x00\xeb"
		"\x13\x08\x00\xeb\x24\x00\x9f\xe5"
		,
		"xxxx???x"
		"???x???x"
		"???x???x"
		"???x???x"
		"???x???x"
		"???x??xx"
		,
		48);
	size_t main_bl_2_ptn_offset;
	if (!ptnMainBL_2.search(arm9, main_offset + main_function_size, main_bl_2_ptn_offset, main_offset)) {
		fprintf(stderr, "Error: Unable to find BL #2 in main function\n");
		return false;
	}
	size_t main_bl_2_offset = main_bl_2_ptn_offset + 8;
	size_t main_bl_3_offset = main_bl_2_ptn_offset + 32;
	size_t main_bl_4_offset = main_bl_2_ptn_offset + 40;
	if (verbose) {
		printf("Address of main BL #2 = 0x%08X\n", arm9_load_address + main_bl_2_offset);
		printf("Address of main BL #3 = 0x%08X\n", arm9_load_address + main_bl_3_offset);
		printf("Address of main BL #4 = 0x%08X\n", arm9_load_address + main_bl_4_offset);
	}

	// ; get the destination address
	// BL              sub_2003138  ; never return
	// ; End of function main
	uint32_t main_last_bl_op = mget4l(&arm9[main_offset + main_function_size - 4]);
	uint32_t submain_offset = (main_offset + main_function_size + 4) + ((main_last_bl_op & 0xffffff) * 4);
	if (verbose) {
		printf("Address of sub_main = 0x%08X\n", arm9_load_address + submain_offset);
	}

	// MOV             R1, #0
	// STR             R1, [SP,#0x30+var_30]
	// LDR             R0, =unk_2292800
	// MOV             R2, #0x64 ; 'd'
	// LDR             R3, =unk_200356C
	// BL              sub_206DDB0
	// MOV             R0, #1              ; 2SF driver install address
	// LDR             R1, =unk_200351C
	BytePattern ptnSubMainEnd(
		"\x00\x10\xa0\xe3\x00\x10\x8d\xe5"
		"\x58\x02\x9f\xe5\x64\x20\xa0\xe3"
		"\x54\x32\x9f\xe5\xff\xaa\x01\xeb"
		"\x01\x00\xa0\xe3\x4c\x12\x9f\xe5"
		,
		"xxxx??xx"
		"??xxxxxx"
		"??xx???x"
		"xxxx??xx"
		,
		32);
	size_t submain_end_ptn_offset;
	if (!ptnSubMainEnd.search(arm9, submain_offset + 0x200, submain_end_ptn_offset, submain_offset)) {
		fprintf(stderr, "Error: Unable to find the end of main function\n");
		return false;
	}
	size_t submain_function_size = (submain_end_ptn_offset + 24) - submain_offset;
	size_t submain_bl_5_offset = submain_end_ptn_offset;

	// sub_main:
	// STMFD           SP!, {R4-R11,LR}
	// SUB             SP, SP, #0xC
	// LDR             R0, =unk_20A61C0             ; NOP
	// BL              sub_20027E8                  ; NOP
	// LDR             R0, =unk_20A61E8             ; NOP
	// BL              sub_20027E8                  ; NOP
	// LDR             R0, =unk_20A6210
	// BL              sub_20027E8
	// LDR             R0, =unk_20A619C             ; NOP
	// BL              sub_2002AD8                  ; NOP
	// BL              sub_2005E4C
	// BL              sub_201234C
	// LDR             R2, =unk_2292828
	// LDR             R1, =unk_20A613C
	// LDR             R0, =unk_20A6238
	// STR             R2, [R1,#(dword_20A6178 - 0x20A613C)]
	// LDR             R1, =off_20A6174
	// MOV             R2, #0
	// BL              sub_200264C
	BytePattern ptnSubMainStart(
		"\xf0\x4f\x2d\xe9\x0c\xd0\x4d\xe2"
		"\x98\x02\x9f\xe5\xa7\xfd\xff\xeb"
		"\x94\x02\x9f\xe5\xa5\xfd\xff\xeb"
		"\x90\x02\x9f\xe5\xa3\xfd\xff\xeb"
		"\x8c\x02\x9f\xe5\x5d\xfe\xff\xeb"
		"\x39\x0b\x00\xeb\x78\x3c\x00\xeb"
		"\x80\x22\x9f\xe5\x80\x12\x9f\xe5"
		"\x80\x02\x9f\xe5\x3c\x20\x81\xe5"
		"\x7c\x12\x9f\xe5\x00\x20\xa0\xe3"
		"\x31\xfd\xff\xeb"
		,
		"?xxx??xx"
		"??xx???x"
		"??xx???x"
		"??xx???x"
		"??xx???x"
		"???x???x"
		"??xx??xx"
		"??xx??xx"
		"??xxxxxx"
		"???x"
		,
		76);
	if (!ptnSubMainStart.match(&arm9[submain_offset], arm9_size - submain_offset)) {
		fprintf(stderr, "Error: Unable to find BLs in sub_main function\n");
		return false;
	}
	size_t submain_bl_1_offset = submain_offset + 24;
	size_t submain_bl_2_offset = submain_offset + 40;
	size_t submain_bl_3_offset = submain_offset + 44;
	size_t submain_bl_4_offset = submain_offset + 48;
	if (verbose) {
		printf("Address of sub_main BL #1 = 0x%08X\n", arm9_load_address + submain_bl_1_offset);
		printf("Address of sub_main BL #2 = 0x%08X\n", arm9_load_address + submain_bl_2_offset);
		printf("Address of sub_main BL #3 = 0x%08X\n", arm9_load_address + submain_bl_3_offset);
		printf("Address of sub_main BL #4 = 0x%08X\n", arm9_load_address + submain_bl_4_offset);
		printf("Address of sub_main BL #5 = 0x%08X\n", arm9_load_address + submain_bl_5_offset);

		printf("\n");
	}

	driver_offset = submain_offset + submain_function_size;
	if (verbose) {
		printf("Address of 2SF driver = 0x%08X\n", arm9_load_address + driver_offset);
	}

	// bgm_play:
	// STMFD           SP!, {R3-R6,LR}
	// SUB             SP, SP, #0x84       ; size depends on engine version
	// MOV             R6, R0
	// LDR             R0, =unk_20A7D70
	// MOV             R5, R1
	// MOV             R4, R2
	// BL              sub_2002B4C
	// LDR             R1, =aBgmPlay3d3d3d ; "bgm play %3d %3d %3d"
	// MOV             R2, R6
	// MOV             R3, R4
	// MOV             R0, #7
	// STR             R5, [SP,#0x98+var_98]
	BytePattern ptnBGMPlayStart(
		"\x78\x40\x2d\xe9\x84\xd0\x4d\xe2"
		"\x00\x60\xa0\xe1\xec\x01\x9f\xe5"
		"\x01\x50\xa0\xe1\x02\x40\xa0\xe1"
		"\xdb\x91\xfe\xeb\xe0\x11\x9f\xe5"
		"\x06\x20\xa0\xe1\x04\x30\xa0\xe1"
		"\x07\x00\xa0\xe3\x00\x50\x8d\xe5"
		,
		"?xxx??xx"
		"???x??xx"
		"???x???x"
		"???x??xx"
		"???x???x"
		"???x???x"
		,
		32);
	size_t bgm_play_offset;
	if (!ptnBGMPlayStart.search(arm9, arm9_size, bgm_play_offset)) {
		fprintf(stderr, "Error: Unable to find bgm_play function\n");
		return false;
	}
	if (verbose) {
		printf("Address of bgm_play = 0x%08X\n", arm9_load_address + bgm_play_offset);
	}
	this->bgm_play_function_offset = bgm_play_offset;

	nopRegions.clear();
	// main
	nopRegions.push_back(Region(main_offset + 12, main_bl_1_offset - (main_offset + 12)));
	nopRegions.push_back(Region(main_bl_1_offset + 8, main_bl_2_offset - (main_bl_1_offset + 8)));
	nopRegions.push_back(Region(main_bl_2_offset + 4, main_bl_3_offset - (main_bl_2_offset + 4)));
	nopRegions.push_back(Region(main_bl_3_offset + 4, main_bl_4_offset - (main_bl_3_offset + 4)));
	nopRegions.push_back(Region(main_bl_4_offset + 4, (main_offset + main_function_size - 4) - (main_bl_4_offset + 4)));
	nopRegions.push_back(Region(main_offset + main_function_size, 12));
	// sub_main
	nopRegions.push_back(Region(submain_offset + 8, submain_bl_1_offset - (submain_offset + 8)));
	nopRegions.push_back(Region(submain_bl_1_offset + 8, submain_bl_2_offset - (submain_bl_1_offset + 8)));
	nopRegions.push_back(Region(submain_bl_4_offset + 28, submain_bl_5_offset - (submain_bl_4_offset + 28)));

	if (verbose) {
		for (size_t i = 0; i < nopRegions.size(); i++) {
			Region & region = nopRegions[i];
			printf("NOP Region #%d = [ address: 0x%08X, size: 0x%X ]\n", i + 1, arm9_load_address + region.offset, region.size);
		}
		printf("\n");
	}

	return true;
}

bool procyon_ripper::save_2sfs(const std::string & basename)
{
	if (exe == NULL || driver_offset == 0 || bgm_play_function_offset == 0) {
		return false;
	}

	// NOP mass attack
	for (size_t i = 0; i < nopRegions.size(); i++) {
		Region & region = nopRegions[i];
		for (uint32_t offset = region.offset; offset < region.offset + region.size; offset += 4) {
			mput4l(0xE1A00000, &arm9[offset]);
		}
	}

	// install driver code:
	// LDR             R0, =0x1
	// LDR             R1, =0x0
	// LDR             R2, =0x100
	// BL              bgm_play
	// 
	// infinite_loop:
	// SWI             #0x40000 ; IntrWait
	// B               infinite_loop
	int32_t bl_bgm_play_rel = (bgm_play_function_offset / 4) - ((driver_offset + 12 + 8) / 4);
	uint32_t bl_bgm_play_op = 0xEB000000 | (bl_bgm_play_rel & 0xffffff);
	mput4l(0xE59F0010, &arm9[driver_offset]);
	mput4l(0xE59F1010, &arm9[driver_offset + 4]);
	mput4l(0xE59F2010, &arm9[driver_offset + 8]);
	mput4l(bl_bgm_play_op, &arm9[driver_offset + 12]);
	mput4l(0xEF040000, &arm9[driver_offset + 16]);
	mput4l(0xEAFFFFFD, &arm9[driver_offset + 20]);
	mput4l(0x00000001, &arm9[driver_offset + 24]);
	mput4l(0x00000000, &arm9[driver_offset + 28]);
	mput4l(0x00000100, &arm9[driver_offset + 32]);

	// export 2sflib
	std::string nds2sflib_path = basename + ".2sflib";
	if (verbose) {
		printf("Output \"%s\"\n", nds2sflib_path.c_str());
	}
	if (!NDS2SF::exe2sf(nds2sflib_path, exe, NDS2SF_EXE_HEADER_SIZE + rom_size)) {
		fprintf(stderr, "Error: Unable to save 2sflib file.\n");
		return false;
	}

	// get 2sflib filename for mini2sf tagging
	char nds2sflib_filename[PATH_MAX];
	strcpy(nds2sflib_filename, nds2sflib_path.c_str());
	path_basename(nds2sflib_filename);

	std::map<std::string, std::string> tags;
	tags["_lib"] = nds2sflib_filename;

	int mini2sf_count = 256;
	uint32_t mini2sf_offset = arm9_rom_offset + driver_offset + 24;
	for (int num = 0; num < mini2sf_count; num++) {
		char mini2sf_path[PATH_MAX];
		sprintf(mini2sf_path, "%s-%04d.mini2sf", basename.c_str(), num);
		
		if (verbose) {
			printf("Output \"%s\"\n", mini2sf_path);
		}
		NDS2SF::make_mini2sf(mini2sf_path, mini2sf_offset, 1, num, tags);
	}

	return true;
}

void printUsage(const char *cmd)
{
	const char *availableOptions[] = {
		"--help", "Show this help",
	};

	printf("%s %s\n", APP_NAME, APP_VER);
	printf("======================\n");
	printf("\n");
	printf("%s. Created by %s.\n", APP_DESC, APP_AUTHOR);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s <NDS File>`\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");

	for (int i = 0; i < sizeof(availableOptions) / sizeof(availableOptions[0]); i += 2)
	{
		printf("%s\n", availableOptions[i]);
		printf("  : %s\n", availableOptions[i + 1]);
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	procyon_ripper ripper;

	int argnum;
	int argi;

	argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(argv[0]);
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "-v") == 0 || strcmp(argv[argi], "--verbose") == 0)
		{
			ripper.verbose = true;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	argnum = argc - argi;
	if (argnum != 1)
	{
		fprintf(stderr, "Error: Too few/more arguments.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Run \"%s --help\" for help.\n", argv[0]);
		return EXIT_FAILURE;
	}

	char nds_path[PATH_MAX];
	strcpy(nds_path, argv[argi]);

	char nds_basename[PATH_MAX];
	strcpy(nds_basename, nds_path);
	path_stripext(nds_basename);

	if (!ripper.load_rom(nds_path)) {
		fprintf(stderr, "Error: Unsupported ROM format.\n");
		return EXIT_FAILURE;
	}

	if (!ripper.scan()) {
		fprintf(stderr, "Error: Code detection failed. (unsupported engine?)\n");
		return EXIT_FAILURE;
	}

	if (!ripper.save_2sfs(nds_basename)) {
		fprintf(stderr, "Error: Unable to save 2sf files.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
