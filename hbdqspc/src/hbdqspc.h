/**
 * Heart Beat DQ6/DQ3 spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef HBDQSPC_H
#define HBDQSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *hbSpcSetLogStreamHandle(FILE *stream);
int hbSpcSetLoopCount(int count);
int hbSpcSetSongIndex(int index);

Smf* hbSpcARAMToMidi(const byte *ARAM);
Smf* hbSpcToMidi(const byte *data, size_t size);
Smf* hbSpcToMidiFromFile(const char *filename);

bool hbSpcImportPatchFixFile(const char *filename);

#endif /* !HBDQSPC_H */
