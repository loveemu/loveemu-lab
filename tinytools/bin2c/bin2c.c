/* bin2c for ct2snsf */

#include <stdio.h>
#include <stdlib.h>

typedef int BOOL;
#define TRUE	1
#define FALSE	0

BOOL bin2c(const char *filename)
{
	FILE * fp;
	int c;
	int offset;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "File open error\n");
		return FALSE;
	}

	offset = 0;
	while ((c = fgetc(fp)) != EOF) {
		if (offset % 8 == 0) {
			printf("\t");
		}

		printf("0x%02X, ", c);

		if (offset % 8 == 7) {
			printf("\n");
		}

		offset++;
	}

	fclose(fp);
	return TRUE;
}

int main(int argc, char *argv[])
{
	int i;

	if (argc != 2) {
		printf("Usage: %s [input file]\n", argv[0]);
		return 1;
	}

	return (bin2c(argv[1]) != FALSE) ? 0 : 1;
}
