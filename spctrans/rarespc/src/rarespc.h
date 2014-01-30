/**
 * "Donkey Kong Country" Rare spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef RARESPC_H
#define RARESPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *rareSpcSetLogStreamHandle(FILE *stream);
int rareSpcSetLoopCount(int count);
int rareSpcSetSongIndex(int index);

Smf* rareSpcARAMToMidi(const byte *ARAM);
Smf* rareSpcToMidi(const byte *data, size_t size);
Smf* rareSpcToMidiFromFile(const char *filename);

bool rareSpcImportPatchFixFile(const char *filename);

#endif /* !RARESPC_H */
