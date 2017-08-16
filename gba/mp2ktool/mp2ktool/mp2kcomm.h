// Small tool for AGB Music Player 2000 (aka. M4A, MP2000, Sappy)
// Programmed by loveemu, published under MIT License.
// Special Thanks To: Sappy, GSF Central, VGMTrans and its author.

// Common definitions
#ifndef MP2K_COMM_INCLUDED
#define MP2K_COMM_INCLUDED

#include <stdio.h>
#include <stdint.h>

//----------------------------------------------------------

#define APP_NAME        "mp2ktool"
#define APP_NAME_SHORT  "mp2ktool"
#define APP_VERSION     "[2013-02-02]"
#define APP_WEBSITE     "http://code.google.com/p/loveemu/"

#define ArrayLength(a)  (sizeof(a) / sizeof(a[0]))

//----------------------------------------------------------

// byte reader/writer (little-endian)
inline uint8_t  read_u8  (uint8_t *data) { return data[0]; }
inline uint16_t read_u16 (uint8_t *data) { return data[0] + (data[1] << 8); }
inline uint32_t read_u24 (uint8_t *data) { return data[0] + (data[1] << 8) + (data[2] << 16); }
inline uint32_t read_u32 (uint8_t *data) { return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24); }
inline int8_t   read_s8  (uint8_t *data) { return (signed) data[0]; }
inline int16_t  read_s16 (uint8_t *data) { return (signed) read_u16(data); }
inline int32_t  read_s32 (uint8_t *data) { return (signed) read_u32(data); }
inline void write_u8  (uint8_t *data, uint8_t value)  { data[0] = value & 0xff; }
inline void write_u16 (uint8_t *data, uint16_t value) { data[0] = value & 0xff; data[1] = (value >> 8) & 0xff; }
inline void write_u24 (uint8_t *data, uint32_t value) { data[0] = value & 0xff; data[1] = (value >> 8) & 0xff; data[2] = (value >> 16) & 0xff; }
inline void write_u32 (uint8_t *data, uint32_t value) { data[0] = value & 0xff; data[1] = (value >> 8) & 0xff; data[2] = (value >> 16) & 0xff; data[3] = (value >> 24) & 0xff; }

//----------------------------------------------------------

long memsearch(uint8_t *dst, size_t dstsize, uint8_t *src, size_t srcsize, size_t dst_offset = 0, size_t alignment = 1, int diff_threshold = 0);

inline bool is_valid_offset(uint32_t offset, uint32_t romsize)
{
	return (offset < romsize);
}

inline bool is_gba_rom_address(uint32_t address)
{
	uint8_t region = (address >> 24) & 0xFE;
	return (region == 8);
}

inline uint32_t gba_address_to_offset(uint32_t address)
{
	if (!is_gba_rom_address(address)) {
		fprintf(stderr, "Warning: the address $%08X is not ROM address\n", address);
	}
	return address & 0x01FFFFFF;
}

#endif // !MP2K_COMM_INCLUDED
