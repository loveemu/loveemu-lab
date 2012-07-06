// original filename: ndstool/source/ndsextract.cpp
// modified by loveemu for Susie plug-in use

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../cioutil.h"
#include "spi00am.h"
#include "nitrofs.h"

// important members in nds header
#define NDSHD_OFFSET_TITLE      0x00
#define NDSHD_OFFSET_GAMECODE   0x0c
#define NDSHD_OFFSET_MAKERCODE  0x10
#define NDSHD_OFFSET_UNITCODE   0x12
#define NDSHD_OFFSET_DEVICETYPE 0x13
#define NDSHD_OFFSET_DEVICECAP  0x14
#define NDSHD_OFFSET_FNT_OFFSET 0x40
#define NDSHD_OFFSET_FNT_SIZE   0x44
#define NDSHD_OFFSET_FAT_OFFSET 0x48
#define NDSHD_OFFSET_FAT_SIZE   0x4c

#ifndef NO_FILE_MAPPING

/** check file info and pass it to callback function. */
int ndsInformFile(uint8 *nds, const char *prefix, const char *entry_name, uint16 file_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	// read header
	uint8 devicecap = nds[NDSHD_OFFSET_DEVICECAP]; // device size. (1<<n Mbit)
	uint32 fat_offset = mget4l(&nds[NDSHD_OFFSET_FAT_OFFSET]);

	// read FAT data
	uint32 finfo_ofs = fat_offset + 8*file_id;
	uint32 top = mget4l(&nds[finfo_ofs]);
	uint32 bottom = mget4l(&nds[finfo_ofs+4]);
	uint32 size = bottom - top;
	if (size > (1U << (17 + devicecap))) {
		//fprintf(stderr, "File %u: Size is too big. FAT offset 0x%x contains invalid data.\n", file_id, fat_offset + 8*file_id);
		return SPI_OUT_OF_ORDER;
	}

	// extract file
	ndsEnumFilesCallback(nds, prefix, entry_name, top, size, userData);

	return SPI_ALL_RIGHT;
}

/** enumerate all files stored in a directory */
int ndsEnumFilesDir(uint8 *nds, char *prefix, uint16 dir_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	int res = SPI_ALL_RIGHT, res2 = SPI_ALL_RIGHT;
	char strbuf[NITROFS_MAXPATHLEN];

	// read header
	uint32 fnt_offset = mget4l(&nds[NDSHD_OFFSET_FNT_OFFSET]);

	uint32 finfo_ofs = fnt_offset + 8*(dir_id & 0xfff);
	uint32 entry_start = mget4l(&nds[finfo_ofs]); // reference location of entry name
	uint16 top_file_id = mget2l(&nds[finfo_ofs+4]); // file ID of top entry 
	uint16 parent_id = mget2l(&nds[finfo_ofs+6]);   // ID of parent directory or directory count (root)

	finfo_ofs = fnt_offset + entry_start;

	for (uint16 file_id=top_file_id; ; file_id++) {
		uint8 entry_type_name_length = nds[finfo_ofs];
		uint8 name_length = entry_type_name_length & 127;
		bool entry_type_directory = (entry_type_name_length & 128) ? true : false;
		if (name_length == 0) break;

		char entry_name[128];
		memcpy(entry_name, &nds[finfo_ofs+1], name_length); entry_name[name_length] = '\0';
		finfo_ofs += 1 + name_length;
		if (entry_type_directory) {
			uint16 dir_id = mget2l(&nds[finfo_ofs]);
			finfo_ofs += 2;

			strcpy(strbuf, prefix);
			strcat(strbuf, entry_name);
			strcat(strbuf, "/");
			res2 = ndsEnumFilesDir(nds, strbuf, dir_id, ndsEnumFilesCallback, userData);
		}
		else
			res2 = ndsInformFile(nds, prefix, entry_name, file_id, ndsEnumFilesCallback, userData);

		if (res == SPI_ALL_RIGHT && res2 != SPI_ALL_RIGHT)
			res = res2;
	}

	return res;
}

/** count the number of files stored in a directory */
static int ndsEnumFileCountSub(uint8 *nds, uint16 dir_id, int *fileCount)
{
	int res = SPI_ALL_RIGHT, res2 = SPI_ALL_RIGHT;

	assert(fileCount);

	// read header
	uint32 fnt_offset = mget4l(&nds[NDSHD_OFFSET_FNT_OFFSET]);

	uint32 finfo_ofs = fnt_offset + 8*(dir_id & 0xfff);
	uint32 entry_start = mget4l(&nds[finfo_ofs]);   // reference location of entry name
	uint16 top_file_id = mget2l(&nds[finfo_ofs+4]); // file ID of top entry 
	uint16 parent_id = mget2l(&nds[finfo_ofs+6]);   // ID of parent directory or directory count (root)

	finfo_ofs = fnt_offset + entry_start;

	for (uint16 file_id=top_file_id; ; file_id++) {
		uint8 entry_type_name_length = nds[finfo_ofs];
		uint8 name_length = entry_type_name_length & 127;
		bool entry_type_directory = (entry_type_name_length & 128) ? true : false;
		if (name_length == 0) break;

		finfo_ofs += (1 + name_length);
		if (entry_type_directory) {
			uint16 dir_id = mget2l(&nds[finfo_ofs]);
			finfo_ofs += 2;
			res2 = ndsEnumFileCountSub(nds, dir_id, fileCount);
		}
		else {
			(*fileCount)++;
		}

		if (res == SPI_ALL_RIGHT && res2 != SPI_ALL_RIGHT)
			res = res2;
	}

	return res;
}

/** get file count stored in NDS ROM (read from FILE*) */
int ndsEnumFileCountFromBuf(uint8 *nds, int *errCode)
{
	int res, fileCount;

	fileCount = 0;
	res = ndsEnumFileCountSub(nds, 0xf000, &fileCount);
	if (errCode)
		*errCode = res;

	return fileCount;
}

/** enumerate all files stored in NDS ROM (read from FILE*) */
int ndsEnumFilesFromBuf(uint8 *nds, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	return ndsEnumFilesDir(nds, "", 0xf000, ndsEnumFilesCallback, userData);
}

#else

/** check file info and pass it to callback function. */
int ndsInformFile(FILE *fNDS, const char *prefix, const char *entry_name, uint16 file_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	long save_filepos = ftell(fNDS);

	// read header
	fseek(fNDS, NDSHD_OFFSET_DEVICECAP, SEEK_SET);
	uint8 devicecap = fgetc(fNDS); // device size. (1<<n Mbit)
	fseek(fNDS, NDSHD_OFFSET_FAT_OFFSET, SEEK_SET);
	uint32 fat_offset = fget4l(fNDS);

	// read FAT data
	fseek(fNDS, fat_offset + 8*file_id, SEEK_SET);
	uint32 top;
	fread(&top, 1, sizeof(top), fNDS);
	uint32 bottom;
	fread(&bottom, 1, sizeof(bottom), fNDS);
	uint32 size = bottom - top;
	if (size > (1U << (17 + devicecap))) {
		//fprintf(stderr, "File %u: Size is too big. FAT offset 0x%x contains invalid data.\n", file_id, fat_offset + 8*file_id);
		return SPI_OUT_OF_ORDER;
	}

	// extract file
	ndsEnumFilesCallback(fNDS, prefix, entry_name, top, size, userData);

	fseek(fNDS, save_filepos, SEEK_SET);
	return SPI_ALL_RIGHT;
}

/** enumerate all files stored in a directory */
int ndsEnumFilesDir(FILE *fNDS, char *prefix, uint16 dir_id, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	int res = SPI_ALL_RIGHT, res2 = SPI_ALL_RIGHT;
	char strbuf[NITROFS_MAXPATHLEN];
	long save_filepos = ftell(fNDS);

	// read header
	fseek(fNDS, NDSHD_OFFSET_FNT_OFFSET, SEEK_SET);
	uint32 fnt_offset = fget4l(fNDS);

	fseek(fNDS, fnt_offset + 8*(dir_id & 0xfff), SEEK_SET);
	uint32 entry_start; // reference location of entry name
	fread(&entry_start, 1, sizeof(entry_start), fNDS);
	uint16 top_file_id; // file ID of top entry 
	fread(&top_file_id, 1, sizeof(top_file_id), fNDS);
	uint16 parent_id;   // ID of parent directory or directory count (root)
	fread(&parent_id, 1, sizeof(parent_id), fNDS);

	fseek(fNDS, fnt_offset + entry_start, SEEK_SET);

	for (uint16 file_id=top_file_id; ; file_id++) {
		uint8 entry_type_name_length;
		fread(&entry_type_name_length, 1, sizeof(entry_type_name_length), fNDS);
		uint8 name_length = entry_type_name_length & 127;
		bool entry_type_directory = (entry_type_name_length & 128) ? true : false;
		if (name_length == 0) break;

		char entry_name[128];
		fread(entry_name, 1, name_length, fNDS); entry_name[name_length] = '\0';
		if (entry_type_directory) {
			uint16 dir_id;
			fread(&dir_id, 1, sizeof(dir_id), fNDS);

			strcpy(strbuf, prefix);
			strcat(strbuf, entry_name);
			strcat(strbuf, "/");
			res2 = ndsEnumFilesDir(fNDS, strbuf, dir_id, ndsEnumFilesCallback, userData);
		}
		else
			res2 = ndsInformFile(fNDS, prefix, entry_name, file_id, ndsEnumFilesCallback, userData);

		if (res == SPI_ALL_RIGHT && res2 != SPI_ALL_RIGHT)
			res = res2;
	}

	fseek(fNDS, save_filepos, SEEK_SET);
	return res;
}

/** count the number of files stored in a directory */
static int ndsEnumFileCountSub(FILE *fNDS, uint16 dir_id, int *fileCount)
{
	int res = SPI_ALL_RIGHT, res2 = SPI_ALL_RIGHT;
	long save_filepos = ftell(fNDS);

	assert(fileCount);

	// read header
	fseek(fNDS, NDSHD_OFFSET_FNT_OFFSET, SEEK_SET);
	uint32 fnt_offset = fget4l(fNDS);

	fseek(fNDS, fnt_offset + 8*(dir_id & 0xfff), SEEK_SET);
	uint32 entry_start; // reference location of entry name
	fread(&entry_start, 1, sizeof(entry_start), fNDS);
	uint16 top_file_id; // file ID of top entry 
	fread(&top_file_id, 1, sizeof(top_file_id), fNDS);
	uint16 parent_id;   // ID of parent directory or directory count (root)
	fread(&parent_id, 1, sizeof(parent_id), fNDS);

	fseek(fNDS, fnt_offset + entry_start, SEEK_SET);

	for (uint16 file_id=top_file_id; ; file_id++) {
		uint8 entry_type_name_length;
		fread(&entry_type_name_length, 1, sizeof(entry_type_name_length), fNDS);
		uint8 name_length = entry_type_name_length & 127;
		bool entry_type_directory = (entry_type_name_length & 128) ? true : false;
		if (name_length == 0) break;

		fseek(fNDS, name_length, SEEK_CUR);
		if (entry_type_directory) {
			uint16 dir_id;
			fread(&dir_id, 1, sizeof(dir_id), fNDS);
			res2 = ndsEnumFileCountSub(fNDS, dir_id, fileCount);
		}
		else {
			(*fileCount)++;
		}

		if (res == SPI_ALL_RIGHT && res2 != SPI_ALL_RIGHT)
			res = res2;
	}

	fseek(fNDS, save_filepos, SEEK_SET);
	return res;
}

/** get file count stored in NDS ROM (read from FILE*) */
int ndsEnumFileCountFromPointer(FILE *fNDS, int *errCode)
{
	int res, fileCount;

	fileCount = 0;
	res = ndsEnumFileCountSub(fNDS, 0xf000, &fileCount);
	if (errCode)
		*errCode = res;

	return fileCount;
}

/** get file count stored in NDS ROM */
int ndsEnumFileCount(const char *ndsfilename, int *errCode)
{
	int res;
	FILE *fNDS;

	fNDS = fopen(ndsfilename, "rb");
	if (!fNDS) {
		if (errCode)
			*errCode = SPI_FILE_READ_ERROR;
		return 0;
	}

	res = ndsEnumFileCountFromPointer(fNDS, errCode);

	fclose(fNDS);
	return res;
}

/** enumerate all files stored in NDS ROM (read from FILE*) */
int ndsEnumFilesFromPointer(FILE *fNDS, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	return ndsEnumFilesDir(fNDS, "", 0xf000, ndsEnumFilesCallback, userData);
}

/** enumerate all files stored in NDS ROM */
int ndsEnumFiles(const char *ndsfilename, NdsEnumFilesCallback ndsEnumFilesCallback, void *userData)
{
	int res;
	FILE *fNDS;

	fNDS = fopen(ndsfilename, "rb");
	if (!fNDS)
		return SPI_FILE_READ_ERROR;

	res = ndsEnumFilesFromPointer(fNDS, ndsEnumFilesCallback, userData);

	fclose(fNDS);
	return res;
}

#endif // NO_FILE_MAPPING
