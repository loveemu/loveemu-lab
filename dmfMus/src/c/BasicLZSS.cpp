/**
 * Generic LZSS decompressor (made for SLPS-02993)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "BasicLZSS.h"

/**
 * Fix circular buffer offset.
 */
int fixCircularBufferOffset(int offset, size_t circularLength)
{
	if (offset < 0)
	{
		offset = circularLength + (offset % circularLength);
	}
	return offset % circularLength;
}

/**
 * Read circular buffer with update.
 * Read/write region can be overwrapped. (required for LZSS decompression)
 */
void *readCircularBufferWithUpdate(void *dst, const void *src, size_t circularLength, int circularOffsetToRead, int circularOffsetToWrite, size_t length)
{
	circularOffsetToRead = fixCircularBufferOffset(circularOffsetToRead, circularLength);
	circularOffsetToWrite = fixCircularBufferOffset(circularOffsetToWrite, circularLength);
	for (size_t bytesTransfered = 0; bytesTransfered < length; bytesTransfered++)
	{
		char c = ((char*)src)[circularOffsetToRead];
		if (dst != NULL)
		{
			((char*)dst)[bytesTransfered] = c;
		}
		((char*)src)[circularOffsetToWrite] = c;

		circularOffsetToRead = fixCircularBufferOffset(circularOffsetToRead + 1, circularLength);
		circularOffsetToWrite = fixCircularBufferOffset(circularOffsetToWrite + 1, circularLength);
	}

	return dst;
}

/**
 * Decompress LZSS.
 */
size_t decompressLZSS(const void *pIn, size_t inBuffSize, void *pOut, size_t outBuffSize, int lzssMatchBitCount, int lzssOffsetBitCount, int lzssLengthBitCount)
{
	bool result = false;

	char *lzssBuffer = NULL;
	char *lzssRefBuffer = NULL;

	const uint8_t *inBytes = (const uint8_t*) pIn;
	uint8_t *outBytes = (uint8_t*) pOut;
	size_t inOffset = 0;
	size_t outOffset = 0;

	int lzssBufferOffset = 0;
	int lzssMatchWordLength = lzssMatchBitCount / 8;
	int lzssRefWordLength = (lzssOffsetBitCount + lzssLengthBitCount) / 8;
	size_t lzssBufferLength = 1 << lzssOffsetBitCount;

	if (lzssMatchBitCount % 8 != 0 || lzssMatchBitCount < 8 || lzssMatchBitCount > 32)
	{
		fprintf(stderr, "Error: A word must be byte-aligned");
		goto finish;
	}
	if ((lzssOffsetBitCount + lzssLengthBitCount) % 8 != 0)
	{
		fprintf(stderr, "Error: A word must be byte-aligned");
		goto finish;
	}

	if (lzssRefWordLength < 1 || lzssRefWordLength > 4)
	{
		fprintf(stderr, "Error: %d byte(s) word is not supported", lzssRefWordLength);
		goto finish;
	}

	lzssBuffer = (char *) malloc(lzssBufferLength);
	if (lzssBuffer == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error");
		goto finish;
	}
	memset(lzssBuffer, 0, lzssBufferLength);

	lzssRefBuffer = (char *) malloc(lzssBufferLength);
	if (lzssRefBuffer == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error");
		goto finish;
	}

	while(true)
	{
		if (inOffset + lzssMatchWordLength > inBuffSize)
		{
			goto quit_decompress;
		}
		uint32_t lzssMatchFlags = 0;
		for (int byteIndex = 0; byteIndex < lzssMatchWordLength; byteIndex++)
		{
			lzssMatchFlags |= (inBytes[inOffset] << (byteIndex * 8));
			inOffset++;
		}

		for (int matchBitIndex = 0; matchBitIndex < lzssMatchBitCount; matchBitIndex++)
		{
			if ((lzssMatchFlags & (1 << matchBitIndex)) != 0)
			{
				// reference to the slide window
				if (inOffset + lzssMatchWordLength > inBuffSize)
				{
					goto quit_decompress;
				}
				uint32_t lzssRefWord = 0;
				for (int byteIndex = 0; byteIndex < lzssMatchWordLength; byteIndex++)
				{
					lzssRefWord |= (inBytes[inOffset] << (byteIndex * 8));
					inOffset++;
				}

				uint32_t lzssOffset = lzssRefWord & ((1 << lzssOffsetBitCount) - 1);
				uint32_t lzssLength = lzssRefWord >> lzssOffsetBitCount;
				lzssLength += 3;
				if (lzssLength < 3 || lzssLength > lzssBufferLength)
				{
					fprintf(stderr, "Error: Unexpected copy length\n");
					goto finish;
				}

				readCircularBufferWithUpdate(lzssRefBuffer, lzssBuffer, lzssBufferLength, lzssBufferOffset - lzssOffset - 1, lzssBufferOffset, lzssLength);
				lzssBufferOffset = fixCircularBufferOffset(lzssBufferOffset + lzssLength, lzssBufferLength);
				if (outOffset + lzssLength > outBuffSize)
				{
					memcpy(outBytes + outOffset, lzssRefBuffer, outBuffSize - outOffset);
					outOffset = outBuffSize;

					fprintf(stderr, "Error: Unexpected EOF\n");
					goto finish;
				}
				memcpy(outBytes + outOffset, lzssRefBuffer, lzssLength);
				outOffset += lzssLength;
			}
			else
			{
				// raw byte
				if (inOffset >= inBuffSize)
				{
					goto quit_decompress;
				}
				if (outOffset >= outBuffSize)
				{
					goto quit_decompress;
				}

				outBytes[outOffset] = inBytes[inOffset];

				lzssBuffer[lzssBufferOffset] = inBytes[inOffset];
				lzssBufferOffset = fixCircularBufferOffset(lzssBufferOffset + 1, lzssBufferLength);

				inOffset++;
				outOffset++;
			}
		}
	}

quit_decompress:
	result = true;

finish:
	if (lzssBuffer != NULL)
	{
		free(lzssBuffer);
	}
	if (lzssRefBuffer != NULL)
	{
		free(lzssRefBuffer);
	}
	return result ? outOffset : 0;
}
