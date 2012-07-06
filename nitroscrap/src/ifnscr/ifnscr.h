/**
 * Susie plug-in: NSCR (NSCR+NCGR+NCLR) decoder
 * 
 * based on spi00in_ex.h by Shimitei
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 */

#ifndef ifnscr_h
#define ifnscr_h

#include <windows.h>
#include "spi00in.h"

#ifndef INLINE
#define INLINE __inline
#endif

typedef unsigned int uint;
typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;

INLINE int getPitchOfDIB(int width, int bpp) {
	return (((width*bpp+31)>>3)&~3);
}

// about this plugin
static const char *pluginfo[] = {
	"00IN",                                 // API version/type
	"NSCR decoder based on n2bmp",          // plugin name, copyright, or something else
	"*.nscr",                               // allowed extensions
	"Nintendo SCreen Resource (*.nscr)",    // file description
};

// required length for header check (<2kB)
#define HEADBUF_SIZE	0x24

BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
BOOL IsSupportedEx(char *filename, char *data);
int GetPictureInfoEx(char *filename, char *data, struct PictureInfo *lpInfo);
int GetPictureEx(char *filename, long datasize, HANDLE *pHBInfo, HANDLE *pHBm,
	SPI_PROGRESS lpPrgressCallback, long lData, char *data);

#endif
