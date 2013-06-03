/**
 * Konami spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef KONSPC_H
#define KONSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *konamiSpcSetLogStreamHandle(FILE *stream);
int konamiSpcSetLoopCount(int count);

Smf* konamiSpcARAMToMidi(const byte *ARAM);
Smf* konamiSpcToMidi(const byte *data, size_t size);
Smf* konamiSpcToMidiFromFile(const char *filename);
bool konamiSpcImportPatchFixFile(const char *filename);

void konamiSpcInit(void);

#endif /* !KONSPC_H */
