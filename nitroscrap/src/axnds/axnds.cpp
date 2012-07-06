/**
 * Susie plug-in: NitroFS Reader
 * This plug-in really couldn't exist without ndstool.
 * I must thank DarkFader, the author of ndstool.
 * 
 * based on spi00am_ex.cpp by Shimitei
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 */

#include <windows.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include "../cioutil.h"
#include "spi00am.h"
#include "nitrofs.h"
#include "axnds.h"

#ifndef NO_FILE_MAPPING
bool spiStoreFileCount(uint8 *nds, const char *path, const char *filename, uint32 offset, uint32 size, void *userData);
bool spiStoreFileInfo(uint8 *nds, const char *path, const char *filename, uint32 offset, uint32 size, void *userData);
#else
bool spiStoreFileCount(FILE *fNDS, const char *path, const char *filename, uint32 offset, uint32 size, void *userData);
bool spiStoreFileInfo(FILE *fNDS, const char *path, const char *filename, uint32 offset, uint32 size, void *userData);
#endif

typedef struct {
	int count;
	fileInfo* info;
} SpiStoreFileInfoParam;

/** plug-in entrypoint */
BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/** check if the plug-in can handle the file */
BOOL IsSupportedEx(char *filename, char *data)
{
	bool res = false;
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return false;

	// game title/code should be printable
	for(int i = 0; i < 12+4; i++) {
		int c = fgetc(fp);
		if (c != 0 && !isprint(c))
			goto exitterm;
	}

	// ARM9 source must be aligned to 4KB boundary
	fseek(fp, 0x20, SEEK_SET);
	if (fget4l(fp) % 0x1000 != 0)
		goto exitterm;

	res = true;

exitterm:
	if (fp)
		fclose(fp);
	return res;
}

#ifndef NO_FILE_MAPPING

inline int GetArchiveInfoExWithFileMapping(LPSTR filename, long len, HLOCAL *lphInf)
{
	int res;
	int fileCount;
	fileInfo* pInfo;
	SpiStoreFileInfoParam infoParam;
	HANDLE fNDS = NULL;
	HANDLE hMap = NULL;
	uint8 *nds = NULL;

	*lphInf = NULL;

	fNDS = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fNDS == INVALID_HANDLE_VALUE) {
		return SPI_FILE_READ_ERROR;
	}
	hMap = CreateFileMapping(fNDS, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMap == INVALID_HANDLE_VALUE) {
		CloseHandle(fNDS);
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}
	nds = (uint8*) MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (!nds) {
		CloseHandle(hMap);
		CloseHandle(fNDS);
		return SPI_FILE_READ_ERROR;
	}

	fileCount = ndsEnumFileCountFromBuf(nds, &res);
	if (res != SPI_ALL_RIGHT)
		goto exitterm;

	pInfo = (fileInfo*) LocalAlloc(LPTR, sizeof(fileInfo) * (fileCount+1));
	if (!pInfo) {
		res = SPI_NO_MEMORY;
		goto exitterm;
	}

	infoParam.count = 0;
	infoParam.info = pInfo;
	res = ndsEnumFilesFromBuf(nds, spiStoreFileInfo, &infoParam);

	*lphInf = (HLOCAL) pInfo;

exitterm:
	if (nds)
		UnmapViewOfFile(nds);
	if (hMap)
		CloseHandle(hMap);
	if (fNDS)
		CloseHandle(fNDS);
	return res;
}

#else

inline int GetArchiveInfoExWithNoFileMapping(LPSTR filename, long len, HLOCAL *lphInf)
{
	int res;
	int fileCount;
	fileInfo* pInfo;
	SpiStoreFileInfoParam infoParam;
	FILE *fNDS = NULL;

	*lphInf = NULL;

	fNDS = fopen(filename, "rb");
	if (!fNDS) {
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}

	fileCount = ndsEnumFileCountFromPointer(fNDS, &res);
	//res = ndsEnumFilesFromPointer(fNDS, spiStoreFileCount, &fileCount);
	if (res != SPI_ALL_RIGHT)
		goto exitterm;

	pInfo = (fileInfo*) LocalAlloc(LPTR, sizeof(fileInfo) * (fileCount+1));
	if (!pInfo)
		goto exitterm;

	infoParam.count = 0;
	infoParam.info = pInfo;
	res = ndsEnumFilesFromPointer(fNDS, spiStoreFileInfo, &infoParam);

	*lphInf = (HLOCAL) pInfo;

exitterm:
	if (fNDS)
		fclose(fNDS);
	return res;
}

#endif

/** get the info of all files in the archive, then store them into lphInf. */
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf)
{
#ifndef NO_FILE_MAPPING
	return GetArchiveInfoExWithFileMapping(filename, len, lphInf);
#else
	return GetArchiveInfoExWithNoFileMapping(filename, len, lphInf);
#endif
}

#ifndef NO_FILE_MAPPING

inline int GetFileExWithFileMapping(char *filename, HLOCAL *dest, fileInfo *pinfo,
		SPI_PROGRESS lpPrgressCallback, long lData)
{
	int res;
	HANDLE fNDS = NULL;
	HANDLE hMap = NULL;
	uint8 *nds = NULL;

	*dest = NULL;

	fNDS = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fNDS == INVALID_HANDLE_VALUE) {
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}
	hMap = CreateFileMapping(fNDS, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMap == INVALID_HANDLE_VALUE) {
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}
	nds = (uint8*) MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (!nds) {
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}

	*dest = LocalAlloc(LMEM_FIXED, pinfo->filesize);
	if (*dest == NULL) {
		res = SPI_NO_MEMORY;
		goto exitterm;
	}

	memcpy(*dest, &nds[pinfo->position], pinfo->filesize);
	res = SPI_ALL_RIGHT;

exitterm:
	if (res != SPI_ALL_RIGHT && *dest) {
		LocalFree(*dest);
		*dest = NULL;
	}
	if (nds)
		UnmapViewOfFile(nds);
	if (hMap)
		CloseHandle(hMap);
	if (fNDS)
		CloseHandle(fNDS);
	return res;
}

#else

inline int GetFileExWithNoFileMapping(char *filename, HLOCAL *dest, fileInfo *pinfo,
		SPI_PROGRESS lpPrgressCallback, long lData)
{
	int res;
	FILE *fNDS = NULL;

	*dest = NULL;

	fNDS = fopen(filename, "rb");
	if (!fNDS) {
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}

	*dest = LocalAlloc(LMEM_FIXED, pinfo->filesize);
	if (*dest == NULL) {
		res = SPI_NO_MEMORY;
		goto exitterm;
	}

	fseek(fNDS, pinfo->position, SEEK_SET);
	if (fread(*dest, 1, pinfo->filesize, fNDS) != pinfo->filesize) {
		res = SPI_FILE_READ_ERROR;
		goto exitterm;
	}

	res = SPI_ALL_RIGHT;

exitterm:
	if (res != SPI_ALL_RIGHT && *dest) {
		LocalFree(*dest);
		*dest = NULL;
	}
	if (fNDS)
		fclose(fNDS);
	return res;
}

#endif

/** read the file which is pointed by fileInfo, from "filename". */
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo,
		SPI_PROGRESS lpPrgressCallback, long lData)
{
#ifndef NO_FILE_MAPPING
	return GetFileExWithFileMapping(filename, dest, pinfo, lpPrgressCallback, lData);
#else
	return GetFileExWithNoFileMapping(filename, dest, pinfo, lpPrgressCallback, lData);
#endif
}

#ifndef NO_FILE_MAPPING
bool spiStoreFileCount(uint8 *nds, const char *path, const char *filename, uint32 offset, uint32 size, void *userData)
#else
bool spiStoreFileCount(FILE *fNDS, const char *path, const char *filename, uint32 offset, uint32 size, void *userData)
#endif
{
	int* fileCount = (int*) userData;
	(*fileCount)++;
	return true;
}

#ifndef NO_FILE_MAPPING
bool spiStoreFileInfo(uint8 *nds, const char *path, const char *filename, uint32 offset, uint32 size, void *userData)
#else
bool spiStoreFileInfo(FILE *fNDS, const char *path, const char *filename, uint32 offset, uint32 size, void *userData)
#endif
{
	SpiStoreFileInfoParam* infoParam = (SpiStoreFileInfoParam*) userData;
	int count = infoParam->count;
	fileInfo* info = &infoParam->info[count];

	memset(info, 0, sizeof(fileInfo));
	strcpy((char *)info->method, "NitroFS"); // FIXME: proper content?
	info->position = offset;
	info->compsize = size;
	info->filesize = size;
	//info->timestamp = (long) 0;
	strncpy(info->path, path, sizeof(info->path)); info->path[sizeof(info->path)-1] = '\0';
	// replace slash by backslash
	char *slash = strchr(info->path, '/');
	while (slash) {
		*slash = '\\';
		slash = strchr(slash, '/');
	}
	strncpy(info->filename, filename, sizeof(info->filename));
	info->filename[sizeof(info->filename)-1] = '\0';
	//info->crc = 0;

	infoParam->count++;
	return true;
}
