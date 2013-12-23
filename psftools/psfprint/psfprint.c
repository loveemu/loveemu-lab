/**
 * psfprint - Print PSF Tags
 * You can extract specific field by using grep or something like that.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int main(int argc, char *argv[])
{
	int c;
	unsigned char s[256];
	unsigned int sizeReserved;
	unsigned int sizeProgram;
	FILE *fp = NULL;

	if (argc != 2)
	{
		printf("Usage: %s input.psf\n", argv[0]);
		return EXIT_FAILURE;
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: %s: Unable to open.\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (fread(s, 3, 1, fp) != 1 || memcmp(s, "PSF", 3) != 0)
	{
		fprintf(stderr, "Error: %s: Not a PSF format.\n", argv[1]);
		fclose(fp);
		return EXIT_FAILURE;
	}
	fgetc(fp);

	if (fread(s, 4, 1, fp) != 1)
	{
		fprintf(stderr, "Error: %s: File read error.\n", argv[1]);
		fclose(fp);
		return EXIT_FAILURE;
	}
	sizeReserved = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);

	if (fread(s, 4, 1, fp) != 1)
	{
		fprintf(stderr, "Error: %s: File read error.\n", argv[1]);
		fclose(fp);
		return EXIT_FAILURE;
	}
	sizeProgram = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);

	fseek(fp, 4 + sizeReserved + sizeProgram, SEEK_CUR);

	if (fread(s, 5, 1, fp) != 1)
	{
		/* no tags, no output */
		fclose(fp);
		return EXIT_SUCCESS;
	}
	if (memcmp(s, "[TAG]", 5) != 0)
	{
		fprintf(stderr, "Error: %s: Tag must begin with [TAG].\n", argv[1]);
		fclose(fp);
		return EXIT_FAILURE;
	}

	while ((c = fgetc(fp)) != EOF)
	{
		putchar(c);
	}

	fclose(fp);
	return EXIT_SUCCESS;
}
