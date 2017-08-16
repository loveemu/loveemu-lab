/**
 * Square SUZUKI Hidenori spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef SUZUHSPC_H
#define SUZUHSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *suzuhSpcSetLogStreamHandle(FILE *stream);
int suzuhSpcSetLoopCount(int count);

Smf* suzuhSpcARAMToMidi(const byte *ARAM);
Smf* suzuhSpcToMidi(const byte *data, size_t size);
Smf* suzuhSpcToMidiFromFile(const char *filename);
bool suzuhSpcImportPatchFixFile(const char *filename);

void suzuhSpcInit(void);

#endif /* !SUZUHSPC_H */
