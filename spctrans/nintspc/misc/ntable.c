#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define countof(a)	(sizeof(a) / sizeof(a[0]))

int main(void)
{
    const int dstTbl[] = { 0x08, 0x12, 0x1b, 0x24, 0x2c, 0x35, 0x3e, 0x47, 0x51, 0x5a, 0x62, 0x6b, 0x7d, 0x8f, 0xa1, 0xb3 }; // old vel
    const int srcTbl[] = { 0x4c, 0x59, 0x6d, 0x7f, 0x87, 0x8e, 0x98, 0xa0, 0xa8, 0xb2, 0xbf, 0xcb, 0xd8, 0xe5, 0xf2, 0xfc }; // kss vel
    //const int srcTbl[] = { 0x19, 0x33, 0x4c, 0x66, 0x72, 0x7f, 0x8c, 0x99, 0xa5, 0xb2, 0xbf, 0xcc, 0xd8, 0xe5, 0xf2, 0xfc }; // new vel
    //const int dstTbl[] = { 0x33, 0x66, 0x80, 0x99, 0xb3, 0xcc, 0xe6, 0xff }; // old dur
    //const int srcTbl[] = { 0x65, 0x7f, 0x98, 0xb2, 0xcb, 0xe5, 0xf2, 0xfc }; // kss dur
    int i, j, tcnt;

	if (countof(dstTbl) != countof(srcTbl))
		return 1;
	tcnt = countof(dstTbl);

    for (i = 0; i < tcnt; i++) {
        printf("%d,%f,%f\n", i, (double) dstTbl[i] / dstTbl[tcnt-1], (double) srcTbl[i] / srcTbl[tcnt-1]);
    }
    puts("");
    for (i = 0; i < tcnt; i++) {
        double minD = 10000;
        int minI = -1;
        double rate = (double) srcTbl[i] / srcTbl[tcnt-1];

        for (j = 0; j < tcnt; j++) {
            double rate2 = (double) dstTbl[j] / dstTbl[tcnt-1];
            double diff = rate2 - rate;

            if (diff < 0)
                diff = -diff;

            if (diff < minD) {
                minD = diff; minI = j;
            }
        }
        printf("%x,%x\n", i, minI);
    }
    return 0;
}
