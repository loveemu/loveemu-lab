// Original source template was written by shimitei
// http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
// 
// Normally, you don't need to look at this file.

#include <windows.h>
#include "spi00in.h"
#include "ifnscr.h"

/*-------------------------------------------------------------------------*/
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return SpiEntryPoint(hModule, ul_reason_for_call, lpReserved);
}

/***************************************************************************
 * Susie plugin functions
 ***************************************************************************/
/*-------------------------------------------------------------------------*/
int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	if (infono < 0 || infono >= (sizeof(pluginfo) / sizeof(char *))) 
		return 0;

	lstrcpyn(buf, pluginfo[infono], buflen);

	return (int) lstrlen(buf);
}
/*-------------------------------------------------------------------------*/
int __stdcall IsSupported(LPSTR filename, DWORD dw)
{
	char *data;
	char buff[HEADBUF_SIZE];
	DWORD ReadBytes;

	if ((dw & 0xFFFF0000) == 0) {
		if (!ReadFile((HANDLE)dw, buff, HEADBUF_SIZE, &ReadBytes, NULL)) {
			return 0;
		}
		data = buff;
	} else {
		data = (char *)dw;
	}

	if (IsSupportedEx(filename, data)) return 1;

	return 0;
}
/*-------------------------------------------------------------------------*/
int __stdcall GetPictureInfo
		(LPSTR buf, long len, unsigned int flag, struct PictureInfo *lpInfo)
{
	int ret = SPI_OTHER_ERROR;
	char headbuf[HEADBUF_SIZE];
	char *data;
	char *filename;
	HANDLE hf;
	DWORD ReadBytes;

	if ((flag & 7) == 0) {
		hf = CreateFile(buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE) return SPI_FILE_READ_ERROR;
		if (SetFilePointer(hf, len, NULL, FILE_BEGIN) != (DWORD)len) {
			CloseHandle(hf);
			return SPI_FILE_READ_ERROR;
		}
		if (!ReadFile(hf, headbuf, HEADBUF_SIZE, &ReadBytes, NULL)) {
			CloseHandle(hf);
			return SPI_FILE_READ_ERROR;
		}
		CloseHandle(hf);
		if (ReadBytes != HEADBUF_SIZE) return SPI_NOT_SUPPORT;
		data = headbuf;
		filename = buf;
	} else {
		if (len < HEADBUF_SIZE) return SPI_NOT_SUPPORT;
		data = (char *)buf;
		filename = NULL;
	}

	if (!IsSupportedEx(filename, data)) {
		ret = SPI_NOT_SUPPORT;
	} else {
		ret = GetPictureInfoEx(filename, data, lpInfo);
	}

	return ret;
}
/*-------------------------------------------------------------------------*/
int __stdcall GetPicture(
		LPSTR buf, long len, unsigned int flag, 
		HANDLE *pHBInfo, HANDLE *pHBm,
		SPI_PROGRESS lpPrgressCallback, long lData)
{
	int ret = SPI_OTHER_ERROR;
	char *data;
	char *filename;
	long datasize;
	HANDLE hf;
	DWORD ReadBytes;

	if ((flag & 7) == 0) {
		hf = CreateFile(buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE) return SPI_FILE_READ_ERROR;
		datasize = GetFileSize(hf, NULL) -len;
		if (datasize < HEADBUF_SIZE) {
			CloseHandle(hf);
			return SPI_NOT_SUPPORT;
		}
		if (SetFilePointer(hf, len, NULL, FILE_BEGIN) != (DWORD)len) {
			CloseHandle(hf);
			return SPI_FILE_READ_ERROR;
		}
		data = (char *)LocalAlloc(LMEM_FIXED, datasize);
		if (data == NULL) {
			CloseHandle(hf);
			return SPI_NO_MEMORY;
		}
		if (!ReadFile(hf, data, datasize, &ReadBytes, NULL)) {
			CloseHandle(hf);
			LocalFree(data);
			return SPI_FILE_READ_ERROR;
		}
		CloseHandle(hf);
		if (ReadBytes != (DWORD)datasize) {
			LocalFree(data);
			return SPI_FILE_READ_ERROR;
		}
		filename = buf;
	} else {
		data = buf;
		datasize = len;
		filename = NULL;
	}

	if (!IsSupportedEx(filename, data)) {
		ret = SPI_NOT_SUPPORT;
	} else {
		ret = GetPictureEx(filename, datasize, pHBInfo, pHBm, lpPrgressCallback, lData, data);
	}

	if ((flag & 7) == 0) LocalFree(data);

	return ret;
}
/*-------------------------------------------------------------------------*/
int __stdcall GetPreview(
	LPSTR buf, long len, unsigned int flag,
	HANDLE *pHBInfo, HANDLE *pHBm,
	SPI_PROGRESS lpPrgressCallback, long lData)
{
	return SPI_NO_FUNCTION;
}
/*-------------------------------------------------------------------------*/
