/**
 * simple i/o routines for C.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "cioutil.h"

/** check if the file exist */
bool fexist(const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "rb");
	if (fp) {
		fclose(fp);
		return true;
	}
	else
		return false;
}

/** get path slash */
char* getPathSlash(char* path)
{
	char *slash = strrchr(path, '/');
	char *backslash = strrchr(path, '\\');
	if (!slash || (slash && backslash > slash))
		slash = backslash;
	if (slash)
		return slash;
	else
		return NULL;
}

/** get path extension */
char* getPathExt(char* path)
{
	char *slash = strrchr(path, '/');
	char *backslash = strrchr(path, '\\');
	char *dot = strrchr(path, '.');
	if (!slash || (slash && backslash > slash))
		slash = backslash;
	if (dot && (!slash || (slash && slash < dot)))
		return dot;
	else
		return NULL;
}

/** remove path extension (SUPPORTS ASCII ONLY!) */
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
