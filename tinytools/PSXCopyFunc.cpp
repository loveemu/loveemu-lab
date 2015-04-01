
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>
#include <algorithm>

#define COUNT(a)	(sizeof(a) / sizeof(a[0]))

class FuncInfo
{
public:
	FuncInfo() :
		name(),
		address(0),
		size(0),
		available(false),
		optimized(false)
	{
	}

public:
	std::string name;
	unsigned int address;
	unsigned int size;
	bool available;
	bool optimized;
};

void recoverFunction(unsigned char *minData, const unsigned char *maxData, unsigned int textSize, unsigned int baseAddress,
	std::map<unsigned int, FuncInfo>& funcs, FuncInfo& targetFunc, int subLevel)
{
	if (subLevel >= 256)
	{
		return;
	}

	char indent[1024];
	memset(indent, ' ', subLevel * 2);
	indent[subLevel * 2] = '\0';

	if (targetFunc.address + targetFunc.size > textSize + baseAddress)
	{
		fprintf(stderr, "Warning: Missing code? in %s at 0x%08x\n", targetFunc.name.c_str(), targetFunc.address);
		return;
	}

	//printf("%s- Recovering %s (%u bytes at 0x%08X)\n", indent, targetFunc.name.c_str(), targetFunc.size, targetFunc.address);
	memcpy(&minData[targetFunc.address - baseAddress], &maxData[targetFunc.address - baseAddress], targetFunc.size);
	targetFunc.optimized = true;

	int funcSizeInWords = (int) (targetFunc.size / 4);
	for (int wordIndex = 0; wordIndex < funcSizeInWords; wordIndex++)
	{
		unsigned int subOffset = targetFunc.address + (wordIndex * 4) - baseAddress;
		unsigned int virtAddress = subOffset + baseAddress;

		unsigned int mipsWord = minData[subOffset] |
			(minData[subOffset + 1] << 8) |
			(minData[subOffset + 2] << 16) |
			(minData[subOffset + 3] << 24);
		char *mipsInstrName = "?";

		/*
		** j        0000 10ii iiii iiii iiii iiii iiii iiii
		** jal      0000 11ii iiii iiii iiii iiii iiii iiii
		** jr       0000 00ss sss0 0000 0000 0000 0000 1000
		** lui      0011 11-- ---t tttt iiii iiii iiii iiii
		** ori      0011 01ss ssst tttt iiii iiii iiii iiii
		** addi     0010 00ss ssst tttt iiii iiii iiii iiii
		** addiu    0010 01ss ssst tttt iiii iiii iiii iiii
		** <http://www.mrc.uidaho.edu/mrc/people/jff/digital/MIPSir.html>
		*/

		if ((mipsWord & 0xfc000000) == 0x08000000 || (mipsWord & 0xfc000000) == 0x0c000000)
		{
			// j, jal
			if ((mipsWord & 0xfc000000) == 0x08000000)
			{
				mipsInstrName = "j";
			}
			else
			{
				mipsInstrName = "jal";
			}

			bool findSub = false;
			unsigned int destValue = mipsWord & 0x03ffffff;
			unsigned int destAddress = (virtAddress & 0xf0000000) | (destValue << 2);
			for (std::map<unsigned int, FuncInfo>::iterator map_ite = funcs.begin(); map_ite != funcs.end(); ++map_ite)
			{
				FuncInfo& subFunc = map_ite->second;

				if (destAddress >= subFunc.address && destAddress < subFunc.address + subFunc.size)
				{
					findSub = true;

					if (subFunc.address >= targetFunc.address && subFunc.address < targetFunc.address + targetFunc.size)
					{
						// ignore recursive call
						break;
					}

					printf("%s  - Recovering %s (%08X: %s 0x%08X)\n", indent, subFunc.name.c_str(), virtAddress, mipsInstrName, destAddress);
					recoverFunction(minData, maxData, textSize, baseAddress, funcs, subFunc, subLevel + 1);
					break;
				}
			}
			if (!findSub)
			{
				printf("%s  - Possible sub_%08X function (%08X: %s 0x%08X)\n", indent, destAddress, virtAddress, mipsInstrName, destAddress);
			}
		}
		else if ((mipsWord & 0xfc000000) == 0x3c000000)
		{
			// lui
			bool isLI = false;
			unsigned int srcAddress = 0;

			if (wordIndex + 1 < funcSizeInWords)
			{
				unsigned int mipsNextWord = minData[subOffset + 4] |
					(minData[subOffset + 4 + 1] << 8) |
					(minData[subOffset + 4 + 2] << 16) |
					(minData[subOffset + 4 + 3] << 24);

				//if (((mipsWord & 0x001f0000) == (mipsNextWord & 0x001f0000)) && // curr.t == next.t
				//	((mipsNextWord & 0x001f0000) == ((mipsNextWord & 0x03e00000) >> 5))) // next.t == next.s
				if ((mipsWord & 0x001f0000) == ((mipsNextWord & 0x03e00000) >> 5)) // curr.t == next.s
				{
					if ((mipsNextWord & 0xfc000000) == 0x34000000)
					{
						// ori
						isLI = true;
						srcAddress = ((mipsWord & 0xffff) << 16) | (mipsNextWord & 0xffff);
					}
					else if ((mipsNextWord & 0xfc000000) == 0x20000000 || (mipsNextWord & 0xfc000000) == 0x24000000)
					{
						// addi, addiu
						isLI = true;
						int value = (mipsNextWord & 0xffff);
						if (value >= 0x8000)
						{
							value -= 0x10000;
						}
						srcAddress = ((mipsWord & 0xffff) << 16) + value;
					}
				}

				// li or la
				if (isLI && srcAddress >= baseAddress && srcAddress < baseAddress + textSize)
				{
					int varSize = 4;

					switch (srcAddress % 4)
					{
						case 1:
						case 3:
							varSize = 1;
							break;

						case 2:
							varSize = 2;
							break;
					}

					// MS932 (Shift_JIS) string?
					bool isString = false;
					int charIndex = 0;

					while (srcAddress + charIndex < baseAddress + textSize)
					{
						int c1 = maxData[srcAddress + charIndex - baseAddress];
						int c2 = (srcAddress + charIndex + 1 < baseAddress + textSize) ? maxData[srcAddress + charIndex + 1 - baseAddress] : 0;

						if (c1 == '\0')
						{
							if (charIndex > 4)
							{
								isString = true;
								varSize = charIndex;
							}
							break;
						}

						if ((c1 >= 0x81 && c1 <= 0x9f) || (c1 >= 0xe0 && c1 <= 0xfc))
						{
							// double byte
							if (c2 >= 0x40 && c2 <= 0xfc)
							{
								charIndex += 2;
							}
							else
							{
								break;
							}
						}
						else
						{
							// single byte
							if ((c1 == '\f' || c1 == '\n' || c1 == '\r' || c1 == '\t' || c1 == '\v') ||
								(c1 >= 0x20 && c1 <= 0x7e) || (c1 >= 0xa1 && c1 <= 0xdf))
							{
								charIndex++;
							}
							else
							{
								break;
							}
						}
					}

					if (srcAddress + varSize <= baseAddress + textSize)
					{
						// this feature is not so useful, but also better to have, I think.
						if (isString)
						{
							char *s_nowrap = (char*)malloc(strlen((const char*)&maxData[srcAddress - baseAddress]) * 2 + 1);
							if (s_nowrap != NULL)
							{
								int i = 0;
								int j = 0;

								do
								{
									switch(maxData[srcAddress + i - baseAddress])
									{
									case '\f':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = 'f';
										break;

									case '\n':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = 'n';
										break;

									case '\r':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = 'r';
										break;

									case '\t':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = 't';
										break;

									case '\v':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = 'v';
										break;

									case '\"':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = '\"';
										break;

									case '\\':
										s_nowrap[j++] = '\\';
										s_nowrap[j++] = '\\';
										break;

									default:
										s_nowrap[j++] = maxData[srcAddress + i - baseAddress];
										break;
									}
								} while (maxData[srcAddress + i++ - baseAddress] != '\0');

								printf("%s  - Recovering \"%s\" (%08X: la $t, 0x%08X)\n", indent, s_nowrap, virtAddress, srcAddress);
								free(s_nowrap);
							}
							else
							{
								fprintf(stderr, "Error: Memory allocation error\n");
							}
						}
						else
						{
							printf("%s  - Recovering var_%08X (assumed %d bytes) (%08X: la $t, 0x%08X)\n", indent, srcAddress, varSize, virtAddress, srcAddress);
						}
						memcpy(&minData[srcAddress - baseAddress], &maxData[srcAddress - baseAddress], varSize);
					}
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	std::map<unsigned int, FuncInfo> funcs;
	char line[1024];
	char s[1024];
	int lineIndex;
	int ret = EXIT_FAILURE;

	FILE *fplist = NULL;
	FILE *fpmin = NULL;
	FILE *fpmax = NULL;
	FILE *fpout = NULL;
	unsigned char *mindata = NULL;
	unsigned char *maxdata = NULL;

	if (argc < 4)
	{
		printf("Usage: %s [funclist.txt] [optimized.rom] [unaltered.rom] [output.rom]\n");
		printf("\n");
		printf("funclist.txt is a list copied from IDA's Function Window\n");
		return EXIT_FAILURE;
	}

	fplist = fopen(argv[1], "r");
	if (fplist == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[1]);
		goto finish;
	}

	lineIndex = 0;
	while (fgets(line, COUNT(line), fplist) != NULL)
	{
		int lineLength = strlen(line);
		if (line[lineLength - 1] == '\n')
		{
			line[lineLength - 1] = '\0';
		}

		char s1[1024];
		char s2[1024];
		char s3[1024];
		char s4[1024];
		if (sscanf(line, "%s %s %s %s", s1, s2, s3, s4) != 4)
		{
			fprintf(stderr, "Error: Unexpected format at line %d\n", lineIndex + 1);
			goto finish;
		}

		FuncInfo func;
		func.name = s1;
		func.address = strtoul(s3, NULL, 16);
		func.size = strtoul(s4, NULL, 16);
		funcs[func.address] = func;

		lineIndex++;
	}
	fclose(fplist);
	fplist = NULL;

	fpmin = fopen(argv[2], "rb");
	if (fpmin == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[2]);
		goto finish;
	}

	fpmax = fopen(argv[3], "rb");
	if (fpmax == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[3]);
		goto finish;
	}

	if (fread(s, 8, 1, fpmin) != 1 || memcmp(s, "PS-X EXE", 8) != 0)
	{
		fprintf(stderr, "Error: %s: Not a PS-X EXE\n", argv[2]);
		goto finish;
	}

	if (fread(s, 8, 1, fpmax) != 1 || memcmp(s, "PS-X EXE", 8) != 0)
	{
		fprintf(stderr, "Error: %s: Not a PS-X EXE\n", argv[3]);
		goto finish;
	}

	unsigned int minOffset;
	fseek(fpmin, 0x18, SEEK_SET);
	minOffset = fgetc(fpmin);
	minOffset |= fgetc(fpmin) << 8;
	minOffset |= fgetc(fpmin) << 16;
	minOffset |= fgetc(fpmin) << 24;

	unsigned int maxOffset;
	fseek(fpmax, 0x18, SEEK_SET);
	maxOffset = fgetc(fpmax);
	maxOffset |= fgetc(fpmax) << 8;
	maxOffset |= fgetc(fpmax) << 16;
	maxOffset |= fgetc(fpmax) << 24;

	if (minOffset != maxOffset)
	{
		fprintf(stderr, "Error: EXEs have different offsets, they are not supported.\n");
		goto finish;
	}

	fpout = fopen(argv[4], "wb");
	if (fpout == NULL)
	{
		fprintf(stderr, "Error: Could not open \"%s\"\n", argv[4]);
		goto finish;
	}

	unsigned char psxHead[0x800];
	rewind(fpmin);
	if (fread(psxHead, 0x800, 1, fpmin) != 1)
	{
		fprintf(stderr, "Error: File read error\n");
		goto finish;
	}
	if (fwrite(psxHead, 0x800, 1, fpout) != 1)
	{
		fprintf(stderr, "Error: File write error\n");
		goto finish;
	}

	fseek(fpmin, 0, SEEK_END);
	long minsize = ftell(fpmin);
	rewind(fpmin);

	fseek(fpmax, 0, SEEK_END);
	long maxsize = ftell(fpmax);
	rewind(fpmax);

	mindata = (unsigned char*) malloc(minsize);
	if (mindata == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error\n");
		goto finish;
	}

	maxdata = (unsigned char*) malloc(maxsize);
	if (maxdata == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error\n");
		goto finish;
	}

	fseek(fpmin, 0x800, SEEK_SET);
	if (fread(mindata, minsize - 0x800, 1, fpmin) != 1)
	{
		fprintf(stderr, "Error: File read error\n");
		goto finish;
	}

	fseek(fpmax, 0x800, SEEK_SET);
	if (fread(maxdata, maxsize - 0x800, 1, fpmax) != 1)
	{
		fprintf(stderr, "Error: File read error\n");
		goto finish;
	}

	printf("Available Functions\n");
	printf("-------------------\n");
	printf("\n");
	for (std::map<unsigned int, FuncInfo>::iterator map_ite = funcs.begin(); map_ite != funcs.end(); ++map_ite)
	{
		FuncInfo& func = map_ite->second;

		unsigned int usedWords = 0;
		int funcSizeInWords = (int) (func.size / 4);
		for (int wordIndex = 0; wordIndex < funcSizeInWords; wordIndex++)
		{
			unsigned int offset = func.address + (wordIndex * 4)  - minOffset;
			unsigned int virtAddress = offset + minOffset;
			unsigned int mipsWord = mindata[offset] |
				(mindata[offset + 1] << 8) |
				(mindata[offset + 2] << 16) |
				(mindata[offset + 3] << 24);
			unsigned int mipsOrgWord = maxdata[offset] |
				(maxdata[offset + 1] << 8) |
				(maxdata[offset + 2] << 16) |
				(maxdata[offset + 3] << 24);

			// if not a nop, consider it an active function
			if (mipsWord != 0)
			{
				usedWords++;
			}
			else if (mipsOrgWord == 0 && usedWords > 0)
			{
				usedWords++;
			}
		}

		if (usedWords != 0)
		{
			printf("- Found %s (%u of %u bytes used)\n", func.name.c_str(), usedWords * 4, func.size);
			func.available = true;
		}
	}
	printf("\n");

	printf("Recovery\n");
	printf("--------\n");
	printf("\n");
	for (std::map<unsigned int, FuncInfo>::iterator map_ite = funcs.begin(); map_ite != funcs.end(); ++map_ite)
	{
		FuncInfo& func = map_ite->second;
		if (func.available)
		{
			printf("- Recovering %s (%u bytes at 0x%08X)\n", func.name.c_str(), func.size, func.address);
			recoverFunction(mindata, maxdata, std::min(minsize, maxsize) - 0x800, minOffset, funcs, func, 0);
		}
	}

	if (fwrite(mindata, minsize - 0x800, 1, fpout) != 1)
	{
		fprintf(stderr, "Error: File write error\n");
		goto finish;
	}

	ret = EXIT_SUCCESS;

finish:
	if (fplist != NULL)
	{
		fclose(fplist);
	}
	if (fpmin != NULL)
	{
		fclose(fpmin);
	}
	if (fpmax != NULL)
	{
		fclose(fpmax);
	}
	if (fpout != NULL)
	{
		fclose(fpout);
	}
	if (mindata != NULL)
	{
		free(mindata);
	}
	if (maxdata != NULL)
	{
		free(maxdata);
	}
	return ret;
}
