/**
 * cmain.c: a skeleton for cui application
 */


#include <stdio.h>
#include <string.h>
#include "cioutil.h"

#include <stdlib.h>
#include "nssseq.h"


#define CAPP_NAME   "sseq2mid"
#define CAPP_CMD    "sseq2mid"
#define CAPP_VER    "20070201"
#define CAPP_AUTHOR "loveemu"

bool g_log = false;
bool g_modifyChOrder = false;
bool g_noReverb = false;
int g_loopCount = 1;
int g_loopStyle = 0;

/* show application usage */
void cappShowUsage(void)
{
  const char* options[] = {
    "", "--help", "show this usage", 
    "-0", "--noreverb", "set 0 to reverb send", 
    "-1", "--1loop", "convert to 1 loop (no loop)", 
    "-2", "--2loop", "convert to 2 loop", 
    "-d", "--loopstyle1", "Duke nukem style loop points (Event 0x74/0x75)",
    "-7", "--loopstyle2", "FF7 PC style loop points (Meta text \"loop(start/end)\"",
    "-l", "--log", "put conversion log", 
    "-m", "--modify-ch", "modify midi channel to avoid rhythm channel"
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
  case '0':
    g_noReverb = true;
    break;

  case '1':
    g_loopCount = 1;
    break;

  case '2':
    g_loopCount = 2;
    break;

  case '7':
    g_loopStyle = 2;
    break;

  case 'd':
    g_loopStyle = 1;
    break;

  case 'l':
    g_log = true;
    break;

  case 'm':
    g_modifyChOrder = true;
    break;

  default:
    return false;
  }
  return true;
}

/* dispatch option string */
bool cappDispatchOptionStr(const char* optString)
{
  if(strcmp(optString, "help") == 0)
  {
    cappShowUsage();
  }
  else if(strcmp(optString, "log") == 0)
  {
    g_log = true;
  }
  else if(strcmp(optString, "modify-ch") == 0)
  {
    g_modifyChOrder = true;
  }
  else if(strcmp(optString, "reverb0") == 0)
  {
    g_noReverb = true;
  }
  else if(strcmp(optString, "1loop") == 0)
  {
    g_loopCount = 1;
  }
  else if(strcmp(optString, "2loop") == 0)
  {
    g_loopCount = 2;
  }
  else if(strcmp(optString, "loopstyle1") == 0)
  {
      g_loopStyle = 1;
  }
  else if(strcmp(optString, "loopstyle2") == 0)
  {
      g_loopStyle = 2;
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
  NSSseq* sseq;

  fprintf(stderr, "%s:\n", path);
  sseq = nsSseqReadFile(path);
  if(sseq)
  {
    char* outputPath;

    outputPath = (char*) malloc(strlen(path) + 5);
    if(outputPath)
    {
      strcpy(outputPath, path);
      removeExt(outputPath);
      strcat(outputPath, ".mid");

      nsSseqSetLoopCount(sseq, g_loopCount);
      nsSseqSetLoopStyle(sseq, g_loopStyle);
      nsSseqSetReverbUse(sseq, g_noReverb);
      nsSseqWriteToMidiFile(sseq, outputPath);
      fprintf(stderr, "conversion succeeded\n");
      free(outputPath);
    }
    nsSseqDelete(sseq);
  }
  else
  {
    fprintf(stderr, "error: nsSseqReadFile() failed\n");
  }
  return result;
}

/* dispatch log output */
void cappDispatchLogMsg(const char* logMsg)
{
  fprintf(stdout, logMsg);
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
