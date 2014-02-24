/*
** PSX Dragon Quesy VII / IV Sound Data Ripper
*/

#define APP_NAME	"PSDQ7Rip"
#define APP_VER		"[2014-02-24]"
#define APP_DESC	"PSX Dragon Quesy VII / IV Sound Data Ripper"
#define APP_AUTHOR	"loveemu <http://loveemu.googlecode.com>"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cbyteio.h"
#include "cpath.h"

#define SND_HEADER_SIZE 0x3c

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
	printf("Syntax: %s [.Q71|.Q41 file]\n", cmd);
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

int main(int argc, char **argv)
{
	bool result = false;
	int argnum;
	int argi;

	off_t off_fsize;
	size_t fsize;
	FILE * fp = NULL;
	FILE * fpw = NULL;
	uint8_t * data = NULL;

	char path_in[PATH_MAX];
	char path_out[PATH_MAX];
	char path_in_base[PATH_MAX];

	argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(argv[0]);
			return EXIT_SUCCESS;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	argnum = argc - argi;
	if (argnum == 0)
	{
		fprintf(stderr, "Error: No input files.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Run \"%s --help\" for help.\n", argv[0]);
		return EXIT_FAILURE;
	}
	else if (argnum > 1)
	{
		fprintf(stderr, "Error: Too many input files.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Run \"%s --help\" for help.\n", argv[0]);
		return EXIT_FAILURE;
	}

	strcpy(path_in, argv[argi]);
	strcpy(path_in_base, path_in);
	path_basename(path_in_base);

	off_fsize = path_getfilesize(path_in);
	if (off_fsize < 0)
	{
		fprintf(stderr, "Error: Input file does not exist.\n");
		fprintf(stderr, "\n");
		goto finish;
	}
	fsize = (size_t) off_fsize;
	if (fsize == 0)
	{
		fprintf(stderr, "Error: Input file is empty.\n");
		fprintf(stderr, "\n");
		goto finish;
	}

	// allocate memory for input file.
	// input file must be huge, but we can handle it enough.
	data = new uint8_t[fsize];

	// open input file
	fp = fopen(path_in, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Unable to open input file.\n");
		fprintf(stderr, "\n");
		goto finish;
	}

	// read whole data!
	if (fread(data, 1, fsize, fp) != fsize)
	{
		fprintf(stderr, "Error: File read error.\n");
		fprintf(stderr, "\n");
		goto finish;
	}

	// close file
	fclose(fp);
	fp = NULL;

	// search sound data
	for (size_t offset = 0; offset + SND_HEADER_SIZE < fsize; offset += 4)
	{
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

		// read/check header items
		seqSize = mget4l(&data[offset]);
		seqId = mget2l(&data[offset + 0x04]);
		numBanks = data[offset + 0x06];
		a07 = data[offset + 0x07];
		a08 = mget4l(&data[offset + 0x08]);

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
		if (seqSize > 0x200000)
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
			size_t baseOffset = offset + 0x0c + (0x0c * bank);
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
			if (whSampSize[bank] > 0x200000 || whRgnSize[bank] > 0x200000)
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

			totalBankSize += whSampSize[bank];
			totalBankSize += whRgnSize[bank];
		}
		totalSize += totalBankSize;
		if (!validBankInfo)
		{
			continue;
		}

		// SEQ signature check (only if SEQ is included)
		if (seqSize != 0 && memcmp(&data[offset + SND_HEADER_SIZE + totalBankSize], "qQES", 4) != 0)
		{
			continue;
		}

		// address range check
		if (totalSize == SND_HEADER_SIZE || offset + totalSize > fsize)
		{
			continue;
		}

		sprintf(path_out, "%s-%08x.snd", path_in_base, offset);

		fpw = fopen(path_out, "wb");
		if (fpw != NULL)
		{
			printf("%s - SEQ:%u(%u)", path_out, seqSize, seqId);
			for (unsigned int bank = 0; bank < 4; bank++)
			{
				if (whSampSize[bank] != 0 || whRgnSize[bank] != 0)
				{
					printf(" WH%u(%u):(%u,%u)", bank, whId[bank], whSampSize[bank], whRgnSize[bank]);
				}
			}
			printf("\n");

			if (fwrite(&data[offset], 1, totalSize, fpw) != totalSize)
			{
				fprintf(stderr, "Error: File write error \"%s\"\n", path_out);
				fprintf(stderr, "\n");
			}

			fclose(fpw);
			fpw = NULL;
		}
		else
		{
			fprintf(stderr, "Error: Unable to open \"%s\"\n", path_out);
			fprintf(stderr, "\n");
		}
	}

	result = true;

finish:
	if (fp != NULL)
	{
		fclose(fp);
	}
	if (data != NULL)
	{
		delete [] data;
	}
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
