/**
 * boring utility routines for C.
 */

#ifndef CUTILS_H
#define CUTILS_H

#include "cioutils.h"

char* removeExt(char* path);
int indexOfHexPat (const byte *buf, const byte *pat, size_t bufSize, const byte *v);

#endif /* !CUTILS_H */
