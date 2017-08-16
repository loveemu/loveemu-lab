/**
 * PS1 Hokuto no Ken - Seikimatsu Kyuuseishu Densetsu (J) (SLPS-02993)
 * Expand PAC archive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "BasicLZSS.h"

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define APP_NAME	"PAC Unpacker (PS1 Hokuto no Ken)"
#define APP_VER		"[2013-12-21]"

// Command path (set by main)
char *glCommandPath = NULL;
// Input filename
char glInFilename[512] = { '\0' };
// Output filename (without number and extension)
char glOutFilename[512] = { '\0' };

/**
 * Read 4 bytes little-endian number from file.
 */
int32_t fget4l(FILE *fp)
{
	int b1, b2, b3, b4;

	b1 = fgetc(fp);
	if (b1 == EOF) {
		return EOF;
	}
	b2 = fgetc(fp);
	if (b2 == EOF) {
		return EOF;
	}
	b3 = fgetc(fp);
	if (b3 == EOF) {
		return EOF;
	}
	b4 = fgetc(fp);
	if (b4 == EOF) {
		return EOF;
	}
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

/**
 * Memory search funciton.
 */
void *_memmem(const void *base, int count, const void *pattern, int length)
{
	if ( count <= 0 )
	{
		return NULL;
	}

	const char *start = static_cast<const char *>( base );
	{
		for(const char *p = start; p + length <= start + count; ++p)
		{
			if ( 0 == memcmp( p, pattern, length ) )
			{
				return const_cast<char *>( p );
			}
		}
	}

	return NULL;
}

/**
 * Show usage of the application.
 */
void printUsage(void)
{
	printf("%s %s\n", APP_NAME, APP_VER);
	printf("====================================================\n");
	printf("\n");
	printf("Syntax\n");
	printf("------\n");
	printf("\n");
	printf("%s (options) [input file]\n", glCommandPath);
	printf("\n");
	printf("Options\n");
	printf("-------\n");
	printf("\n");
	printf("--help\n");
	printf("  : show this help\n");
	printf("\n");
	printf("-o [filename]\n");
	printf("  : specify output filename (without extension)\n");
	printf("\n");
	printf("--extension-sound\n");
	printf("  : set file extension for sound files (sif, vh, vb)\n");
	printf("\n");
}

/**
 * Program main.
 */
int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	// closable objects
	FILE *fp = NULL;
	FILE *fpw = NULL;
	uint8_t *file_entry_data = NULL;
	uint8_t *file_raw_data = NULL;

	// user options
	bool rawExport = false;
	bool autoSoundExtension = false;

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
		else if (strcmp(argv[argi], "--raw") == 0)
		{
			rawExport = true;
		}
		else if (strcmp(argv[argi], "--extension-sound") == 0)
		{
			autoSoundExtension = true;
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
		strcpy(glOutFilename, glInFilename);

		// remove extension
		char *pdot = strrchr(glOutFilename, '.');
		char *pslash = strrchr(glOutFilename, '/');
		char *pbslash = strrchr(glOutFilename, '\\');
		if (pdot != NULL)
		{
			if (pslash == NULL || (pbslash != NULL && pslash > pbslash))
			{
				pslash = pbslash;
			}
			if (pslash == NULL || pdot > pslash)
			{
				*pdot = '\0';
			}
		}
	}

	// open input file
	fp = fopen(glInFilename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "File open error (read) [%s]\n", glInFilename);
		goto finish;
	}

	// process each files
	int minFileOffset = INT_MAX;
	for (int fileNo = 0; fileNo < 16; fileNo++)
	{
		// check the boundary (file count is variable)
		if (fileNo * 8 >= minFileOffset)
		{
			break;
		}

		// seek to file info table
		if (fseek(fp, fileNo * 8, SEEK_SET) != 0)
		{
			fprintf(stderr, "File seek error [%s, file %d]\n", glInFilename, fileNo);
			goto finish;
		}

		// read offset
		int32_t file_offset = fget4l(fp);
		if (file_offset == EOF)
		{
			fprintf(stderr, "Unexpected EOF [%s, offset %ld]\n", glInFilename, ftell(fp));
			goto finish;
		}

		// read length
		int32_t file_length = fget4l(fp);
		if (file_length == EOF)
		{
			fprintf(stderr, "Unexpected EOF [%s, offset %ld]\n", glInFilename, ftell(fp));
			goto finish;
		}

		// skip if not used
		if (file_offset == 0)
		{
			continue;
		}

		// update start offset
		if (minFileOffset > file_offset)
		{
			minFileOffset = file_offset;
		}

		// first length check
		if (file_length <= 4)
		{
			fprintf(stderr, "File too short [%s, file %d]\n", glInFilename, fileNo);
			continue;
		}

		// seek to file
		if (fseek(fp, file_offset, SEEK_SET) != 0)
		{
			fprintf(stderr, "File seek error [%s, offset 0x%08X]\n", glInFilename, file_offset);
			goto finish;
		}

		// alloc file buffer
		file_entry_data = (uint8_t*) malloc(file_length);
		if (file_entry_data == NULL)
		{
			fprintf(stderr, "Memory allocation error\n");
			goto finish;
		}

		// read whole file data
		if (fread(file_entry_data, file_length, 1, fp) != 1)
		{
			fprintf(stderr, "File read error [%s]\n", glInFilename);
			goto finish;
		}

		// determine header size
		int headerSize = 0x10;
		bool lzssCompressed = false;
		int fileTranscodeType = file_entry_data[0] | (file_entry_data[1] << 8) | (file_entry_data[2] << 16) | (file_entry_data[3] << 24);
		if (fileTranscodeType == 3)
		{
			headerSize = 0x1c;
		}
		if (fileTranscodeType == 1 || fileTranscodeType == 3)
		{
			lzssCompressed = true;
		}

		// it must have a header
		if (file_length <= headerSize)
		{
			free(file_entry_data);
			file_entry_data = NULL;

			fprintf(stderr, "File too short [%s, file %d, offset 0x%08X]\n", glInFilename, fileNo, file_offset);
			continue;
		}

		int rawFileSize = file_length;
		int lzssBufferBitCount = 0;

		// read header fields
		// Note: upper 16 bits seems to be often ignored in actual driver
		rawFileSize = file_entry_data[4] | (file_entry_data[5] << 8) | (file_entry_data[6] << 16) | (file_entry_data[7] << 24);
		lzssBufferBitCount = file_entry_data[8] | (file_entry_data[9] << 8) | (file_entry_data[10] << 16) | (file_entry_data[11] << 24);

		// simple header check
		if (!rawExport && lzssCompressed)
		{
			if (rawFileSize < 0 || rawFileSize > 0x200000 || lzssBufferBitCount < 1 || lzssBufferBitCount > 15)
			{
				free(file_entry_data);
				file_entry_data = NULL;

				fprintf(stderr, "Corrupt header [%s, file %d, offset 0x%08X]\n", glInFilename, fileNo, file_offset);
				continue;
			}
		}

		// determine output filename
		char outFilename[512];
		if (autoSoundExtension)
		{
			char outExtension[512];
			switch (fileNo % 3)
			{
			case 0:
				strcpy(outExtension, ".sif");
				break;

			case 1:
				strcpy(outExtension, ".vh");
				break;

			case 2:
				strcpy(outExtension, ".vb");
				break;

			// to make sure
			default:
				strcpy(outExtension, ".bin");
				break;
			}
			sprintf(outFilename, "%s_%c%s", glOutFilename, 'A' + (fileNo / 3), outExtension);
		}
		else
		{
			sprintf(outFilename, "%s_%02d.bin", glOutFilename, fileNo);
		}


		// open output file
		fpw = fopen(outFilename, "wb");
		if (fpw == NULL)
		{
			fprintf(stderr, "File open error (write) [%s]\n", outFilename);
			goto finish;
		}

		// export file data
		if (rawExport)
		{
			if (fwrite(file_entry_data, file_length, 1, fpw) != 1)
			{
				fprintf(stderr, "File write error\n");
				goto finish;
			}
		}
		else
		{
			if (lzssCompressed)
			{
				// alloc file buffer
				file_raw_data = (uint8_t*) malloc(rawFileSize);
				if (file_raw_data == NULL)
				{
					fprintf(stderr, "Memory allocation error\n");
					goto finish;
				}

				// decompress
				size_t rawFileSizeWritten = decompressLZSS(
					file_entry_data + headerSize, file_length - headerSize,
					file_raw_data, rawFileSize,
					16, lzssBufferBitCount, 16 - lzssBufferBitCount);
				if (rawFileSizeWritten != 0)
				{
					if (rawFileSizeWritten != (size_t) rawFileSize)
					{
						fprintf(stderr, "Warning: Mismatch file size (got %d bytes, expected %d bytes) [%s]\n", rawFileSizeWritten, rawFileSize, glInFilename);
					}

					if (fwrite(file_raw_data, rawFileSizeWritten, 1, fpw) != 1)
					{
						fprintf(stderr, "File write error\n");
						goto finish;
					}
				}
				else
				{
					fprintf(stderr, "Decompression failed [%s, file %d, offset 0x%08X]\n", glInFilename, fileNo, file_offset);
				}

				free(file_raw_data);
				file_raw_data = NULL;
			}
			else
			{
				// raw file
				if (fileTranscodeType != 0)
				{
					fprintf(stderr, "Unknown compression type [%s, file %d, offset 0x%08X]\n", glInFilename, fileNo, file_offset);
				}

				// special output for .VB file
				if (autoSoundExtension && (fileNo % 3) == 2)
				{
					uint8_t zeroblock[16];
					memset(zeroblock, 0, 16);
					if (fwrite(zeroblock, 16, 1, fpw) != 1)
					{
						fprintf(stderr, "File write error\n");
						goto finish;
					}
				}

				if (fwrite(file_entry_data + headerSize, file_length - headerSize, 1, fpw) != 1)
				{
					fprintf(stderr, "File write error\n");
					goto finish;
				}
			}
		}

		fclose(fpw);
		fpw = NULL;

		free(file_entry_data);
		file_entry_data = NULL;
	}

	ret = EXIT_SUCCESS;

finish:
	if (fp != NULL)
	{
		fclose(fp);
	}
	if (fpw != NULL)
	{
		fclose(fpw);
	}
	if (file_entry_data != NULL)
	{
		free(file_entry_data);
	}
	if (file_raw_data != NULL)
	{
		free(file_raw_data);
	}
	return ret;
}
