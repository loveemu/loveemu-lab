/**
 * boring utility routines for C.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "cioutils.h"
#include "cutils.h"

#ifdef WIN32
#pragma comment(lib, "shlwapi.lib")
#include <shlwapi.h>
#endif

/** remove path extention (SUPPORTS ASCII ONLY!) */
char* removeExt(char* path)
{
#ifdef WIN32
  PathRemoveExtensionA(path);
  return path;
#else
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
#endif
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
