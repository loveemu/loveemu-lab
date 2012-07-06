/**
 * small routines for my spc2midi program.
 * http://loveemu.yh.land.to/
 */

#ifndef SPCSEQ_H
#define SPCSEQ_H

#include "cioutil.h"

typedef struct TagNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int len;            // total length (tick)
    bool tied;          // if the note tied
    int step;           // wait amount for note cmd
    int key;            // key
    int transpose;      // transpose
    int durRate;        // duration rate
    int velRate;        // velocity rate
    int patch;          // instrument
} NoteParam;

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
