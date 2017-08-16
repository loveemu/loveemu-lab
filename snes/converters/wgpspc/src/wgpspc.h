/**
 * Wagan Paradise spc2midi.
 * http://loveemu.yh.land.to/
 */

#ifndef WGPSPC_H
#define WGPSPC_H

#include "cioutil.h"
#include "libsmfc.h"
#include "libsmfcx.h"

FILE *wgpSpcSetLogStreamHandle(FILE *stream);
int wgpSpcSetLoopCount(int count);
int wgpSpcSetSongIndex(int index);

Smf* wgpSpcARAMToMidi(const byte *ARAM);
Smf* wgpSpcToMidi(const byte *data, size_t size);
Smf* wgpSpcToMidiFromFile(const char *filename);

bool wgpSpcImportPatchFixFile(const char *filename);

#endif /* !WGPSPC_H */
