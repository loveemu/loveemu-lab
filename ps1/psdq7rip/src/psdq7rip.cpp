/*
** PSX Dragon Quesy VII / IV Sound Data Ripper
*/

#define APP_NAME	"PSDQ7Rip"
#define APP_VER		"[2014-02-27]"
#define APP_DESC	"PSX Dragon Quesy VII / IV Sound Data Ripper"
#define APP_AUTHOR	"loveemu <http://loveemu.googlecode.com>"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>

#include "cbyteio.h"
#include "cpath.h"

#define PSX_MEMORY_SIZE         0x200000
#define MAX_OUTPUT_FILE_SIZE    PSX_MEMORY_SIZE

#define SND_HEADER_SIZE         0x3c

/**
 * Show usage of the application.
 */
void printUsage(const char *cmd)
{
	const char *availableOptions[] = {
		"--help", "Show this help",
	};

	printf("%s %s\n", APP_NAME, APP_VER);
	printf("======================\n");
	printf("\n");
	printf("%s. Created by %s.\n", APP_DESC, APP_AUTHOR);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: %s [HBD1PS1D.Q?? file]\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");

	for (int i = 0; i < sizeof(availableOptions) / sizeof(availableOptions[0]); i += 2)
	{
		printf("%s\n", availableOptions[i]);
		printf("  : %s\n", availableOptions[i + 1]);
		printf("\n");
	}
}

/**
 * Search PSDQ sound archive.
 */
bool scanDQ7SndFile(const char * filename)
{
	bool succeeded = false;

	// closable objects
	FILE * inFile = NULL;
	uint8_t * data = NULL;

	int fileCount = 0;

	off_t off_fsize;
	size_t fileSize;
	size_t dataOffset;
	size_t dataBufferSize;

	char in_basename[PATH_MAX];
	char out_filename[PATH_MAX];

	strcpy(in_basename, filename);
	path_basename(in_basename);

	// get whole input file size
	off_fsize = path_getfilesize(filename);
	fileSize = (size_t) off_fsize;

	// open input file
	inFile = fopen(filename, "rb");
	if (inFile == NULL)
	{
		fprintf(stderr, "Error: %s: Unable to open.\n", filename);
		goto finish;
	}

	// allocate memory buffer
	data = (uint8_t*) malloc(MAX_OUTPUT_FILE_SIZE * 2);
	if (data == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error.\n");
		goto finish;
	}

	// initialize buffer info
	dataOffset = 0;
	dataBufferSize = MAX_OUTPUT_FILE_SIZE * 2;
	if (dataOffset + dataBufferSize > fileSize)
	{
		dataBufferSize = fileSize - dataOffset;
	}

	// read first two blocks
	if (fread(data, 1, dataBufferSize, inFile) != dataBufferSize)
	{
		fprintf(stderr, "Error: %s: File read error.\n", filename);
		goto finish;
	}

	// scan for sound data
	for (size_t offset = 0; offset + SND_HEADER_SIZE < fileSize; offset += 4)
	{
		off_t offsetInBuf = offset - dataOffset;

		FILE * fpw = NULL;

		uint32_t seqSize;
		uint16_t seqId;
		uint8_t numBanks;
		uint8_t a07;
		uint32_t a08;

		uint32_t whSampSize[4];
		uint32_t whRgnSize[4];
		uint16_t whId[4];
		uint16_t wh0a[4];

		uint32_t totalSize = SND_HEADER_SIZE;
		uint32_t totalBankSize = 0;

		// read next block if needed
		if (offsetInBuf >= MAX_OUTPUT_FILE_SIZE)
		{
			dataOffset += MAX_OUTPUT_FILE_SIZE;
			dataBufferSize = MAX_OUTPUT_FILE_SIZE * 2;
			if (dataOffset + dataBufferSize > fileSize)
			{
				dataBufferSize = fileSize - dataOffset;
			}

			if (dataBufferSize <= MAX_OUTPUT_FILE_SIZE)
			{
				// last block has been already read, copy it to the beginning.
				memcpy(data, &data[MAX_OUTPUT_FILE_SIZE], dataBufferSize);
			}
			else
			{
				size_t newBlockSize = dataBufferSize - MAX_OUTPUT_FILE_SIZE;

				// shift the read block
				memcpy(data, &data[MAX_OUTPUT_FILE_SIZE], MAX_OUTPUT_FILE_SIZE);

				// read the next block
				fseek(inFile, dataOffset + MAX_OUTPUT_FILE_SIZE, SEEK_SET);
				if (fread(&data[MAX_OUTPUT_FILE_SIZE], 1, newBlockSize, inFile) != newBlockSize)
				{
					fprintf(stderr, "Error: %s: File read error.\n", filename);
					goto finish;
				}
			}

			offsetInBuf = offset - dataOffset;
		}

		// read/check header items
		seqSize = mget4l(&data[offsetInBuf]);
		seqId = mget2l(&data[offsetInBuf + 0x04]);
		numBanks = data[offsetInBuf + 0x06];
		a07 = data[offsetInBuf + 0x07];
		a08 = mget4l(&data[offsetInBuf + 0x08]);

		totalSize += seqSize;

		// bank count cannot be greater than 4,
		// because of the file design.
		if (numBanks > 4)
		{
			continue;
		}
		// SEQ id 0xFFFF is used for invalid id,
		// it must not be used, perhaps.
		if (seqId == 0xFFFF)
		{
			continue;
		}
		// SEQ size check
		if (seqSize > 0 && seqSize < 0x13)
		{
			continue;
		}
		// address range check
		if (seqSize > PSX_MEMORY_SIZE)
		{
			continue;
		}
		// alignment check (poor guess)
		if (seqSize % 4 != 0)
		{
			continue;
		}

		// items for each sample bank
		bool validBankInfo = true;
		bool firstEmptyBank = true;
		for (unsigned int bank = 0; bank < 4; bank++)
		{
			size_t baseOffset = offsetInBuf + 0x0c + (0x0c * bank);
			whSampSize[bank] = mget4l(&data[baseOffset]);
			whRgnSize[bank] = mget4l(&data[baseOffset + 0x04]);
			whId[bank] = mget2l(&data[baseOffset + 0x08]);
			wh0a[bank] = mget2l(&data[baseOffset + 0x0a]);

			// bank size check
			if ((whSampSize[bank] == 0 && whRgnSize[bank] != 0) ||
				(whSampSize[bank] != 0 && whRgnSize[bank] == 0))
			{
				validBankInfo = false;
				break;
			}
			// empty bank check
			if (whSampSize[bank] == 0 && whRgnSize[bank] == 0)
			{
				firstEmptyBank = false;
			}
			else if (!firstEmptyBank)
			{
				// non-empty bank should not be appeared after empty bank
				validBankInfo = false;
				break;
			}
			// address range check
			if (whSampSize[bank] > PSX_MEMORY_SIZE || whRgnSize[bank] > PSX_MEMORY_SIZE)
			{
				validBankInfo = false;
				break;
			}
			// this engine can load only 2 banks at maximum at the same time.
			// actually I do not know the valid range, anyway there must be a limit.
			if (whId[bank] > 4 && whId[bank] != 0xFFFF)
			{
				validBankInfo = false;
				break;
			}
			// VAG alignment check
			if (whSampSize[bank] % 16 != 0)
			{
				validBankInfo = false;
				break;
			}
			// region alignment check
			if (whRgnSize[bank] % 4 != 0)
			{
				validBankInfo = false;
				break;
			}

			// check VAG content
			if (whSampSize[bank] > 0)
			{
				// check unused bits of the first loop flag
				if ((data[offsetInBuf + SND_HEADER_SIZE + totalBankSize + 1] & 0xF8) != 0)
				{
					validBankInfo = false;
					break;
				}
			}

			totalBankSize += whSampSize[bank];
			totalBankSize += whRgnSize[bank];

			if (SND_HEADER_SIZE + totalBankSize > MAX_OUTPUT_FILE_SIZE)
			{
				validBankInfo = false;
				break;
			}
		}
		if (!validBankInfo)
		{
			continue;
		}
		totalSize += totalBankSize;

		// limit final output size
		if (totalSize > MAX_OUTPUT_FILE_SIZE)
		{
			continue;
		}

		// SEQ signature check (only if SEQ is included)
		if (seqSize != 0 && memcmp(&data[offsetInBuf + SND_HEADER_SIZE + totalBankSize], "qQES", 4) != 0)
		{
			continue;
		}

		// address range check
		if (totalSize == SND_HEADER_SIZE || offsetInBuf + totalSize > fileSize)
		{
			continue;
		}

		// determine output filename
		fileCount++;
		sprintf(out_filename, "%s-%04d-%08x.snd", in_basename, fileCount, offset);

		// export sound file
		fpw = fopen(out_filename, "wb");
		if (fpw != NULL)
		{
			printf("%s - SEQ:%u(%u)", out_filename, seqSize, seqId);
			for (unsigned int bank = 0; bank < 4; bank++)
			{
				if (whSampSize[bank] != 0 || whRgnSize[bank] != 0)
				{
					printf(" WH%u(%u):(%u,%u)", bank, whId[bank], whSampSize[bank], whRgnSize[bank]);
				}
			}
			printf("\n");

			if (fwrite(&data[offsetInBuf], 1, totalSize, fpw) != totalSize)
			{
				fprintf(stderr, "Error: %s: File write error.\n", out_filename);
				fprintf(stderr, "\n");
			}

			fclose(fpw);
			fpw = NULL;
		}
		else
		{
			fprintf(stderr, "Error: %s: Unable to open.\n", out_filename);
			fprintf(stderr, "\n");
		}

		// skip saved block
		// (for SEQ only, because I doubt false-positive)
		if (seqSize != 0)
		{
			offset += (totalSize & ~3) - 4;
		}
	}

	succeeded = true;

finish:
	if (data != NULL)
	{
		free(data);
	}
	if (inFile != NULL)
	{
		fclose(inFile);
	}
	return succeeded;
}

/**
 * Program main.
 */
int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	// set command path
	char * cmd = argv[0];

	// no parameters?
	if (argc == 1)
	{
		printUsage(cmd);
		goto finish;
	}

	// parse options
	int argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(cmd);
			goto finish;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"", argv[argi]);
			goto finish;
		}
		argi++;
	}
	argc -= argi;
	argv += argi;

	// check number of arguments
	if (argc != 1)
	{
		fprintf(stderr, "Error: Too few/many arguments.\n");
		goto finish;
	}

	// scan for sound data
	if (!scanDQ7SndFile(argv[0]))
	{
		goto finish;
	}

	ret = EXIT_SUCCESS;

finish:
	return ret;
}
