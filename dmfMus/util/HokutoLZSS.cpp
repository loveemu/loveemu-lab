/**
 * PS1 Hokuto no Ken - Seikimatsu Kyuuseishu Densetsu (J) (SLPS-02993)
 * LZSS decompressor for DMF sequence archive in BGMALL*.PAC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define APP_NAME	"LZSS Decompressor"
#define APP_VER		"interim"

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
	printf("  --offset <n>      skip first <n> input bytes\n");
	printf("\n");
}

/**
 * Fix circular buffer offset.
 */
int fixCircularBufferOffset(int offset, size_t circularLength)
{
	if (offset < 0)
	{
		offset = circularLength + (offset % circularLength);
	}
	return offset % circularLength;
}

/**
 * Read circular buffer with update.
 * Read/write region can be overwrapped. (required for LZSS decompression)
 */
void *readCircularBufferWithUpdate(void *dst, const void *src, size_t circularLength, int circularOffsetToRead, int circularOffsetToWrite, size_t length)
{
	circularOffsetToRead = fixCircularBufferOffset(circularOffsetToRead, circularLength);
	circularOffsetToWrite = fixCircularBufferOffset(circularOffsetToWrite, circularLength);
	for (size_t bytesTransfered = 0; bytesTransfered < length; bytesTransfered++)
	{
		char c = ((char*)src)[circularOffsetToRead];
		if (dst != NULL)
		{
			((char*)dst)[bytesTransfered] = c;
		}
		((char*)src)[circularOffsetToWrite] = c;

		circularOffsetToRead = fixCircularBufferOffset(circularOffsetToRead + 1, circularLength);
		circularOffsetToWrite = fixCircularBufferOffset(circularOffsetToWrite + 1, circularLength);
	}

	return dst;
}

/**
 * Decompress LZSS.
 */
bool decompressLZSS(FILE *inFile, FILE *outFile, int lzssMatchBitCount, int lzssOffsetBitCount, int lzssLengthBitCount)
{
	bool result = false;

	char *lzssBuffer = NULL;
	char *lzssRefBuffer = NULL;

	if (lzssMatchBitCount % 8 != 0 || lzssMatchBitCount < 8 || lzssMatchBitCount > 32)
	{
		fprintf(stderr, "Error: A word must be byte-aligned");
		goto finish;
	}
	if ((lzssOffsetBitCount + lzssLengthBitCount) % 8 != 0)
	{
		fprintf(stderr, "Error: A word must be byte-aligned");
		goto finish;
	}

	size_t lzssBufferLength = 1 << lzssOffsetBitCount;
	int lzssMatchWordLength = lzssMatchBitCount / 8;
	int lzssRefWordLength = (lzssOffsetBitCount + lzssLengthBitCount) / 8;
	if (lzssRefWordLength < 1 || lzssRefWordLength > 4)
	{
		fprintf(stderr, "Error: %d byte(s) word is not supported", lzssRefWordLength);
		goto finish;
	}

	lzssBuffer = (char *) malloc(lzssBufferLength);
	if (lzssBuffer == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error");
		goto finish;
	}
	memset(lzssBuffer, 0, lzssBufferLength);
	int lzssBufferOffset = 0;

	lzssRefBuffer = (char *) malloc(lzssBufferLength);
	if (lzssRefBuffer == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error");
		goto finish;
	}

	int c;
	while(true)
	{
		uint32_t lzssMatchFlags = 0;
		for (int byteIndex = 0; byteIndex < lzssMatchWordLength; byteIndex++)
		{
			c = fgetc(inFile);
			if (c == EOF)
			{
				goto quit_decompress;
			}
			lzssMatchFlags |= (c << (byteIndex * 8));
		}

		for (int matchBitIndex = 0; matchBitIndex < lzssMatchBitCount; matchBitIndex++)
		{
			if ((lzssMatchFlags & (1 << matchBitIndex)) != 0)
			{
				// reference to the slide window
				uint32_t lzssRefWord = 0;
				for (int byteIndex = 0; byteIndex < lzssMatchWordLength; byteIndex++)
				{
					c = fgetc(inFile);
					if (c == EOF)
					{
						goto quit_decompress;
					}
					lzssRefWord |= (c << (byteIndex * 8));
				}

				uint32_t lzssOffset = lzssRefWord & ((1 << lzssOffsetBitCount) - 1);
				uint32_t lzssLength = lzssRefWord >> lzssOffsetBitCount;
				lzssLength += 3;
				if (lzssLength < 3 || lzssLength > lzssBufferLength)
				{
					fprintf(stderr, "Error: Unexpected copy length\n");
					goto finish;
				}

				//printf("DEBUG: IN:%08x OUT:%08x LEN:%d REF:%d READ:%d WRITE:%d\n", ftell(inFile), ftell(outFile), lzssLength, lzssOffset, lzssBufferOffset - lzssOffset - 1, lzssOffset);

				readCircularBufferWithUpdate(lzssRefBuffer, lzssBuffer, lzssBufferLength, lzssBufferOffset - lzssOffset - 1, lzssBufferOffset, lzssLength);
				lzssBufferOffset = fixCircularBufferOffset(lzssBufferOffset + lzssLength, lzssBufferLength);
				if (fwrite(lzssRefBuffer, lzssLength, 1, outFile) != 1)
				{
					fprintf(stderr, "Error: File write error");
					goto finish;
				}
			}
			else
			{
				// raw byte
				c = fgetc(inFile);
				if (c == EOF)
				{
					goto quit_decompress;
				}
				if (fputc(c, outFile) == EOF)
				{
					fprintf(stderr, "Error: File write error");
					goto finish;
				}

				lzssBuffer[lzssBufferOffset] = c;
				lzssBufferOffset = fixCircularBufferOffset(lzssBufferOffset + 1, lzssBufferLength);
			}
		}
	}

quit_decompress:
	result = true;

finish:
	if (lzssBuffer != NULL)
	{
		free(lzssBuffer);
	}
	if (lzssRefBuffer != NULL)
	{
		free(lzssRefBuffer);
	}
	return result;
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

	// closable objects
	FILE *inFile = NULL;
	FILE *outFile = NULL;

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

	// open output file
	outFile = fopen(glOutFilename, "wb");
	if (outFile == NULL)
	{
		fprintf(stderr, "Error: Unable to open \"%s\"\n", glOutFilename);
		goto finish;
	}

	// try decompression
	fseek(inFile, lzssStartOffset, SEEK_SET);
	if (!decompressLZSS(inFile, outFile, lzssMatchBitCount, lzssOffsetBitCount, lzssLengthBitCount))
	{
		fprintf(stderr, "Error: Something went wrong\n");
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
	return ret;
}
