// Original source template was written by shimitei
// http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
// 
// Normally, you don't need to look at this file.

#ifndef spi00am_h
#define spi00am_h

#include <windows.h>

/*-------------------------------------------------------------------------*/
// File info struct
/*-------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
typedef struct fileInfo
{
  unsigned char method[8];
  unsigned long position;
  unsigned long compsize;
  unsigned long filesize;
  long /*time_t*/ timestamp;
  char path[200];
  char filename[200];
  unsigned long crc;
} fileInfo;
#pragma pack(pop)

/*-------------------------------------------------------------------------*/
// Error codes
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
// Prototypes
/*-------------------------------------------------------------------------*/
//int PASCAL ProgressCallback(int nNum, int nDenom, long lData);
typedef int (CALLBACK *SPI_PROGRESS)(int, int, long);
extern "C"
{
  int __declspec(dllexport) __stdcall GetPluginInfo
      (int infono, LPSTR buf, int buflen);
  int __declspec(dllexport) __stdcall IsSupported(LPSTR filename, DWORD dw);
  int __declspec(dllexport) __stdcall GetArchiveInfo(
      LPSTR buf, long len, unsigned int flag, HLOCAL *lphInf);
  int __declspec(dllexport) __stdcall GetFileInfo(LPSTR buf,long len,
      LPSTR filename, unsigned int flag, fileInfo *lpInfo);
  int __declspec(dllexport) __stdcall GetFile(LPSTR src,long len,
         LPSTR dest, unsigned int flag,
         SPI_PROGRESS prgressCallback, long lData);
  int __declspec(dllexport) __stdcall GetPreview
      (LPSTR buf,long len, unsigned int flag,
       HANDLE *pHBInfo, HANDLE *pHBm,
       SPI_PROGRESS lpPrgressCallback, long lData);
}

#endif
