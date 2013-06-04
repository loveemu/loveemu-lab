/**
 * simple i/o routines for C.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "cioutil.h"

/** remove path extention (SUPPORTS ASCII ONLY!) */
char* removeExt(char* path)
{
  size_t i;

  i = strlen(path);
  if(i > 1)
  {
    i--;
    for(; i > 0; i--)
    {
      char c = path[i];

      if(c == '.')
      {
        path[i] = '\0';
        break;
      }
      else if(c == '/' || c == '\\')
      {
        break;
      }
    }
  }
  return path;
}

/**
 * search hex sequence from buffer, then returns its position.
 * \x5c (\) is a escape sequence. use \x5c\x5c for byte \x5c.
 * \x2e (.) matches with all characters.
 * \xf0 - \xff are used for variables.
 * \x00 means the end of the pattern.
 */
int indexOfHexPat (const byte *buf, const byte *pat, size_t bufSize, const byte *v)
{
  size_t patLen;
  size_t index;
  size_t bufIndex;
  size_t patIndex;
  byte bufB, patB;
  bool matchFailed;
  byte var[16] = {
      0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
      0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
  };

  // variables, for flexible search
  if (v) {
    for (index = 0; index < 16; index++) {
      var[index] = v[index];
    }
  }

  // calc pattern size first
  patLen = 0;
  index = 0;
  while (pat[index] != '\0') {
    switch (pat[index]) {
    case 0x5c:
      index += 2;
      break;
    default:
      index++;
    }
    patLen++;
  }

  // failure: pattern is larger than buffer
  if (bufSize < patLen)
    return -1;

  // search the pattern from beginning
  for (index = 0; index <= bufSize - patLen; index++) {
    patIndex = 0;
    matchFailed = false;
    for (bufIndex = index; !matchFailed && bufIndex < index + patLen; bufIndex++) {
      bufB = buf[bufIndex];
      patB = pat[patIndex];
      switch (patB) {
      case 0x2e:
        break;
      case 0x5c:
      case 0xf0: case 0xf1: case 0xf2: case 0xf3:
      case 0xf4: case 0xf5: case 0xf6: case 0xf7:
      case 0xf8: case 0xf9: case 0xfa: case 0xfb:
      case 0xfc: case 0xfd: case 0xfe: case 0xff:
        if (patB == 0x5c) {
            patB = pat[++patIndex];
        }
        else {
            patB = var[patB - 0xf0];
        }
        // fall through
      default:
        if (bufB != patB)
          matchFailed = true;
      }
      patIndex++;
    }
    if (!matchFailed)
      return (int) index;
  }
  // not found
  return -1;
}

#define SBPRINTF_BLOCK_SIZE 1024

/** create easy logging object. */
StringStreamBuf *newStringStreamBuf (void)
{
  StringStreamBuf *newBuf = (StringStreamBuf*) calloc(1, sizeof(StringStreamBuf));

  if (newBuf) {
    char *buf = (char*) calloc(SBPRINTF_BLOCK_SIZE, sizeof(char));

    if (buf) {
      newBuf->s = buf;
      newBuf->size = SBPRINTF_BLOCK_SIZE;
    }
  }
  return newBuf;
}

/** delete easy logging object. */
void delStringStreamBuf (StringStreamBuf *buf)
{
  if (buf) {
    if (buf->s)
      free((void*) buf->s);
    free(buf);
  }
}

/** printf for easy logging object. */
int sbprintf (StringStreamBuf *buf, const char *format, ...)
{
  va_list va;
  int result;
  static char sBuf[SBPRINTF_BLOCK_SIZE];
  size_t sLen;

  if (!buf || !buf->s)
    return 0;

  va_start(va, format);
  result = vsprintf(sBuf, format, va);
  va_end(va);

  sLen = strlen(sBuf);
  if (buf->len + sLen >= buf->size) {
    size_t newSize = buf->size + SBPRINTF_BLOCK_SIZE;
    char *newStrBuf;

    while (buf->len + sLen >= newSize)
        newSize += SBPRINTF_BLOCK_SIZE;
    newStrBuf = (char*) realloc((void*)buf->s, newSize * sizeof(char));
    if (!newStrBuf)
      return 0;
    buf->s = newStrBuf;
    buf->size = newSize;
  };

  strcat((char*) buf->s, sBuf);
  buf->len += sLen;
  return result;
}

/** empty easy logging object. */
void sbclear (StringStreamBuf *buf)
{
  char *newStrBuf;

  if (!buf || !buf->s)
    return;

  ((char*) buf->s)[0] = '\0';
  buf->len = 0;
  newStrBuf = realloc((void*) buf->s, SBPRINTF_BLOCK_SIZE * sizeof(char));
  if (newStrBuf) {
    buf->s = newStrBuf;
    buf->size = SBPRINTF_BLOCK_SIZE;
  }
}
