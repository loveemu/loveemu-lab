// Small tool for AGB Music Player 2000 (aka. M4A, MP2000, Sappy)
// Programmed by loveemu, published under MIT License.
// Special Thanks To: Sappy, GSF Central, VGMTrans and its author.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>

#include "mp2kcomm.h"
#include "agbm4a.h"

//----------------------------------------------------------
// data bin from saptapper

static uint8_t m4a_bin_selectsong[0x1E] = {
	0x00, 0xB5, 0x00, 0x04, 0x07, 0x4A, 0x08, 0x49, 
	0x40, 0x0B, 0x40, 0x18, 0x83, 0x88, 0x59, 0x00, 
	0xC9, 0x18, 0x89, 0x00, 0x89, 0x18, 0x0A, 0x68, 
	0x01, 0x68, 0x10, 0x1C, 0x00, 0xF0,
};

#define M4A_INIT_PATT_COUNT 2
#define M4A_INIT_LEN 2
static uint8_t m4a_bin_init[M4A_INIT_PATT_COUNT][M4A_INIT_LEN] = { 
	{0x70, 0xB5},
	{0xF0, 0xB5}
};

#define M4A_MAIN_PATT_COUNT 1
#define M4A_MAIN_LEN 2
static uint8_t m4a_bin_main[M4A_MAIN_PATT_COUNT][M4A_MAIN_LEN] = {
	{0x00, 0xB5}
};

#define M4A_VSYNC_PATT_COUNT 1
#define M4A_VSYNC_LEN 5
static uint8_t m4a_bin_vsync[M4A_VSYNC_PATT_COUNT][M4A_VSYNC_LEN] = {
	{0x4A, 0x03, 0x68, 0x9B, 0x1A}
};

//----------------------------------------------------------

#define GBA_HEADER_SIZE     0xc0

#define GBA_ROMTITLE_OFFSET	0xa0
#define GBA_ROMTITLE_LENGTH	12
char *agb_getromtitle(uint8_t *gbarom, size_t gbasize, char *out_romtitle)
{
	if (gbarom == NULL || gbasize < (GBA_ROMTITLE_OFFSET + GBA_ROMTITLE_LENGTH))
	{
		return NULL;
	}
	memcpy(out_romtitle, &gbarom[GBA_ROMTITLE_OFFSET], GBA_ROMTITLE_LENGTH);
	out_romtitle[GBA_ROMTITLE_LENGTH] = '\0';
	return out_romtitle;
}

#define GBA_ROMID_OFFSET	0xac
#define GBA_ROMID_LENGTH	4
char *agb_getromid(uint8_t *gbarom, size_t gbasize, char *out_romid)
{
	if (gbarom == NULL || gbasize < (GBA_ROMID_OFFSET + GBA_ROMID_LENGTH))
	{
		return NULL;
	}
	memcpy(out_romid, &gbarom[GBA_ROMID_OFFSET], GBA_ROMID_LENGTH);
	out_romid[GBA_ROMID_LENGTH] = '\0';
	return out_romid;
}

//----------------------------------------------------------

#define M4A_OFFSET_SONGTABLE 40
void m4a_searchblock(uint8_t *gbarom, size_t gbasize, long& m4a_selectsong_offset, long& m4a_main_offset, long& m4a_init_offset, long& m4a_vsync_offset)
{
	m4a_selectsong_offset = -1;
	m4a_main_offset = -1;
	m4a_init_offset = -1;
	m4a_vsync_offset = -1;

	long m4a_selectsong_search_offset = 0;
	long m4a_songtable_offset = -1;
	while (m4a_selectsong_search_offset != -1)
	{
		m4a_selectsong_offset = memsearch(gbarom, gbasize, m4a_bin_selectsong, sizeof(m4a_bin_selectsong), m4a_selectsong_search_offset, 1, 0);
		if (m4a_selectsong_offset != -1)
		{
#ifdef _DEBUG
			fprintf(stdout, "selectsong candidate: $%08X\n", m4a_selectsong_offset);
#endif

			// obtain song table address
			uint32_t m4a_songtable_address = read_u32(&gbarom[m4a_selectsong_offset + M4A_OFFSET_SONGTABLE]);
			if (!is_gba_rom_address(m4a_songtable_address))
			{
#ifdef _DEBUG
				fprintf(stdout, "Song table address error: not a ROM address $%08X\n", m4a_songtable_address);
#endif
				m4a_selectsong_search_offset = m4a_selectsong_offset + 1;
				continue;
			}
			uint32_t m4a_songtable_offset_tmp = gba_address_to_offset(m4a_songtable_address);
			if (!is_valid_offset(m4a_songtable_offset_tmp + 4 - 1, gbasize))
			{
#ifdef _DEBUG
				fprintf(stdout, "Song table address error: address out of range $%08X\n", m4a_songtable_address);
#endif
				m4a_selectsong_search_offset = m4a_selectsong_offset + 1;
				continue;
			}

			// song table must have more than one song
			int songindex = 0;
			int validsongcount = 0;
			for (int songindex = 0; validsongcount < 1; songindex++)
			{
				uint32_t songaddroffset = m4a_songtable_offset_tmp + (songindex * 8);
				if (!is_valid_offset(songaddroffset + 4 - 1, gbasize))
				{
					break;
				}

				uint32_t songaddr = read_u32(&gbarom[songaddroffset]);
				if (songaddr == 0)
				{
					continue;
				}

				if (!is_gba_rom_address(songaddr))
				{
#ifdef _DEBUG
					fprintf(stdout, "Song address error: not a ROM address $%08X\n", songaddr);
#endif
					break;
				}
				if (!is_valid_offset(gba_address_to_offset(songaddr) + 4 - 1, gbasize))
				{
#ifdef _DEBUG
					fprintf(stdout, "Song address error: address out of range $%08X\n", songaddr);
#endif
					break;
				}
				validsongcount++;
			}
			if (validsongcount < 1)
			{
				m4a_selectsong_search_offset = m4a_selectsong_offset + 1;
				continue;
			}

			m4a_songtable_offset = (long) m4a_songtable_offset_tmp;
			break;
		}
		else
		{
			m4a_selectsong_search_offset = -1;
		}
	}
	if (m4a_selectsong_offset == -1)
	{
		return;
	}

	uint32_t m4a_main_offset_tmp = m4a_selectsong_offset;
	if (!is_valid_offset(m4a_main_offset_tmp + M4A_MAIN_LEN - 1, gbasize))
	{
		return;
	}
	while (m4a_main_offset_tmp > 0 && m4a_main_offset_tmp > ((uint32_t) m4a_selectsong_offset - 0x20))
	{
		for (int mainpattern = 0; mainpattern < M4A_MAIN_PATT_COUNT; mainpattern++)
		{
			if (memcmp(&gbarom[m4a_main_offset_tmp], &m4a_bin_main[mainpattern][0], M4A_INIT_LEN) == 0)
			{
				m4a_main_offset = (long) m4a_main_offset_tmp;
				break;
			}
		}
		m4a_main_offset_tmp--;
	}
	if (m4a_main_offset == -1)
	{
		return;
	}

	uint32_t m4a_init_offset_tmp = m4a_main_offset;
	if (!is_valid_offset(m4a_init_offset_tmp + M4A_INIT_LEN - 1, gbasize))
	{
		return;
	}
	while (m4a_init_offset_tmp > 0 && m4a_init_offset_tmp > ((uint32_t) m4a_main_offset - 0x100))
	{
		for (int initpattern = 0; initpattern < M4A_INIT_PATT_COUNT; initpattern++)
		{
			if (memcmp(&gbarom[m4a_init_offset_tmp], &m4a_bin_init[initpattern][0], M4A_INIT_LEN) == 0)
			{
				m4a_init_offset = (long) m4a_init_offset_tmp;
				break;
			}
		}
		m4a_init_offset_tmp--;
	}
	if (m4a_init_offset == -1)
	{
		return;
	}

	uint32_t m4a_vsync_offset_tmp = m4a_init_offset;
	if (!is_valid_offset(m4a_vsync_offset_tmp + M4A_VSYNC_LEN - 1, gbasize))
	{
		return;
	}
	while (m4a_vsync_offset_tmp > 0 && m4a_vsync_offset_tmp > ((uint32_t) m4a_init_offset - 0x800))
	{
		for (int vsyncpattern = 0; vsyncpattern < M4A_VSYNC_PATT_COUNT; vsyncpattern++)
		{
			if (memcmp(&gbarom[m4a_vsync_offset_tmp], &m4a_bin_vsync[vsyncpattern][0], M4A_VSYNC_LEN) == 0)
			{
				m4a_vsync_offset = (long) m4a_vsync_offset_tmp;
				break;
			}
		}
		m4a_vsync_offset_tmp--;
	}
	if (m4a_vsync_offset == -1)
	{
		return;
	}
}

static int m4a_getsongtablelength_dumpable(uint8_t *gbarom, size_t gbasize, uint32_t m4a_songtable_offset, int *validsongcount, bool verbose)
{
	int songentrycount = 0;
	int songcount = 0;
	int lastsongindex = -1;
	uint32_t m4a_songpointer_offset = m4a_songtable_offset;
	while (is_valid_offset(m4a_songpointer_offset + 8 - 1, gbasize))
	{
		uint32_t m4a_songheader_address = read_u32(&gbarom[m4a_songpointer_offset]);
		uint32_t m4a_songtable_ms = read_u16(&gbarom[m4a_songpointer_offset + 4]);
		uint32_t m4a_songtable_me = read_u16(&gbarom[m4a_songpointer_offset + 6]);

		// null entry is allowed
		if (m4a_songheader_address == 0)
		{
			//printf("song> %3d  0x%08X -> 0\n", songentrycount, m4a_songpointer_offset);
			songentrycount++;
			m4a_songpointer_offset += 8;
			continue;
		}

		// ROM address check
		if (!is_gba_rom_address(m4a_songheader_address) || !is_valid_offset(gba_address_to_offset(m4a_songheader_address) + 12 - 1, gbasize))
		{
			break;
		}
		uint32_t m4a_songheader_offset = gba_address_to_offset(m4a_songheader_address);
		if (m4a_songheader_offset % 4 != 0)
		{
			break;
		}

		// music player # ?
		if (m4a_songtable_ms > 32 || m4a_songtable_me > 32)
		{
			break;
		}

		// check song detail (especially for stos)
		uint8_t m4a_track_count = read_u8(&gbarom[m4a_songheader_offset]);
		if (m4a_track_count > 16)
		{
			break;
		}
		else if (m4a_track_count == 0)
		{
			//printf("song> %3d  0x%08X -> 0\n", songentrycount, m4a_songpointer_offset);
			songentrycount++;
			m4a_songpointer_offset += 8;
			continue;
		}
		// check tone data
		uint32_t m4a_tonedata_address = read_u32(&gbarom[m4a_songheader_offset + 4]);
		if (!is_gba_rom_address(m4a_tonedata_address) || !is_valid_offset(gba_address_to_offset(m4a_tonedata_address) + 12 - 1, gbasize) || m4a_tonedata_address % 4 != 0)
		{
			break;
		}
		//uint32_t m4a_tonedata_offset = gba_address_to_offset(m4a_tonedata_address);
		//uint32_t m4a_wavedata_address = read_u32(&gbarom[m4a_tonedata_offset + 4]);
		//if (!is_gba_rom_address(m4a_wavedata_address) || !is_valid_offset(gba_address_to_offset(m4a_wavedata_address) + 16 - 1, gbasize))
		//{
		//	break;
		//}
		// check tracks
		int trackindex;
		for (trackindex = 0; trackindex < m4a_track_count; trackindex++)
		{
			uint32_t m4a_track_address = read_u32(&gbarom[m4a_songheader_offset + 8 + (4 * trackindex)]);
			if (!is_gba_rom_address(m4a_track_address) || !is_valid_offset(gba_address_to_offset(m4a_track_address), gbasize))
			{
				break;
			}
			uint32_t m4a_track_offset = gba_address_to_offset(m4a_track_address);

			// usually first byte must be >= 0x80
			if (read_u8(&gbarom[m4a_track_offset]) < 0x80)
			{
				break;
			}
		}
		if (trackindex != m4a_track_count)
		{
			break;
		}

		// normal song entry
		if (verbose) {
			printf("song> %3d  0x%08X -> 0x%08X\n", songentrycount, m4a_songpointer_offset, m4a_songheader_address);
		}
		lastsongindex = songentrycount;
		songentrycount++;
		songcount++;
		m4a_songpointer_offset += 8;
	}
	if (lastsongindex >= 0) {
		// remove last null entries
		songentrycount = lastsongindex + 1;
	}
	if (verbose) {
		printf("%d entries, %d songs\n", songentrycount, songcount);
	}

	if (validsongcount != NULL) {
		*validsongcount = songcount;
	}
	return songentrycount;
}

int m4a_getsongtablelength(uint8_t *gbarom, size_t gbasize, uint32_t m4a_songtable_offset, int *validsongcount)
{
	return m4a_getsongtablelength_dumpable(gbarom, gbasize, m4a_songtable_offset, validsongcount, false);
}

bool m4a_isvalidmplaytableitem(uint8_t *gbarom, size_t gbasize, uint32_t mplaytableitem_offset)
{
	if (!is_valid_offset(mplaytableitem_offset + 12 - 1, gbasize))
	{
		return false;
	}

	uint32_t mp_songheader_address = read_u32(&gbarom[mplaytableitem_offset]);
	uint32_t mp_track_address = read_u32(&gbarom[mplaytableitem_offset + 4]);

	// work area must be WRAM or IRAM
	uint8_t mp_songheader_memregion = mp_songheader_address >> 24;
	uint8_t mp_track_memregion = mp_track_address >> 24;
	if ((mp_songheader_memregion != 2 && mp_songheader_memregion != 3) ||
		(mp_track_memregion != 2 && mp_track_memregion != 3))
	{
		return false;
	}
	// and the address must be aligned to dword
	if (mp_songheader_address % 4 != 0 || mp_track_address % 4 != 0)
	{
		return false;
	}

	return true;
}

long m4a_searchmplaytable_from_songtableptr(uint8_t *gbarom, size_t gbasize, uint32_t m4a_songtable_ptr_offset)
{
	if (m4a_songtable_ptr_offset < GBA_HEADER_SIZE)
	{
		return -1;
	}

	// work memory address is usually stored to near to song table pointer
	// exception: Metroid Zero Mission (and more games maybe)
	uint32_t m4a_mplayer_address = read_u32(&gbarom[m4a_songtable_ptr_offset - 4]);
	if (!is_gba_rom_address(m4a_mplayer_address) || !is_valid_offset(gba_address_to_offset(m4a_mplayer_address), gbasize))
	{
		return -1;
	}

	uint32_t m4a_mplaytable_offset = gba_address_to_offset(m4a_mplayer_address);
	if (!m4a_isvalidmplaytableitem(gbarom, gbasize, m4a_mplaytable_offset))
	{
		return -1;
	}

	return (long) m4a_mplaytable_offset;
}

long m4a_searchmplaytable_from_songtable(uint8_t *gbarom, size_t gbasize, uint32_t m4a_songtable_offset)
{
	long m4a_mplaytable_offset = -1;

	if (m4a_songtable_offset < GBA_HEADER_SIZE)
	{
		return -1;
	}

	// MPlayTable pointer is not always located at expected address, oh well.
	// MPlayTable is located prior to SongTable anyway, in most cases.
	// then, try a backward search.
	int mplaytablecount = 0;
	uint32_t mplaytableitem_offset_tmp = m4a_songtable_offset - 12;
	uint32_t mplaytableitem_offset = mplaytableitem_offset_tmp;
	while (mplaytableitem_offset > GBA_HEADER_SIZE)
	{
		if (!m4a_isvalidmplaytableitem(gbarom, gbasize, mplaytableitem_offset_tmp))
		{
			break;
		}
		mplaytablecount++;
		mplaytableitem_offset = mplaytableitem_offset_tmp;
		mplaytableitem_offset_tmp -= 12;
	}
	if (mplaytablecount > 0)
	{
		m4a_mplaytable_offset = (long) mplaytableitem_offset;
	}
	return m4a_mplaytable_offset;
}

// stos bruteforce search
long m4a_searchsongtableptr(uint8_t *gbarom, size_t gbasize, uint32_t start_offset)
{
	long m4a_songtable_pointer = -1;
	uint32_t m4a_songtable_pointer_tmp = (start_offset + 3) & ~3;
	while (is_valid_offset(m4a_songtable_pointer_tmp + 4 - 1, gbasize))
	{
		uint32_t m4a_songtable_address = read_u32(&gbarom[m4a_songtable_pointer_tmp]);
		if (is_gba_rom_address(m4a_songtable_address) && is_valid_offset(gba_address_to_offset(m4a_songtable_address) + 4 - 1, gbasize))
		{
			uint32_t m4a_songtable_offset_tmp = gba_address_to_offset(m4a_songtable_address);

			int available_song_count = 0;
			int songtablelength = m4a_getsongtablelength(gbarom, gbasize, m4a_songtable_offset_tmp, &available_song_count);
			if (songtablelength > 0 && available_song_count > 0)
			{
				// prevent false-positive detection (especially for very short table)
				long m4a_mplaytable_offset = m4a_searchmplaytable_from_songtable(gbarom, gbasize, m4a_songtable_offset_tmp);
				if (m4a_mplaytable_offset != -1)
				{
					m4a_songtable_pointer = (long) m4a_songtable_pointer_tmp;
					break;
				}
			}
		}
		m4a_songtable_pointer_tmp += 4;
	}
	return m4a_songtable_pointer;
}

long m4a_searchsongtable(uint8_t *gbarom, size_t gbasize, uint32_t start_offset)
{
	long m4a_songtable_offset = -1;
	long m4a_songtable_pointer = m4a_searchsongtableptr(gbarom, gbasize, start_offset);
	if (m4a_songtable_pointer != -1)
	{
		m4a_songtable_offset = (long) gba_address_to_offset(read_u32(&gbarom[m4a_songtable_pointer]));
	}
	return m4a_songtable_offset;
}

void m4a_searchsongtableandmplaytable(uint8_t *gbarom, size_t gbasize, uint32_t start_offset, long &m4a_songtable_offset, long &m4a_mplaytable_offset)
{
	m4a_songtable_offset = -1;
	m4a_mplaytable_offset = -1;

	long m4a_songtable_pointer = m4a_searchsongtableptr(gbarom, gbasize, start_offset);
	if (m4a_songtable_pointer != -1)
	{
		uint32_t m4a_songtable_offset_tmp = gba_address_to_offset(read_u32(&gbarom[m4a_songtable_pointer]));
		m4a_songtable_offset = (long) m4a_songtable_offset_tmp;

		// work memory address is usually stored to near to song table pointer
		// exception: Metroid Zero Mission (and more games maybe)
		// disabled, due to false-positive problem.
		//long m4a_mplaytable_offset_from_header = m4a_searchmplaytable_from_songtableptr(gbarom, gbasize, m4a_songtable_pointer);
		//if (m4a_mplaytable_offset_from_header != -1)
		//{
		//	m4a_mplaytable_offset = m4a_mplaytable_offset_from_header;
		//	return;
		//}

		// MPlayTable pointer is not located at expected address, oh well.
		// MPlayTable is located prior to SongTable anyway, in most cases.
		long m4a_mplaytable_offset_searched = m4a_searchmplaytable_from_songtable(gbarom, gbasize, m4a_songtable_offset);
		if (m4a_mplaytable_offset_searched != -1)
		{
			m4a_mplaytable_offset = m4a_mplaytable_offset_searched;
			return;
		}
	}
}

//----------------------------------------------------------

static bool print_gbaheader(uint8_t *gbarom, size_t gbasize)
{
	if (gbarom == NULL)
	{
		fprintf(stderr, "Error: no ROM input.\n");
		return false;
	}
	if (gbasize < GBA_HEADER_SIZE)
	{
		fprintf(stderr, "Error: GBA header error.\n");
		return false;
	}

	printf("=== GBA ROM Header ===\n");

	char gbatitle[GBA_ROMTITLE_LENGTH + 1];
	agb_getromtitle(gbarom, gbasize, gbatitle);
	printf("gba_header_romtitle\t%s\n", gbatitle);

	char gbaid[GBA_ROMID_LENGTH + 1];
	agb_getromid(gbarom, gbasize, gbaid);
	printf("gba_header_romid\t%s\n", gbaid);

	printf("\n");

	return true;
}

static void print_rom_offset(const char *offset_name, long offset)
{
	printf("%-24s", offset_name);
	if (offset != -1)
		printf("0x%08X", offset);
	else
		printf("null");
	printf("\n");
}

static int m4a_printsongtableall(uint8_t *gbarom, size_t gbasize, uint32_t m4a_songtable_offset)
{
	return m4a_getsongtablelength_dumpable(gbarom, gbasize, m4a_songtable_offset, NULL, true);
}

static void m4a_printsongtable(uint8_t *gbarom, size_t gbasize, uint32_t m4a_songtable_offset)
{
	printf("=== Song Table ===\n");
	print_rom_offset("m4a_songtable", m4a_songtable_offset);

	m4a_printsongtableall(gbarom, gbasize, m4a_songtable_offset);
	printf("\n");
}

static bool m4a_printinfo(uint8_t *gbarom, size_t gbasize)
{
	if (gbarom == NULL)
	{
		fprintf(stderr, "Error: no ROM input.\n");
		return false;
	}

	if (!print_gbaheader(gbarom, gbasize))
	{
		return false;
	}

	printf("=== Code Block ===\n");
	long m4a_selectsong_offset = -1;
	long m4a_main_offset = -1;
	long m4a_init_offset = -1;
	long m4a_vsync_offset = -1;
	m4a_searchblock(gbarom, gbasize, m4a_selectsong_offset, m4a_main_offset, m4a_init_offset, m4a_vsync_offset);
	print_rom_offset("m4a_selectsong", m4a_selectsong_offset);
	print_rom_offset("m4a_main", m4a_main_offset);
	print_rom_offset("m4a_init", m4a_init_offset);
	print_rom_offset("m4a_vsync", m4a_vsync_offset);
	printf("\n");

	// if saptapper-compatible, obtain song table address from pointer area
	long m4a_songtable_ptr_offset = -1;
	long m4a_songtable_offset_from_ptr = -1;
	long m4a_songtable_offset = -1;
	if (m4a_selectsong_offset != -1)
	{
		uint32_t m4a_songtable_address = read_u32(&gbarom[m4a_selectsong_offset + M4A_OFFSET_SONGTABLE]);
		if (is_gba_rom_address(m4a_songtable_address))
		{
			m4a_songtable_offset_from_ptr = (long) gba_address_to_offset(m4a_songtable_address);
			m4a_songtable_ptr_offset = m4a_selectsong_offset + M4A_OFFSET_SONGTABLE;
			if (!is_valid_offset(m4a_songtable_offset_from_ptr, gbasize))
			{
				m4a_songtable_offset_from_ptr = -1;
				m4a_songtable_ptr_offset = -1;
			}
		}
	}
	m4a_songtable_offset = m4a_songtable_offset_from_ptr;

	// bruteforce search for SongTable and MPlayTable
	printf("=== Table Search ===\n");
	long m4a_songtable_ptr_offset_searched = m4a_searchsongtableptr(gbarom, gbasize, GBA_HEADER_SIZE);
	long m4a_songtable_offset_searched = -1;
	if (m4a_songtable_ptr_offset_searched != -1)
	{
		print_rom_offset("m4a_songtable*", m4a_songtable_ptr_offset_searched);
		m4a_songtable_offset_searched = gba_address_to_offset(read_u32(&gbarom[m4a_songtable_ptr_offset_searched]));
	}
	if (m4a_songtable_offset == -1)
	{
		m4a_songtable_offset = m4a_songtable_offset_searched;
		m4a_songtable_ptr_offset = m4a_songtable_ptr_offset_searched;
	}
	if (m4a_songtable_offset != m4a_songtable_offset_searched)
	{
		print_rom_offset("m4a_songtable", m4a_songtable_offset);
		print_rom_offset("m4a_songtable(searched)", m4a_songtable_offset_searched);
		m4a_songtable_offset = m4a_songtable_offset_searched;
		m4a_songtable_ptr_offset = m4a_songtable_ptr_offset_searched;
	}
	else
	{
		print_rom_offset("m4a_songtable", m4a_songtable_offset);
	}
	long m4a_mplaytable_offset = -1;
	if (m4a_songtable_offset != -1)
	{
		long m4a_mplaytable_offset_from_ptr = m4a_searchmplaytable_from_songtableptr(gbarom, gbasize, m4a_songtable_ptr_offset);
		if (m4a_mplaytable_offset_from_ptr != -1)
		{
			m4a_mplaytable_offset = m4a_mplaytable_offset_from_ptr;
		}
		else
		{
			long m4a_mplaytable_offset_searched = m4a_searchmplaytable_from_songtable(gbarom, gbasize, m4a_songtable_offset);
			if (m4a_mplaytable_offset_searched != -1)
			{
				m4a_mplaytable_offset = m4a_mplaytable_offset_searched;
			}
		}
	}
	print_rom_offset("m4a_mplaytable", m4a_mplaytable_offset);
	printf("\n");

	return true;
}

static bool read_file_all(const char *filename, uint8_t **pbuf, uint32_t *psize)
{
	if (pbuf == NULL)
	{
		return false;
	}

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		return false;
	}

	size_t size = 0;
	fseek(fp, 0, SEEK_END);
	size = (size_t) ftell(fp);
	rewind(fp);

	*pbuf = (uint8_t*) malloc(size);
	if (*pbuf == NULL)
	{
		fclose(fp);
		return false;
	}

	fread(*pbuf, size, 1, fp);
	fclose(fp);

	if (psize != NULL)
	{
		*psize = size;
	}

	return true;
}

void getfilename(const char *path, char *fname)
{
#ifdef _MSC_VER
	char drive[_MAX_PATH];
	char dir[_MAX_PATH];
	char fnamebase[_MAX_PATH];
	char ext[_MAX_PATH];

	// use built-in path functions (multibyte support)
	_splitpath(path, drive, dir, fnamebase, ext);
	_makepath(fname, NULL, NULL, fnamebase, ext);
#else
	int sep;
	for (sep = strlen(path) - 1; sep >= 0; sep--)
	{
		if (path[sep] == '/' || path[sep] == '\\')
		{
			break;
		}
	}
	strcpy(fname, &path[sep + 1]);
#endif
}

static void show_mp2ktool_usage()
{
	const char *ops[] = {
		"info ROM.gba", "search m4a block and show their basic info.",
		"songlist (SongTable address) ROM.gba", "show list of items in song table.",
		NULL, NULL,
		"songtable ROM.gba", "search song table offset.",
		"songtableptr ROM.gba", "search song table pointer offset.",
		"mplaytable ROM.gba", "search music player table offset.",
		"ofslist ROM.gba", "show table offsets in format of gba2midi ofslist.txt",
		"ofslist+ ROM.gba", "show table offsets in format of m4aroms.csv",
		NULL, NULL,
		"header romtitle ROM.gba", "show 4 bytes ROM title in GBA ROM header.",
		"header romid ROM.gba", "show 4 bytes ROM ID in GBA ROM header.",
	};

	printf("%s %s\n", APP_NAME, APP_VERSION);
	printf("%s\n", APP_WEBSITE);
	printf("Syntax: %s <operation>\n", APP_NAME_SHORT);
	printf("\n");
	printf("[Operations]\n");
	for (int i = 0; i < ArrayLength(ops); i += 2)
	{
		if (ops[i+1] != NULL)
		{
			printf("  %s\n", ops[i]);
			printf("    %s\n", ops[i+1]);
		}
		else if (ops[i] != NULL)
		{
			printf("  %s\n", ops[i]);
		}
		else
		{
			printf("\n");
		}
	}
}

int main(int argc, char *argv[])
{
	// no arguments, show help
	if (argc <= 1)
	{
		show_mp2ktool_usage();
		return 1;
	}

	int argi = 1;
	char *op = argv[argi++];

	// read whole gba rom to memory
	char *gba_filepath = NULL;
	char gba_filename[_MAX_PATH];
	uint8_t *gbarom = NULL;
	size_t gbasize = 0;

	// do operation
	bool result = false;
	if (strcmp(op, "info") == 0)
	{
		if (argi + 1 > argc)
		{
			show_mp2ktool_usage();
			goto finish;
		}

		gba_filepath = argv[argi++];
		getfilename(gba_filepath, gba_filename);
		if (!read_file_all(gba_filepath, &gbarom, &gbasize))
		{
			fprintf(stderr, "Error: unable to read ROM file.\n");
			goto finish;
		}

		result = m4a_printinfo(gbarom, gbasize);
	}
	else if (strcmp(op, "header") == 0)
	{
		if (argi + 2 > argc)
		{
			show_mp2ktool_usage();
			goto finish;
		}

		char *itemname = argv[argi++];

		gba_filepath = argv[argi++];
		getfilename(gba_filepath, gba_filename);
		if (!read_file_all(gba_filepath, &gbarom, &gbasize))
		{
			fprintf(stderr, "Error: unable to read ROM file.\n");
			goto finish;
		}

		if (strcmp(itemname, "romid") == 0)
		{
			char gbaid[GBA_ROMID_LENGTH + 1];
			agb_getromid(gbarom, gbasize, gbaid);
			printf("%s\n", gbaid);
			result = true;
		}
		else if (strcmp(itemname, "romtitle") == 0)
		{
			char gbatitle[GBA_ROMTITLE_LENGTH + 1];
			agb_getromtitle(gbarom, gbasize, gbatitle);
			printf("%s\n", gbatitle);
			result = true;
		}
		else
		{
			fprintf(stderr, "Error: unknown item \"%s\"\n", itemname);
		}
	}
	else if (strcmp(op, "stos") == 0 || strcmp(op, "ofslist") == 0 || strcmp(op, "ofslist+") == 0 || strcmp(op, "songtable") == 0 || strcmp(op, "songtableptr") == 0 || strcmp(op, "mplaytable") == 0)
	{
		if (argi + 1 > argc)
		{
			show_mp2ktool_usage();
			goto finish;
		}

		gba_filepath = argv[argi++];
		getfilename(gba_filepath, gba_filename);
		if (!read_file_all(gba_filepath, &gbarom, &gbasize))
		{
			fprintf(stderr, "Error: unable to read ROM file.\n");
			goto finish;
		}

		if (strcmp(op, "stos") == 0 || strcmp(op, "songtable") == 0)
		{
			long m4a_songtable_offset = 0;
			while ((m4a_songtable_offset = m4a_searchsongtable(gbarom, gbasize, m4a_songtable_offset)) != -1)
			{
				printf("%08X\n", m4a_songtable_offset);
				result = true;
				break;
			}
			if (!result)
			{
				printf("null\n");
			}
		}
		else if (strcmp(op, "songtableptr") == 0)
		{
			long m4a_songtable_pointer = m4a_searchsongtableptr(gbarom, gbasize, GBA_HEADER_SIZE);
			if (m4a_songtable_pointer != -1)
			{
				printf("%08X\n", m4a_songtable_pointer);
				result = true;
			}
			if (!result)
			{
				printf("null\n");
			}
		}
		else if (strcmp(op, "mplaytable") == 0)
		{
			long m4a_songtable_offset = -1;
			long m4a_mplaytable_offset = -1;
			m4a_searchsongtableandmplaytable(gbarom, gbasize, GBA_HEADER_SIZE, m4a_songtable_offset, m4a_mplaytable_offset);
			if (m4a_mplaytable_offset != -1)
			{
				printf("%08X\n", m4a_mplaytable_offset);
				result = true;
			}
			if (!result)
			{
				printf("null\n");
			}
		}
		else if (strcmp(op, "ofslist") == 0 || strcmp(op, "ofslist+") == 0)
		{
			long m4a_songtable_offset = -1;
			long m4a_mplaytable_offset = -1;
			m4a_searchsongtableandmplaytable(gbarom, gbasize, GBA_HEADER_SIZE, m4a_songtable_offset, m4a_mplaytable_offset);
			if (m4a_songtable_offset != -1)
			{
				char gbaid[GBA_ROMID_LENGTH + 1];
				agb_getromid(gbarom, gbasize, gbaid);

				printf("%s,%08X,\"%s\",\"\"", gbaid, m4a_songtable_offset, gba_filename);
				if (strcmp(op, "ofslist+") == 0)
				{
					if (m4a_mplaytable_offset != -1)
					{
						printf(",%08X", m4a_mplaytable_offset);
					}
				}
				printf("\n");

				result = true;
			}
			if (!result)
			{
				printf("null\n");
			}
		}
	}
	else if (strcmp(op, "songlist") == 0)
	{
		char *m4a_songtable_offset_str = NULL;
		uint32_t m4a_songtable_offset = 0;
		if (argi + 1 > argc)
		{
			show_mp2ktool_usage();
			goto finish;
		}
		else if (argi + 2 <= argc)
		{
			m4a_songtable_offset_str = argv[argi++];
		}

		gba_filepath = argv[argi++];
		getfilename(gba_filepath, gba_filename);
		if (!read_file_all(gba_filepath, &gbarom, &gbasize))
		{
			fprintf(stderr, "Error: unable to read ROM file.\n");
			goto finish;
		}

		if (m4a_songtable_offset_str != NULL)
		{
			m4a_songtable_offset = strtoul(m4a_songtable_offset_str, NULL, 16);
		}
		else
		{
			long m4a_songtable_offset_searched = m4a_searchsongtable(gbarom, gbasize, GBA_HEADER_SIZE);
			if (m4a_songtable_offset_searched == -1)
			{
				fprintf(stderr, "Error: unable to find a song table.\n");
				goto finish;
			}
			m4a_songtable_offset = (long) m4a_songtable_offset_searched;
		}

		m4a_printsongtable(gbarom, gbasize, m4a_songtable_offset);
		result = true;
	}
	//else if (strcmp(op, "test") == 0)
	//{
	//	if (argi + 1 > argc)
	//	{
	//		show_mp2ktool_usage();
	//		goto finish;
	//	}
    //
	//	gba_filepath = argv[argi++];
	//	getfilename(gba_filepath, gba_filename);
	//	if (!read_file_all(gba_filepath, &gbarom, &gbasize))
	//	{
	//		fprintf(stderr, "Error: unable to read ROM file.\n");
	//		goto finish;
	//	}
    //
	//	//M4ASong a(gbarom, gbasize, 0xaabc);
	//	//a.test();
	//}
	else
	{
		fprintf(stderr, "Error: unknown command \"%s\"\n", op);
	}

finish:
	if (gbarom != NULL)
	{
		free(gbarom);
	}

	return result ? 0 : 1;
}
