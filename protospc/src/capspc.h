/**
 * Kirby Super Star spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef NINTSPC_H
#define NINTSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *nintSpcSetLogStreamHandle(FILE *stream);
int nintSpcSetLoopCount(int count);

Smf* nintSpcARAMToMidi(const byte *ARAM);
Smf* nintSpcToMidi(const byte *data, size_t size);
Smf* nintSpcToMidiFromFile(const char *filename);
bool nintSpcImportPatchFixFile(const char *filename);

void nintSpcInit(void);

#endif /* !NINTSPC_H */
