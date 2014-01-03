// PS1 Dragon Quest SEQq to MIDI converter

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include "cioutils.h"

#define APP_VERSION "[2014-01-03]"

bool quiet = false;

bool seqq2mid(FILE *fSEQq, FILE *fMIDI)
{
	char s[8192];
	int seqVersion;
	int seqId;
	int seqTPQN;
	int seqInitTempo;
	int seqTimeSigNumer;
	int seqTimeSigDenom;
	int seqUsedChanCount;

	int seqByte;
	int seqDelta;
	int seqAbsTime = 0;
	int seqOpcode = 0;
	int seqEventLength = 0;
	int seqVarIntLen;
	int seqEventAddr;

	// read qQES header
	if (fread(s, 4, 1, fSEQq) != 1 || memcmp(s, "qQES", 4) != 0)
	{
		fprintf(stderr, "Error: Invalid signature\n");
		return false;
	}
	seqVersion = fget2b(fSEQq);
	seqId = fget2b(fSEQq);
	seqTPQN = fget2b(fSEQq);
	seqInitTempo = fget3b(fSEQq);
	seqTimeSigNumer = fgetc(fSEQq);
	seqTimeSigDenom = fgetc(fSEQq);
	seqUsedChanCount = fgetc(fSEQq);

	if (!quiet)
	{
		printf("Header\n");
		printf("------\n");
		printf("\n");
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

	if (!quiet)
	{
		printf("Events\n");
		printf("------\n");
		printf("\n");
	}

	// event stream
	while (1)
	{
		seqEventAddr = ftell(fMIDI);

		// delta-time
		seqDelta = 0;
		seqVarIntLen = 0;
		do
		{
			seqByte = fgetc(fSEQq);
			if (seqByte == EOF) {
				fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", ftell(fSEQq));
				return false;
			}
			seqDelta = (seqDelta << 7) | (seqByte & 0x7F);
			seqVarIntLen++;
		} while(seqVarIntLen < 4 && (seqByte & 0x80) != 0);

		// get the next byte, and check the end marker!
		seqByte = fgetc(fSEQq);
		if (seqDelta == 0x3FAF && seqByte == 0x00) // FF 2F 00 o_O
		{
			fputc(0x00, fMIDI);
			fputc(0xFF, fMIDI);
			fputc(0x2F, fMIDI);
			fputc(0x00, fMIDI);
			break;
		}

		// write delta-time to MIDI file
		for (int i = seqVarIntLen; i >= 0; i--)
		{
			fputc((seqDelta >> (i * 7)) & 0x7F | (i != 0 ? 0x80 : 0), fMIDI);
		}

		// running status rule
		if ((seqByte & 0x80) != 0)
		{
			seqOpcode = seqByte;
			seqByte = fgetc(fSEQq);
			fputc(seqOpcode, fMIDI);
		}
		else
		{
			if (seqOpcode < 0x80)
			{
				fprintf(stderr, "Error: Unexpected opcode at 0x%08X\n", ftell(fSEQq));
				return false;
			}

			if (seqOpcode >= 0xF0)
			{
				fputc(seqOpcode, fMIDI);
			}
		}

		// check first byte
		if (seqByte == EOF)
		{
			fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", ftell(fSEQq));
			return false;
		}

		switch (seqOpcode & 0xF0)
		{
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xE0:
				seqEventLength = 2;
				break;

			case 0xC0:
			case 0xD0:
				seqEventLength = 1;
				break;
		}

		if (seqOpcode >= 0xF0)
		{
			switch (seqOpcode)
			{
				case 0xFF:
					{
						int seqMetaType;
						int seqMetaLength;

						seqMetaType = seqByte;
						fputc(seqMetaType, fMIDI);

						if (!quiet)
						{
							printf("- %06X: %06d %02X %02X", seqEventAddr, seqAbsTime, seqOpcode, seqMetaType);
						}

						// data length
						seqMetaLength = 0;
						seqVarIntLen = 0;
						do
						{
							seqByte = fgetc(fSEQq);
							if (seqByte == EOF) {
								if (!quiet)
								{
									printf("\n");
								}
								fprintf(stderr, "Error: Unexpected EOF at 0x%08X\n", ftell(fSEQq));
								return false;
							}
							fputc(seqByte, fMIDI);
							if (!quiet)
							{
								printf(" %02X", seqByte);
							}
							seqMetaLength = (seqMetaLength << 7) | (seqByte & 0x7F);
							seqVarIntLen++;
						} while(seqVarIntLen < 4 && (seqByte & 0x80) != 0);

						if (seqMetaLength > 0)
						{
							assert(sizeof(s) >= seqMetaLength);
							if (fread(s, seqMetaLength, 1, fSEQq) != 1) {
								fprintf(stderr, "Error: File read error at 0x%08X\n", ftell(fSEQq));
								return false;
							}
							if (fwrite(s, seqMetaLength, 1, fMIDI) != 1) {
								fprintf(stderr, "Error: File write error at 0x%08X\n", ftell(fMIDI));
								return false;
							}
							if (!quiet)
							{
								for (int i = 0; i < seqMetaLength; i++)
								{
									printf(" %02X", s[i]);
								}
							}
						}

						if (!quiet)
						{
							printf("\n");
						}

						if (seqMetaType == 0x2F)
						{
							// Apparently not used! :(
							goto quit_main_loop;
						}
					}
					break;

				default:
					if (!quiet)
					{
						printf("- %06X: %06d %02X %02X\n", seqEventAddr, seqAbsTime, seqOpcode, seqByte);
					}
					fprintf(stderr, "Error: Unknown status 0x%02X at 0x%08X\n", seqOpcode, ftell(fMIDI));
					return false;
			}
		}
		else
		{
			fputc(seqByte, fMIDI);
			if (seqEventLength > 1)
			{
				if (fread(s, seqEventLength - 1, 1, fSEQq) != 1) {
					fprintf(stderr, "Error: File read error at 0x%08X\n", ftell(fSEQq));
					return false;
				}
				if (fwrite(s, seqEventLength - 1, 1, fMIDI) != 1) {
					fprintf(stderr, "Error: File write error at 0x%08X\n", ftell(fMIDI));
					return false;
				}
				if (!quiet)
				{
					printf("- %06X: %06d %02X %02X", seqEventAddr, seqAbsTime, seqOpcode, seqByte);
					for (int i = 0; i < seqEventLength - 1; i++)
					{
						printf(" %02X", s[i]);
					}
					printf("\n");
				}
			}
		}

		seqAbsTime += seqDelta;
	}
	printf("\n");
quit_main_loop:

	int seqTrackSize = ftell(fMIDI) - 0x16;
	fseek(fMIDI, 0x12, SEEK_SET);
	fput4b(seqTrackSize, fMIDI);

	return true;
}

int main(int argc, char *argv[])
{
	FILE *fin = NULL;
	FILE *fout = NULL;
	int result = EXIT_FAILURE;
	int argi = 1;
	int argnum;

	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "-q") == 0)
		{
			quiet = true;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}
	argnum = argc - argi;

	if (argnum != 2)
	{
		printf("PS1 Dragon Quest SEQq to MIDI converter (experimental) %s\n", APP_VERSION);
		printf("loveemu <http://loveemu.googlecode.com/>\n");
		printf("\n");
		printf("Usage: %s input.seqq output.mid\n", argv[0]);
		goto finish;
	}

	fin = fopen(argv[argi], "rb");
	if (fin == NULL)
	{
		fprintf(stderr, "Error: file open error - %s\n", argv[argi]);
		goto finish;
	}

	fout = fopen(argv[argi + 1], "wb");
	if (fout == NULL)
	{
		fprintf(stderr, "Error: file open error - %s\n", argv[argi + 1]);
		goto finish;
	}

	result = seqq2mid(fin, fout) ? EXIT_SUCCESS : EXIT_FAILURE;

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
