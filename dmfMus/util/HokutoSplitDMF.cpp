/**
 * PS1 Hokuto no Ken - Seikimatsu Kyuuseishu Densetsu (J) (SLPS-02993)
 * Split combined multiple songs to each files (experimental)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	FILE* fp = NULL;
	uint8_t* data = NULL;

	if (argc == 1) {
		puts("No file input.");
		goto finish;
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		fprintf(stderr, "File open error (read).\n");
		goto finish;
	}

	char outfilename[512];
	strcpy(outfilename, argv[1]);
	// remove extension
	char *pdot = strrchr(outfilename, '.');
	char *pslash = strrchr(outfilename, '/');
	char *pbslash = strrchr(outfilename, '\\');
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

	fseek(fp, 0, SEEK_END);
	long filesize = ftell(fp);
	rewind(fp);

	data = (uint8_t*) malloc(filesize);
	if (data == NULL)
	{
		fprintf(stderr, "Memory allocation error.\n");
		goto finish;
	}

	if (fread(data, filesize, 1, fp) != 1)
	{
		fprintf(stderr, "File read error.\n");
		goto finish;
	}

	// start main part

	long lastFileOffset = -1;
	int fileCount = 0;
	for (long currOffset = 0; currOffset <= filesize; currOffset++)
	{
		bool saveLastFile = false;
		bool matchFile = false;

		if (currOffset == filesize)
		{
			saveLastFile = true;
		}
		else
		{
			if (memcmp(&data[currOffset], "pBAV", 4) == 0 || 
				memcmp(&data[currOffset], "pQES", 4) == 0)
			{
				saveLastFile = true;
			}
			else if (memcmp(&data[currOffset], "DMF\0", 4) == 0)
			{
				saveLastFile = true;
				matchFile = true;
			}
		}

		if (saveLastFile && lastFileOffset != -1)
		{
			fileCount++;

			char fwname[64];
			sprintf(fwname, "%s_%08x.dmf", outfilename, lastFileOffset);

			FILE *fpw = fopen(fwname, "wb");
			if (fpw != NULL)
			{
				long fwsize = currOffset - lastFileOffset;
				if (fwrite(&data[lastFileOffset], fwsize, 1, fpw) != 1) {
					fprintf(stderr, "File write error.\n");
				}
				fclose(fpw);
			}
			else
			{
				fprintf(stderr, "File open error (write).\n");
			}

			lastFileOffset = -1;
		}
		if (matchFile)
		{
			lastFileOffset = currOffset;
		}
	}

	printf("Found %d files.\n", fileCount);

finish:
	if (data != NULL) {
		free(data);
	}
	if (fp != NULL) {
		fclose(fp);
	}
	return ret;
}
