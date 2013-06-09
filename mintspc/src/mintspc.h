/**
 * Mint spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef MINTSPC_H
#define MINTSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *mintSpcSetLogStreamHandle(FILE *stream);
int mintSpcSetLoopCount(int count);

Smf* mintSpcARAMToMidi(const byte *ARAM);
Smf* mintSpcToMidi(const byte *data, size_t size);
Smf* mintSpcToMidiFromFile(const char *filename);
bool mintSpcImportPatchFixFile(const char *filename);

void mintSpcInit(void);

#endif /* !MINTSPC_H */
