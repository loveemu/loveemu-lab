/**
 * PS1 Hokuto no Ken - Seikimatsu Kyuuseishu Densetsu (J) (SLPS-02993)
 * Expand PAC archive (experimental)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define APP_NAME	"PAC Unpacker"
#define APP_VER		"interim"

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
	printf("\n");
	printf("Syntax:\n");
	printf("  %s (options) <input file>\n", glCommandPath);
	printf("\n");
	printf("Options:\n");
	printf("  --help            show this help\n");
	printf("  -o <filename>     specify output filename (without extension)\n");
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
	uint8_t *file_data = NULL;

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
		fprintf(stderr, "File open error (read)\n");
		goto finish;
	}

	// process each files
	for (int fileNo = 0; fileNo < 12; fileNo++)
	{
		// seek to file info table
		if (fseek(fp, fileNo * 8, SEEK_SET) != 0)
		{
			fprintf(stderr, "File seek error [%d]\n", fileNo * 8);
			goto finish;
		}

		// read offset
		int32_t file_offset = fget4l(fp);
		if (file_offset == EOF)
		{
			fprintf(stderr, "Unexpected EOF\n");
			goto finish;
		}

		// read length
		int32_t file_length = fget4l(fp);
		if (file_length == EOF)
		{
			fprintf(stderr, "Unexpected EOF\n");
			goto finish;
		}

		// skip if not used
		if (file_length == 0)
		{
			continue;
		}

		// seek to file
		if (fseek(fp, file_offset, SEEK_SET) != 0)
		{
			fprintf(stderr, "File seek error [%d]\n", file_offset);
			goto finish;
		}

		// alloc file buffer
		file_data = (uint8_t*) malloc(file_length);
		if (file_data == NULL)
		{
			fprintf(stderr, "Memory allocation error\n");
			goto finish;
		}

		// read whole file data
		if (fread(file_data, file_length, 1, fp) != 1)
		{
			fprintf(stderr, "File read error\n");
			goto finish;
		}

		// determine output file extension
		char outExtension[512];
		if (_memmem(file_data, MIN(file_length, 128), "pBAV", 4) != NULL)
		{
			strcpy(outExtension, ".vh");
		}
		else if (file_length >= 8 && memcmp(&file_data[file_length - 8], "wwwwwwww", 8) == 0)
		{
			strcpy(outExtension, ".vb");
		}
		else if (file_length >= 3 && _memmem(file_data, MIN(file_length, 128), "MF", 2) != NULL)
		{
			// LZSS compressed DMF
			strcpy(outExtension, ".lzs");
		}
		else
		{
			strcpy(outExtension, ".bin");
		}

		// determine output filename
		char outFilename[512];
		sprintf(outFilename, "%s_%02d%s", glOutFilename, fileNo, outExtension);

		// open output file
		fpw = fopen(outFilename, "wb");
		if (fpw == NULL)
		{
			fprintf(stderr, "File open error (write)\n");
			goto finish;
		}

		// export file data
		if (fwrite(file_data, file_length, 1, fpw) != 1)
		{
			fprintf(stderr, "File write error\n");
			goto finish;
		}

		fclose(fpw);
		fpw = NULL;

		free(file_data);
		file_data = NULL;
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
	if (file_data != NULL)
	{
		free(file_data);
	}
	return ret;
}
