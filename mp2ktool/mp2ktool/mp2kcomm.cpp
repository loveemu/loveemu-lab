// Small tool for AGB Music Player 2000 (aka. M4A, MP2000, Sappy)
// Programmed by loveemu, published under MIT License.
// Special Thanks To: Sappy, GSF Central, VGMTrans and its author.

#include <stdint.h>
#include "mp2kcomm.h"

//----------------------------------------------------------

long memsearch(uint8_t *dst, size_t dstsize, uint8_t *src, size_t srcsize, size_t dst_offset, size_t alignment, int diff_threshold)
{
	if (alignment == 0)
	{
		return -1;
	}

	// alignment
	if (dst_offset % alignment != 0)
	{
		dst_offset += alignment - (dst_offset % alignment);
	}

	for (size_t offset = dst_offset; (offset + srcsize) <= dstsize; offset += alignment)
	{
		// memcmp(&dst[offset], src, srcsize)
		int diff = 0;
		for (size_t i = 0; i < srcsize; i++)
		{
			if (dst[offset + i] != src[i])
			{
				diff++;
			}
			if (diff > diff_threshold)
			{
				break;
			}
		}
		if (diff <= diff_threshold)
		{
			return offset;
		}
	}
	return -1;
}
