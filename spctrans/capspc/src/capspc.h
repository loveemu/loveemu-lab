/**
 * Capcom spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef CAPSPC_H
#define CAPSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *capSpcSetLogStreamHandle (FILE *stream);
int capSpcSetLoopCount (int count);

Smf* capSpcARAMToMidi (const byte *ARAM);
Smf* capSpcToMidi (const byte *data, size_t size);
Smf* capSpcToMidiFromFile (const char *filename);
bool capSpcImportPatchFixFile (const char *filename);

#endif /* !CAPSPC_H */
