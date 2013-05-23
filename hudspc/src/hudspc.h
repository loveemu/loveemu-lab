/**
 * Hudson spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef HUDSPC_H
#define HUDSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *hudsonSpcSetLogStreamHandle(FILE *stream);
int hudsonSpcSetLoopCount(int count);

Smf* hudsonSpcARAMToMidi(const byte *ARAM);
Smf* hudsonSpcToMidi(const byte *data, size_t size);
Smf* hudsonSpcToMidiFromFile(const char *filename);
bool hudsonSpcImportPatchFixFile(const char *filename);

void hudsonSpcInit(void);

#endif /* !HUDSPC_H */
