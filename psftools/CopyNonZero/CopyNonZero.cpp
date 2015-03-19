/**
 * Copy non-zero bytes from two files
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	FILE* fpo = NULL;
	int ret = EXIT_FAILURE;

	if (argc != 4)
	{
		printf("Usage: %s input1.bin input2.bin output.bin\n", argv[0]);
		return EXIT_FAILURE;
	}

	fp1 = fopen(argv[1], "rb");
	if (fp1 == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[1]);
		goto finish;
	}

	fp2 = fopen(argv[2], "rb");
	if (fp2 == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[2]);
		goto finish;
	}

	fpo = fopen(argv[3], "wb");
	if (fpo == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[3]);
		goto finish;
	}

	int c1, c2, c;
	int offset = 0;
	while ((c1 = fgetc(fp1)) != EOF)
	{
		if ((c2 = fgetc(fp2)) == EOF)
		{
			break;
		}

		if (c1 == c2)
		{
			c = c1;
		}
		else
		{
			if (c1 == 0)
			{
				c = c2;
			}
			else if (c2 == 0)
			{
				c = c1;
			}
			else
			{
				printf("%08X: %02X != %02X\n", offset, c1, c2);
				c = c1;
			}
		}

		fputc(c, fpo);
		offset++;
	}

	ret = EXIT_SUCCESS;

finish:
	return ret;
}
