/**
 * Hudson SFX SOUND DRIVER spc2midi.
 * Based on Super Bomberman 3 analysis.
 * http://loveemu.yh.land.to/
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "spcseq.h"
#include "hudspc.h"

#define APPNAME "Hudson SPC2MIDI"
#define APPSHORTNAME "hudspc"
#define VERSION "[2014-02-15]"

static int hudsonSpcLoopMax = 2;            // maximum loop count of parser
static int hudsonSpcTextLoopMax = 1;        // maximum loop count of text output
static double hudsonSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool hudsonSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool hudsonSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int hudsonSpcTimeBase = 48;
static int hudsonSpcForceSongIndex = -1;
static int hudsonSpcForceSongListAddr = -1;

static bool hudsonSpcPatchFixOverride = false;
static PatchFixInfo hudsonSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int hudsonSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    // Earlier Version:
    // Super Bomberman 2, Hagane
    SPC_VER_V0,
    // Version 1.xx:
    // Super Bomberman 3 (1.16)
    // Super Genjin 2 (1.16E)
    // Caravan Shooting Collection (1.17s)
    SPC_VER_V1,
    // Ver 2.xx
    // TODO: An American Tail: Fievel Goes West (No Version String)
    // Do-Re-Mi Fantasy (2.10)
    // Tengai Makyou Zero (2.27b)
    // Super Bomberman 4 (2.28)
    // Kishin Douji Zenki 3 (2.28)
    // Same Game (2.30a)
    // Super Bomberman 5 (2.32)
    // Bomberman B-Daman (2.32)
    SPC_VER_V2,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

#define HUDSPC_VERSION_STRING_HEADER_L  "SFX SOUND DRIVER Version " // Version 1
#define HUDSPC_VERSION_STRING_HEADER_S  "SFX SOUND DRIVER Ver "     // Version 2

typedef struct TagHudsonSpcTrackStat HudsonSpcTrackStat;
typedef struct TagHudsonSpcSeqStat HudsonSpcSeqStat;
typedef void (*HudsonSpcEvent) (HudsonSpcSeqStat *, SeqEventReport *);

typedef struct TagHudsonSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int seqHeaderAddr;
    HudsonSpcEvent event[256]; // vcmds
    HudsonSpcEvent subevent[256]; // subcmds
    PatchFixInfo patchFix[256];
    char versionString[256];
    int versionStringAddr;
    int versionNumber;
    bool seqDetected;
} HudsonSpcVerInfo;

typedef struct TagHudsonSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} HudsonSpcNoteParam;

struct TagHudsonSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int trackLoopAddr;      // track loop address on ARAM
    bool trackLoopIsSet;    // track loop flag to prevent double run
    bool rhythmChannel;     // rhythm channel / melody channel
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    HudsonSpcNoteParam note;     // current note param
    HudsonSpcNoteParam lastNote; // note params for last note
    int looped;             // how many times looped (internal)
    byte quantize;          // quantize (0-8: qN, 9-: @qN)
    byte octave;            // octave
    byte volume;            // voice volume
    int patch;              // patch number (for pitch fix)
    byte callStack[0x10];   // subroutine stack
    int callStackPtr;       // subroutine stack ptr
    int callStackSize;      // subroutine stack size
};

struct TagHudsonSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    byte timebaseShift;         // timebase shift amount
    int hdTimebaseShift;        // header - timebase shift amount
    int hdInstTableAddr;        // header - instrument table address
    int hdInstTableSize;        // header - instrument table size
    int hdRhythmTableAddr;      // header - rhythm kit table address
    int hdRhythmTableSize;      // header - rhythm kit table size
    int hdUnkAddrTableAddr;     // header - unknown address table address
    int hdUnkAddrTableSize;     // header - unknown address table size
    int hdEchoParamAddr;        // header - initial echo params address
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    HudsonSpcVerInfo ver;       // game version info
    HudsonSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
    byte reg[0x100];            // user RAM work area
    int regCount;               // user RAM work area size
    byte regCmp;                // value for user RAM comparison
    bool regCarry;              // carry flag for user RAM comparison
};

static void hudsonSpcSetEventList (HudsonSpcSeqStat *seq);

//----

static FILE *mystdout = NULL;
static int myprintf (const char *format, ...)
{
    va_list va;
    int result = 0;

    if (mystdout) {
        va_start(va, format);
        result = vfprintf(mystdout, format, va);
        va_end(va);
    }
    return result;
}

//----

/** sets html stream to new target. */
FILE *hudsonSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int hudsonSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = hudsonSpcLoopMax;
    hudsonSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool hudsonSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        hudsonSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        hudsonSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            hudsonSpcPatchFix[patch].bankSelM = patch >> 7;
            hudsonSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            hudsonSpcPatchFix[patch].bankSelM = 0;
            hudsonSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        hudsonSpcPatchFix[patch].patchNo = patch & 0x7f;
        hudsonSpcPatchFix[patch].key = 0;
        hudsonSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        hudsonSpcPatchFix[src].bankSelM = bankM & 0x7f;
        hudsonSpcPatchFix[src].bankSelL = bankL & 0x7f;
        hudsonSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        hudsonSpcPatchFix[src].key = key;
        hudsonSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    hudsonSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *hudsonSpcVerToStrHtml (HudsonSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_V0:
        return "SFX SOUND DRIVER Version x.xx (Earlier Version)";
    case SPC_VER_V1:
        return "SFX SOUND DRIVER Version 1.16";
        //switch(seq->ver.versionNumber)
        //{
        //case 116:
        //    return "SFX SOUND DRIVER Version 1.16";
        //case 117:
        //    return "SFX SOUND DRIVER Version 1.17s";
        //default:
        //    if (seq->ver.versionStringAddr != -1)
        //    {
        //        return seq->ver.versionString;
        //    }
        //    else
        //    {
        //        return "SFX SOUND DRIVER Version 1.xx";
        //    }
        //}
    case SPC_VER_V2:
        return "SFX SOUND DRIVER Ver 2.32";
        //switch(seq->ver.versionNumber)
        //{
        //case 210:
        //    return "SFX SOUND DRIVER Ver 2.10";
        //case 227:
        //    return "SFX SOUND DRIVER Ver 2.27b";
        //case 228:
        //    return "SFX SOUND DRIVER Ver 2.28";
        //case 230:
        //    return "SFX SOUND DRIVER Ver 2.30a";
        //case 232:
        //    return "SFX SOUND DRIVER Ver 2.32";
        //default:
        //    if (seq->ver.versionStringAddr != -1)
        //    {
        //        return seq->ver.versionString;
        //    }
        //    else
        //    {
        //        return "SFX SOUND DRIVER Version 2.xx";
        //    }
        //}
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void hudsonSpcResetTrackParam (HudsonSpcSeqStat *seq, int track)
{
    HudsonSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->octave = 2;
    tr->quantize = 8;
    tr->volume = 100;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->callStackPtr = 0;
    tr->callStackSize = 0x10;
    tr->trackLoopIsSet = false;
    tr->rhythmChannel = false;
}

/** reset before play/convert song. */
static void hudsonSpcResetParam (HudsonSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 120;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    memset(seq->reg, 0, sizeof(seq->reg));
    seq->regCount = 8;
    seq->regCmp = 0;
    seq->regCarry = false;

    seq->timebaseShift = 2;
    seq->hdTimebaseShift = -1;
    seq->hdInstTableAddr = -1;
    seq->hdRhythmTableAddr = -1;
    seq->hdUnkAddrTableAddr = -1;
    seq->hdEchoParamAddr = -1;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        HudsonSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        hudsonSpcResetTrackParam(seq, track);
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            seq->ver.patchFix[patch].bankSelM = patch >> 7;
            seq->ver.patchFix[patch].bankSelL = 0;
        }
        else
        {
            seq->ver.patchFix[patch].bankSelM = 0;
            seq->ver.patchFix[patch].bankSelL = patch >> 7;
        }
        seq->ver.patchFix[patch].patchNo = patch & 0x7f;
        seq->ver.patchFix[patch].key = 0;
        seq->ver.patchFix[patch].mmlKey = 0;
    }
    // copy patch fix if needed
    if (hudsonSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &hudsonSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }

}

/** returns what version the sequence is, and sets individual info. */
static int hudsonSpcCheckVer (HudsonSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;
    int versionStringHeaderLen = strlen(HUDSPC_VERSION_STRING_HEADER_L);
    int v1xxSetSongTableAddr = -1;

    seq->timebase = hudsonSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.seqDetected = false;

    seq->ver.versionNumber = 0;
    strcpy(seq->ver.versionString, "");
    // Version 1?
    seq->ver.versionStringAddr = indexOfHexPat(aRAM, HUDSPC_VERSION_STRING_HEADER_L, SPC_ARAM_SIZE, NULL);
    // Version 2?
    if (seq->ver.versionStringAddr == -1)
    {
        seq->ver.versionStringAddr = indexOfHexPat(aRAM, HUDSPC_VERSION_STRING_HEADER_S, SPC_ARAM_SIZE, NULL);
        versionStringHeaderLen = strlen(HUDSPC_VERSION_STRING_HEADER_S);
    }
    // grab version string from RAM
    if (seq->ver.versionStringAddr != -1)
    {
        int posEndOfVersion = seq->ver.versionStringAddr + versionStringHeaderLen;
        int versionStringLength;
        while (posEndOfVersion < SPC_ARAM_SIZE && (aRAM[posEndOfVersion] != ' ' && aRAM[posEndOfVersion] != ','))
        {
            posEndOfVersion++;
        }
        versionStringLength = posEndOfVersion - seq->ver.versionStringAddr;
        strncpy(seq->ver.versionString, &aRAM[seq->ver.versionStringAddr], versionStringLength);
        seq->ver.versionString[versionStringLength] = '\0';

        // grab version number
        if (aRAM[seq->ver.versionStringAddr + versionStringHeaderLen + 1] == '.')
        {
            seq->ver.versionNumber =
                (aRAM[seq->ver.versionStringAddr + versionStringHeaderLen] - '0') * 100
                + (aRAM[seq->ver.versionStringAddr + versionStringHeaderLen + 2] - '0') * 10
                + (aRAM[seq->ver.versionStringAddr + versionStringHeaderLen + 3] - '0');
        }
    }

    // if version string is available, detect version by using it
    if (seq->ver.versionStringAddr != -1)
    {
        if (seq->ver.versionNumber >= 100 && seq->ver.versionNumber <= 199)
        {
            version = SPC_VER_V1;
        }
        else if (seq->ver.versionNumber >= 200 && seq->ver.versionNumber <= 299)
        {
            version = SPC_VER_V2;
        }
        else
        {
            version = SPC_VER_UNKNOWN;
        }
    }
    else
    {
        // unknown version without no version string, then,
        // search note length table, to know if the song is the variant of SFX SOUND DRIVER
        int noteLenTableAddr = indexOfHexPat(aRAM, "\xc0\x60\x30\x18\x0c\x06\x03\x01", SPC_ARAM_SIZE, NULL);
        if (noteLenTableAddr != -1)
        {
            int v1xxSetSongTableAddr = indexOfHexPat(aRAM, "\\\xf6..\xc4\x0d\\\xfc\\\xf6..\xc4\x0e\x2f", SPC_ARAM_SIZE, NULL);
            if (v1xxSetSongTableAddr != -1 && mget2l(&aRAM[v1xxSetSongTableAddr + 1]) != mget2l(&aRAM[v1xxSetSongTableAddr + 7]))
            {
                v1xxSetSongTableAddr = -1;
            }

            // TODO: check assembly code and detect the version
            if (v1xxSetSongTableAddr != -1)
            {
                seq->ver.versionNumber = 1;
                version = SPC_VER_V0;
            }
            else
            {
                // set default version
                seq->ver.versionNumber = 232;
                version = SPC_VER_V2;
            }
        }
    }

    if (hudsonSpcForceSongListAddr >= 0)
    {
        seq->ver.seqListAddr = hudsonSpcForceSongListAddr;
        if (version == SPC_VER_UNKNOWN)
        {
            // force set default version
            seq->ver.versionNumber = 232;
            version = SPC_VER_V2;
        }
    }
    else if (version != SPC_VER_UNKNOWN) {
        switch(version)
        {
        case SPC_VER_V0:
            seq->ver.seqListAddr = mget2l(&aRAM[0x0d]);
            break;
        case SPC_VER_V1:
            seq->ver.seqListAddr = mget2l(&aRAM[0x07c2]);
            break;
        case SPC_VER_V2:
            seq->ver.seqListAddr = mget2l(&aRAM[0x0803]);
            break;
        }
    }

    if (hudsonSpcForceSongIndex >= 0) {
        seq->ver.songIndex = hudsonSpcForceSongIndex;
    }
    else {
        seq->ver.songIndex = 0; // TODO: NYI: autosearch
    }

    if (seq->ver.seqListAddr >= 0 && seq->ver.songIndex >= 0) {
        int seqHeaderAddrPtr = mget2l(&aRAM[seq->ver.seqListAddr]) + (seq->ver.songIndex * 2);
        if (seqHeaderAddrPtr < SPC_ARAM_SIZE) {
            seq->ver.seqHeaderAddr = mget2l(&aRAM[seqHeaderAddrPtr]);
        }
        else {
            version = SPC_VER_UNKNOWN;
        }
    }

    seq->ver.id = version;
    hudsonSpcSetEventList(seq);
    return version;
}

/** extra header 01: set channel addresses. */
static bool hudsonSpcSeqHedSetTrackAddresses (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    bool result = false;
    int seqHeaderReadPtr = *pSeqHeaderReadPtr;
    byte trActiveBits;
    int tr;

    trActiveBits = mget1(&seq->aRAM[seqHeaderReadPtr]);
    seqHeaderReadPtr++;
    // track list (reverse order)
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        // if active, read more info
        if ((trActiveBits & (1 << tr)) != 0) {
            seq->track[tr].pos = mget2l(&seq->aRAM[seqHeaderReadPtr]);
            seq->track[tr].trackLoopAddr = seq->track[tr].pos;
            seqHeaderReadPtr += 2;

            seq->track[tr].active = true;
            result = true;
        }
    }

    *pSeqHeaderReadPtr = seqHeaderReadPtr;
    return result;
}

/** extra header 02: set timebase (note length shift). */
static bool hudsonSpcSeqHedSetTimeBase (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    seq->timebaseShift = seq->aRAM[*pSeqHeaderReadPtr] & 0x03;
    seq->hdTimebaseShift = seq->timebaseShift;
    (*pSeqHeaderReadPtr)++;
    return true;
}

/** extra header 03: set instrument table (old, in bytes). */
static bool hudsonSpcSeqHedSetInstrumentTable (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr] * 4;
    (*pSeqHeaderReadPtr)++;
    seq->hdInstTableAddr = *pSeqHeaderReadPtr;
    seq->hdInstTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** extra header 03 old: set instrument table (old, in bytes). */
static bool hudsonSpcSeqHedSetInstrumentTableOld (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr];
    (*pSeqHeaderReadPtr)++;
    seq->hdInstTableAddr = *pSeqHeaderReadPtr;
    seq->hdInstTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** extra header 04: set rhythm kit table (old, in bytes). */
static bool hudsonSpcSeqHedSetRhythmTable (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr] * 4;
    (*pSeqHeaderReadPtr)++;
    seq->hdRhythmTableAddr = *pSeqHeaderReadPtr;
    seq->hdRhythmTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** extra header 04 old: set rhythm kit table (old, in bytes). */
static bool hudsonSpcSeqHedSetRhythmTableOld (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr];
    (*pSeqHeaderReadPtr)++;
    seq->hdRhythmTableAddr = *pSeqHeaderReadPtr;
    seq->hdRhythmTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** extra header 05: set unknown address table. */
static bool hudsonSpcSeqHed05 (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr] * 2;
    (*pSeqHeaderReadPtr)++;
    seq->hdUnkAddrTableAddr = *pSeqHeaderReadPtr;
    seq->hdUnkAddrTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** extra header 06: set unknown table. */
static bool hudsonSpcSeqHed06 (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr] * 2;
    (*pSeqHeaderReadPtr)++;
    //seq->hdUnkAddrTableAddr = *pSeqHeaderReadPtr;
    //seq->hdUnkAddrTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** extra header 07: set initial echo param. */
static bool hudsonSpcSeqHedSetEchoParam (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    int arg1;
    seq->hdEchoParamAddr = *pSeqHeaderReadPtr;

    arg1 = seq->aRAM[*pSeqHeaderReadPtr];
    (*pSeqHeaderReadPtr)++;

    if (arg1 == 0)
    {
        (*pSeqHeaderReadPtr) += 6;
    }
    return true;
}

/** extra header 08: set unknown byte. */
static bool hudsonSpcSeqHed08 (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    int arg1 = seq->aRAM[*pSeqHeaderReadPtr];
    (*pSeqHeaderReadPtr)++;
    return true;
}

/** extra header 09: set unknown table. */
static bool hudsonSpcSeqHed09 (HudsonSpcSeqStat *seq, int *pSeqHeaderReadPtr)
{
    byte tableSize;

    tableSize = seq->aRAM[*pSeqHeaderReadPtr] * 2;
    (*pSeqHeaderReadPtr)++;
    //seq->hdUnkAddrTableAddr = *pSeqHeaderReadPtr;
    //seq->hdUnkAddrTableSize = tableSize;
    (*pSeqHeaderReadPtr) += tableSize;
    return true;
}

/** detect now playing and prepare for analyze. */
static bool hudsonSpcDetectSeq (HudsonSpcSeqStat *seq)
{
    int seqHeaderReadPtr;
    bool result = true;
    byte extraHeaderEvent;
    int extraHeaderEventAddr = -1;
    int numHeaderEventCount;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    switch(seq->ver.id)
    {
    case SPC_VER_V2:
        numHeaderEventCount = 10;
        break;
    default:
        numHeaderEventCount = 6;
        break;
    }

    seqHeaderReadPtr = seq->ver.seqHeaderAddr;
    hudsonSpcResetParam(seq);

    // read channel addresses, for version 1.x
    if (numHeaderEventCount == 6)
    {
        result &= hudsonSpcSeqHedSetTrackAddresses(seq, &seqHeaderReadPtr);
        if (!result)
        {
            return false;
        }
    }

    // read extra header
    while (true)
    {
        // prevent access violation
        if (seqHeaderReadPtr >= 0xffff)
        {
            fprintf(stderr, "Error: Access violation in extra header\n");
            result = false;
            break;
        }

        // end of header
        extraHeaderEvent = seq->aRAM[seqHeaderReadPtr];
        extraHeaderEventAddr = seqHeaderReadPtr;
        if (extraHeaderEvent == 0x00)
        {
            break;
        }

        // advance pointer, prevent access violation
        seqHeaderReadPtr++;
        if (seqHeaderReadPtr >= 0xffff)
        {
            fprintf(stderr, "Error: Access violation in extra header\n");
            result = false;
            break;
        }

        // check range
        if (extraHeaderEvent >= numHeaderEventCount)
        {
            fprintf(stderr, "Error: Unknown header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
            result = false;
            break;
        }

        // dispatch
        if (numHeaderEventCount == 6) // SPC_VER_V0, SPC_VER_V1
        {
            // Ver 1.x
            switch (extraHeaderEvent)
            {
            // set timebase
            case 0x01:
                result &= hudsonSpcSeqHedSetTimeBase(seq, &seqHeaderReadPtr);
                break;

            // set instrument table (old)
            case 0x02:
                result &= hudsonSpcSeqHedSetInstrumentTableOld(seq, &seqHeaderReadPtr);
                break;

            // set rhythm kit table
            case 0x03:
                result &= hudsonSpcSeqHedSetRhythmTableOld(seq, &seqHeaderReadPtr);
                break;

            // set instrument table
            case 0x04:
                result &= hudsonSpcSeqHedSetInstrumentTable(seq, &seqHeaderReadPtr);
                break;

            // set unknown address table
            case 0x05:
                fprintf(stderr, "Warning: Skipped unknown header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result &= hudsonSpcSeqHed05(seq, &seqHeaderReadPtr);
                break;

            default:
                fprintf(stderr, "Error: Unidentified header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result = false;
                break;
            }
        }
        else if (numHeaderEventCount == 10) // SPC_VER_V2
        {
            // Ver 2.x
            switch (extraHeaderEvent)
            {
            // set channel addresses
            case 0x01:
                result &= hudsonSpcSeqHedSetTrackAddresses(seq, &seqHeaderReadPtr);
                break;

            // set timebase
            case 0x02:
                result &= hudsonSpcSeqHedSetTimeBase(seq, &seqHeaderReadPtr);
                break;

            // set instrument table
            case 0x03:
                result &= hudsonSpcSeqHedSetInstrumentTable(seq, &seqHeaderReadPtr);
                break;

            // set rhythm kit table
            case 0x04:
                result &= hudsonSpcSeqHedSetRhythmTable(seq, &seqHeaderReadPtr);
                break;

            // set unknown address table
            case 0x05:
                fprintf(stderr, "Warning: Skipped unknown header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result &= hudsonSpcSeqHed05(seq, &seqHeaderReadPtr);
                break;

            // 
            case 0x06:
                fprintf(stderr, "Warning: Skipped unknown header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result &= hudsonSpcSeqHed06(seq, &seqHeaderReadPtr);
                break;

            // 
            case 0x07:
                result &= hudsonSpcSeqHedSetEchoParam(seq, &seqHeaderReadPtr);
                break;

            // 
            case 0x08:
                fprintf(stderr, "Warning: Skipped unknown header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result &= hudsonSpcSeqHed08(seq, &seqHeaderReadPtr);
                break;

            // 
            case 0x09:
                fprintf(stderr, "Warning: Skipped unknown header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result &= hudsonSpcSeqHed09(seq, &seqHeaderReadPtr);
                break;

            default:
                fprintf(stderr, "Error: Unidentified header event %02X at $%04X\n", extraHeaderEvent, extraHeaderEventAddr);
                result = false;
                break;
            }
        }

        if (!result)
        {
            break;
        }
    }
    return result;
}

/** create new spc2mid object. */
static HudsonSpcSeqStat *newHudsonSpcSeq (const byte *aRAM)
{
    HudsonSpcSeqStat *newSeq = (HudsonSpcSeqStat *) calloc(1, sizeof(HudsonSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        hudsonSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = hudsonSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delHudsonSpcSeq (HudsonSpcSeqStat **seq)
{
    if (*seq) {
        // do not kill smf here

        free(*seq);
        *seq = NULL;
    }
}

//----

/** output html header. */
static void printHtmlHeader (void)
{
    myprintf("<?xml version=\"1.0\" ?>\n");
    myprintf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
    myprintf("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n");
    myprintf("  <head>\n");
    myprintf("    <link rel=\"stylesheet\" type=\"text/css\" media=\"screen,tv,projection\" href=\"%s\" />\n", mycssfile);
    myprintf("    <title>Data View - %s %s</title>\n", APPNAME, VERSION);
    myprintf("  </head>\n");
    myprintf("  <body>\n");
}

/** output html footer. */
static void printHtmlFooter (void)
{
    myprintf("  </body>\n");
    myprintf("</html>\n");
}

/** output seq info list. */
static void printHtmlInfoList (HudsonSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    if (seq->ver.versionStringAddr != -1)
    {
        myprintf("          <li>Version: %s at $%04X", seq->ver.versionString, seq->ver.versionStringAddr);
        if (strcmp(seq->ver.versionString, hudsonSpcVerToStrHtml(seq)) != 0)
        {
            myprintf("<ul><li>Interpreted as %s</li></ul>", hudsonSpcVerToStrHtml(seq));
        }
        myprintf("</li>\n");
    }
    else
    {
        myprintf("          <li>Version (Guess): %s</li>\n", hudsonSpcVerToStrHtml(seq));
    }
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    myprintf(" (Song $%02x)", seq->ver.songIndex);
    myprintf("</li>\n");
    myprintf("          <li>Timebase: %d (48 >> %d)</li>\n", 48 >> seq->hdTimebaseShift, seq->hdTimebaseShift);
    if (seq->hdInstTableAddr >= 0)
    {
        myprintf("          <li>Instrument Table: $%04X (%d bytes)</li>\n", seq->hdInstTableAddr, seq->hdInstTableSize);
    }
    if (seq->hdRhythmTableAddr >= 0)
    {
        myprintf("          <li>Rhythm Kit Table: $%04X (%d bytes)</li>\n", seq->hdRhythmTableAddr, seq->hdRhythmTableSize);
    }
    if (seq->hdUnkAddrTableAddr >= 0)
    {
        myprintf("          <li>Unknown Address Table: $%04X (%d bytes)</li>\n", seq->hdUnkAddrTableAddr, seq->hdUnkAddrTableSize);
    }
    if (seq->hdEchoParamAddr >= 0)
    {
        myprintf("          <li>Echo parameters defined at $%04X</li>\n", seq->hdEchoParamAddr);
    }
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (HudsonSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int i;

    if (seq == NULL || ev == NULL)
        return;

    myprintf("            <tr class=\"track%d %s\">", ev->track + 1, ev->classStr);
    myprintf("<td class=\"track\">%d</td>", ev->track + 1);
    myprintf("<td class=\"tick\">%d</td>", ev->tick);
    myprintf("<td class=\"address\">$%04X</td>", ev->addr);
    myprintf("<td class=\"hex\">");

    // hex dump
    for (i = 0; i < ev->size; i++) {
        if (i > 0)
            myprintf(" ");
        myprintf("%02X", seq->aRAM[ev->addr + i]);
    }
    myprintf("</td>");
    myprintf("<td class=\"note\">%s</td>", ev->note);
    myprintf("</tr>\n");
}

/** output event table header. */
static void printEventTableHeader (HudsonSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("        <h3>Sequence</h3>\n");
    myprintf("        <div class=\"section\">\n");
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

//----

/** output event table footer. */
static void printEventTableFooter (HudsonSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double hudsonSpcTempo (HudsonSpcSeqStat *seq)
{
    return (double) seq->tempo; // do not care about numerical error atm
}

/** convert SPC velocity into MIDI one. */
static int hudsonSpcMidiVelOf (int value)
{
    if (hudsonSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int hudsonSpcMidiVolOf (int value)
{
    if (hudsonSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** create new smf object and link to spc seq. */
static Smf *hudsonSpcCreateSmf (HudsonSpcSeqStat *seq)
{
    static char songTitle[512];
    Smf* smf;
    int tr;
    byte echoOnBits = 0x00;

    smf = smfCreate(seq->timebase);
    if (!smf)
        return NULL;
    seq->smf = smf;

    sprintf(songTitle, "%s %s", APPNAME, VERSION);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, songTitle);

    switch (hudsonSpcMidiResetType) {
      case SMF_RESET_GS:
        smfInsertGM1SystemOn(smf, 0, 0, 0);
        smfInsertSysex(smf, 0, 0, 0, (const byte *) "\xf0\x41\x10\x42\x12\x40\x00\x7f\x00\x41\xf7", 11);
        break;
      case SMF_RESET_XG:
        smfInsertGM1SystemOn(smf, 0, 0, 0);
        smfInsertSysex(smf, 0, 0, 0, (const byte *) "\xf0\x43\x10\x4c\x00\x00\x7e\x00\xf7", 9);
        break;
      case SMF_RESET_GM2:
        smfInsertGM1SystemOn(smf, 0, 0, 0);
        smfInsertSysex(smf, 0, 0, 0, (const byte *) "\xf0\x7e\x7f\x09\x03\xf7", 6);
        break;
      default:
        smfInsertGM1SystemOn(smf, 0, 0, 0);
    }
    smfInsertTempoBPM(smf, 0, 0, hudsonSpcTempo(seq));

    // put track name first
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].pos);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }

    if (seq->hdEchoParamAddr != -1)
    {
        bool echoUserDefined = (seq->aRAM[seq->hdEchoParamAddr] == 0);
        byte EVOL_L, EVOL_R, EDL, EFB, FIR;
        char echoDumpText[256];

        if (echoUserDefined)
        {
            EVOL_L = seq->aRAM[seq->hdEchoParamAddr + 1];
            EVOL_R = seq->aRAM[seq->hdEchoParamAddr + 2];
            EDL = seq->aRAM[seq->hdEchoParamAddr + 3];
            EFB = seq->aRAM[seq->hdEchoParamAddr + 4];
            FIR = seq->aRAM[seq->hdEchoParamAddr + 5];
            echoOnBits = seq->aRAM[seq->hdEchoParamAddr + 6];
            sprintf(echoDumpText, "Header: Use User-Defined Echo Params, EVOL(L) = %d, EVOL(R) = %d, EDL = %d, EFB = %d, FIR = %d, EON = %02X", EVOL_L, EVOL_R, EDL, EFB, FIR, echoOnBits);
        }
        else
        {
            EVOL_L = seq->aRAM[0x0858 + 1];
            EVOL_R = seq->aRAM[0x0858 + 2];
            EDL = seq->aRAM[0x0858 + 3];
            EFB = seq->aRAM[0x0858 + 4];
            FIR = seq->aRAM[0x0858 + 5];
            echoOnBits = seq->aRAM[0x0858 + 6];
            sprintf(echoDumpText, "Header: Use Default Echo Params, EVOL(L) = %d, EVOL(R) = %d, EDL = %d, EFB = %d, FIR = %d, EON = %02X", EVOL_L, EVOL_R, EDL, EFB, FIR, echoOnBits);
        }
        smfInsertMetaText(seq->smf, 0, 0, SMF_META_TEXT, echoDumpText);
    }

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, hudsonSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, (echoOnBits & (1 << tr)) ? 100 : 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void hudsonSpcTruncateNote (HudsonSpcSeqStat *seq, int track)
{
    HudsonSpcTrackStat *tr = &seq->track[track];

    if (tr->lastNote.active && tr->lastNote.dur > 0) {
        int lastTick = tr->lastNote.tick + tr->lastNote.dur;
        int diffTick = lastTick - seq->tick;

        if (diffTick > 0) {
            tr->lastNote.dur -= diffTick;
            if (tr->lastNote.dur == 0)
                tr->lastNote.active = false;
        }
    }
}

/** truncate note for each track. */
static void hudsonSpcTruncateNoteAll (HudsonSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        hudsonSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool hudsonSpcDequeueNote (HudsonSpcSeqStat *seq, int track)
{
    HudsonSpcTrackStat *tr = &seq->track[track];
    HudsonSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel = 100;

        dur = lastNote->dur;
        if (dur == 0)
            dur++;

        key = lastNote->key + lastNote->transpose
            + seq->ver.patchFix[tr->lastNote.patch].key
            + (tr->rhythmChannel ? 0 : SPC_NOTE_KEYSHIFT);
        //vel = lastNote->vel;
        //if (vel == 0)
        //    vel++;

        result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void hudsonSpcDequeueNoteAll (HudsonSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        hudsonSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void hudsonSpcInactiveTrack(HudsonSpcSeqStat *seq, int track)
{
    int tr;

    seq->track[track].active = false;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            return;
    }
    seq->active = false;
}

/** increment loop count. */
static void hudsonSpcAddTrackLoopCount(HudsonSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (hudsonSpcLoopMax > 0) ? hudsonSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= hudsonSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void hudsonSpcSeqAdvTick(HudsonSpcSeqStat *seq)
{
    int minTickStep = 0;
    int tr;

    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        if (seq->track[tr].active) {
            if (minTickStep == 0)
                minTickStep = seq->track[tr].tick - seq->tick;
            else
                minTickStep = min(minTickStep, seq->track[tr].tick - seq->tick);
        }
    }
    seq->tick += minTickStep;
    seq->time += (double) 60 / hudsonSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void hudsonSpcEventUnknownInline (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
}

/** vcmds: unidentified event. */
static void hudsonSpcEventUnidentified (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    hudsonSpcEventUnknownInline(seq, ev);
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void hudsonSpcEventUnknown0 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    hudsonSpcEventUnknownInline(seq, ev);
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void hudsonSpcEventUnknown1 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    hudsonSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void hudsonSpcEventUnknown2 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    hudsonSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void hudsonSpcEventUnknown3 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    hudsonSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void hudsonSpcEventNOP (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** vcmds: no operation (2 bytes). */
static void hudsonSpcEventNOP2 (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "NOP2, arg1 = %d", arg1);
}

/** vcmd 00-cf: note. */
static void hudsonSpcEventNote (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int note = 0;
    int len;
    int dur;
    bool rest;

    int lenBits = noteByte & 7;
    bool tieBit = (noteByte & 8) != 0;
    bool slur;
    bool tie;
    int keyBits = noteByte >> 4;

    if (lenBits != 0) {
        const byte lenTbl[8] = { 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01 };
        // actual driver does index += ($b4 & 3), to implement timebase shift,
        // this tool does not emulate it, but handles it in other ways.
        int index = (lenBits - 1);
        if (index >= 8) {
            fprintf(stderr, "Note length index overflow, index = %d\n", index);
            index = 7;
        }
        len = lenTbl[index];
    }
    else {
        ev->size++;
        len = mget1(&seq->aRAM[*p]);
        len <<= seq->timebaseShift;
        (*p)++;
    }

    if (!tieBit) {
        if (tr->quantize <= 8) {
            dur = len * tr->quantize / 8;
        }
        else {
            dur = len - ((tr->quantize - 8) << seq->timebaseShift);
            if (dur < 0) {
                dur = 0; // really?
            }
        }
    }
    else {
        dur = len;
    }

    rest = (keyBits == 0);
    if (!rest) {
        note = (tr->octave * 12) + (keyBits - 1);
    }

    slur = (!rest && tr->lastNote.active && tr->lastNote.tied &&
        tr->lastNote.key != note);
    tie = (!rest && tr->lastNote.active && tr->lastNote.tied &&
        tr->lastNote.key == note);

    if (rest) {
        sprintf(ev->note, "Rest%s, len = %d", tieBit ? (slur ? " (Slur)" : " (Tied)") : "", len);
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, note + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key
            + (tr->rhythmChannel ? 0 : SPC_NOTE_KEYSHIFT));
        sprintf(argDumpStr, "%s, len = %d", tieBit ? (slur ? " (Slur)" : " (Tied)") : "", len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    if (!rest && slur)
    {
        if (!hudsonSpcLessTextInSMF)
           smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }

    // output old note first
    if (!tie)
    {
        hudsonSpcDequeueNote(seq, ev->track);
    }

    // set new note
    if (!rest) {
        if (tie) {
            tr->lastNote.dur += dur;
        }
        else {
            tr->lastNote.tick = ev->tick;
            tr->lastNote.dur = dur;
            tr->lastNote.key = note;
            //tr->lastNote.vel = tr->note.vel;
            tr->lastNote.transpose = seq->transpose + tr->note.transpose;
            tr->lastNote.patch = tr->note.patch;
        }
        tr->lastNote.tied = tieBit;
        tr->lastNote.active = true;
    }
    tr->tick += len;
}

/** vcmd d1: set tempo. */
static void hudsonSpcEventSetTempo (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;

    sprintf(ev->note, "Set Tempo, tempo = %.1f", hudsonSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, hudsonSpcTempo(seq));
}

/** vcmd d2: set octave. */
static void hudsonSpcEventSetOctave (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Octave, octave = %d", arg1);
    strcat(ev->classStr, " ev-octave");

    if (arg1 > 5) {
        arg1 = 5;
    }
    tr->octave = arg1;
}

/** vcmd d3: increase octave. */
static void hudsonSpcEventOctaveUp (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    if (tr->octave < 5) {
        tr->octave++;
    }

    sprintf(ev->note, "Octave Up, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octaveup");
}

/** vcmd d4: decrease octave. */
static void hudsonSpcEventOctaveDown (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    if (tr->octave > 0) {
        tr->octave--;
    }

    sprintf(ev->note, "Octave Down, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octavedown");
}

/** vcmd d5: set quantize. */
static void hudsonSpcEventSetQuantize (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->quantize = arg1;

    sprintf(ev->note, "Set Quantize, q = %d", arg1);
    strcat(ev->classStr, " ev-quantize");
}

/** vcmd d6: set instrument. */
static void hudsonSpcEventInstrument (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->note.patch = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[arg1].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[arg1].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[arg1].patchNo);

    sprintf(ev->note, "Set Instrument, patch = %d", arg1);
    strcat(ev->classStr, " ev-patch");
}

/** vcmd d9: set volume. */
static void hudsonSpcEventVolume (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->volume = arg1;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, hudsonSpcMidiVolOf(tr->volume));
}

/** vcmd da: set panpot. */
static void hudsonSpcEventPanpot (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int panIndex;
    int panValue;
    byte leftVolScale;
    byte rightVolScale;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    const byte panTable[] = {
        0x00, 0x07, 0x0d, 0x14, 0x1a, 0x21, 0x27, 0x2e,
        0x34, 0x3a, 0x40, 0x45, 0x4b, 0x50, 0x55, 0x5a,
        0x5e, 0x63, 0x67, 0x6b, 0x6e, 0x71, 0x74, 0x77,
        0x79, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x7f,
    };

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    // Note: if arg1 is out of range, the music engine will perform buggy a little, after all.
    panIndex = (arg1 & 0x1f);
    if (panIndex > 0x1e) {
        panIndex = 0x1e;
    }
    leftVolScale = panTable[panIndex];
    rightVolScale = panTable[0x1e - panIndex];

    if (leftVolScale + rightVolScale != 0)
    {
        panValue = (int)((rightVolScale / (double)(leftVolScale + rightVolScale)) * 126 + 1 + 0.5);
        if (panValue == 1)
        {
            panValue = 0;
        }
    }
    else
    {
        panValue = 0;
    }

    sprintf(ev->note, "Panpot, balance = %d/%d (index %d)", leftVolScale, rightVolScale, panIndex);
    strcat(ev->classStr, " ev-pan");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, panValue);
}

/** vcmd db: set reverse phase. */
static void hudsonSpcEventReversePhase (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Reverse Phase, L = %s, R = %s", (arg1 & 2) ? "on" : "off", (arg1 & 1) ? "on" : "off");
    strcat(ev->classStr, " ev-reversephase");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dc: add volume. */
static void hudsonSpcEventAddVolume (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int newVolume;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    newVolume = tr->volume + arg1;
    if (newVolume < 0) {
        newVolume = 0;
    }
    else if (newVolume > 0xff) {
    	newVolume = 0xff;
    }
    tr->volume = newVolume;

    sprintf(ev->note, "Volume (Add), vol += %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, hudsonSpcMidiVolOf(tr->volume));
}

/** vcmd ee: set volume from table. */
static void hudsonSpcEventVolumeFromTable (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int volIndex;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    const byte volTable[] = {
        0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
        0x06, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09,
        0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0x17, 0x18, 0x1a, 0x1b, 0x1d, 0x1e, 0x20, 0x22,
        0x24, 0x26, 0x28, 0x2b, 0x2d, 0x30, 0x33, 0x36,
        0x39, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0x51, 0x55,
        0x5a, 0x60, 0x66, 0x6c, 0x72, 0x79, 0x80, 0x87,
        0x8f, 0x98, 0xa1, 0xaa, 0xb5, 0xbf, 0xcb, 0xd7,
        0xe3, 0xf1, 0xff,
    };

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    volIndex = arg1;
    if (volIndex > 0x5a) {
        volIndex = 0x5a;
    }
    tr->volume = volTable[arg1];

    sprintf(ev->note, "Volume From Table, vol = %d (index %d)", tr->volume, arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, hudsonSpcMidiVolOf(tr->volume));
}

/** vcmd e7: transpose (absolute). */
static void hudsonSpcEventTransposeAbs (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;

    sprintf(ev->note, "Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e8: transpose (relative). */
static void hudsonSpcEventTransposeRel (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose += arg1;

    sprintf(ev->note, "Transpose, key += %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e9: pitch attack envelope on. */
static void hudsonSpcEventPitchAttackEnvOn (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    if (arg1 != 0)
    {
        if (arg2 != 0)
        {
            sprintf(ev->note, "Pitch Attack Envelope On, speed = %d, depth = %d, direction = %s", arg1, arg2, (arg3 != 0) ? "up" : "down");
        }
        else
        {
            sprintf(ev->note, "Pitch Attack Envelope On, speed = %d", arg1);
        }
    }
    else
    {
        strcpy(ev->note, "Pitch Attack Envelope On");
    }
    strcat(ev->classStr, " ev-pitchattackenvon");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ea: pitch attack envelope on. */
static void hudsonSpcEventPitchAttackEnvOff (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Attack Envelope Off");
    strcat(ev->classStr, " ev-pitchattackenvoff");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dd: loop start. */
static void hudsonSpcEventLoopStart (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];

    sprintf(ev->note, "Loop Start, count = %d", arg1);
    strcat(ev->classStr, " ev-loopstart");

    if (tr->callStackPtr + 3 > tr->callStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    tr->callStack[tr->callStackPtr++] = (byte)(*p);
    tr->callStack[tr->callStackPtr++] = (byte)((*p) >> 8);
    tr->callStack[tr->callStackPtr++] = arg1;
    (*p)++;

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd de: loop end. */
static void hudsonSpcEventLoopEnd (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    byte loopCount;

    sprintf(ev->note, "Loop End/Continue");
    strcat(ev->classStr, " ev-loopend");

    if (tr->callStackPtr == 0) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    loopCount = tr->callStack[tr->callStackPtr - 1];
    if (--loopCount == 0) {
        // repeat end, fall through
        sprintf(ev->note, "Loop End");
        if (tr->callStackPtr < 3) {
            fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
            hudsonSpcInactiveTrack(seq, ev->track);
            return;
        }
        tr->callStackPtr -= 3;
    }
    else {
        // repeat again
        sprintf(ev->note, "Loop Continue, count = %d", loopCount);
        tr->callStack[tr->callStackPtr - 1] = loopCount;
        *p = mget2l(&tr->callStack[tr->callStackPtr - 3]) + 1;
    }

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd df: call subroutine. */
static void hudsonSpcEventSubroutine (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);

    sprintf(ev->note, "Call Subroutine, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-call");

    if (tr->callStackPtr + 2 > tr->callStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    tr->callStack[tr->callStackPtr++] = (byte)(*p);
    tr->callStack[tr->callStackPtr++] = (byte)((*p) >> 8);
    *p = arg1;
}

/** vcmd e0: jump. */
static void hudsonSpcEventJump (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");

    // assumes backjump = loop
    if (arg1 < *p) {
        hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = arg1;

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd eb: set track loop position. */
static void hudsonSpcEventSetTrackLoop (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "Set Track Loop Position, dest = $%04X", *p);
    strcat(ev->classStr, " ev-settrackloop");

    tr->trackLoopAddr = *p;

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ec: goto track loop position. */
static void hudsonSpcEventJumpTrackLoop (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "Repeat From Track Loop Position, dest = $%04X", tr->trackLoopAddr);
    strcat(ev->classStr, " ev-gototrackloop");

    // assumes backjump = loop
    if (tr->trackLoopAddr < *p) {
        hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = tr->trackLoopAddr;

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ed: set track loop position (only work with the first call). */
static void hudsonSpcEventSetTrackLoopAlt (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "Set Track Loop Position (One-Shot), dest = $%04X, ignore = %s", *p, tr->trackLoopIsSet ? "true" : "false");
    strcat(ev->classStr, " ev-settrackloop");

    if (!tr->trackLoopIsSet)
    {
        tr->trackLoopAddr = *p;
        tr->trackLoopIsSet = true;
    }

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e1: detune. */
static void hudsonSpcEventDetune (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Tuning, key += %d / 256", arg1);
    strcat(ev->classStr, " ev-tuning");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNM, 0);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNL, 1);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYM, 64 + (arg1 / 4));
    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYL, 0);
}

/** vcmd e2: set vibrato. */
static void hudsonSpcEventSetVibrato (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;

    if (seq->ver.id == SPC_VER_V2)
    {
        ev->size += 3;
        arg1 = seq->aRAM[*p];
        (*p)++;
        arg2 = seq->aRAM[*p];
        (*p)++;
        arg3 = seq->aRAM[*p];
        (*p)++;

        sprintf(ev->note, "Set Vibrato, arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    }
    else
    {
        ev->size += 2;
        arg1 = seq->aRAM[*p];
        (*p)++;
        arg2 = seq->aRAM[*p];
        (*p)++;

        if ((arg1 & 0x7f) != 0)
        {
            sprintf(ev->note, "Set Vibrato, rate = %d, depth = %d", arg1, arg2);
        }
        else
        {
            sprintf(ev->note, "Set Vibrato, depth = 0");
        }
    }
    strcat(ev->classStr, " ev-vibrato");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e3: set vibrato delay. */
static void hudsonSpcEventSetVibratoDelay (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Vibrato Delay, delay = %d", arg1);
    strcat(ev->classStr, " ev-vibratodelay");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f1: set portamento. */
static void hudsonSpcEventSetPortamento (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    bool po_on;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    po_on = (arg1 != 0);

    sprintf(ev->note, "Set Portamento %s, speed = %d, arg2 = %d", po_on ? "On" : "Off", arg1, arg2);
    strcat(ev->classStr, " ev-portamento");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, po_on ? 127 : 0);
}

/** vcmd e4: set echo volume. */
static void hudsonSpcEventEchoVolume (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Volume, L = %d, R = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echovol");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e5: set echo delay, feedback, FIR. */
static void hudsonSpcEventEchoParam (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, delay = %d, feedback = %d, FIR = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-echoparam");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e6: echo on. */
static void hudsonSpcEventEchoOn (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd fe: subcmd. */
static void hudsonSpcEventSubEvent (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool unknown = false;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    ev->subcode = arg1;
    sprintf(ev->note, "Subcmd %02X ", arg1);

    seq->ver.subevent[ev->subcode](seq, ev);
}

/** vcmd ff: end subroutine / end of track. */
static void hudsonSpcEventEndSubroutine (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    if (tr->callStackPtr == 0) {
        sprintf(ev->note, "End of Track");
        strcat(ev->classStr, " ev-end");

        hudsonSpcInactiveTrack(seq, ev->track);
    }
    else {
        sprintf(ev->note, "End Subroutine");
        strcat(ev->classStr, " ev-ret");

        if (tr->callStackPtr < 2) {
            fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
            hudsonSpcInactiveTrack(seq, ev->track);
            return;
        }

        tr->callStackPtr -= 2;
        *p = mget2l(&tr->callStack[tr->callStackPtr]) + 2;
    }

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd: unidentified event. */
static void hudsonSpcEventSubUnidentified (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->unidentified = true;

    strcpy(argDumpStr, "(Unknown)");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-subcmd unknown");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    fprintf(stderr, "Error: Encountered unidentified event %02X %02X at $%04X [Track %d]\n", ev->code, ev->subcode, *p, ev->track + 1);
}

/** subcmd: unknown event (0 byte). */
static void hudsonSpcEventSubUnknown0 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    strcpy(argDumpStr, "(Unknown)");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-subcmd unknown");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    fprintf(stderr, "Warning: Skipped unknown event %02X %02X at $%04X [Track %d]\n", ev->code, ev->subcode, *p, ev->track + 1);
}

/** subcmd: unknown event (1 byte). */
static void hudsonSpcEventSubUnknown1 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Unknown, arg1 = %d)", arg1);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-subcmd unknown");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    fprintf(stderr, "Warning: Skipped unknown event %02X %02X at $%04X [Track %d]\n", ev->code, ev->subcode, *p, ev->track + 1);
}

/** subcmd: unknown event (2 bytes). */
static void hudsonSpcEventSubUnknown2 (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Unknown, arg1 = %d, arg2 = %d)", arg1, arg2);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-subcmd unknown");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    fprintf(stderr, "Warning: Skipped unknown event %02X %02X at $%04X [Track %d]\n", ev->code, ev->subcode, *p, ev->track + 1);
}

/** subcmd: mov reg, #imm. */
static void hudsonSpcEventSubMOVImm (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(mov $%02x, #%d)", arg1, arg2);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-mov");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (arg1 >= seq->regCount)
    {
        fprintf(stderr, "User RAM Access Violation, reg = $%02x\n", arg1);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    seq->reg[arg1] = arg2;
    seq->regCmp = seq->reg[arg1];
}

/** subcmd: mov reg, reg. */
static void hudsonSpcEventSubMOV (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(mov $%02x, $%02x)", arg1, arg2);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-mov");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (arg1 >= seq->regCount)
    {
        fprintf(stderr, "User RAM Access Violation, reg = $%02x\n", arg1);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }
    if (arg2 >= seq->regCount)
    {
        fprintf(stderr, "User RAM Access Violation, reg = $%02x\n", arg2);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    seq->reg[arg1] = seq->reg[arg2];
    seq->regCmp = seq->reg[arg1];
}

/** subcmd: cmp reg, #imm. */
static void hudsonSpcEventSubCMPImm (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(cmp $%02x, #%d)", arg1, arg2);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-cmp");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (arg1 >= seq->regCount)
    {
        fprintf(stderr, "User RAM Access Violation, reg = $%02x\n", arg1);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    seq->regCmp = seq->reg[arg1] - arg2;
    seq->regCarry = (seq->reg[arg1] > arg2);
}

/** subcmd: cmp reg, reg. */
static void hudsonSpcEventSubCMP (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(cmp $%02x, $%02x)", arg1, arg2);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-cmp");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (arg1 >= seq->regCount)
    {
        fprintf(stderr, "User RAM Access Violation, reg = $%02x\n", arg1);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }
    if (arg2 >= seq->regCount)
    {
        fprintf(stderr, "User RAM Access Violation, reg = $%02x\n", arg2);
        hudsonSpcInactiveTrack(seq, ev->track);
        return;
    }

    seq->regCmp = seq->reg[arg1] - seq->reg[arg2];
    seq->regCarry = (seq->reg[arg1] > seq->reg[arg2]);
}

/** subcmd: bne (branch on Z=0). */
static void hudsonSpcEventSubBNE (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    bool branch;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    branch = (seq->regCmp != 0);

    sprintf(argDumpStr, "(bne $%04x, branch = %s)", arg1, branch ? "true" : "false");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-branch ev-op-bne");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (branch)
    {
        // assumes backjump = loop
        //if (arg1 < *p) {
        //    hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
        //}
        *p = arg1;
    }
}

/** subcmd: beq (branch on Z=1). */
static void hudsonSpcEventSubBEQ (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    bool branch;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    branch = (seq->regCmp == 0);

    sprintf(argDumpStr, "(beq $%04x, branch = %s)", arg1, branch ? "true" : "false");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-branch ev-op-beq");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (branch)
    {
        // assumes backjump = loop
        //if (arg1 < *p) {
        //    hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
        //}
        *p = arg1;
    }
}

/** subcmd: bcs (branch on C=1). */
static void hudsonSpcEventSubBCS (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    bool branch;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    branch = seq->regCarry;

    sprintf(argDumpStr, "(bcs $%04x, branch = %s)", arg1, branch ? "true" : "false");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-branch ev-op-bcs");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (branch)
    {
        // assumes backjump = loop
        //if (arg1 < *p) {
        //    hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
        //}
        *p = arg1;
    }
}

/** subcmd: bcc (branch on C=0). */
static void hudsonSpcEventSubBCC (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    bool branch;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    branch = !seq->regCarry;

    sprintf(argDumpStr, "(bcc $%04x, branch = %s)", arg1, branch ? "true" : "false");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-branch ev-op-bcc");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (branch)
    {
        // assumes backjump = loop
        //if (arg1 < *p) {
        //    hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
        //}
        *p = arg1;
    }
}

/** subcmd: bmi (branch on N=1). */
static void hudsonSpcEventSubBMI (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    bool branch;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    branch = ((seq->regCmp & 0x80) != 0);

    sprintf(argDumpStr, "(bmi $%04x, branch = %s)", arg1, branch ? "true" : "false");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-branch ev-op-bmi");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (branch)
    {
        // assumes backjump = loop
        //if (arg1 < *p) {
        //    hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
        //}
        *p = arg1;
    }
}

/** subcmd: bpl (branch on N=0). */
static void hudsonSpcEventSubBPL (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    bool branch;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    branch = ((seq->regCmp & 0x80) != 0);

    sprintf(argDumpStr, "(bpl $%04x, branch = %s)", arg1, branch ? "true" : "false");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-op ev-op-branch ev-op-bpl");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (branch)
    {
        // assumes backjump = loop
        //if (arg1 < *p) {
        //    hudsonSpcAddTrackLoopCount(seq, ev->track, 1);
        //}
        *p = arg1;
    }
}

/** subcmd: set attack rate (AR). */
static void hudsonSpcEventSubSetAR (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Set Attack Rate, AR = %d)", arg1 & 15);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-setAR");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd: set decay rate (DR). */
static void hudsonSpcEventSubSetDR (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Set Decay Rate, DR = %d)", arg1 & 7);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-setDR");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd: set sustain level (SL). */
static void hudsonSpcEventSubSetSL (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Set Sustain Level, SL = %d)", arg1 & 7);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-setSL");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd: set sustain rate (SR). */
static void hudsonSpcEventSubSetSR (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Set Sustain Rate, SR = %d)", arg1 & 31);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-setSR");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd: set pseudo release rate (RR). */
static void hudsonSpcEventSubSetRR (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, "(Set Release Rate, GAIN = 0xa0 | %d)", arg1);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-setRR");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd: nop. */
static void hudsonSpcEventSubNOP (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    strcpy(argDumpStr, "(NOP)");
    strcat(ev->note, argDumpStr);
}

/** subcmd 00: end of track (alternate). */
static void hudsonSpcEventSubEndOfTrack (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];

    strcpy(argDumpStr, "(End Of Track Alt)");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-end");
    hudsonSpcInactiveTrack(seq, ev->track);
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd 01: echo off. */
static void hudsonSpcEventSubEchoOff (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];

    strcpy(argDumpStr, "(Echo Off)");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-echooff");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 0);
}

/** subcmd 03: rhythm channel on. */
static void hudsonSpcEventSubRhythmOn (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    strcpy(argDumpStr, "(Rhythm Channel On)");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-rhythmon");
    tr->rhythmChannel = true;

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // put program change to SMF (better than nothing)
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[255].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[255].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[255].patchNo);
}

/** subcmd 04: rhythm channel off. */
static void hudsonSpcEventSubRhythmOff (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    strcpy(argDumpStr, "(Rhythm Channel Off)");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-rhythmoff");
    tr->rhythmChannel = false;

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** subcmd 05,06,07: set vibrato type. */
static void hudsonSpcEventSubVibratoType (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    char argDumpStr[256];

    sprintf(argDumpStr, "(Vibrato Type, type = %d)", ev->subcode - 0x05);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-vibratotype");

    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** set pointers of each event. */
static void hudsonSpcSetEventList (HudsonSpcSeqStat *seq)
{
    int code;
    HudsonSpcEvent *event = seq->ver.event;
    HudsonSpcEvent *subevent = seq->ver.subevent;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (HudsonSpcEvent) hudsonSpcEventUnidentified;
    }
    for(code = 0x00; code <= 0xff; code++) {
        subevent[code] = (HudsonSpcEvent) hudsonSpcEventSubUnidentified;
    }

    for (code = 0x00; code <= 0xcf; code++) {
        event[code] = (HudsonSpcEvent) hudsonSpcEventNote;
    }
    event[0xd0] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xd1] = (HudsonSpcEvent) hudsonSpcEventSetTempo;
    event[0xd2] = (HudsonSpcEvent) hudsonSpcEventSetOctave;
    event[0xd3] = (HudsonSpcEvent) hudsonSpcEventOctaveUp;
    event[0xd4] = (HudsonSpcEvent) hudsonSpcEventOctaveDown;
    event[0xd5] = (HudsonSpcEvent) hudsonSpcEventSetQuantize;
    event[0xd6] = (HudsonSpcEvent) hudsonSpcEventInstrument;
    event[0xd7] = (HudsonSpcEvent) hudsonSpcEventNOP2;
    event[0xd8] = (HudsonSpcEvent) hudsonSpcEventNOP2;
    event[0xd9] = (HudsonSpcEvent) hudsonSpcEventVolume;
    event[0xda] = (HudsonSpcEvent) hudsonSpcEventPanpot;
    event[0xdb] = (HudsonSpcEvent) hudsonSpcEventReversePhase;
    event[0xdc] = (HudsonSpcEvent) hudsonSpcEventAddVolume;
    event[0xdd] = (HudsonSpcEvent) hudsonSpcEventLoopStart;
    event[0xde] = (HudsonSpcEvent) hudsonSpcEventLoopEnd;
    event[0xdf] = (HudsonSpcEvent) hudsonSpcEventSubroutine;
    event[0xe0] = (HudsonSpcEvent) hudsonSpcEventJump;
    event[0xe1] = (HudsonSpcEvent) hudsonSpcEventDetune;
    event[0xe2] = (HudsonSpcEvent) hudsonSpcEventSetVibrato;
    event[0xe3] = (HudsonSpcEvent) hudsonSpcEventSetVibratoDelay;
    event[0xe4] = (HudsonSpcEvent) hudsonSpcEventEchoVolume;
    event[0xe5] = (HudsonSpcEvent) hudsonSpcEventEchoParam;
    event[0xe6] = (HudsonSpcEvent) hudsonSpcEventEchoOn;
    event[0xe7] = (HudsonSpcEvent) hudsonSpcEventTransposeAbs;
    event[0xe8] = (HudsonSpcEvent) hudsonSpcEventTransposeRel;
    event[0xe9] = (HudsonSpcEvent) hudsonSpcEventPitchAttackEnvOn;
    event[0xea] = (HudsonSpcEvent) hudsonSpcEventPitchAttackEnvOff;
    event[0xeb] = (HudsonSpcEvent) hudsonSpcEventSetTrackLoop;
    event[0xec] = (HudsonSpcEvent) hudsonSpcEventJumpTrackLoop;
    event[0xed] = (HudsonSpcEvent) hudsonSpcEventSetTrackLoopAlt;
    event[0xee] = (HudsonSpcEvent) hudsonSpcEventVolumeFromTable;
    event[0xef] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
    event[0xf0] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xf1] = (HudsonSpcEvent) hudsonSpcEventSetPortamento;
    event[0xf2] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf3] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf4] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf5] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf6] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf7] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf8] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xf9] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xfa] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xfb] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xfc] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xfd] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xfe] = (HudsonSpcEvent) hudsonSpcEventSubEvent;
    event[0xff] = (HudsonSpcEvent) hudsonSpcEventEndSubroutine;
    subevent[0x00] = (HudsonSpcEvent) hudsonSpcEventSubEndOfTrack;
    subevent[0x01] = (HudsonSpcEvent) hudsonSpcEventSubEchoOff;
    subevent[0x02] = (HudsonSpcEvent) hudsonSpcEventSubUnknown0;
    subevent[0x03] = (HudsonSpcEvent) hudsonSpcEventSubRhythmOn;
    subevent[0x04] = (HudsonSpcEvent) hudsonSpcEventSubRhythmOff;
    subevent[0x05] = (HudsonSpcEvent) hudsonSpcEventSubVibratoType;
    subevent[0x06] = (HudsonSpcEvent) hudsonSpcEventSubVibratoType;
    subevent[0x07] = (HudsonSpcEvent) hudsonSpcEventSubVibratoType;
    subevent[0x08] = (HudsonSpcEvent) hudsonSpcEventSubUnknown0;
    subevent[0x09] = (HudsonSpcEvent) hudsonSpcEventSubUnknown0;

    if (seq->ver.id == SPC_VER_V2)
    {
        event[0xd0] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
        event[0xd7] = (HudsonSpcEvent) hudsonSpcEventNOP;
        event[0xd8] = (HudsonSpcEvent) hudsonSpcEventNOP;
        //event[0xe2] = (HudsonSpcEvent) hudsonSpcEventSetVibrato;
        event[0xee] = (HudsonSpcEvent) hudsonSpcEventNOP;
        event[0xf2] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
        event[0xf3] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
        subevent[0x05] = (HudsonSpcEvent) hudsonSpcEventSubNOP;
        subevent[0x06] = (HudsonSpcEvent) hudsonSpcEventSubNOP;
        subevent[0x07] = (HudsonSpcEvent) hudsonSpcEventSubNOP;
        subevent[0x0a] = (HudsonSpcEvent) hudsonSpcEventSubUnknown0;
        subevent[0x0b] = (HudsonSpcEvent) hudsonSpcEventSubUnknown0;
        subevent[0x0c] = (HudsonSpcEvent) hudsonSpcEventSubUnknown0;
        subevent[0x0d] = (HudsonSpcEvent) hudsonSpcEventSubUnknown1;
        subevent[0x0e] = (HudsonSpcEvent) hudsonSpcEventSubNOP;
        subevent[0x0f] = (HudsonSpcEvent) hudsonSpcEventSubNOP;
        subevent[0x10] = (HudsonSpcEvent) hudsonSpcEventSubMOVImm;
        subevent[0x11] = (HudsonSpcEvent) hudsonSpcEventSubMOV;
        subevent[0x12] = (HudsonSpcEvent) hudsonSpcEventSubCMPImm;
        subevent[0x13] = (HudsonSpcEvent) hudsonSpcEventSubCMP;
        subevent[0x14] = (HudsonSpcEvent) hudsonSpcEventSubBNE;
        subevent[0x15] = (HudsonSpcEvent) hudsonSpcEventSubBEQ;
        subevent[0x16] = (HudsonSpcEvent) hudsonSpcEventSubBCS;
        subevent[0x17] = (HudsonSpcEvent) hudsonSpcEventSubBCC;
        subevent[0x18] = (HudsonSpcEvent) hudsonSpcEventSubBMI;
        subevent[0x19] = (HudsonSpcEvent) hudsonSpcEventSubBPL;
        subevent[0x1a] = (HudsonSpcEvent) hudsonSpcEventSubSetAR;
        subevent[0x1b] = (HudsonSpcEvent) hudsonSpcEventSubSetDR;
        subevent[0x1c] = (HudsonSpcEvent) hudsonSpcEventSubSetSL;
        subevent[0x1d] = (HudsonSpcEvent) hudsonSpcEventSubSetSR;
        subevent[0x1e] = (HudsonSpcEvent) hudsonSpcEventSubSetRR;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* hudsonSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    HudsonSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newHudsonSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = hudsonSpcCreateSmf(seq);

    printHtmlInfoListMore(seq);

    myprintf("          </ul></li>\n");
    myprintf("        </ul>\n");
    myprintf("      </div>\n\n");

    myprintf("      <h2>Data Dump</h2>\n");
    myprintf("      <div class=\"section\" id=\"data-dump\">\n");
    myprintf("        <p>You can filter output by using stylesheet. Write %s as you like!</p>\n", mycssfile);

    printEventTableHeader(seq, smf);

    while (seq->active && !abortFlag) {

        SeqEventReport ev;

        for (ev.track = 0; ev.track < SPC_TRACK_MAX; ev.track++) {

            HudsonSpcTrackStat *evtr = &seq->track[ev.track];

            while (seq->active && evtr->active && evtr->tick <= seq->tick) {

                bool inSub;

                // init event report
                ev.tick = seq->tick;
                ev.addr = evtr->pos;
                ev.size = 0;
                ev.unidentified = false;
                strcpy(ev.note, "");

                // read first byte
                ev.size++;
                ev.code = aRAM[ev.addr];
                sprintf(ev.classStr, "ev%02X", ev.code);
                evtr->pos++;
                // in subroutine?
                inSub = false; // NYI
                strcat(ev.classStr, inSub ? " sub" : "");

                //if (ev.code != seq->ver.pitchSlideByte)
                //    evtr->prevTick = evtr->tick;
                evtr->used = true;
                // dispatch event
                seq->ver.event[ev.code](seq, &ev);

                // dump event report
                if (hudsonSpcTextLoopMax == 0 || seq->looped < hudsonSpcTextLoopMax)
                    printHtmlEventDump(seq, &ev);

                if (ev.unidentified) {
                    abortFlag = true;
                    goto quitConversion;
                }
            }
        }

        // end of seq, quit
        if (!seq->active) {
            // rewind tracks to end point
            for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                seq->track[tr].tick = seq->tick;
                if (seq->track[tr].used)
                    smfSetEndTimingOfTrack(seq->smf, tr, seq->tick);
            }
        }
        else {
            hudsonSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= hudsonSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, hudsonSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    hudsonSpcTruncateNoteAll(seq);
    hudsonSpcDequeueNoteAll(seq);

    printEventTableFooter(seq, smf);
    if (!abortFlag) {
        myprintf("        <p>Congratulations! MIDI conversion went successfully!</p>\n");
    }
    else {
        myprintf("        <p>Conversion aborted! Apparently something went wrong...</p>\n");
    }
    myprintf("      </div>\n");

finalize:
    myprintf("    </div>\n");
    printHtmlFooter();

    if (seq) {
        delHudsonSpcSeq(&seq);
    }

    return smf;

abort:
    if (smf != NULL) {
        smfDelete(smf);
        smf = NULL;
    }

    goto finalize;
}

/** convert spc to midi data from SPC file located in memory. */
Smf* hudsonSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = hudsonSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* hudsonSpcToMidiFromFile (const char *filename)
{
    Smf* smf = NULL;
    FILE *fp;
    byte *data = NULL;
    size_t size;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        goto finalize;
    }

    fseek(fp, 0, SEEK_END);
    size = (size_t) ftell(fp);
    rewind(fp);

    data = (byte*) malloc(size);
    if (data == NULL) {
        goto finalize;
    }
    fread(data, size, 1, fp);

    smf = hudsonSpcToMidi(data, size);

finalize:

    if (fp != NULL) {
        fclose(fp);
    }

    if (data != NULL) {
        free(data);
    }

    return smf;
}

//----

static int gArgc;
static char **gArgv;
static bool manDisplayed = false;

typedef bool (*CmdDispatcher) (void);

typedef struct TagCmdOptDefs {
    char *name;
    char shortName;
    int numArgs;
    CmdDispatcher dispatch;
    char *syntax;
    char *description;
} CmdOptDefs;

static bool cmdOptHelp (void);
static bool cmdOptLoop (void);
static bool cmdOptPatchFix (void);
static bool cmdOptGS (void);
static bool cmdOptXG (void);
static bool cmdOptGM1 (void);
static bool cmdOptGM2 (void);
static bool cmdOptSong (void);
static bool cmdOptSongList (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm1", '\0', 0, cmdOptGM1, "", "Insert GM1 System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
    { "song", '\0', 1, cmdOptSong, "<index>", "force set song index" },
    { "songlist", '\0', 1, cmdOptSongList, "<addr>", "force set song (list) address" },
};

//----

/** display how to use. */
void man (void)
{
    const char *cmdname = APPSHORTNAME;
    int op;

    fprintf(stderr, "%s - %s %s\n", APPSHORTNAME, APPNAME, VERSION);
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
    fprintf(stderr, "http://loveemu.yh.land.to/\n");

    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    for (op = 0; op < countof(optDef); op++) {
        if (optDef[op].description) {
            if (optDef[op].dispatch) {
                fprintf(stderr, " %s%c  %s%-8s  %-15s  %s\n",
                    (optDef[op].shortName != '\0') ? "-" : " ",
                    (optDef[op].shortName != '\0') ? optDef[op].shortName : ' ',
                    optDef[op].name ? "--" : "  ",
                    optDef[op].name ? optDef[op].name : "",
                    optDef[op].syntax ? optDef[op].syntax : "",
                    optDef[op].description ? optDef[op].description : "");
            }
            else
                fprintf(stderr, "\n");
        }
    }
    fprintf(stderr, "\n");

    manDisplayed = true;
}

/** display about application. */
void about (void)
{
    const char *cmdname = APPSHORTNAME;

    fprintf(stderr, "%s - %s %s\n", APPSHORTNAME, APPNAME, VERSION);
    fprintf(stderr, "Programmed by loveemu - http://loveemu.yh.land.to/\n");
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
}

//----

/** show usage */
static bool cmdOptHelp (void)
{
    man();
    return true;
}

/** set loop count */
static bool cmdOptLoop (void)
{
    int loopCount = strtol(gArgv[0], NULL, 0);
    hudsonSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    hudsonSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    hudsonSpcForceSongListAddr = songListAddr;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (hudsonSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    hudsonSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    hudsonSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    hudsonSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    hudsonSpcMidiResetType = SMF_RESET_GM2;
    return true;
}

/** handle command-line options. */
static bool handleCmdLineOpts (void)
{
    int op;

    // dispatch options
    while (gArgc > 0 && gArgv[0][0] == '-') {
        bool shortOpt = (gArgv[0][1] != '-');
        int optLen;
        int chIndex;

        // match for each option
        optLen = (int) strlen(gArgv[0]);
        for (chIndex = 1; chIndex < (shortOpt ? optLen : 2); chIndex++) {
            bool unknown = true;

            for (op = 0; op < countof(optDef); op++) {
                if (optDef[op].dispatch
                        && ((!shortOpt && optDef[op].name && strcmp(&gArgv[0][2], optDef[op].name) == 0)
                        || (shortOpt && optDef[op].shortName != '\0' && gArgv[0][chIndex] == optDef[op].shortName))) {
                    unknown = false;
                    if (!shortOpt) {
                        gArgc--;
                        gArgv++;
                        if (gArgc >= optDef[op].numArgs) {
                            if (!optDef[op].dispatch())
                                return false;
                            gArgc -= optDef[op].numArgs;
                            gArgv += optDef[op].numArgs;
                        }
                        else {
                            fprintf(stderr, "Error: too few arguments for option \"--%s\".\n", optDef[op].name);
                            gArgv += gArgc;
                            gArgc = 0;
                            return false;
                        }
                    }
                    else {
                        assert(optDef[op].numArgs == 0);
                        if (!optDef[op].dispatch())
                            return false;
                    }
                    break;
                }
            }
            if (unknown) {
                if (!shortOpt)
                    fprintf(stderr, "Error: unknown option \"%s\".\n", gArgv[0]);
                else
                    fprintf(stderr, "Error: unknown option \"-%c\".\n", gArgv[0][chIndex]);
                gArgc--;
                gArgv++;
                return false;
            }
        }
        if (shortOpt) {
            gArgc--;
            gArgv++;
        }
    }
    return true;
}

//----

/** application main. */
int main (int argc, char *argv[])
{
    Smf* smf;
    FILE *htmlFile = NULL;
    bool result = true;

    // handle options
    gArgc = argc - 1;
    gArgv = argv + 1;
    handleCmdLineOpts();

    // too few or much args
    if (gArgc < 2 || gArgc > 3) {
        if (!manDisplayed) {
            about();
            fprintf(stderr, "Run with --help, for more details.\n");
            return (argc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        else
            return EXIT_SUCCESS;
    }

    // set html handle
    if (gArgc >= 3) {
        htmlFile = fopen(gArgv[2], "w");
        if (htmlFile != NULL)
            hudsonSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = hudsonSpcToMidiFromFile(gArgv[0]);
    // then output result
    if (smf != NULL) {
        smfWriteFile(smf, gArgv[1]);
    }
    else {
        fprintf(stderr, "Error: Conversion failed.\n");
        result = false;
    }

    if (htmlFile != NULL)
        fclose(htmlFile);

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
