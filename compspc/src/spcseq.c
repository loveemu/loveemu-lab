/**
 * small routines for my spc2midi program.
 * http://loveemu.yh.land.to/
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "cioutil.h"
#include "spcseq.h"

/** return note name into buffer. */
void getNoteName (char *name, int note)
{
    //char *nameTable[] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
    char *nameTable[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int n = note % 12;
    int oct = note / 12;

    oct--;
    sprintf(name, "%s%d", nameTable[n], oct);
}

/** check if input is SPC file. */
bool isSpcSoundFile (const byte *data, size_t size)
{
    if (size < 0x10100) {
        return false;
    }

    if (memcmp(data, "SNES-SPC700 Sound File Data", 27) != 0) {
        return false;
    }

    return true;
}
