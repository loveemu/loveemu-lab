/**
 * small routines for my spc2midi program.
 * http://loveemu.yh.land.to/
 */

#ifndef SPCSEQ_H
#define SPCSEQ_H

#include "cioutil.h"

static const double spcARTable[0x10] = {
    4.1, 2.6, 1.5, 1.0, 0.640, 0.380, 0.260, 0.160,
    0.096, 0.064, 0.040, 0.024, 0.016, 0.010, 0.006, 0
};
static const double spcDRTable[0x08] = {
    1.2, 0.740, 0.440, 0.290, 0.180, 0.110, 0.074, 0.037
};
static const double spcSRTable[0x20] = {
    0, 38, 28, 24, 19, 14, 12, 9.4, 7.1, 5.9, 4.7, 3.5, 2.9, 2.4, 1.8, 1.5,
    1.2, 0.880, 0.740, 0.590, 0.440, 0.370, 0.290, 0.220, 0.180, 0.150, 0.110,
    0.0092, 0.0074, 0.0055, 0.0037, 0.0018
};

static const int spcNCKTable[0x20] = {
        0,    16,    21,    25,    31,    42,    50,    63,
       84,   100,   125,   167,   200,   250,   333,   400,
      500,   667,   800,  1000,  1300,  1600,  2000,  2700,
     3200,  4000,  5300,  6400,  8000, 10700, 16000, 32000,
};

typedef struct TagPatchFixInfo {
    int patchNo;        // patch number after fixed
    int bankSelL;       // bank number (LSB) after fixed
    int bankSelM;       // bank number (MSB) after fixed
    int key;            // transpose amount (semitones)
    int mmlKey;         // transpose amount for MML (semitones)
} PatchFixInfo;

typedef struct TagSeqEventReport {
    int track;          // track number
    int tick;           // timing (tick)
    int addr;           // address of the event
    int size;           // size of the event
    int code;           // event type (first byte)
    bool unidentified;  // unidentified event or not
    char note[256];     // note of the event
    char classStr[256]; // html classes
} SeqEventReport;

void getNoteName (char *name, int note);
bool isSpcSoundFile (const byte *data, size_t size);

#endif /* !SPCSEQ_H */
