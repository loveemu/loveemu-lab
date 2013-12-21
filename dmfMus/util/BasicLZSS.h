/**
 * Generic LZSS decompressor (made for SLPS-02993)
 */

#ifndef HOKUTO_LZSS_CORE
#define HOKUTO_LZSS_CORE

#include <stdio.h>

size_t decompressLZSS(const void *pIn, size_t inBuffSize, void *pOut, size_t outBuffSize, int lzssMatchBitCount, int lzssOffsetBitCount, int lzssLengthBitCount);

#endif /* !HOKUTO_LZSS_CORE */
