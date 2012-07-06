/**
 * cmain.c: a skeleton for cui application
 */


#include <stdio.h>
#include <string.h>
#include "cioutil.h"

#include <stdlib.h>
#include "nsswav.h"


#define CAPP_NAME   "swav2wav"
#define CAPP_CMD    "swav2wav"
#define CAPP_VER    "20090506"
#define CAPP_AUTHOR "loveemu"


/* show application usage */
void cappShowUsage(void)
{
  const char* options[] = {
    "", "--help", "show this usage", 
  };
  int optIndex;

  puts("usage  : "CAPP_CMD" (options) [input-files]");
  puts("options:");
  for(optIndex = 0; optIndex < countof(options); optIndex += 3)
  {
    printf("  %-2s  %-16s  %s\n", options[optIndex], options[optIndex + 1], options[optIndex + 2]);
  }
  puts("____");
  puts(CAPP_NAME" ["CAPP_VER"] by "CAPP_AUTHOR);
}

/* dispatch option char */
bool cappDispatchOptionChar(const char optChar)
{
  switch(optChar)
  {
  }
  return false;
}

/* dispatch option string */
bool cappDispatchOptionStr(const char* optString)
{
  if(strcmp(optString, "help") == 0)
  {
    cappShowUsage();
  }
  else
  {
    return false;
  }
  return true;
}

/* dispatch file path */
bool cappDispatchFilePath(const char* path)
{
  bool result = false;
  NSSwav* swav;

  fprintf(stderr, "%s:\n", path);
  swav = nsSwavReadFile(path);
  if(swav)
  {
    char* outputPath;

    outputPath = (char*) malloc(strlen(path) + 5);
    if(outputPath)
    {
      int loopStart = swav->hasLoop ? swav->loopStart : 0;

      strcpy(outputPath, path);
      removeExt(outputPath);
      strcat(outputPath, ".wav");

      if(loopStart != 0)
      {
        fprintf(stderr, "loop point #%d\n", loopStart);
      }
      nsSwavWriteToWaveFile(swav, outputPath);
      fprintf(stderr, "conversion succeeded\n");
      free(outputPath);
    }
    nsSwavDelete(swav);
  }
  else
  {
    fprintf(stderr, "error: nsSwavReadFile() failed\n");
  }
  return result;
}

/* swav2wav application main */
int main(int argc, char* argv[])
{
  int argi = 1;
  int argci;

  if(argc == 1) /* no arguments */
  {
    cappShowUsage();
  }
  else
  {
    /* options */
    while((argi < argc) && (argv[argi][0] == '-'))
    {
      if(argv[argi][1] == '-') /* --string */
      {
        cappDispatchOptionStr(&argv[argi][2]);
      }
      else /* -letters (alphanumeric only) */
      {
        argci = 1;
        while(argv[argi][argci] != '\0')
        {
          cappDispatchOptionChar(argv[argi][argci]);
          argci++;
        }
      }
      argi++;
    }

    /* input files */
    for(; argi < argc; argi++)
    {
      cappDispatchFilePath(argv[argi]);
    }
  }
  return 0;
}
