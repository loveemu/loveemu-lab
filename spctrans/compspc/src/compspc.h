/**
 * "Puyo Puyo" Compile spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef COMPSPC_H
#define COMPSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *compSpcSetLogStreamHandle(FILE *stream);
int compSpcSetLoopCount(int count);
int compSpcSetSongIndex(int index);

Smf* compSpcARAMToMidi(const byte *ARAM);
Smf* compSpcToMidi(const byte *data, size_t size);
Smf* compSpcToMidiFromFile(const char *filename);

bool compSpcImportPatchFixFile(const char *filename);

#endif /* !COMPSPC_H */
