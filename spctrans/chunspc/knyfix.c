// Kamaitachi no Yoru SPC fix
// Method contributed by Asuka Rangray

#include <stdio.h>
#include <stdlib.h>

void fileCopyBytes(FILE *dst, FILE *src, int start, int end)
{
	int i;
	int c;

	if (start > end)
		return;

	for (i = start; i <= end; i++) {
		fseek(src, i, SEEK_SET);
		c = fgetc(src);
		fseek(dst, i, SEEK_SET);
		fputc(c, dst);
	}
}

int main(int argc, char *argv[])
{
	FILE *fBase = NULL;
	FILE *fPatch = NULL;
	int result = EXIT_FAILURE;
	int c;

	if (argc < 2) {
		fprintf(stderr, "Error: too few input files\n");
		return EXIT_FAILURE;
	}

	// file 1:
	// spc file which starts from the beginning of a song,
	// but lacks some instruments. (Chunsoft's music engine
	// delayloads some samples just after the start of a song)
	fBase = fopen(argv[1], "r+b");
	if (fBase == NULL)
		goto onexit;

	// file 2:
	// spc file which has fully loaded necessary instruments.
	// If it's closer to the beginning of a song, it's better.
	fPatch = fopen(argv[2], "r+b");
	if (fPatch == NULL)
		goto onexit;

	// experimental: swap the file if needed.
	// this supports drag and drop from GUI.
	fseek(fBase, 0x006d2, SEEK_SET);
	fseek(fPatch, 0x006d2, SEEK_SET);
	c = fgetc(fBase);
	// detect if all the instruments have been loaded.
	if ((c != fgetc(fPatch)) && c == 0xff) {
		// swap pointers
		FILE *t = fBase;
		fPatch = fBase;
		fBase = t;

		printf("Base  SPC : %s\n", argv[2]);
		printf("Patch SPC : %s\n", argv[1]);
	}
	else {
		printf("Base  SPC : %s\n", argv[1]);
		printf("Patch SPC : %s\n", argv[2]);
	}

	// step 1: copy $02a0-ffff of ARAM
	fileCopyBytes(fBase, fPatch, 0x003a0, 0x100ff);
	// step 2: copy $0180-019f of ARAM
	fileCopyBytes(fBase, fPatch, 0x00280, 0x0029f);
	#if 0
	// step 3: copy pitch registers if needed
	fileCopyBytes(fBase, fPatch, 0x10102, 0x10103); // CH#0
	fileCopyBytes(fBase, fPatch, 0x10112, 0x10113); // CH#1
	fileCopyBytes(fBase, fPatch, 0x10122, 0x10123); // CH#2
	fileCopyBytes(fBase, fPatch, 0x10132, 0x10133); // CH#3
	fileCopyBytes(fBase, fPatch, 0x10142, 0x10143); // CH#4
	fileCopyBytes(fBase, fPatch, 0x10152, 0x10153); // CH#5
	fileCopyBytes(fBase, fPatch, 0x10162, 0x10163); // CH#6
	fileCopyBytes(fBase, fPatch, 0x10172, 0x10173); // CH#7
	#endif
	printf("Patching done.\n");

	result = EXIT_SUCCESS;

onexit:
	if (fPatch)
		fclose(fPatch);
	if (fBase)
		fclose(fBase);
	return result;
}
