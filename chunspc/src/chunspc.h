/**
 * Chunsoft spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef CHUNSPC_H
#define CHUNSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *chunSpcSetLogStreamHandle(FILE *stream);
int chunSpcSetLoopCount(int count);

Smf* chunSpcARAMToMidi(const byte *ARAM);
Smf* chunSpcToMidi(const byte *data, size_t size);
Smf* chunSpcToMidiFromFile(const char *filename);
bool chunSpcImportPatchFixFile(const char *filename);

void chunSpcInit(void);

#endif /* !CHUNSPC_H */
