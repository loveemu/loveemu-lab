// Original source template was written by shimitei
// http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
// 
// Normally, you don't need to look at this file.

#ifndef spi00in_h
#define spi00in_h

#include <windows.h>

/*-------------------------------------------------------------------------*/
/* Image info struct */
/*-------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
typedef struct PictureInfo
{
	long left,top;
	long width;
	long height;
	WORD x_density;
	WORD y_density;
	short colorDepth;
	HLOCAL hInfo;
} PictureInfo;
#pragma pack(pop)

/*-------------------------------------------------------------------------*/
/* Error codes */
/*-------------------------------------------------------------------------*/
#define SPI_NO_FUNCTION         -1
#define SPI_ALL_RIGHT           0
#define SPI_ABORT               1
#define SPI_NOT_SUPPORT         2
#define SPI_OUT_OF_ORDER        3
#define SPI_NO_MEMORY           4
#define SPI_MEMORY_ERROR        5
#define SPI_FILE_READ_ERROR     6
#define SPI_WINDOW_ERROR        7
#define SPI_OTHER_ERROR         8
#define SPI_FILE_WRITE_ERROR    9
#define SPI_END_OF_FILE         10

/*-------------------------------------------------------------------------*/
/* Prototypes */
/*-------------------------------------------------------------------------*/
typedef int (CALLBACK *SPI_PROGRESS)(int, int, long);
	int __declspec(dllexport) __stdcall GetPluginInfo
		(int infono, LPSTR buf, int buflen);
	int __declspec(dllexport) __stdcall IsSupported(LPSTR filename, DWORD dw);
	int __declspec(dllexport) __stdcall GetPictureInfo
		(LPSTR buf,long len, unsigned int flag, PictureInfo *lpInfo);
	int __declspec(dllexport) __stdcall GetPicture
		(LPSTR buf,long len, unsigned int flag,
		 HANDLE *pHBInfo, HANDLE *pHBm,
		 SPI_PROGRESS lpPrgressCallback, long lData);
	int __declspec(dllexport) __stdcall GetPreview
		(LPSTR buf,long len, unsigned int flag,
		 HANDLE *pHBInfo, HANDLE *pHBm,
		 SPI_PROGRESS lpPrgressCallback, long lData);

#endif
