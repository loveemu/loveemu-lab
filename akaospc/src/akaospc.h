/**
 * Square AKAO spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef AKAOSPC_H
#define AKAOSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *akaoSpcSetLogStreamHandle(FILE *stream);
int akaoSpcSetLoopCount(int count);

Smf* akaoSpcARAMToMidi(const byte *ARAM);
Smf* akaoSpcToMidi(const byte *data, size_t size);
Smf* akaoSpcToMidiFromFile(const char *filename);
bool akaoSpcImportPatchFixFile(const char *filename);

void akaoSpcInit(void);

#endif /* !AKAOSPC_H */
