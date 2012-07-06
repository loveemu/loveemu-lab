/**
 * Susie plug-in: NSCR (NSCR+NCGR+NCLR) decoder
 * 
 * based on spi00in_ex.c by Shimitei
 * http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
 */

#include <windows.h>
#include <stdlib.h>
#include "ifnscr.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../cioutil.h"

/** plugin entrypoint */
BOOL APIENTRY SpiEntryPoint(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void setFileNameOfDepends(char *nscr_fname, char *ncgr_fname, char *nclr_fname) {
	char *slash, *dot;
	char basename[_MAX_PATH];
	char basename_ncgr[_MAX_PATH];
	char basename_nclr[_MAX_PATH];
	int len;

	// remove directory path
	slash = getPathSlash(nscr_fname);
	if (slash)
		strcpy(basename, &slash[1]);
	else
		strcpy(basename, nscr_fname);
	// remove extension
	dot = getPathExt(basename);
	if (dot)
		*dot = '\0';

	strcpy(basename_ncgr, basename);
	len = strlen(basename_ncgr) - 1;
	sprintf(ncgr_fname, "%s.ncgr", basename_ncgr);
	while (!fexist(ncgr_fname)) {
		if (basename_ncgr[0] == '\0') {
			ncgr_fname = NULL;
			break;
		}
		basename_ncgr[len] = '\0';
		len--;
		sprintf(ncgr_fname, "%s.ncgr", basename_ncgr);
	}

	strcpy(basename_nclr, basename);
	len = strlen(basename_nclr) - 1;
	sprintf(nclr_fname, "%s.nclr", basename_nclr);
	while (!fexist(nclr_fname)) {
		if (basename_nclr[0] == '\0') {
			nclr_fname = NULL;
			break;
		}
		basename_nclr[len] = '\0';
		len--;
		sprintf(nclr_fname, "%s.nclr", basename_nclr);
	}
}

/** check if the data can be decoded by the plugin */
BOOL IsSupportedEx(char *filename, char *data)
{
	BOOL res = FALSE;
	char ncgr_fname[_MAX_PATH];
	char nclr_fname[_MAX_PATH];
	uint8 *nscr = data;
	FILE *ncgr = NULL, *nclr = NULL;
	size_t nscr_size, ncgr_size, nclr_size;
	uint16 width, height, bpp;
	uint16 hTileCount, vTileCount;
	uint32 palType, palSize, palCount;
	uint16 bgType;

	if (!filename)
		return false;

	// set NCGR/NCLR filename first
	setFileNameOfDepends(filename, ncgr_fname, nclr_fname);

	// check signature
	if (nscr[0] != 'R' || nscr[1] != 'C' || nscr[2] != 'S' || nscr[3] != 'N')
		goto exitTerm;
	// and first section
	if (nscr[0x10] != 'N' || nscr[0x11] != 'R' || nscr[0x12] != 'C' || nscr[0x13] != 'S')
		goto exitTerm;

	// SCRN section overview:
	// 00h	4	Signature "SRCN"
	// 04h	4	Section Size (including header)
	// 08h	2	Width (multiple of 8?)
	// 0ah	2	Height (multiple of 8?)
	// 0ch	2	Unknown (Affine Parameter?)
	// 0eh	2	BG Type (0=Text BG, 1=Affine BG, or 2=Affine Extended BG)
	// 10h	4	Screen data size
	// 14h	n	Screen data

	ncgr = fopen(ncgr_fname, "rb");
	nclr = fopen(nclr_fname, "rb");
	if (!ncgr || !nclr)
		goto exitTerm;

	// check signature
	if (fgetc(ncgr) != 'R' || fgetc(ncgr) != 'G' || fgetc(ncgr) != 'C' || fgetc(ncgr) != 'N')
		goto exitTerm;
	if (fgetc(nclr) != 'R' || fgetc(nclr) != 'L' || fgetc(nclr) != 'C' || fgetc(nclr) != 'N')
		goto exitTerm;
	// and first section
	fseek(ncgr, 0x10, SEEK_SET);
	if (fgetc(ncgr) != 'R' || fgetc(ncgr) != 'A' || fgetc(ncgr) != 'H' || fgetc(ncgr) != 'C')
		goto exitTerm;
	fseek(nclr, 0x10, SEEK_SET);
	if (fgetc(nclr) != 'T' || fgetc(nclr) != 'T' || fgetc(nclr) != 'L' || fgetc(nclr) != 'P')
		goto exitTerm;

	// get file size (from header)
	nscr_size = mget4l(&nscr[8]);
	fseek(ncgr, 8, SEEK_SET); ncgr_size = fget4l(ncgr);
	fseek(nclr, 8, SEEK_SET); nclr_size = fget4l(ncgr);
	if (ncgr_size <= 0x30 || nclr_size <= 0x24)
		goto exitTerm;

	// get image info
	width = mget2l(&nscr[0x18]);
	height = mget2l(&nscr[0x1a]);
	bgType = mget2l(&nscr[0x1e]);
	bpp = 8;
	fseek(nclr, 0x18, SEEK_SET); palType = fget4l(nclr);
	if (palType == 3) { // 4bit
		fseek(nclr, 0x20, SEEK_SET); palSize = fget4l(nclr);
		//if (palSize < 0x200)
		//	palSize = 0x200 - palSize;
		palCount = palSize / 2;
	}
	else if (palType == 4) { // 8bit
		palSize = 0x200, palCount = 256;
	}
	if (width % 8 != 0 || height % 8 != 0)
		goto exitTerm; // because a tile must be 8x8
	if (width >= 0xfff0 || height >= 0xfff0)
		goto exitTerm; // compressed image
	if (bgType != 0)
		goto exitTerm; // perhaps we can read only text-bg for now
	if (palType != 3 && palType != 4) {
		assert(palType == 3 || palType == 4);
		goto exitTerm; // unknown bit depth
	}
	if (palCount > 256)
		goto exitTerm; // just in case
	hTileCount = width / 8;
	vTileCount = height / 8;

	// TODO: add more check if needed

	res = TRUE;

exitTerm:
	if (ncgr)
		fclose(ncgr);
	if (nclr)
		fclose(nclr);
	return res;
}

/** return some basic info of the picture */
int GetPictureInfoEx(char *filename, char *data, struct PictureInfo *lpInfo)
{
	int res = SPI_ALL_RIGHT;
	uint8 *nscr = data;
	uint16 width, height, bpp;
	uint32 palType;
	char ncgr_fname[_MAX_PATH];
	char nclr_fname[_MAX_PATH];
	FILE *ncgr = NULL, *nclr = NULL;

	if (!filename)
		return SPI_NOT_SUPPORT; // FIXME: proper error code?

	// set NCGR/NCLR filename/handle first
	setFileNameOfDepends(filename, ncgr_fname, nclr_fname);
	ncgr = fopen(ncgr_fname, "rb");
	nclr = fopen(nclr_fname, "rb");
	if (!ncgr || !nclr) {
		res = SPI_FILE_READ_ERROR;
		goto exitTerm;
	}

	// get image info
	width = mget2l(&nscr[0x18]);
	height = mget2l(&nscr[0x1a]);
	bpp = 8;
	fseek(nclr, 0x18, SEEK_SET); palType = fget4l(nclr);

	lpInfo->left          = 0;
	lpInfo->top           = 0;
	lpInfo->width         = width;
	lpInfo->height        = height;
	lpInfo->x_density     = 0;
	lpInfo->y_density     = 0;
	lpInfo->colorDepth    = bpp;
	lpInfo->hInfo         = NULL;

exitTerm:
	if (ncgr)
		fclose(ncgr);
	if (nclr)
		fclose(nclr);
	return res;
}

/** decode NDS 16bit color into RGB */
INLINE void decodeDSColor16(uint8* r, uint8 *g, uint8 *b, uint16 color) {
	uint8 r_, g_, b_;
	r_ = (color & 0x1f) * 8;
	g_ = (color >> 5 & 0x1f) * 8;
	b_ = (color >> 10 & 0x1f) * 8;

	r_ += r_ >> 5, g_ += g_ >> 5, b_ += b_ >> 5; // abcde000 => abcdeabc
	*r = r_, *g = g_, *b = b_;
}

/** convert the picture into a DIB image */
int GetPictureEx(char *filename, long datasize, HANDLE *pHBInfo, HANDLE *pHBm,
			 SPI_PROGRESS lpPrgressCallback, long lData, char *data)
{
	int res = SPI_ALL_RIGHT;
	uint8 *nscr = data;
	uint16 width, height, bpp;
	uint32 pitch, imageSize;
	uint32 palType, palSize, palCount;
	uint16 hTileCount, vTileCount;
	uint16 hTileCountHi, vTileCountHi;
	uint16 hTileCountLo, vTileCountLo;
	char ncgr_fname[_MAX_PATH];
	char nclr_fname[_MAX_PATH];
	FILE *ncgr = NULL, *nclr = NULL;
	BITMAPINFO* bmpinfo;
	BITMAPINFOHEADER *bmih;
	RGBQUAD *palette;
	BYTE* bmp;
	int palNo;

	if (!filename)
		return SPI_NOT_SUPPORT; // FIXME: proper error code?

	// set NCGR/NCLR filename/handle first
	setFileNameOfDepends(filename, ncgr_fname, nclr_fname);
	ncgr = fopen(ncgr_fname, "rb");
	nclr = fopen(nclr_fname, "rb");
	if (!ncgr || !nclr) {
		res = SPI_FILE_READ_ERROR;
		goto exitTerm;
	}

	// get image info
	width = mget2l(&nscr[0x18]);    // width mod 8 == 0
	height = mget2l(&nscr[0x1a]);   // height mod 8 == 0
	bpp = 8; // TODO: convert 16 color image as 4bpp image, not 8bpp.
	fseek(nclr, 0x18, SEEK_SET); palType = fget4l(nclr); // 3 or 4
	if (palType == 3) { // 4bit
		fseek(nclr, 0x20, SEEK_SET); palSize = fget4l(nclr);
		//if (palSize < 0x200)
		//	palSize = 0x200 - palSize;
		palCount = palSize / 2;
	}
	else if (palType == 4) { // 8bit
		fseek(nclr, 0x20, SEEK_SET); palSize = fget4l(nclr);
		assert(palSize == 0x200);
		palCount = 256;
	}
	else {
		res = SPI_NOT_SUPPORT;
		goto exitTerm;
	}
	pitch = getPitchOfDIB(width, bpp);
	imageSize = pitch * height;
	hTileCount = width / 8, vTileCount = height / 8;
	// attempt to support a larger image than 256x192 (e.g. DokiMajo2 Boss/BattleField)
	// 256x192 = 32x24=768 tiles
	hTileCountLo = hTileCount % 32, vTileCountLo = vTileCount % 24;
	hTileCountHi = hTileCount - hTileCountLo, vTileCountHi = vTileCount - vTileCountLo;

	// allocate memory for new bitmap
	*pHBInfo = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256));
	*pHBm = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, imageSize);
	if (!pHBInfo || !pHBm) {
		res = SPI_NO_MEMORY;
		goto exitTerm;
	}
	bmpinfo = (BITMAPINFO*) LocalLock(*pHBInfo);
	if (!bmpinfo) {
		res = SPI_MEMORY_ERROR;
		goto exitTerm;
	}
	bmp = (BYTE*) LocalLock(*pHBm);
	if (!bmp) {
		LocalUnlock(*pHBInfo);
		res = SPI_MEMORY_ERROR;
		goto exitTerm;
	}

	bmih = &bmpinfo->bmiHeader;
	palette = bmpinfo->bmiColors;

	// put image info
	bmih->biSize            = sizeof(BITMAPINFOHEADER);
	bmih->biWidth           = width;
	bmih->biHeight          = height;
	bmih->biPlanes          = 1;
	bmih->biBitCount        = bpp;
	bmih->biCompression     = BI_RGB;
	bmih->biSizeImage       = imageSize;
	bmih->biXPelsPerMeter   = 0;
	bmih->biYPelsPerMeter   = 0;
	bmih->biClrUsed         = 0;
	bmih->biClrImportant    = 0;

	// set palette info
	fseek(nclr, 40, SEEK_SET);  // TTLP:18h
	for (palNo = 0; palNo < (int)palCount; palNo++) {
		uint16 col = fget2l(nclr);
		uint8 r, g, b;

		decodeDSColor16(&r, &g, &b, col);
		palette[palNo].rgbRed = r;
		palette[palNo].rgbGreen = g;
		palette[palNo].rgbBlue = b;
		palette[palNo].rgbReserved = (col & 0x8000) ? 255 : 0;
	}

	// decode tiled image, a tile is 8x8 image.
	// tile references start from NSCR/NRCS:14h,
	// and the length of a tile reference is 16bit.
	nscr = &nscr[0x24];
	if (palType == 4) { // 8bit color tile
		int tileNo, hTileNo, vTileNo;

		for (tileNo = 0; tileNo < hTileCount * vTileCount; tileNo++) {
			uint16 tileInfo = mget2l(nscr); // rrrrddnnnnnnnnnn (r=reserved?, d=direction, n=tile index)
			uint16 tileIndex = tileInfo & 1023;
			uint8 tileDir = (tileInfo>>10)&3;
			uint8 *tileWriter;
			int x, y;

			// this is just a guess [citation needed] :P
			if (true) {
				int tilePerPanel = 32 * vTileCount;
				int panelNo = tileNo / tilePerPanel;
				int tileNoInPanel = tileNo % tilePerPanel;
				int hTileCountOfPanel = (panelNo < hTileCountHi/32) ? 32 : hTileCountLo;

				hTileNo = panelNo * 32 + tileNoInPanel % hTileCountOfPanel;
				vTileNo = tileNoInPanel / hTileCountOfPanel;
			}
			else {
				hTileNo = tileNo % hTileCount;
				vTileNo = tileNo / hTileCount;
			}

			nscr += 2;

			fseek(ncgr, 48 + tileIndex * 64, SEEK_SET);
			switch(tileDir) {
			case 0: // no flip (left to right, top to bottom), start from (hTileNo*8, vTileNo*8)
				tileWriter = &bmp[(height-vTileNo*8-1)*pitch+hTileNo*8];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 8; x++) {
						*tileWriter = fgetc(ncgr);
						tileWriter++;
					}
					tileWriter -= width + 8; // 1 line down
				}
				break;
			case 1: // horizontal flip (right to left, top to bottom), start from (hTileNo*8+7, vTileNo*8)
				tileWriter = &bmp[(height-vTileNo*8-1)*pitch+hTileNo*8+7];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 8; x++) {
						*tileWriter = fgetc(ncgr);
						tileWriter--;
					}
					tileWriter -= width - 8; // 1 line down
				}
				break;
			case 2: // vertical flip (left to right, bottom to top), start from (hTileNo*8, vTileNo*8+7)
				tileWriter = &bmp[(height-vTileNo*8-7-1)*pitch+hTileNo*8];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 8; x++) {
						*tileWriter = fgetc(ncgr);
						tileWriter++;
					}
					tileWriter += width - 8; // 1 line up
				}
				break;
			case 3: // horizontal+vertical flip (right to left, bottom to top), start from (hTileNo*8+7, vTileNo*8+7)
				tileWriter = &bmp[(height-vTileNo*8-7-1)*pitch+hTileNo*8+7];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 8; x++) {
						*tileWriter = fgetc(ncgr);
						tileWriter--;
					}
					tileWriter += width + 8; // 1 line up
				}
				break;
			}
		}
	}
	else if (palType == 3) { // 4bit color tile
		int tileNo, hTileNo, vTileNo;
		for (tileNo = 0; tileNo < hTileCount * vTileCount; tileNo++) {
			uint16 tileInfo = mget2l(nscr); // ccccddnnnnnnnnnn (c=upper 4bit of palette index, d=direction, n=tile index)
			uint16 tileIndex = tileInfo & 1023;
			uint8 tileDir = (tileInfo>>10)&3;
			uint8 palHigh = (tileInfo >> 12);
			uint8 *tileWriter;
			int x, y;

			if (palCount >= 16)
				palHigh %= (palCount >> 4); // fix MoetanDS/SYSTEM/*.nscr
			palHigh <<= 4;

			// this is just a guess [citation needed] :P
			if (true) {
				int tilePerPanel = 32 * vTileCount;
				int panelNo = tileNo / tilePerPanel;
				int tileNoInPanel = tileNo % tilePerPanel;
				int hTileCountOfPanel = (panelNo < hTileCountHi/32) ? 32 : hTileCountLo;

				hTileNo = panelNo * 32 + tileNoInPanel % hTileCountOfPanel;
				vTileNo = tileNoInPanel / hTileCountOfPanel;
			}
			else {
				hTileNo = tileNo % hTileCount;
				vTileNo = tileNo / hTileCount;
			}

			nscr += 2;

			fseek(ncgr, 48 + tileIndex * 32, SEEK_SET);
			switch(tileDir) {
			case 0: // no flip (left to right, top to bottom), start from (hTileNo*8, vTileNo*8)
				tileWriter = &bmp[(height-vTileNo*8-1)*pitch+hTileNo*8];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 4; x++) {
						uint8 palByte = fgetc(ncgr);
						uint8 palLo = palHigh | (palByte & 0xf);
						uint8 palHi = palHigh | (palByte >> 4);
						tileWriter[0] = palLo;
						tileWriter[1] = palHi;
						tileWriter += 2;
					}
					tileWriter -= width + 8; // 1 line down
				}
				break;
			case 1: // horizontal flip (right to left, top to bottom), start from (hTileNo*8+7, vTileNo*8)
				tileWriter = &bmp[(height-vTileNo*8-1)*pitch+hTileNo*8+6];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 4; x++) {
						uint8 palByte = fgetc(ncgr);
						uint8 palLo = palHigh | (palByte & 0xf);
						uint8 palHi = palHigh | (palByte >> 4);
						tileWriter[0] = palHi;
						tileWriter[1] = palLo;
						tileWriter -= 2;
					}
					tileWriter -= width - 8; // 1 line down
				}
				break;
			case 2: // vertical flip (left to right, bottom to top), start from (hTileNo*8, vTileNo*8+7)
				tileWriter = &bmp[(height-vTileNo*8-7-1)*pitch+hTileNo*8];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 4; x++) {
						uint8 palByte = fgetc(ncgr);
						uint8 palLo = palHigh | (palByte & 0xf);
						uint8 palHi = palHigh | (palByte >> 4);
						tileWriter[0] = palLo;
						tileWriter[1] = palHi;
						tileWriter += 2;
					}
					tileWriter += width - 8; // 1 line up
				}
				break;
			case 3: // horizontal+vertical flip (right to left, bottom to top), start from (hTileNo*8+7, vTileNo*8+7)
				tileWriter = &bmp[(height-vTileNo*8-7-1)*pitch+hTileNo*8+6];
				for (y = 0; y < 8; y++) {
					for (x = 0; x < 4; x++) {
						uint8 palByte = fgetc(ncgr);
						uint8 palLo = palHigh | (palByte & 0xf);
						uint8 palHi = palHigh | (palByte >> 4);
						tileWriter[0] = palHi;
						tileWriter[1] = palLo;
						tileWriter -= 2;
					}
					tileWriter += width + 8; // 1 line up
				}
				break;
			}
		}
	}

	LocalUnlock(*pHBInfo);
	LocalUnlock(*pHBm);
	fclose(ncgr);
	fclose(nclr);
	return res;

exitTerm:
	if (ncgr)
		fclose(ncgr);
	if (nclr)
		fclose(nclr);
	if (*pHBInfo) {
		LocalFree(*pHBInfo);
		*pHBInfo = NULL;
	}
	if (*pHBm) {
		LocalFree(*pHBm);
		*pHBm = NULL;
	}
	return res;
}
