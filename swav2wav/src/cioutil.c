/*
 * ciotuil.c: simple i/o routines for C
 */


#include <stdio.h>
#include <string.h>
#include "cioutil.h"


/* remove path extention (SUPPORTS ASCII ONLY!) */
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
