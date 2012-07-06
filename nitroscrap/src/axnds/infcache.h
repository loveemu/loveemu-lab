// Original source template was written by shimitei
// http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
// 
// Normally, you don't need to look at this file.

#ifndef infcache_h
#define infcache_h

#include <windows.h>
#include "critsect.h"
#include "spi00am.h"

typedef struct ArcInfo
{
  HLOCAL hinfo;
  char path[MAX_PATH];
} ArcInfo;

#define INFOCACHE_MAX 0x10
class InfoCache
{
public:
  InfoCache();
  ~InfoCache();
  void Clear(void);
  void Add(char *filepath, HLOCAL *ph);
  int Dupli(char *filepath, HLOCAL *ph, fileInfo *pinfo);
private:
  CriticalSection cs;
  ArcInfo arcinfo[INFOCACHE_MAX];
  int nowno;
  bool GetCache(char *filepath, HLOCAL *ph);
};

#endif
