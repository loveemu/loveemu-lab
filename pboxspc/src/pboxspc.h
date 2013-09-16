/**
 * Pandora Box spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef PBOXSPC_H
#define PBOXSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *pboxSpcSetLogStreamHandle(FILE *stream);
int pboxSpcSetLoopCount(int count);

Smf* pboxSpcARAMToMidi(const byte *ARAM);
Smf* pboxSpcToMidi(const byte *data, size_t size);
Smf* pboxSpcToMidiFromFile(const char *filename);
bool pboxSpcImportPatchFixFile(const char *filename);

void pboxSpcInit(void);

#endif /* !PBOXSPC_H */
