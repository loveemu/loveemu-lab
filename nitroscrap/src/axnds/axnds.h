/**
 * Susie plug-in: NitroFS Reader
 * 
 * based on spi00am_ex.h by Shimitei
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 */

#ifndef axnds_h
#define axnds_h

#include <windows.h>
#include "spi00am.h"

// about the plugin
static const char *pluginfo[] = {
  "00AM",               // plug-in version/type
  "NitroFS Reader",     // plug-in description
  "*.nds",              // files supported
  "Nintendo DS ROM",    // file description
};

// required length for initial check (<2kB)
#define HEADBUF_SIZE    512

BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
BOOL IsSupportedEx(char *filename, char *data);
int GetArchiveInfoEx(LPSTR filename, long len, HLOCAL *lphInf);
int GetFileEx(char *filename, HLOCAL *dest, fileInfo *pinfo, SPI_PROGRESS lpPrgressCallback, long lData);

#endif
