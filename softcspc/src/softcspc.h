/**
 * Software Creations spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef SOFTCSPC_H
#define SOFTCSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *softcSpcSetLogStreamHandle(FILE *stream);
int softcSpcSetLoopCount(int count);

Smf* softcSpcARAMToMidi(const byte *ARAM);
Smf* softcSpcToMidi(const byte *data, size_t size);
Smf* softcSpcToMidiFromFile(const char *filename);
bool softcSpcImportPatchFixFile(const char *filename);

void softcSpcInit(void);

#endif /* !SOFTCSPC_H */
