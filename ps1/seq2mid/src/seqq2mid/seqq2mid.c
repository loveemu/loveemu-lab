// PS1 Dragon Quest SEQq to MIDI converter

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "../common/cioutils.h"
#include "../common/cutils.h"

#define APP_NAME    "seqq2mid"
#define APP_VERSION "[2014-01-03]"
#define APP_DESC    "PS1 Dragon Quest SEQq to MIDI Converter"
#define APP_AUTHOR  "loveemu <http://loveemu.googlecode.com/>"

bool seqq2mid(FILE *fSEQq, FILE *fMIDI, bool quiet)
{
	bool result = false;
	char s[8192];
	int i;

	int seqVersion;
	int seqId;
	int seqTPQN;
	int seqInitTempo;
	int seqTimeSigNumer;
	int seqTimeSigDenom;
	int seqUsedChanCount;

	int seqStartOffset;
	int seqByte;
	int seqDelta;
	int seqAbsTime = 0;
	int seqOpcode = 0;
	int seqEventLength = 0;
	int seqEventAddr;
	int seqTrackSize;

	const int seqEventLengths[8] = { 2, 2, 2, 2, 1, 1, 2, 0 };

	rewind(fSEQq);

	// search qQES signature
	// in fact, a in-ROM sequence seems to have 0x3C bytes header before it
	if (fseekmem(fSEQq, "qQES", 4) != 0)
	{
		fprintf(stderr, "Error: Invalid signature\n");
		return false;
	}
	seqStartOffset = (int) ftell(fSEQq);
	fseek(fSEQq, 4, SEEK_CUR);

	// read qQES header
	seqVersion = fget2b(fSEQq);
	seqId = fget2b(fSEQq);
	seqTPQN = fget2b(fSEQq);
	seqInitTempo = fget3b(fSEQq);
	seqTimeSigNumer = fgetc(fSEQq);
	seqTimeSigDenom = fgetc(fSEQq);
	seqUsedChanCount = fgetc(fSEQq);

	// show header info
	if (!quiet)
	{
		printf("Header\n");
		printf("------\n");
		printf("\n");
		if (seqStartOffset != 0)
		{
			printf("- Offset: 0x%04X\n", seqStartOffset);
		}
		printf("- Version: %d\n", seqVersion);
		printf("- ID: 0x%04X\n", seqId);
		printf("- Resolution: %d\n", seqTPQN);
		printf("- Tempo: %d\n", seqInitTempo);
		printf("- Rhythm: %d/%d\n", seqTimeSigNumer, 1 << seqTimeSigDenom);
		printf("- Channels: %d\n", seqUsedChanCount);
		printf("\n");
	}

	// write MThd header
	if (fwrite("MThd", 4, 1, fMIDI) != 1)
	{
		fprintf(stderr, "Error: File write error\n");
		return false;
	}
	fput4b(6, fMIDI);
	fput2b(0, fMIDI);
	fput2b(1, fMIDI);
	fput2b(seqTPQN, fMIDI);

	// write MTrk header
	fwrite("MTrk", 4, 1, fMIDI);
	fput4b(0, fMIDI);
	// put initial tempo
	fputc(0x00, fMIDI);
	fputc(0xFF, fMIDI);
	fputc(0x51, fMIDI);
	fputc(0x03, fMIDI);
	fput3b(seqInitTempo, fMIDI);
	// put initial time signature
	fputc(0x00, fMIDI);
	fputc(0xFF, fMIDI);
	fputc(0x58, fMIDI);
	fputc(0x04, fMIDI);
	fputc(seqTimeSigNumer, fMIDI);
	fputc(seqTimeSigDenom, fMIDI);
	fputc(0x18, fMIDI);
	fputc(0x08, fMIDI);
	// every SMF should have GM Reset
	fputc(0x00, fMIDI);
	fputc(0xF0, fMIDI);
	fputc(0x05, fMIDI);
	fputc(0x7E, fMIDI);
	fputc(0x7F, fMIDI);
	fputc(0x09, fMIDI);
	fputc(0x01, fMIDI);
	fputc(0xF7, fMIDI);
	// and GM2 Reset
	fputc(0x00, fMIDI);
	fputc(0xF0, fMIDI);
	fputc(0x05, fMIDI);
	fputc(0x7E, fMIDI);
	fputc(0x7F, fMIDI);
	fputc(0x09, fMIDI);
	fputc(0x02, fMIDI);
	fputc(0xF7, fMIDI);

	if (!quiet)
	{
		printf("Events\n");
		printf("------\n");
		printf("\n");
	}

	// event stream
	while (1)
	{
		seqEventAddr = (int) ftell(fSEQq);

		// delta-time
		seqDelta = fgetvb(fSEQq);
		if (seqDelta == EOF)
		{
			fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", (int) ftell(fSEQq));
			break;
		}

		// get the next byte
		seqByte = fgetc(fSEQq);
		if (seqByte == EOF)
		{
			fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", (int) ftell(fSEQq));
			break;
		}
		// check the end marker!
		if (seqDelta == 0x3FAF && seqByte == 0x00) // FF 2F 00 o_O
		{
			fputc(0x00, fMIDI);
			fputc(0xFF, fMIDI);
			fputc(0x2F, fMIDI);
			fputc(0x00, fMIDI);
			result = true;
			break;
		}

		// write delta-time to MIDI file
		fputvb(seqDelta, fMIDI);

		// handle running status rule
		if ((seqByte & 0x80) != 0)
		{
			seqOpcode = seqByte;
			fputc(seqOpcode, fMIDI);
		}
		else
		{
			fseek(fSEQq, -1, SEEK_CUR);

			if (seqOpcode < 0x80)
			{
				fprintf(stderr, "Error: Unexpected opcode at 0x%08X\n", (int) ftell(fSEQq));
				break;
			}

			// SMF special events always must have status-byte
			if (seqOpcode >= 0xF0)
			{
				fputc(seqOpcode, fMIDI);
			}
		}

		// get parameter length
		seqEventLength = seqEventLengths[(seqOpcode & 0x70) >> 4];

		// dispatch event
		if (seqOpcode >= 0xF0)
		{
			switch (seqOpcode)
			{
				case 0xF0:
				case 0xF7:
					// sysex (just in case)
					{
						int seqSysExLength;

						if (!quiet)
						{
							printf("- %06X: %06d %02X", seqEventAddr, seqAbsTime, seqOpcode);
						}

						// read data length
						seqSysExLength = fgetvb(fSEQq);
						if (seqSysExLength == EOF)
						{
							if (!quiet)
							{
								printf("\n");
							}
							fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", (int) ftell(fSEQq));
							goto quit_main_loop;
						}
						fputvb(seqSysExLength, fMIDI);
						if (!quiet)
						{
							fseek(fSEQq, -varintlen(seqSysExLength), SEEK_CUR);
							for (i = 0; i < varintlen(seqSysExLength); i++)
							{
								printf(" %02X", fgetc(fSEQq));
							}
						}

						// copy data
						if (seqSysExLength > 0)
						{
							if (seqSysExLength > sizeof(s))
							{
								fprintf(stderr, "Error: Too long message at 0x%08X (the message may be valid, but this tool does not support it because of laziness.)\n", (int) ftell(fSEQq));
								goto quit_main_loop;
							}

							if (fread(s, seqSysExLength, 1, fSEQq) != 1) {
								fprintf(stderr, "Error: File read error at 0x%08X\n", (int) ftell(fSEQq));
								goto quit_main_loop;
							}
							if (fwrite(s, seqSysExLength, 1, fMIDI) != 1) {
								fprintf(stderr, "Error: File write error at 0x%08X\n", (int) ftell(fMIDI));
								goto quit_main_loop;
							}
							if (!quiet)
							{
								for (i = 0; i < seqSysExLength; i++)
								{
									printf(" %02X", (unsigned char) s[i]);
								}
							}
						}

						// end of event
						if (!quiet)
						{
							printf("\n");
						}
					}
					break;

				case 0xFF:
					// meta event
					{
						int seqMetaType;
						int seqMetaLength;

						if (!quiet)
						{
							printf("- %06X: %06d %02X", seqEventAddr, seqAbsTime, seqOpcode);
						}

						seqMetaType = fgetc(fSEQq);
						if (seqMetaType == EOF)
						{
							fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", (int) ftell(fSEQq));
							goto quit_main_loop;
						}
						fputc(seqMetaType, fMIDI);
						if (!quiet)
						{
							printf(" %02X", seqMetaType);
						}

						// read data length (note that generic SEQ does not have it)
						seqMetaLength = fgetvb(fSEQq);
						if (seqMetaLength == EOF)
						{
							if (!quiet)
							{
								printf("\n");
							}
							fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", (int) ftell(fSEQq));
							goto quit_main_loop;
						}
						fputvb(seqMetaLength, fMIDI);
						if (!quiet)
						{
							fseek(fSEQq, -varintlen(seqMetaLength), SEEK_CUR);
							for (i = 0; i < varintlen(seqMetaLength); i++)
							{
								printf(" %02X", fgetc(fSEQq));
							}
						}

						// copy data
						if (seqMetaLength > 0)
						{
							if (seqMetaLength > sizeof(s))
							{
								fprintf(stderr, "Error: Too long message at 0x%08X (the message may be valid, but this tool does not support it because of laziness.)\n", (int) ftell(fSEQq));
								goto quit_main_loop;
							}

							if (fread(s, seqMetaLength, 1, fSEQq) != 1) {
								fprintf(stderr, "Error: File read error at 0x%08X\n", (int) ftell(fSEQq));
								goto quit_main_loop;
							}
							if (fwrite(s, seqMetaLength, 1, fMIDI) != 1) {
								fprintf(stderr, "Error: File write error at 0x%08X\n", (int) ftell(fMIDI));
								goto quit_main_loop;
							}
							if (!quiet)
							{
								for (i = 0; i < seqMetaLength; i++)
								{
									printf(" %02X", (unsigned char) s[i]);
								}
							}
						}

						// end of event
						if (!quiet)
						{
							printf("\n");
						}

						// check end of track
						if (seqMetaType == 0x2F)
						{
							// Apparently it's not used! :(
							// See delta-time thing for actual end of track handler
							result = true;
							goto quit_main_loop;
						}
					}
					break;

				default:
					if (!quiet)
					{
						printf("- %06X: %06d %02X %02X\n", seqEventAddr, seqAbsTime, seqOpcode, seqByte);
					}
					fprintf(stderr, "Error: Unknown status 0x%02X at 0x%08X\n", seqOpcode, (int) ftell(fMIDI));
					goto quit_main_loop;
			}
		}
		else
		{
			// generic events (direct copy)
			if (seqEventLength > 0)
			{
				if (fread(s, seqEventLength, 1, fSEQq) != 1) {
					fprintf(stderr, "Error: File read error at 0x%08X\n", (int) ftell(fSEQq));
					break;
				}
				if (fwrite(s, seqEventLength, 1, fMIDI) != 1) {
					fprintf(stderr, "Error: File write error at 0x%08X\n", (int) ftell(fMIDI));
					break;
				}
				if (!quiet)
				{
					printf("- %06X: %06d %02X", seqEventAddr, seqAbsTime, seqOpcode);
					for (i = 0; i < seqEventLength; i++)
					{
						printf(" %02X", (unsigned char) s[i]);
					}
					printf("\n");
				}
			}
		}

		seqAbsTime += seqDelta;
	}
	if (!quiet)
	{
		printf("\n");
	}

quit_main_loop:

	// set final track size
	seqTrackSize = (int) ftell(fMIDI) - 0x16;
	fseek(fMIDI, 0x12, SEEK_SET);
	fput4b(seqTrackSize, fMIDI);

	return result;
}

void printUsage(void)
{
	printf("About\n");
	printf("-----\n");
	printf("\n");
	printf("%s %s - %s,\n", APP_NAME, APP_VERSION, APP_DESC);
	printf("Created by %s\n", APP_AUTHOR);
	printf("\n");
	printf("Usage: %s input.seqq\n", APP_NAME);
	printf("\n");
	printf("### Options ####\n");
	printf("\n");
	printf("%s\n  : %s\n\n", "--help", "show this help");
	printf("%s\n  : %s\n\n", "-q", "suppress event dumps");
	printf("%s\n  : %s\n\n", "-o path", "set output path");
}

int main(int argc, char *argv[])
{
	FILE *fin = NULL;
	FILE *fout = NULL;
	int result = EXIT_FAILURE;
	int argi = 1;
	int argnum;
	char inpath[2048] = { '\0' };
	char outpath[2048] = { '\0' };

	bool quiet = false;

	if (argc <= 1)
	{
		printUsage();
		return EXIT_SUCCESS;
	}

	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage();
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "-q") == 0)
		{
			quiet = true;
		}
		else if (strcmp(argv[argi], "-o") == 0)
		{
			if (argi + 1 < argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
			}
			strcpy(outpath, argv[argi + 1]);
			argi++;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}
	argnum = argc - argi;

	if (argnum != 1)
	{
		fprintf(stderr, "Error: Too few/many arguments\n");
		goto finish;
	}

	strcpy(inpath, argv[argi]);
	if (strcmp(outpath, "") == 0)
	{
		strcpy(outpath, inpath);
		removeExt(outpath);
		strcat(outpath, ".mid");
	}

	fin = fopen(inpath, "rb");
	if (fin == NULL)
	{
		fprintf(stderr, "Error: file open error - %s\n", inpath);
		goto finish;
	}

	fout = fopen(outpath, "wb");
	if (fout == NULL)
	{
		fprintf(stderr, "Error: file open error - %s\n", outpath);
		goto finish;
	}

	result = seqq2mid(fin, fout, quiet) ? EXIT_SUCCESS : EXIT_FAILURE;

finish:
	if (fin != NULL)
	{
		fclose(fin);
	}
	if (fout != NULL)
	{
		fclose(fout);
	}
	return result;
}
