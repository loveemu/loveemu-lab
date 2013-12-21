/**
 * PS1 Hokuto no Ken - Seikimatsu Kyuuseishu Densetsu (J) (SLPS-02993)
 * LZSS decompressor for DMF sequence archive in BGMALL*.PAC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "BasicLZSS.h"

#define APP_NAME	"LZSS Decompressor (PS1 Hokuto no Ken)"
#define APP_VER		"[2013-12-21]"

// Command path (set by main)
char *glCommandPath = NULL;
// Input filename
char glInFilename[512] = { '\0' };
// Output filename
char glOutFilename[512] = { '\0' };

/**
 * Show usage of the application.
 */
void printUsage(void)
{
	printf("%s %s\n", APP_NAME, APP_VER);
	printf("\n");
	printf("Syntax:\n");
	printf("  %s (options) <input file>\n", glCommandPath);
	printf("\n");
	printf("Options:\n");
	printf("  --help            show this help\n");
	printf("  -o <filename>     specify output filename\n");
	printf("  -z <size>         LZSS dictionary size (usually 11, 15 at maximum)\n");
	printf("  --offset <n>      skip first <n> input bytes\n");
	printf("  --max <n>         maximum output size (memory buffer size)\n");
	printf("\n");
}

/**
 * Program main.
 */
int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	// user options
	int lzssMatchBitCount = 16;
	int lzssOffsetBitCount = 11;
	int lzssLengthBitCount = 5;
	int lzssStartOffset = 0;
	int lzssMaxRawSize = 0x200000;

	// closable objects
	FILE *inFile = NULL;
	FILE *outFile = NULL;
	uint8_t *inBytes = NULL;
	uint8_t *outBytes = NULL;

	// set command path
	glCommandPath = argv[0];

	// parse options
	int argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage();
			goto finish;
		}
		else if (strcmp(argv[argi], "-o") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				goto finish;
			}
			strcpy(glOutFilename, argv[argi + 1]);
			argi++;
		}
		else if (strcmp(argv[argi], "-z") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				goto finish;
			}
			lzssOffsetBitCount = strtol(argv[argi + 1], NULL, 10);
			if (lzssOffsetBitCount < 1 || lzssOffsetBitCount > 15)
			{
				fprintf(stderr, "Error: Invalid dictionary size \"%s\"\n", argv[argi + 1]);
				goto finish;
			}
			lzssLengthBitCount = 16 - lzssOffsetBitCount;
			argi++;
		}
		else if (strcmp(argv[argi], "--offset") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				goto finish;
			}
			lzssStartOffset = (int) strtol(argv[argi + 1], NULL, 0);
			argi++;
		}
		else if (strcmp(argv[argi], "--max") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				goto finish;
			}
			lzssMaxRawSize = strtol(argv[argi + 1], NULL, 0);
			if (lzssMaxRawSize < 1)
			{
				fprintf(stderr, "Error: Illegal memory size \"%s\"\n", argv[argi + 1]);
				goto finish;
			}
			argi++;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			goto finish;
		}
		argi++;
	}
	argc -= argi;
	argv += argi;

	// check number of arguments
	if (argc != 1)
	{
		printUsage();
		goto finish;
	}

	// determine filenames
	strcpy(glInFilename, argv[0]);
	if (strcmp(glOutFilename, "") == 0)
	{
		int inNameLen = strlen(glInFilename);
		strcpy(glOutFilename, glInFilename);

		if (inNameLen >= 4 && strcmp(&glOutFilename[inNameLen - 4], ".lzs") == 0)
		{
			glOutFilename[inNameLen - 4] = '\0';
		}
		else if (inNameLen >= 5 && strcmp(&glOutFilename[inNameLen - 5], ".lzss") == 0)
		{
			glOutFilename[inNameLen - 5] = '\0';
		}
		strcat(glOutFilename, ".bin");
	}

	// check LZSS bit field width
	if ((lzssOffsetBitCount + lzssLengthBitCount) % 8 != 0)
	{
		fprintf(stderr, "Error: A word must be byte-aligned");
		goto finish;
	}

	// open input file
	inFile = fopen(glInFilename, "rb");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error: Unable to open \"%s\"\n", glInFilename);
		goto finish;
	}

	// get file size
	fseek(inFile, 0, SEEK_END);
	long inFileSize = ftell(inFile);
	rewind(inFile);

	// check offset range
	if (lzssStartOffset >= inFileSize)
	{
		fprintf(stderr, "Error: Start offset out of range\n");
		goto finish;
	}

	// allocate input buffer
	long inLZSSSize = inFileSize - lzssStartOffset;
	inBytes = (uint8_t*) malloc(inLZSSSize);
	if (inBytes == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error\n");
		goto finish;
	}

	// allocate output buffer
	outBytes = (uint8_t*) malloc(lzssMaxRawSize);
	if (outBytes == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error\n");
		goto finish;
	}

	// read input data
	fseek(inFile, lzssStartOffset, SEEK_SET);
	if (fread(inBytes, inLZSSSize, 1, inFile) != 1)
	{
		fprintf(stderr, "Error: File read error\n");
		goto finish;
	}

	// open output file
	outFile = fopen(glOutFilename, "wb");
	if (outFile == NULL)
	{
		fprintf(stderr, "Error: Unable to open \"%s\"\n", glOutFilename);
		goto finish;
	}

	// try decompression
	size_t bytesWritten = decompressLZSS(inBytes, inLZSSSize, outBytes, lzssMaxRawSize, lzssMatchBitCount, lzssOffsetBitCount, lzssLengthBitCount);
	if (bytesWritten == 0)
	{
		fprintf(stderr, "Error: LZSS decompression failed\n");
		goto finish;
	}

	// write output data
	if (fwrite(outBytes, bytesWritten, 1, outFile) != 1)
	{
		fprintf(stderr, "Error: File write error\n");
		goto finish;
	}

	ret = EXIT_SUCCESS;

finish:
	if (inFile != NULL)
	{
		fclose(inFile);
	}
	if (outFile != NULL)
	{
		fclose(outFile);
	}
	if (inBytes != NULL)
	{
		free(inBytes);
	}
	if (outBytes != NULL)
	{
		free(outBytes);
	}
	return ret;
}
