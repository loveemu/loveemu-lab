/**
 * Nintendo spc2midi.
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
#include "nintspc.h"

#define APPNAME         "Nintendo SPC2MIDI"
#define APPSHORTNAME    "nintspc"
#define VERSION         "[2014-02-15]"
#define AUTHOR          "loveemu"
#define WEBSITE         "http://loveemu.yh.land.to/"

static int nintSpcLoopMax = 2;              // maximum loop count of parser
static int nintSpcTextLoopMax = 1;          // maximum loop count of text output
static double nintSpcTimeLimit = 2400;      // time limit of conversion (for safety)
static bool nintSpcLessTextInSMF = false;   // decreases amount of texts in SMF output

static bool nintSpcSongFromPort = true;     // get song index from APU port
static bool nintSpcMMLAbsTick = false;      // always use = symbol for notes

static bool nintSpcVolIsLinear = false;     // assumes volume curve between SPC and MIDI is linear
static int nintSpcPitchBendSens = 0;        // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool nintSpcSeparatePerc = false;    // separate percussion notes to other channel

static int nintSpcForceSongIndex = -1;
static int nintSpcForceSongListAddr = -1;
static int nintSpcForceBlockPtrAddr = -1;
static int nintSpcForceDurTableAddr = -1;
static int nintSpcForceVelTableAddr = -1;
static bool nintSpcParseForce = false;
static FILE* nintSpcMMLLog = NULL;
static FILE* nintSpcARAMRefLog = NULL;

static bool nintSpcAutoQFix = true;
static int nintSpcMMLDurFix[8] = { 0, 1, 2, 3, 4, 5, 6, 7 }; // normal (smw->std)
static int nintSpcMMLVelFix[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }; // normal
// { 1, 3, 5, 7, 8, 9, 10, 11, 12, 12, 13, 13, 14, 14, 15, 15 }; // smw->std

static bool nintSpcPatchFixOverride = false;
static PatchFixInfo nintSpcPatchFix[256];

static int nintSpcContConvCnt = 0;
static int nintSpcContConvNum = 1;

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int nintSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_OLD,            // Super Mario World & Pilotwings
    SPC_VER_STD,            // vcmd e0-fa
    SPC_VER_STD_AT_LEAST,   // has the same spec as SPC_VER_STD, at least
    SPC_VER_STD_MODIFIED,   // seems to be based on SPC_VER_STD, but it's somewhat different from original
    SPC_VER_EXT1,           // has vcmd fb-fe, Super Metroid family
    SPC_VER_YSFR,           // Yoshi's Safari
    SPC_VER_LEM,            // Lemmings
    SPC_VER_TA,             // Tetris Attack
    SPC_VER_FE3,            // Fire Emblem 3 (Monshou no Nazo)
    SPC_VER_FE4,            // Fire Emblem 4 (Seisen no Keifu)
    SPC_VER_KONAMI,         // Old Konami Driver
};

const byte NINT_STD_EVT_LEN_TABLE[] = {
    1, 1, 2, 3, 0, 1, 2, 1,
    2, 1, 1, 3, 0, 1, 2, 3,
    1, 3, 3, 0, 1, 3, 0, 3,
    3, 3, 1,
};

const byte NINT_TA_EVT_LEN_TABLE[] = { // Tetris Attack
    1, 1, 2, 3, 0, 1, 2, 1,
    2, 1, 1, 3, 0, 1, 2, 3,
    1, 3, 3, 0, 1, 3, 0, 3,
    3, 3, 1, 0, 0, 2, 2, 0,
    1, 1, 1, 1,
};

// MIDI limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TIMEBASE        48
#define SPC_SONG_MAX        0x80
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

enum {
    SPC_NOTEPARAM_STD = 0,
    SPC_NOTEPARAM_DIR,      // set value directly, Lemmings mothod
    SPC_NOTEPARAM_INT,      // Intelligent Systems method
};

typedef struct TagNintSpcTrackStat NintSpcTrackStat;
typedef struct TagNintSpcSeqStat NintSpcSeqStat;
typedef void (*NintSpcEvent) (NintSpcSeqStat *, SeqEventReport *);

typedef struct TagNintSpcVerInfo {
    int id;
    byte endBlockByte;
    byte noteInfoByteMin;
    byte noteInfoByteMax;
    byte noteByteMin;
    byte noteByteMax;
    byte tieByte;
    byte restByte;
    byte percByteMin;
    byte percByteMax;
    byte vcmdByteMin;
    byte pitchSlideByte;
    int seqListAddr;
    int blockPtrAddr;
    int durTableAddr;
    int velTableAddr;
    int fireEmbDurVelTableAddr;
    int vcmdListAddr;
    int vcmdLensAddr;
    int defaultTempo;
    bool percBaseIsNYI;
    int noteInfoType;
    int konamiAddrBase;
    NintSpcEvent event[256]; // vcmds
    PatchFixInfo patchFix[256];
} NintSpcVerInfo;

typedef struct TagNintSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int lastDur;        // length for note vcmd (tick)
    int dur;            // duration (tick)
    int vel;            // velocity
    int durRate;        // duration rate (n/256)
    bool tied;          // if the note tied
    bool perc;          // if the note is percussion
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
    int mmlOct;         // mml octave
} NintSpcNoteParam;

struct TagNintSpcTrackStat {
    bool active;        // true if the channel is still active
    bool used;          // true if the channel used once or not
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    int nextTick;       // next timing (for pitch slide)
    NintSpcNoteParam note;     // current note param
    NintSpcNoteParam lastNote; // note params for last note
    int loopStart;      // loop start address for loop command
    int loopCount;      // repeat count for loop command
    int retnAddr;       // return address for loop command
    byte lastPerc;
    bool newPerc;
    int konamiRepeatStart;
    int konamiRepeatCount;
    StringStreamBuf *mml;
    bool mmlWritten;
};

struct TagNintSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    byte* aRAMRef;              // 
    Smf* smf;                   // link for smf output
    int tick;                   // timing (tick)
    int blockStartTick;         // timing of block start (tick)
    double time;                // timing (s)
    int tempo;                  // current tempo (bpm)
    int masterVolume;           // master volume
    int transpose;              // key shift (note number, signed)
    int addrOfHeader;           // sequence header address
    int blockPtr;               // current pos in block list
    int blockPtrAlt;            // alternate blockPtr (for logging)
    int blockLoopCnt;           // repeat count for block
    bool endBlock;              // if reached to end of block
    int looped;                 // how many times the song looped (internal)
    int blockLooped;            // how many times looped block (internal)
    int songIndex;              // song index in song table
    bool active;                // if the seq is still active
    byte fe3ByteCA;             // Fire Emblem $(00)ca
    NintSpcVerInfo ver;         // game version info
    NintSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void nintSpcSetEventList (NintSpcSeqStat *seq);

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

static char noteLenText[64];
void getNoteLenForMML (char *text, int tick, int division)
{
    const int dotMax = 6;
    int note = division * 4;
    int l, dot;

  if (!nintSpcMMLAbsTick && nintSpcMMLLog) {
    for (l = 1; l <= note; l++) {
        int cTick = 0;

        for (dot = 0; dot <= dotMax; dot++) {
            int ld = (l << dot);

            if (note % ld)
                break;
            cTick += note / ld;
            if (tick == cTick) {
                sprintf(text, "%d", l);
                for (; dot > 0; dot--)
                    strcat(text, ".");
                return;
            }
        }
    }
  }
    sprintf(text, "=%d", tick);
}

//----

/** sets html stream to new target. */
FILE *nintSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int nintSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = nintSpcLoopMax;
    nintSpcLoopMax = count;
    return oldLoopCount;
}

/** sets song index to convert. */
int nintSpcSetSongIndex (int index)
{
    int oldSongIndex;

    oldSongIndex = nintSpcForceSongIndex;
    nintSpcForceSongIndex = index;
    return oldSongIndex;
}

/** sets if read song index from APU port. */
bool nintSpcSetSongFromPort (bool sw)
{
    int oldState;

    oldState = nintSpcSongFromPort;
    nintSpcSongFromPort = sw;
    return oldState;
}

/** read patch fix info file. */
bool nintSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        nintSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        nintSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            nintSpcPatchFix[patch].bankSelM = patch >> 7;
            nintSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            nintSpcPatchFix[patch].bankSelM = 0;
            nintSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        nintSpcPatchFix[patch].patchNo = patch & 0x7f;
        nintSpcPatchFix[patch].key = 0;
        nintSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        nintSpcPatchFix[src].bankSelM = bankM & 0x7f;
        nintSpcPatchFix[src].bankSelL = bankL & 0x7f;
        nintSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        nintSpcPatchFix[src].key = key;
        nintSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    nintSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

/** returns version string of music engine. */
static const char *nintSpcVerToStrHtml (int version)
{
    switch (version) {
    case SPC_VER_OLD:
        return "Nintendo / Old Engine (SMW family)";
    case SPC_VER_STD:
        return "Nintendo / Standard";
    case SPC_VER_STD_AT_LEAST:
        return "Nintendo / Unknown Extended Version";
    case SPC_VER_STD_MODIFIED:
        return "Nintendo / Unknown Modified Version";
    case SPC_VER_EXT1:
        return "Nintendo / Extended (Super Metroid family)";
    case SPC_VER_YSFR:
        return "Nintendo / Extended (Yoshi's Safari family)";
    case SPC_VER_LEM:
        return "Nintendo / Extended (Lemmings)";
    case SPC_VER_TA:
        return "Nintendo / Intelligent Systems / Tetris Attack";
    case SPC_VER_FE3:
        return "Nintendo / Intelligent Systems / Fire Emblem 3";
    case SPC_VER_FE4:
        return "Nintendo / Intelligent Systems / Fire Emblem 4";
    case SPC_VER_KONAMI:
        return "Konami / Old Engine";
    default:
        return "Unknown Version / Unsupported";
    }
}

//----

/** truncate note. */
static void nintSpcTruncateNote (NintSpcSeqStat *seq, int track)
{
    NintSpcTrackStat *tr = &seq->track[track];

    if (tr->lastNote.active && tr->lastNote.dur > 0) {
        int lastTick = tr->lastNote.tick + tr->lastNote.dur;
        int diffTick = lastTick - seq->tick;

        if (diffTick > 0) {
            if (!seq->looped)
                sbprintf(seq->track[track].mml, "\n; warning: %d ticks truncated\n", diffTick);

            tr->lastNote.dur -= diffTick;
            if (tr->lastNote.dur == 0)
                tr->lastNote.active = false;
        }
    }
}

/** truncate note for each track. */
static void nintSpcTruncateNoteAll (NintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        nintSpcTruncateNote(seq, tr);
    }
}

/** convert SPC velocity into MIDI one. */
static int nintSpcMidiVelOf (int value)
{
    if (nintSpcVolIsLinear)
        return (int) floor(pow((double) value/255, 2) * 127 + 0.5);
    else
        return value/2;
/*
    if (nintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
*/
}

/** finalize note. */
static bool nintSpcDequeueNote (NintSpcSeqStat *seq, int track)
{
    NintSpcTrackStat *tr = &seq->track[track];
    NintSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        if (lastNote->tied)
            dur = (lastNote->dur * lastNote->durRate) >> 8;
        else
            dur = (lastNote->dur - lastNote->lastDur)
                + ((lastNote->lastDur * lastNote->durRate) >> 8);
        if (dur == 0)
            dur++;

        if (lastNote->perc)
            key = lastNote->key;
        else
            key = lastNote->key + lastNote->transpose
                + seq->ver.patchFix[tr->lastNote.patch].key + SPC_NOTE_KEYSHIFT;

        vel = nintSpcMidiVelOf(lastNote->vel);
        if (vel == 0)
            vel++;

        result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void nintSpcDequeueNoteAll (NintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        nintSpcDequeueNote(seq, tr);
    }
}

/** update seq activity. */
static bool nintSpcUpdateActivity(NintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            return true;
    }
    seq->active = false;
    return false;
}

/** inactivate track. */
static bool nintSpcInactiveTrack(NintSpcSeqStat *seq, int track)
{
    seq->track[track].active = false;
    return nintSpcUpdateActivity(seq);
}

//----

/** reset for each track. */
static void nintSpcResetTrackParam (NintSpcSeqStat *seq, int track)
{
    NintSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->nextTick = tr->tick;
    tr->note.dur = 1;
    tr->note.durRate = 0xfc; // just in case
    tr->note.patch = 0;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->lastNote.mmlOct = -801;
    tr->loopCount = 0;
    tr->retnAddr = 0;
    tr->newPerc = true;
    tr->konamiRepeatStart = 0;
    tr->konamiRepeatCount = 0;
}

/** reset before play/convert song. */
static void nintSpcResetParam (NintSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->blockStartTick = seq->tick;
    seq->tempo = seq->ver.defaultTempo;
    seq->masterVolume = 0xc0;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;
    seq->fe3ByteCA = 0x80;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        NintSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        nintSpcResetTrackParam(seq, track);
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
    if (nintSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &nintSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int nintSpcCheckVer (NintSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;
    int pos1, pos2;
    int vcmdStart = -1;
    int vcmdListAddr = -1;
    int vcmdLensAddr = -1;
    int vcmdListAddrAsm = -1;
    int vcmdLensAddrAsm = -1;

    // standard params
    seq->ver.noteInfoType = SPC_NOTEPARAM_STD;
    seq->ver.endBlockByte = 0x00;
    seq->ver.noteInfoByteMin = 0x01;
    seq->ver.noteInfoByteMax = 0x7f;
    seq->ver.noteByteMin = 0x80;
    seq->ver.noteByteMax = 0xc7;
    seq->ver.tieByte = 0xc8;
    seq->ver.restByte = 0xc9;
    seq->ver.percByteMin = 0xca;
    seq->ver.percByteMax = 0xdf;
    seq->ver.defaultTempo = 0x20;
    seq->ver.percBaseIsNYI = false;
    seq->ver.konamiAddrBase = 0;

    // cmp   a,#$..
    // bcc   $05
    // call  $....
    // bra   $..
    pos1 = indexOfHexPat(aRAM, (const byte *) "\x68.\x90\x05\x3f..\x2f.", SPC_ARAM_SIZE, NULL);
    if (pos1 >= 0) {
        int vcmdCallAsm = -1;

        vcmdStart = aRAM[pos1 + 1];
        pos2 = mget2l(&aRAM[pos1 + 5]);

        // asl   a
        // mov   y,a
        // mov   a,$....+y
        // push  a
        // mov   a,$....+y
        // push  a
        // mov   a,y
        // lsr   a
        // mov   y,a
        // mov   a,$....+y
        // beq   $....
        vcmdCallAsm = indexOfHexPat(&aRAM[pos2], (const byte *) "\x1c\xfd\xf6..\x2d\xf6..\x2d\xdd\\\x5c\xfd\xf6..\xf0.", 18, NULL);
        if (vcmdCallAsm >= 0)
        {
            vcmdListAddrAsm = mget2l(&aRAM[pos2 + 7]);
            vcmdLensAddrAsm = mget2l(&aRAM[pos2 + 14]);
            vcmdListAddr = vcmdListAddrAsm + ((vcmdStart * 2) & 0xff);
            vcmdLensAddr = vcmdLensAddrAsm + (vcmdStart & 0x7f);
        }
        else {
            // (Clock Tower)
            // 07cc: 80        setc
            // 07cd: a8 e0     sbc   a,#$e0
            // 07cf: 1c        asl   a
            // 07d0: fd        mov   y,a
            // 07d1: f6 6d 07  mov   a,$076d+y
            // 07d4: 2d        push  a
            // 07d5: f6 6c 07  mov   a,$076c+y
            // 07d8: 2d        push  a
            // 07d9: dd        mov   a,y
            // 07da: 5c        lsr   a
            // 07db: fd        mov   y,a
            // 07dc: f6 ac 07  mov   a,$07ac+y
            // 07df: fd        mov   y,a
            // 07e0: f0 03     beq   $07e5
            vcmdCallAsm = indexOfHexPat(&aRAM[pos2], (const byte *) "\x80\xa8.\x1c\xfd\xf6..\x2d\xf6..\x2d\xdd\\\x5c\xfd\xf6..\xfd\xf0.", 22, NULL);
            if (vcmdCallAsm >= 0)
            {
                vcmdListAddr = mget2l(&aRAM[pos2 + 10]);
                vcmdLensAddr = mget2l(&aRAM[pos2 + 17]);
            }
        }
        if (vcmdCallAsm >= 0)
        {
            // check the lengths of standard vcmds
            if (memcmp(&aRAM[vcmdLensAddr], NINT_STD_EVT_LEN_TABLE, countof(NINT_STD_EVT_LEN_TABLE)) == 0)
            {
                if (vcmdListAddr + 0x1b * 2 == vcmdLensAddr)
                {
                    version = SPC_VER_STD;
                }
                else if (vcmdListAddr + 0x1f * 2 == vcmdLensAddr
                    && aRAM[vcmdLensAddr + 0x1b] == 0x02
                    && aRAM[vcmdLensAddr + 0x1c] == 0x00
                    && aRAM[vcmdLensAddr + 0x1d] == 0x00
                    && aRAM[vcmdLensAddr + 0x1e] == 0x00)
                {
                    version = SPC_VER_EXT1;
                }
                else
                {
                    version = SPC_VER_STD_AT_LEAST;
                }
            }
            else
            {
                // TODO: need more check...?
                version = SPC_VER_STD_MODIFIED;
            }
        }
        // asl   a
        // mov   x,a
        // mov   a,#$00
        // jmp   ($....+x)
        else if (indexOfHexPat(&aRAM[pos2], (const byte *) "\x1c\x5d\xe8\\\x00\x1f..", 7, NULL) >= 0) {
            version = SPC_VER_OLD;
            vcmdListAddrAsm = mget2l(&aRAM[pos2 + 5]);
            vcmdListAddr = vcmdListAddrAsm + ((vcmdStart * 2) & 0xff);

            // obtain length table as well, it's not necessary though.
            // 
            // cmp   a,#$..
            // bcc   label_1
            // push  y
            // mov   y,a
            // pop   a
            // clrc
            // adc   a,$126f+y
            // mov   y,a
            // bra   $....
            // label_1:
            if ((pos2 = indexOfHexPat(aRAM, (const byte *) "\x68.\x90\x0a\x6d\xfd\xae\x60\x96..\xfd\x2f.", SPC_ARAM_SIZE, NULL)) >= 0) {
                vcmdLensAddrAsm = mget2l(&aRAM[pos2 + 9]);
                vcmdLensAddr = vcmdLensAddrAsm + vcmdStart;
            }
        }
    }

    seq->ver.vcmdByteMin = (vcmdStart >= 0) ? vcmdStart : 0xe0;
    seq->ver.pitchSlideByte = (vcmdStart >= 0) ? (vcmdStart + 0x19) : 0xf9;
    seq->ver.vcmdListAddr = vcmdListAddr;
    seq->ver.vcmdLensAddr = vcmdLensAddr;

    // needs autosearch
    seq->ver.seqListAddr = -1;
    seq->ver.blockPtrAddr = -1;
    seq->ver.durTableAddr = -1;
    seq->ver.velTableAddr = -1;
    seq->ver.fireEmbDurVelTableAddr = -1;

    // mov   y,#$00
    // mov   a,($..)+y
    // incw  $..
    // push  a
    // mov   a,($..)+y
    // incw  $..
    // mov   y,a
    // pop   a
    if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x8d\\\x00\xf7.\x3a.\x2d\xf7.\x3a.\xfd\xae", SPC_ARAM_SIZE, NULL)) >= 0) {
        if (   aRAM[pos1 + 0x3] == aRAM[pos1 + 0x5]
            && aRAM[pos1 + 0x5] == aRAM[pos1 + 0x8]
            && aRAM[pos1 + 0x8] == aRAM[pos1 + 0xa]
        )
            seq->ver.blockPtrAddr = aRAM[pos1 + 0x3];
    }
    // variant: Dragon Ball Z: Super Butouden 2
    // mov   y,#$00
    // mov   a,($4d)+y
    // mov   $00,a
    // mov   $4f,a
    // incw  $4d
    // mov   a,($4d)+y
    // mov   $01,a
    // mov   $50,a
    // incw  $4d
    else if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x8d\\\x00\xf7.\xc4.\xc4.\x3a.\\\xf7.\xc4.\xc4.\x3a.", SPC_ARAM_SIZE, NULL)) >= 0) {
        if (   aRAM[pos1 + 0x3] == aRAM[pos1 + 0x9]
            && aRAM[pos1 + 0x9] == aRAM[pos1 + 0xb]
            && aRAM[pos1 + 0x9] == aRAM[pos1 + 0x11]
            && (aRAM[pos1 + 0x5] + 1) == aRAM[pos1 + 0xd]
            && (aRAM[pos1 + 0x7] + 1) == aRAM[pos1 + 0xf]
        )
            seq->ver.blockPtrAddr = aRAM[pos1 + 0x3];
    }

    if (seq->ver.blockPtrAddr >= 0x00 && seq->ver.blockPtrAddr <= 0xfe) {
        // mov   a,$....+x
        // mov   y,a
        // mov   a,$....+x
        // movw  blockPtrAddr,ya
        byte songLoad1[] = { 0xf5, '.', '.', 0xfd, 0xf5, '.', '.', 0xda, '\\', '.', 0 };
        byte songLoad2[] = { 0x1c, 0x5d, 0xf5, '.', '.', 0xfd, 0xd0, 0x03, 0xc4, '.', 0x6f, 0xf5, '.', '.', 0xda, '\\', '.', 0 };
        // mov   a,$....+y
        // mov   blockPtrAddr,a
        // mov   a,$....+y
        // mov   blockPtrAddr+1,a
        byte songLoad3[] = { 0xf6, '.', '.', 0xc4, '\\', '.', 0xf6, '.', '.', 0xc4, '\\', '.', 0 };
        // variant: Yoshi's Safari
        // 1488: fd        mov   y,a
        // 1489: f7 48     mov   a,($48)+y
        // 148b: c4 4c     mov   $4c,a
        // 148d: fc        inc   y
        // 148e: f7 48     mov   a,($48)+y
        // 1490: c4 4d     mov   $4d,a
        byte songLoad4[] = { 0xfd, 0xf7, '.', 0xc4, '.', 0xfc, 0xf7, '.', 0xc4, '.', 0 };

        songLoad1[9]  = (byte) seq->ver.blockPtrAddr;
        songLoad2[16] = (byte) seq->ver.blockPtrAddr;
        songLoad3[5]  = (byte) seq->ver.blockPtrAddr;
        songLoad3[11] = (byte) (seq->ver.blockPtrAddr + 1);
        songLoad4[4]  = (byte) seq->ver.blockPtrAddr;
        songLoad4[9]  = (byte) (seq->ver.blockPtrAddr + 1);

        if ((pos1 = indexOfHexPat(aRAM, songLoad1, SPC_ARAM_SIZE, NULL)) >= 0) {
            seq->ver.seqListAddr = mget2l(&aRAM[pos1 + 5]);
        }
        else if ((pos1 = indexOfHexPat(aRAM, songLoad2, SPC_ARAM_SIZE, NULL)) >= 0) {
            seq->ver.seqListAddr = mget2l(&aRAM[pos1 + 12]);
        }
        else if ((pos1 = indexOfHexPat(aRAM, songLoad3, SPC_ARAM_SIZE, NULL)) >= 0) {
            seq->ver.seqListAddr = mget2l(&aRAM[pos1 + 1]);
        }
        else if ((pos1 = indexOfHexPat(aRAM, songLoad4, SPC_ARAM_SIZE, NULL)) >= 0 &&
                aRAM[pos1 + 2] == aRAM[pos1 + 7]) {
            byte songPtrAddr = aRAM[pos1 + 2];

            // 0880: 8f 00 46  mov   $46,#$00
            // 0883: 8f 1c 47  mov   $47,#$1c
            // 0886: 8f 00 48  mov   $48,#$00
            // 0889: 8f 1e 49  mov   $49,#$1e
            // 088c: 8f 00 4a  mov   $4a,#$00
            // 088f: 8f 1f 4b  mov   $4b,#$1f
            if ((pos2 = indexOfHexPat(aRAM, (const byte *) "\x8f..\x8f..\x8f..\x8f..\x8f..\x8f..", SPC_ARAM_SIZE, NULL)) >= 0 &&
                    aRAM[pos2 + 2]  + 1 == aRAM[pos2 + 5] &&
                    aRAM[pos2 + 5]  + 1 == aRAM[pos2 + 8] &&
                    aRAM[pos2 + 8]  + 1 == aRAM[pos2 + 11] &&
                    aRAM[pos2 + 11] + 1 == aRAM[pos2 + 14] &&
                    aRAM[pos2 + 14] + 1 == aRAM[pos2 + 17] &&
                    aRAM[pos2 + 8] == songPtrAddr) {
                seq->ver.seqListAddr = aRAM[pos2 + 7] | (aRAM[pos2 + 10] << 8);
            }
        }
    }

    // push  a
    // xcn   a
    // and   a,#$07
    // mov   y,a
    // mov   a,$....+y
    // mov   $....+x,a
    // pop   a
    // and   a,#$0f
    // mov   y,a
    // mov   a,$....+y
    // mov   $....+x,a
    if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x2d\x9f\x28\x07\xfd\xf6..\xd5..\xae\x28\x0f\xfd\xf6..\xd5..", SPC_ARAM_SIZE, NULL)) >= 0) {
        seq->ver.durTableAddr = mget2l(&aRAM[pos1 + 6]);
        seq->ver.velTableAddr = mget2l(&aRAM[pos1 + 16]);
    }
    // mov   $..,a
    // lsr   $..
    // asl   a
    // adc   a,$..
    // mov   $....+x,a
    // call  $....
    // bmi   $07
    // asl   a
    // mov   $....+x,a
    else if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\xc4.\x4b.\x1c\x84.\xd5..\x3f..\x30\x07\x1c\xd5..", SPC_ARAM_SIZE, NULL)) >= 0) {
        seq->ver.noteInfoType = SPC_NOTEPARAM_DIR;
    }

    // Special Case: Gradius 3 (Konami)
    // mov   y,#$00
    // mov   a,($40)+y
    // incw  $40
    // push  a
    // mov   a,($40)+y
    // beq   $0737
    // incw  $40
    // mov   y,a
    // pop   a
    if (version == SPC_VER_STD_MODIFIED && (pos1 = indexOfHexPat(aRAM, (const byte *) "\x8d\\\x00\xf7.\x3a.\x2d\xf7.\\\xf0.\x3a.\xfd\xae", SPC_ARAM_SIZE, NULL)) >= 0 &&
        aRAM[pos1 + 0x3] == aRAM[pos1 + 0x5] &&
        aRAM[pos1 + 0x5] == aRAM[pos1 + 0x8] && 
        aRAM[pos1 + 0x8] == aRAM[pos1 + 0xc])
    {
        byte codeSongLoad[] = { 0xf5, '.', '.', 0xf0, '.', 0xfd, 0xf5, '.', '.', 0xda, '\\', '.' /* seq->ver.blockPtrAddr */, 0 };

        seq->ver.blockPtrAddr = aRAM[pos1 + 0x3];
        codeSongLoad[11] = (byte) seq->ver.blockPtrAddr;

        // mov   x,a
        // mov   a,$....+x
        // beq   $..
        // mov   y,a
        // mov   a,$....+x
        // movw  blockPtrAddr,ya
        if ((pos1 = indexOfHexPat(aRAM, codeSongLoad, SPC_ARAM_SIZE, NULL)) >= 0)
        {
            seq->ver.seqListAddr = mget2l(&aRAM[pos1 + 7]);

            // push  a
            // xcn   a
            // and   a,#$07
            // mov   y,a
            // mov   a,$....+y
            // mov   $0201+x,a         ;   set dur% from high nybble
            // pop   a
            // and   a,#$0f
            // mov   y,a
            // mov   a,$....+y
            // clrc
            // adc   a,$....+x
            // mov   $....+x,a         ;   set per-note vol from low nybble
            if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x2d\x9f\x28\x07\xfd\xf6..\xd5..\xae\x28\x0f\xfd\xf6..\x60\x95..\xd5..", SPC_ARAM_SIZE, NULL)) >= 0)
            {
                seq->ver.durTableAddr = mget2l(&aRAM[pos1 + 6]);
                seq->ver.velTableAddr = mget2l(&aRAM[pos1 + 16]);

                if (seq->ver.vcmdListAddr != -1 && seq->ver.vcmdLensAddr != -1)
                {
                    version = SPC_VER_KONAMI;

                    // 0724: 8d 00     mov   y,#$00
                    // 0726: f7 40     mov   a,($40)+y
                    // 0728: 3a 40     incw  $40
                    // 072a: 2d        push  a
                    // 072b: f7 40     mov   a,($40)+y
                    // 072d: f0 08     beq   $0737
                    // 072f: 3a 40     incw  $40
                    // 0731: fd        mov   y,a
                    // 0732: ae        pop   a
                    // 0733: 7a 4b     addw  ya,$4b
                    if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x8d\\\x00\xf7.\x3a.\x2d\xf7.\xf0\x08\x3a.\xfd\xae\x7a.", SPC_ARAM_SIZE, NULL)) >= 0)
                    {
                        seq->ver.konamiAddrBase = mget2l(&aRAM[aRAM[pos1 + 16]]);
                    }
                }
            }
        }
    }

    // Special Case: Yoshi's Safari
    // ; dispatch vcmd in A (e0-ff)
    // 10ce: 28 1f     and   a,#$1f
    // 10d0: 1c        asl   a
    // 10d1: fd        mov   y,a
    // 10d2: f6 dc 10  mov   a,$10dc+y
    // 10d5: 2d        push  a
    // 10d6: f6 db 10  mov   a,$10db+y
    // 10d9: 2d        push  a
    // 10da: 6f        ret
    if (version == SPC_VER_UNKNOWN && (pos1 = indexOfHexPat(aRAM, (const byte *) "\x28\x1f\x1c\xfd\xf6..\x2d\xf6..\x2d\x6f", SPC_ARAM_SIZE, NULL)) >= 0 &&
            mget2l(&aRAM[pos1 + 5]) == mget2l(&aRAM[pos1 + 9]) + 1)
    {
        seq->ver.vcmdListAddr = mget2l(&aRAM[pos1 + 9]);

        // 0b2a: 80        setc
        // 0b2b: a8 e0     sbc   a,#$e0
        // 0b2d: cb 00     mov   $00,y
        // 0b2f: fd        mov   y,a
        // 0b30: f6 eb 0b  mov   a,$0beb+y ; read vcmd length
        if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x80\xa8\xe0\xcb.\xfd\xf6..", SPC_ARAM_SIZE, NULL)) >= 0)
        {
            seq->ver.vcmdLensAddr = mget2l(&aRAM[pos1 + 7]);

            // 0a47: 28 0f     and   a,#$0f
            // 0a49: fd        mov   y,a
            // 0a4a: f6 0b 0c  mov   a,$0c0b+y
            // 0a4d: d5 90 03  mov   $0390+x,a         ; set per-note vol from low nybble
            // 0a50: ae        pop   a
            // 0a51: 5c        lsr   a
            // 0a52: 5c        lsr   a
            // 0a53: 5c        lsr   a
            // 0a54: 5c        lsr   a
            // 0a55: fd        mov   y,a
            // 0a56: f6 80 1d  mov   a,$1d80+y
            // 0a59: d5 40 06  mov   $0640+x,a         ; set dur% from high nybble
            if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x28\x0f\xfd\xf6..\xd5..\xae\\\x5c\\\x5c\\\x5c\\\x5c\xfd\xf6..\xd5..", SPC_ARAM_SIZE, NULL)) >= 0)
            {
                seq->ver.velTableAddr = mget2l(&aRAM[pos1 + 4]);
                seq->ver.durTableAddr = mget2l(&aRAM[pos1 + 16]);

                if (seq->ver.blockPtrAddr != -1 && seq->ver.seqListAddr != -1)
                {
                    version = SPC_VER_YSFR;
                }
            }
        }
        seq->ver.vcmdByteMin = 0xe0;
    }

    // (Fire Emblem 3/4)
    // ; dispatch vcmd in A (d6-ff)
    // 0845: 1c        asl   a                 ; d6-ff => ac-fe (8 bit)
    // 0846: fd        mov   y,a
    // 0847: f6 22 07  mov   a,$0722+y
    // 084a: 2d        push  a
    // 084b: f6 21 07  mov   a,$0721+y
    // 084e: 2d        push  a                 ; push jump address from table
    // 084f: dd        mov   a,y
    // 0850: 5c        lsr   a
    // 0851: fd        mov   y,a
    // 0852: f6 c7 07  mov   a,$07c7+y         ; vcmd length
    // 0855: f0 06     beq   $085d             ; if non zero
    if (version == SPC_VER_UNKNOWN && (pos1 = indexOfHexPat(aRAM, (const byte *) "\x1c\xfd\xf6..\x2d\xf6..\x2d\xdd\\\x5c\xfd\xf6..\xf0.", SPC_ARAM_SIZE, NULL)) >= 0 &&
            mget2l(&aRAM[pos1 + 3]) == mget2l(&aRAM[pos1 + 7]) + 1)
    {
        // 074b: 68 d6     cmp   a,#$d6
        // 074d: 90 05     bcc   $0754
        // 074f: 3f 45 08  call  $0845             ; vcmds d6-ff
        byte codeCallVcmdDispatcher[] = { 0x68, '.', 0x90, '.', 0x3f, pos1 & 0xff, (pos1 >> 8) & 0xff, '\0' };
        if ((pos2 = indexOfHexPat(aRAM, codeCallVcmdDispatcher, SPC_ARAM_SIZE, NULL)) >= 0)
        {
            seq->ver.vcmdByteMin = aRAM[pos2 + 1];
            seq->ver.vcmdListAddr = ((seq->ver.vcmdByteMin * 2) & 0xff) + mget2l(&aRAM[pos1 + 7]);
            seq->ver.vcmdLensAddr = (seq->ver.vcmdByteMin & 0x7f) + mget2l(&aRAM[pos1 + 14]);
            seq->ver.noteInfoType = SPC_NOTEPARAM_INT;

            if (seq->ver.durTableAddr != -1 && seq->ver.velTableAddr != -1)
            {
                // It has usual N-SPC dur/vel code.
                // However, it also has its own dur/vel code. (compatible with FE4)

                // ; intelligent style - set from larger table
                // 062f: 68 40     cmp   a,#$40
                // 0631: b0 0c     bcs   $063f
                // ; 00-3f - set dur% from least 6 bits
                // 0633: 28 3f     and   a,#$3f
                // 0635: fd        mov   y,a
                // 0636: f6 00 ff  mov   a,$ff00+y
                // 0639: d5 01 02  mov   $0201+x,a
                // 063c: 5f 43 07  jmp   $0743
                // ; 40-7f - set per-note vol from least 6 bits
                // 063f: 28 3f     and   a,#$3f
                // 0641: fd        mov   y,a
                // 0642: f6 00 ff  mov   a,$ff00+y
                // 0645: d5 10 02  mov   $0210+x,a
                // 0648: 5f 22 07  jmp   $0722
                if ((pos1 = indexOfHexPat(aRAM, "\x68\x40\xb0\x0c\x28\x3f\xfd\xf6..\xd5..\x5f..\x28\x3f\xfd\xf6..\xd5..\x5f..", SPC_ARAM_SIZE, NULL)) >= 0 &&
                    mget2l(&aRAM[pos1 + 8]) == mget2l(&aRAM[pos1 + 20]))
                {
                    seq->ver.fireEmbDurVelTableAddr = mget2l(&aRAM[pos1 + 8]);
                    version = SPC_VER_FE3;
                }
            }
            else
            {
                // It does not have usual N-SPC dur/vel code.

                // 0932: 68 40     cmp   a,#$40
                // 0934: 28 3f     and   a,#$3f
                // 0936: fd        mov   y,a
                // 0937: f6 38 10  mov   a,$1038+y
                // 093a: b0 05     bcs   $0941
                // 093c: d5 11 02  mov   $0211+x,a         ;   00-3f - set dur%
                // 093f: 2f ee     bra   $092f             ;   check more bytes
                // 0941: d5 20 02  mov   $0220+x,a         ;   40-7f - set vel
                if ((pos1 = indexOfHexPat(aRAM, "\x68\x40\x28\x3f\xfd\xf6..\xb0\x05\xd5..\x2f.\xd5..", SPC_ARAM_SIZE, NULL)) >= 0)
                {
                    seq->ver.fireEmbDurVelTableAddr = mget2l(&aRAM[pos1 + 6]);
                    version = SPC_VER_FE4;
                }
            }
        }
    }

    if (version == SPC_VER_STD_AT_LEAST && seq->ver.vcmdByteMin == 0xda && seq->ver.vcmdLensAddr != -1 &&
        memcmp(&aRAM[seq->ver.vcmdLensAddr], NINT_TA_EVT_LEN_TABLE, sizeof(NINT_TA_EVT_LEN_TABLE)) == 0)
    {
        version = SPC_VER_TA;
    }

    // 0ad0: 30 1e     bmi   $0af0             ; vcmds 01-7f - note info:
    // 0ad2: d5 00 02  mov   $0200+x,a         ; set duration by opcode
    // 0ad5: 3f 85 0b  call  $0b85             ; read next byte
    // 0ad8: 30 16     bmi   $0af0             ; process it, if < $80
    // 0ada: c4 11     mov   $11,a
    // 0adc: 4b 11     lsr   $11
    // 0ade: 1c        asl   a                 ; a  = (a << 1) | (a & 1)
    // 0adf: 84 11     adc   a,$11             ; a += (a >> 1)
    // 0ae1: d5 01 02  mov   $0201+x,a         ; set duration rate
    // 0ae4: 3f 85 0b  call  $0b85             ; read next byte
    // 0ae7: 30 07     bmi   $0af0             ; process it, if < $80
    // 0ae9: 1c        asl   a                 ; a *= 2
    // 0aea: d5 10 02  mov   $0210+x,a         ; set per-note volume (velocity)
    if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x30\x1e\xd5..\x3f..\x30.\xc4.\x4b.\x1c\x84.\xd5..\x3f..\x30\x07\x1c\xd5..", SPC_ARAM_SIZE, NULL)) >= 0)
    {
        seq->ver.noteInfoType = SPC_NOTEPARAM_DIR;
        version = SPC_VER_LEM;
    }

    if (nintSpcForceSongListAddr >= 0)
        seq->ver.seqListAddr = nintSpcForceSongListAddr;
    if (nintSpcForceBlockPtrAddr >= 0)
        seq->ver.blockPtrAddr = nintSpcForceBlockPtrAddr;
    if (nintSpcForceDurTableAddr >= 0)
        seq->ver.durTableAddr = nintSpcForceDurTableAddr;
    if (nintSpcForceVelTableAddr >= 0)
        seq->ver.velTableAddr = nintSpcForceVelTableAddr;

    if (seq->ver.seqListAddr == -1
        || seq->ver.blockPtrAddr == -1
        || (seq->ver.noteInfoType == SPC_NOTEPARAM_STD && (seq->ver.durTableAddr == -1 || seq->ver.velTableAddr == -1))
        || (seq->ver.noteInfoType == SPC_NOTEPARAM_INT && seq->ver.fireEmbDurVelTableAddr == -1))
    {
        version = SPC_VER_UNKNOWN;
    }
    //else if (version == SPC_VER_UNKNOWN) {
    //    version = SPC_VER_STD_AT_LEAST;
    //    fprintf(stderr, "Warning: Collected enough info, but couldn't guess version info exactly.\n");
    //}

    // individualities
    switch (version) {
      case SPC_VER_OLD:
        seq->ver.noteByteMax = 0xc5;
        seq->ver.tieByte = 0xc6;
        seq->ver.restByte = 0xc7;
        seq->ver.percByteMin = 0xd0;
        seq->ver.percByteMax = 0xd9;
        seq->ver.vcmdByteMin = 0xda;
        seq->ver.pitchSlideByte = 0xdd;
        seq->ver.defaultTempo = 0x36;
        break;
    }

    if (version != SPC_VER_OLD && vcmdListAddr >= 0) {
        pos1 = mget2l(&aRAM[vcmdListAddr + 0x1a * 2]);
        if (aRAM[pos1] == 0x6f) // ret
            seq->ver.percBaseIsNYI = true;
    }

    // build MML q fix table automatically
    if (nintSpcAutoQFix && seq->ver.durTableAddr != -1 && seq->ver.velTableAddr != -1) {
        const byte smwDur[] = { 0x33, 0x66, 0x80, 0x99, 0xb3, 0xcc, 0xe6, 0xff };
        const byte smwVel[] = { 0x08, 0x12, 0x1b, 0x24, 0x2c, 0x35, 0x3e, 0x47, 0x51, 0x5a, 0x62, 0x6b, 0x7d, 0x8f, 0xa1, 0xb3 };
        const byte *dst, *src;
        int *result;
        int tableLen;
        int i, j;
        int tableNum;

        dst = smwDur;
        src = &aRAM[seq->ver.durTableAddr];
        result = nintSpcMMLDurFix;
        tableLen = 8;
        for (tableNum = 0; tableNum < 2; tableNum++) {
            for (i = 0; i < tableLen; i++) {
                int minIndex;
                double minDist = 256;
                double srcRate = (double) src[i] / src[tableLen-1];
                double dstRate;
                double diff, lastDiff;

                for (j = 0; j < tableLen; j++) {
                    dstRate = (double) dst[j] / dst[tableLen-1];
                    diff = fabs(dstRate - srcRate);

                    if (j > 0 && diff > lastDiff)
                        break;
                    lastDiff = diff;

                    if (diff < minDist) {
                        minDist = diff;
                        minIndex = j;
                    }
                }
                result[i] = minIndex;
            }

            dst = smwVel;
            src = &aRAM[seq->ver.velTableAddr];
            result = nintSpcMMLVelFix;
            tableLen = 16;
        }

        fprintf(stderr, "Duration Curve: ");
        for (i = 0; i < 8; i++) {
            fprintf(stderr, "%s%d", i ? ", " : "", nintSpcMMLDurFix[i]);
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "Velocity Curve: ");
        for (i = 0; i < 16; i++) {
            fprintf(stderr, "%s%d", i ? ", " : "", nintSpcMMLVelFix[i]);
        }
        fprintf(stderr, "\n");
    }

    seq->ver.id = version;
    nintSpcSetEventList(seq);
    return version;
}

/** read next block from block ptr. */
bool nintSpcReadNewBlock (NintSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int blockAddr;
    int prevBlockLenTick;
    int tr;

    // 
    prevBlockLenTick = seq->tick - seq->blockStartTick;
    if (prevBlockLenTick) {
        for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
            if (!seq->track[tr].active && !seq->looped) {
                //int restTick = prevBlockLenTick;

                sbprintf(seq->track[tr].mml, "; %d ticks\n", prevBlockLenTick);
                sbprintf(seq->track[tr].mml, "r=%d\n", prevBlockLenTick);
/*
                while (restTick) {
                    if (restTick < 96) {
                        getNoteLenForMML(noteLenText, restTick, SPC_TIMEBASE);
                        sbprintf(seq->track[tr].mml, "r%s ", noteLenText);
                        restTick = 0;
                    }
                    else {
                        getNoteLenForMML(noteLenText, 96, SPC_TIMEBASE);
                        sbprintf(seq->track[tr].mml, "r%s ", noteLenText);
                        restTick -= 96;
                    }
                }
                sbprintf(seq->track[tr].mml, "\n");
*/
            }
        }
    }
    seq->blockStartTick = seq->tick;

    if (seq->ver.id == SPC_VER_YSFR)
    {
        bool infiniteLoop = false;
        do
        {
            seq->blockPtrAlt = seq->blockPtr;
            seq->blockLooped = 0;

            blockAddr = mget2l(&aRAM[seq->blockPtr]);
            memset(&seq->aRAMRef[seq->blockPtr], 0x7f, 2);
            seq->blockPtr += 2;

            if (blockAddr == 0) {
                // end
                seq->active = false;
                return false;
            }
            else if (blockAddr <= 0xff)
            {
                // jump (repeat until)
                bool doJump = true;
                if (seq->blockLoopCnt == 0)
                {
                    seq->blockLoopCnt = blockAddr;
                    infiniteLoop |= (seq->blockLoopCnt == 0 || seq->blockLoopCnt == 0xff);
                }
                else if (seq->blockLoopCnt != 0xff) // $ff = infinite loop
                {
                    infiniteLoop |= (seq->blockLoopCnt == 0);

                    seq->blockLoopCnt--;
                    doJump = (seq->blockLoopCnt != 0);
                }
                else
                {
                    infiniteLoop |= (seq->blockLoopCnt == 0xff);
                }

                blockAddr = mget2l(&aRAM[seq->blockPtr]);
                memset(&seq->aRAMRef[seq->blockPtr], 0x7f, 2);
                if (doJump)
                    seq->blockPtr = blockAddr;
                continue;
            }
            // else: play the section, fail through
            if (infiniteLoop)
            {
                seq->looped++;
                if (nintSpcLoopMax > 0 && seq->looped >= nintSpcLoopMax) {
                    seq->active = false;
                    return false;
                }
            }
            break;
        } while(true);
    }
    else
    {
        if (seq->blockLoopCnt == 0) {
            // read next block
            seq->blockPtr += 2; // skip old block addr
            seq->blockPtrAlt = seq->blockPtr;
            seq->blockLooped = 0;

            blockAddr = mget2l(&aRAM[seq->blockPtr]);
            memset(&seq->aRAMRef[seq->blockPtr], 0x7f, 2);
            if (blockAddr == 0) {
                seq->active = false;
                return false;
            }
            else if ((blockAddr & 0xff00) == 0) {
                seq->blockLoopCnt = utos1(blockAddr & 0xff);
                seq->blockPtr += 2;
                blockAddr = mget2l(&aRAM[seq->blockPtr]);
                memset(&seq->aRAMRef[seq->blockPtr], 0x7f, 2);
                if (blockAddr == 0) {
                    seq->active = false;
                    return false;
                }
                blockAddr += seq->ver.konamiAddrBase;

                if (seq->blockLoopCnt < 0) {
                    seq->looped++;
                    if (nintSpcLoopMax > 0 && seq->looped >= nintSpcLoopMax) {
                        seq->active = false;
                        return false;
                    }
      
                    seq->blockLoopCnt = 1;
                    seq->blockPtr = blockAddr;
                    return nintSpcReadNewBlock(seq);
                }
            }
            else
            {
                blockAddr += seq->ver.konamiAddrBase;
            }
        }
        else {
            // repeat block
            if (seq->blockLoopCnt > 0)
                seq->blockLoopCnt--;
            seq->blockLooped++;
            blockAddr = mget2l(&aRAM[seq->blockPtr]) + seq->ver.konamiAddrBase;
            memset(&seq->aRAMRef[seq->blockPtr], 0x7f, 2);
        }
    }

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        int newPos;
        int newPosPtr = blockAddr + tr * 2;

        if (newPosPtr >= 0xffff)
        {
            fprintf(stderr, "Error: Access violation during reading track %d pointer at $%04X.\n", tr + 1, newPosPtr);
            seq->active = false;
            return false;
        }

        newPos = mget2l(&aRAM[newPosPtr]);
        memset(&seq->aRAMRef[newPosPtr], 0x7f, 2);

        // cancel repeats
        seq->track[tr].loopCount = 0;

        if (!seq->looped)
            sbprintf(seq->track[tr].mml, "\n; $%04X / $%04X\n", seq->blockPtrAlt, newPos);

        // if not used (hi-byte is zero)
        if ((newPos & 0xff00) == 0) {
            // if the track 'was' used
            if (seq->track[tr].active) {
                nintSpcDequeueNote(seq, tr);
            }
            seq->track[tr].active = false;
            continue;
        }
        seq->track[tr].pos = newPos + seq->ver.konamiAddrBase;
        seq->track[tr].active = (newPos != 0);
    }
    seq->endBlock = false;

    //seq->active = true;
    return nintSpcUpdateActivity(seq);
}

/** detects now playing and find sequence header for it. */
bool nintSpcDetectSeq (NintSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int seqListAddr;
    int blockPtrAddr;
    int headerOfs;
    int songIndex;
    int songIndexInPort;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    seqListAddr = seq->ver.seqListAddr;
    blockPtrAddr = seq->ver.blockPtrAddr;

    songIndex = nintSpcForceSongIndex;
    if (nintSpcForceSongIndex < 0) {
        int songId;
        int dist, minDist = SPC_ARAM_SIZE;
        int curBlock = mget2l(&aRAM[blockPtrAddr]);
        int songAddr;

        if (curBlock != 0) {
            // adjust current block
            curBlock -= 2;

            // auto search
            songIndex = 0;
            for (songId = 0; songId < SPC_SONG_MAX; songId++) {
                songAddr = mget2l(&aRAM[seqListAddr + songId * 2]);
                dist = abs(curBlock - songAddr);
                if (songAddr > curBlock) {
                    dist++; // penalty
                }
                if (dist < minDist) {
                    songIndex = songId;
                    minDist = dist;
                }
            }
        }
        else
        {
            int songIndexTemp;
            for (songIndexTemp = 0; songIndexTemp < 8; songIndexTemp++)
            {
                curBlock = mget2l(&aRAM[seqListAddr + songIndexTemp * 2]);
                if (curBlock != 0)
                {
                    songIndex = songIndexTemp;
                    break;
                }
            }
        }

        // experimental: after all, get song number from APU port.
        if (nintSpcSongFromPort) {
            switch (seq->ver.id) {
            case SPC_VER_OLD:
                songIndexInPort = aRAM[0xf6];
                break;
            case SPC_VER_YSFR:
                songIndexInPort = 0; // NYI
                break;
            default:
                songIndexInPort = aRAM[0xf4];
            }
            if (songIndexInPort != 0 && songIndexInPort != 0xff)
                songIndex = songIndexInPort;
        }
    }

    songIndex += nintSpcContConvCnt;
    headerOfs = mget2l(&aRAM[seqListAddr + songIndex * 2]);
    seq->songIndex = songIndex;
    seq->addrOfHeader = headerOfs;
    seq->blockPtr = headerOfs;
    seq->blockPtrAlt = seq->blockPtr;
    seq->blockLoopCnt = 0;
    seq->endBlock = false;

    if (headerOfs == 0)
        return false;

    nintSpcResetParam(seq);
    memset(seq->aRAMRef, 0, SPC_ARAM_SIZE);

    // enter to first block
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].active = false;
    }
    if (seq->ver.id != SPC_VER_YSFR)
        seq->blockPtr -= 2;
    return nintSpcReadNewBlock(seq);
}

/** create new spc2mid object. */
static NintSpcSeqStat *newNintSpcSeq (const byte *aRAM)
{
    NintSpcSeqStat *newSeq = (NintSpcSeqStat *) calloc(1, sizeof(NintSpcSeqStat));

    if (newSeq) {
        int tr;

        newSeq->aRAM = aRAM;

        newSeq->aRAMRef = (byte *) calloc(SPC_ARAM_SIZE, sizeof(byte));
        if (!newSeq->aRAMRef) {
            free(newSeq);
            return NULL;
        }

        nintSpcCheckVer(newSeq);
        if (!nintSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }

        // mml logger
        for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
            StringStreamBuf *log;

            log = newStringStreamBuf();
            if (!log) {
                for (; tr >= 0; tr--) {
                    delStringStreamBuf(newSeq->track[tr].mml);
                }
                free(newSeq->aRAMRef);
                free(newSeq);
                return NULL;
            }
            newSeq->track[tr].mml = log;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delNintSpcSeq (NintSpcSeqStat **seq)
{
    if (*seq) {
        // do not kill smf here

        int tr;

        for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
            delStringStreamBuf((*seq)->track[tr].mml);
        }
        free((*seq)->aRAMRef);
        free(*seq);
        *seq = NULL;
    }
}

//----

/** outputs html header. */
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

/** outputs html footer. */
static void printHtmlFooter (void)
{
    myprintf("  </body>\n");
    myprintf("</html>\n");
}

/** output seq info list. */
static void printHtmlInfoList (NintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");
    myprintf("          <li>Version: %s", nintSpcVerToStrHtml(seq->ver.id));
    if (seq->ver.id == SPC_VER_YSFR)
    {
        myprintf("            <ul>\n");
        myprintf("              <li>This version has a different volume balance calculation algorithm from usual version, but this tool does not care about that.</li>\n");
        myprintf("            </ul>\n");
    }
    myprintf("</li>\n");
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Block Pointer: $%02X</li>\n", seq->ver.blockPtrAddr);
    if (seq->ver.noteInfoType != SPC_NOTEPARAM_DIR) {
        if (seq->ver.id != SPC_VER_FE4)
        {
            myprintf("          <li>Duration Table: $%04X</li>\n", seq->ver.durTableAddr);
            myprintf("          <li>Velocity Table: $%04X</li>\n", seq->ver.velTableAddr);
        }
        if (seq->ver.id == SPC_VER_FE3 || seq->ver.id == SPC_VER_FE4)
        {
            myprintf("          <li>Fire Emblem Dur/Vel Table: $%04X</li>\n", seq->ver.fireEmbDurVelTableAddr);
        }
    }
    if (seq->ver.id == SPC_VER_KONAMI)
    {
            myprintf("          <li>Address Base: $%04X</li>\n", seq->ver.konamiAddrBase);
    }
    myprintf("          <li>Voice Commands<ul>\n");
    myprintf("            <li>First Command: $%02X</li>\n", seq->ver.vcmdByteMin);
    myprintf("            <li>Dispatch Table: $%04X</li>\n", seq->ver.vcmdListAddr);
    myprintf("            <li>Length Table: $%04X</li>\n", seq->ver.vcmdLensAddr);
    myprintf("          </ul></li>\n");
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (NintSpcSeqStat *seq)
{
    int tr;
    int blockPtr;
    int blockAddr;
    int blockCnt;
    int dumpBlockPtr;
    const byte *aRAM = seq->aRAM;

    if (seq == NULL)
        return;

    myprintf("          <li>Sequence: $%04X (Song $%02X)<ul>\n", seq->addrOfHeader, seq->songIndex);

    blockPtr = seq->addrOfHeader;
    do {
        bool hasBlock;

        dumpBlockPtr = blockPtr;
        blockAddr = mget2l(&aRAM[blockPtr]);
        if (blockAddr != 0 && (blockAddr & 0xff00) == 0) {
            blockCnt = utos1(blockAddr & 0xff);
            if (blockCnt > 0)
                blockCnt++;
            blockPtr += 2;
            blockAddr = mget2l(&aRAM[blockPtr]);
        }
        else
        {
            blockCnt = 0;
        }

        hasBlock = (blockAddr & 0xff00) && (blockCnt >= 0);
        if ((blockAddr & 0xff00) != 0)
            blockAddr += seq->ver.konamiAddrBase;
        myprintf("            <li>");
        if (hasBlock)
            myprintf("<a href=\"#block-%04x\">", dumpBlockPtr);
        myprintf("Block $%04X", dumpBlockPtr);
        if (hasBlock)
            myprintf("</a>");
        if (blockCnt < 0)
            myprintf(" -> [$%02X] $%04X", blockCnt & 0xff, blockAddr);
        else {
            myprintf(": $%04X", blockAddr);
            if (blockCnt && blockAddr != 0)
                myprintf(" * %d", blockCnt);
        }

        if (hasBlock) {
            myprintf("<ul>\n");
            myprintf("              <li>");

            for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                int trackAddr = mget2l(&aRAM[blockAddr + tr * 2]);
                if (tr)
                    myprintf(" ");
                if (trackAddr != 0)
                    trackAddr += seq->ver.konamiAddrBase;
                myprintf("%d:$%04X", tr + 1, trackAddr);
            }

            myprintf("</li>\n");
            myprintf("            </ul>");
        }

        myprintf("</li>\n");
        blockPtr += 2;
    } while (blockAddr != 0 && blockCnt >= 0);
}

/** output event dump. */
static void printHtmlEventDump (NintSpcSeqStat *seq, SeqEventReport *ev)
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

/** outputs event table header. */
static void printEventTableHeader (NintSpcSeqStat *seq)
{
    int blockPtr = seq->blockPtrAlt;
    int tr;

    if (seq == NULL)
        return;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        static char trackName[256];

        //if (!seq->track[tr].active)
        //    continue;

        sprintf(trackName, "$%04X / $%04X", seq->blockPtrAlt, seq->track[tr].pos);
        if (!nintSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, seq->track[tr].tick, tr, SMF_META_TEXT, trackName);
    }

    myprintf("        <h3>Block $%04X</h3>\n", blockPtr);
    myprintf("        <div class=\"section\" id=\"block-%04x\">\n", blockPtr);
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

/** outputs event table footer. */
static void printEventTableFooter (NintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** output mml log. */
static void printMML (NintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    if (nintSpcMMLLog) {
        int mmlTemp;
        int tr;

        fprintf(nintSpcMMLLog, "\"VCMD_PATCH=$da\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PANPOT=$db\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PAN_FADE=$dc\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_VIBRATO_ON=$de\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_VIBRATO_OFF=$df\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_MASTER_VOL_FADE=$e1\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_MASTER_VOLUME=$e0\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_SET_TEMPO=$e2\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_TEMPO_FADE=$e3\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_GLOBAL_TRANSPOSE=$e4\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PERVOICE_TRANSPOSE=\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_TREMOLO_ON=$e5\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_TREMOLO_OFF=$e5 $00 $00 $00\" ; $e6\n");
        fprintf(nintSpcMMLLog, "\"VCMD_VOLUME=$e7\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_VOL_FADE=$e8\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_SUBROUTINE=$e9\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_VIBRATO_FADE=$ea\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PITCHENV_TO=$eb\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PITCHENV_FROM=$ec\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PITCHENV_OFF=$eb $00 $00 $00\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_TUNING=$ee\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_ECHO_ON=$ef\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_ECHO_OFF=$f0\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_ECHO_PARAM=$f1\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_ECHO_VOL_FADE=$f2\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PITCH_SLIDE=$dd\"\n");
        fprintf(nintSpcMMLLog, "\"VCMD_PERC_PATCH_BASE=\"\n");
        fprintf(nintSpcMMLLog, "\n");
        for (mmlTemp = 0; mmlTemp < 256; mmlTemp++)
            fprintf(nintSpcMMLLog, "\"PATCH%03d=@%d h0 $ed $7f $e0\"\n", mmlTemp, 4);
        fprintf(nintSpcMMLLog, "\n");
        for (mmlTemp = 0; mmlTemp < seq->ver.percByteMax - seq->ver.percByteMin + 1; mmlTemp++)
            fprintf(nintSpcMMLLog, "\"PERC%03dN=@%dc\"\n", mmlTemp, (mmlTemp % 10) + 21);
        for (mmlTemp = 0; mmlTemp < seq->ver.percByteMax - seq->ver.percByteMin + 1; mmlTemp++)
            fprintf(nintSpcMMLLog, "\"PERC%03dX=@%dc\"\n", mmlTemp, (mmlTemp % 10) + 21);

        for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
            if (!seq->track[tr].used)
                continue;

            fprintf(nintSpcMMLLog, "\n#%d\n", tr);
            fprintf(nintSpcMMLLog, "%s\n", seq->track[tr].mml->s);
        }
    }
}

//----

/** returns note name from vbyte. */
void getNoteNameFromVbyte (char *buf, int vbyte, int keyFix)
{
    int key = (vbyte & 0x7f) + SPC_NOTE_KEYSHIFT + keyFix;

    if (key >= 0x00 && key <= 0x7f)
        getNoteName(buf, key);
    else
        sprintf(buf, "Note %d", key);
}

/** read duration rate from table. */
static int nintSpcDurRateOf (NintSpcSeqStat *seq, int index)
{
    return mget1(&seq->aRAM[seq->ver.durTableAddr + index]);
}

/** read velocity from table. */
static int nintSpcVelRateOf (NintSpcSeqStat *seq, int index)
{
    return mget1(&seq->aRAM[seq->ver.velTableAddr + index]);
}

/** convert SPC tempo into bpm. */
static double nintSpcTempoOf (int tempo)
{
    return (double) tempo * 60000000 / 24576000; // 24576000 = (timer0) 2ms * 48 * 256?
}

/** convert SPC tempo into bpm. */
static double nintSpcTempo (NintSpcSeqStat *seq)
{
    return nintSpcTempoOf(seq->tempo);
}

/** convert SPC channel volume into MIDI one. */
static int nintSpcMidiVolOf (int value)
{
    return value/2; // Note: Nintendo SPC uses exponencial curve for volume

    //if (nintSpcVolIsLinear)
    //    return (int) floor(pow((double) value/255, 2) * 127 + 0.5);
    //else
    //    return value/2;
}

/** create new smf object and link to spc seq. */
static Smf *nintSpcCreateSmf (NintSpcSeqStat *seq)
{
    static char songTitle[512];
    Smf* smf;
    int tr;

    smf = smfCreate(SPC_TIMEBASE);
    if (!smf)
        return NULL;
    seq->smf = smf;

    sprintf(songTitle, "%s %s", APPNAME, VERSION);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, songTitle);

    smfInsertTempoBPM(smf, 0, 0, nintSpcTempo(seq));
    switch (nintSpcMidiResetType) {
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
    //smfInsertNintSpcMasterVolume(smf, 0, 0, seq->masterVolume);

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 14);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);

        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].active ? seq->track[tr].pos : 0x0000);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }
    return smf;
}

//----

static char argDumpStr[512];

static void nintSpcEventNote(NintSpcSeqStat *seq, SeqEventReport *ev);

/** advance seq tick. */
static void nintSpcSeqAdvTick(NintSpcSeqStat *seq)
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
    seq->time += (double) 60 / nintSpcTempo(seq) * minTickStep / SPC_TIMEBASE;
}

/** dump vcmd to mml. */
static void nintSpcAddVcmdToMML (NintSpcSeqStat *seq, const char *cmd, SeqEventReport *ev, bool newLine)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];

    if (!seq->looped && ev->size) {
        int i;

        if (cmd)
            sbprintf(tr->mml, "%s", cmd);
        else
            sbprintf(tr->mml, "$%02x", seq->aRAM[ev->addr]);

        for (i = 1; i < ev->size; i++) {
            sbprintf(tr->mml, " $%02x", seq->aRAM[ev->addr + i]);
        }

        if (newLine)
            sbprintf(tr->mml, "\n");
    }
    tr->mmlWritten = true;
}

/** vcmds: unknown event (without status change). */
static void nintSpcEventUnknownInline (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X\n", ev->code);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X\n", ev->code);
}

/** vcmds: unidentified event. */
static void nintSpcEventUnidentified (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    nintSpcEventUnknownInline(seq, ev);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void nintSpcEventUnknown0 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    nintSpcEventUnknownInline(seq, ev);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void nintSpcEventUnknown1 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void nintSpcEventUnknown2 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void nintSpcEventUnknown3 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void nintSpcEventUnknown4 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    int *p = &seq->track[ev->track].pos;

    ev->size += 4;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void nintSpcEventUnknown5 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    int *p = &seq->track[ev->track].pos;

    ev->size += 5;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;
    arg5 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (6 byte args). */
static void nintSpcEventUnknown6 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5, arg6;
    int *p = &seq->track[ev->track].pos;

    ev->size += 6;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;
    arg5 = seq->aRAM[*p];
    (*p)++;
    arg6 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d, arg6 = %d", arg1, arg2, arg3, arg4, arg5, arg6);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (7 byte args). */
static void nintSpcEventUnknown7 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5, arg6, arg7;
    int *p = &seq->track[ev->track].pos;

    ev->size += 7;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;
    arg5 = seq->aRAM[*p];
    (*p)++;
    arg6 = seq->aRAM[*p];
    (*p)++;
    arg7 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d, arg6 = %d, arg7 = %d", arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (8 byte args). */
static void nintSpcEventUnknown8 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
    int *p = &seq->track[ev->track].pos;

    ev->size += 8;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;
    arg5 = seq->aRAM[*p];
    (*p)++;
    arg6 = seq->aRAM[*p];
    (*p)++;
    arg7 = seq->aRAM[*p];
    (*p)++;
    arg8 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d, arg6 = %d, arg7 = %d, arg8 = %d", arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (36 byte args). */
static void nintSpcEventUnknown36 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;

    ev->size += 36;
    (*p) += 36;

    nintSpcEventUnknownInline(seq, ev);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: reserved. */
static void nintSpcEventReserved (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Reserved (Event %02X)", ev->code);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void nintSpcEventNOP (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00: end of block. */
static void nintSpcEventEndOfBlock (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];

    if (tr->loopCount == 0) {
        sprintf(ev->note, "End Of Block");
        seq->endBlock = true;
    }
    else {
        tr->loopCount--;
        if (tr->loopCount == 0) {
            tr->pos = tr->retnAddr;
            sprintf(ev->note, "Return, addr = $%04X", tr->retnAddr);

            if (!seq->looped)
                sbprintf(tr->mml, "\n; subroutine / return\n", tr->loopCount);
        }
        else {
            sprintf(ev->note, "Loop, addr = $%04X", tr->loopStart);
            tr->pos = tr->loopStart;

            if (!seq->looped)
                sbprintf(tr->mml, "\n; subroutine / loop %d\n", tr->loopCount);
        }
    }

    tr->lastNote.mmlOct = -801;
    tr->mmlWritten = true;
}

/** vcmds: note info params. */
static void nintSpcEventNoteInfo (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    int arg1 = ev->code, arg2, arg3;
    NintSpcTrackStat *tr = &seq->track[ev->track];
    bool hasNextArg;

    sprintf(ev->note, "Note Param, length = %d", arg1);
    strcat(ev->classStr, " ev-noteparam");

    arg2 = seq->aRAM[*p];
    hasNextArg = (arg2 < seq->ver.noteByteMin);

    tr->note.dur = arg1;
    if (hasNextArg) {
        int durRateIndex, durVal;
        int velIndex, velVal;

        switch (seq->ver.noteInfoType) {
        case SPC_NOTEPARAM_DIR:
            tr->note.durRate = ((arg2 << 1) + (arg2 >> 1) + (arg2 & 1)) & 0xff; // uh, what a weird formula (approx % ?)
            sprintf(argDumpStr, ", dur = $%02X", tr->note.durRate);
            strcat(ev->note, argDumpStr);

            (*p)++;
            ev->size++;

            arg2 = seq->aRAM[*p];
            if (arg2 < 0x80) {
                tr->note.vel = arg2 << 1;
                sprintf(argDumpStr, ", vel = $%02X", tr->note.vel);
                strcat(ev->note, argDumpStr);

                (*p)++;
                ev->size++;
            }
            break;
        case SPC_NOTEPARAM_INT:
            if (seq->ver.id != SPC_VER_FE3 || (seq->fe3ByteCA & 0x80) != 0)
            {
                // Intelligent Systems style:
                // The code below does not care the case,
                // that a number less than 0x80 continues for more than 3 bytes
                // in an unexpected order. (for instance, arg1 >= 0x40 && arg2 < 0x40)
                // I think they are just unexpected thing, and I hate complication.
                do {
                    arg2 = seq->aRAM[*p];
                
                    (*p)++;
                    ev->size++;
                              //
                    if (arg2 < 0x40) {
                        durRateIndex = arg2 & 0x3f;
                        durVal = seq->aRAM[seq->ver.fireEmbDurVelTableAddr + durRateIndex];
                        tr->note.durRate = durVal;
                              //
                        sprintf(argDumpStr, ", dur = %d:$%02X", durRateIndex, durVal);
                        strcat(ev->note, argDumpStr);
                    }
                    else {
                        velIndex = arg2 & 0x3f;
                        velVal = seq->aRAM[seq->ver.fireEmbDurVelTableAddr + velIndex];
                        tr->note.vel = velVal;
                              //
                        sprintf(argDumpStr, ", vel = %d:$%02X", velIndex, velVal);
                        strcat(ev->note, argDumpStr);
                    }
                              //
                    arg3 = seq->aRAM[*p];
                } while (arg2 < 0x40 && arg3 < 0x80);
                break;
            }
            // FALL THROUGH, usual N-SPC note params
        default:
            (*p)++;
            ev->size++;

            durRateIndex = (arg2 >> 4) & 7;
            velIndex = arg2 & 15;

            tr->note.durRate = nintSpcDurRateOf(seq, durRateIndex);
            tr->note.vel = nintSpcVelRateOf(seq, velIndex);

            sprintf(argDumpStr, ", dur/vel = $%02X", arg2);
            strcat(ev->note, argDumpStr);

            if (!seq->looped) {
                int fixedQVal = (nintSpcMMLDurFix[(arg2 >> 4) & 7] << 4) | nintSpcMMLVelFix[arg2 & 15];

                sbprintf(tr->mml, "q%02x", fixedQVal);
                if (fixedQVal != arg2) {
                    sbprintf(tr->mml, "; q%02x", arg2);
                }
                sbprintf(tr->mml, "\n");
            }
            tr->mmlWritten = true;
        }
    }

    if (seq->ver.noteInfoType == SPC_NOTEPARAM_STD)
        tr->mmlWritten = true;
}

/** vcmds: note. */
static void nintSpcEventNote(NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int note = ev->code;
    NintSpcTrackStat *tr = &seq->track[ev->track];
    const int mmlOctOfs = 1;
    int mmlKey, mmlOct, mmlLastOct, mmlKeyFix;
    const char *mmlnote[] = { "c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b" };

    // outputput old note first
    nintSpcTruncateNote(seq, ev->track);
    nintSpcDequeueNote(seq, ev->track);

    // set new note
    tr->lastNote.tick = ev->tick;
    tr->lastNote.dur = tr->note.dur;
    tr->lastNote.lastDur = tr->note.dur;
    tr->lastNote.key = (note & 0x7f);
    tr->lastNote.perc = false;
    tr->lastNote.durRate = tr->note.durRate;
    tr->lastNote.vel = tr->note.vel;
    tr->lastNote.transpose = seq->transpose + tr->note.transpose;
    tr->lastNote.patch = tr->note.patch;
    tr->lastNote.tied = false;
    tr->lastNote.active = true;

    // step
    tr->nextTick = tr->tick + tr->note.dur;

    sprintf(ev->note, "Note ");
    getNoteNameFromVbyte(argDumpStr, note, seq->ver.patchFix[tr->note.patch].key);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-note");

    tr->newPerc = true;

    mmlLastOct = tr->lastNote.mmlOct;
    mmlKeyFix = seq->ver.patchFix[tr->note.patch].mmlKey;

    mmlKey = (note & 0x7f) + mmlKeyFix;
    mmlOct = (mmlKey / 12) + mmlOctOfs;
    while (mmlKey < 0)
        mmlKey += 12;
    mmlKey %= 12;

    if (!seq->looped) {
        if (mmlOct != mmlLastOct) {
            if (mmlLastOct == -801)
                sbprintf(tr->mml, "o%d ", mmlOct);
            else {
                if (mmlOct > mmlLastOct)
                    for(; mmlLastOct != mmlOct; mmlLastOct++)
                        sbprintf(tr->mml, "> ");
                else
                    for(; mmlLastOct != mmlOct; mmlLastOct--)
                        sbprintf(tr->mml, "< ");
            }
        }

        getNoteLenForMML(noteLenText, tr->note.dur, SPC_TIMEBASE);
        sbprintf(tr->mml, "%s%s ", mmlnote[mmlKey], noteLenText);
    }
    tr->note.mmlOct = mmlOct;
    tr->lastNote.mmlOct = mmlOct;
    tr->mmlWritten = true;
}

/** vcmd: tie. */
static void nintSpcEventTie (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];

    if (tr->lastNote.active) {
        tr->lastNote.dur += tr->note.dur;
    }
    tr->lastNote.tied = true;

    tr->lastNote.lastDur = tr->note.dur;
    tr->lastNote.durRate = tr->note.durRate;
    tr->nextTick = tr->tick + tr->note.dur;

    sprintf(ev->note, "Tie");
    strcat(ev->classStr, " ev-tie");

    if (!seq->looped) {
        getNoteLenForMML(noteLenText, tr->note.dur, SPC_TIMEBASE);
        sbprintf(tr->mml, "^%s ", noteLenText);
    }
    tr->mmlWritten = true;
}

/** vcmd: rest. */
static void nintSpcEventRest (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];

    // outputput old note first
    nintSpcTruncateNote(seq, ev->track);
    nintSpcDequeueNote(seq, ev->track);

    tr->lastNote.lastDur = tr->note.dur;
    //tr->lastNote.durRate = tr->note.durRate;
    tr->nextTick = tr->tick + tr->note.dur;

    sprintf(ev->note, "Rest");
    strcat(ev->classStr, " ev-rest");

    if (!seq->looped) {
        getNoteLenForMML(noteLenText, tr->note.dur, SPC_TIMEBASE);
        sbprintf(tr->mml, "r%s ", noteLenText);
    }
    tr->mmlWritten = true;
}

/** vcmds: percussion note. */
static void nintSpcEventPercNote (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int note = ev->code | 0x80;
    NintSpcTrackStat *tr = &seq->track[ev->track];

    // outputput old note first
    nintSpcTruncateNote(seq, ev->track);
    nintSpcDequeueNote(seq, ev->track);

    // set new note
    tr->lastNote.tick = ev->tick;
    tr->lastNote.dur = tr->note.dur;
    tr->lastNote.key = note - seq->ver.percByteMin;
    tr->lastNote.perc = true;
    tr->lastNote.lastDur = tr->note.dur;
    tr->lastNote.durRate = tr->note.durRate;
    tr->lastNote.vel = tr->note.vel;
    tr->lastNote.tied = false;
    tr->lastNote.active = true;
    tr->lastNote.mmlOct = -801;

    // step
    tr->nextTick = tr->tick + tr->note.dur;

    sprintf(ev->note, "Perc %d", note - seq->ver.percByteMin + 1);
    strcat(ev->classStr, " ev-perc ev-note");

    if (!tr->newPerc && note != tr->lastPerc)
        tr->newPerc = true;
    tr->lastPerc = note;

    if (!seq->looped) {
        getNoteLenForMML(noteLenText, tr->note.dur, SPC_TIMEBASE);
        sbprintf(tr->mml, "PERC%03d%s%s ", note - seq->ver.percByteMin, tr->newPerc ? "N" : "X", noteLenText);
    }
    tr->mmlWritten = true;
    tr->newPerc = false;
}

/** vcmd: set patch. */
static void nintSpcEventSetPatch (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->note.patch = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[arg1].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[arg1].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[arg1].patchNo);
    tr->newPerc = true;

    sprintf(ev->note, "Set Patch, patch = %d", arg1);
    strcat(ev->classStr, " ev-patch");

    if (!seq->looped)
        sbprintf(tr->mml, "PATCH%03d\n", arg1);
    tr->mmlWritten = true;
}

/** vcmd: set patch with ADSR. */
static void nintSpcEventSetPatchWithADSR (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = mget2b(&seq->aRAM[*p]);
    (*p) += 2;

    tr->note.patch = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[arg1].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[arg1].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[arg1].patchNo);
    tr->newPerc = true;

    sprintf(ev->note, "Set Patch with ADSR, patch = %d, ADSR = $%04X", arg1, arg2);
    strcat(ev->classStr, " ev-patch-adsr");

    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: panpot. */
static void nintSpcEventPanpot (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot, val = $%02X", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pan");

    if (!seq->looped) {
        if (arg1 <= 20)
            sbprintf(tr->mml, "y%d\n", arg1);
        else
            sbprintf(tr->mml, "VCMD_PANPOT $%02x\n", arg1);
    }
    tr->mmlWritten = true;
}

/** vcmd: panpot fade. */
static void nintSpcEventPanFade (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Pan Fade, length = $%02X, to = $%02X", arg1, arg2);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-panfade");

    nintSpcAddVcmdToMML(seq, "VCMD_PAN_FADE", ev, true);
}

/** vcmd: vibrato on. */
static void nintSpcEventVibratoOn (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato On, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratoon");

    nintSpcAddVcmdToMML(seq, "VCMD_VIBRATO_ON", ev, true);
}

/** vcmd: vibrato off. */
static void nintSpcEventVibratoOff (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Vibrato Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratooff");

    nintSpcAddVcmdToMML(seq, "VCMD_VIBRATO_OFF", ev, true);
}

/** vcmd: master volume. */
static void nintSpcEventMasterVolume (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume, val = %d", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-mastervol");

    if (!seq->looped)
        sbprintf(tr->mml, "w%d\n", arg1);
    tr->mmlWritten = true;
}

/** vcmd: master volume fade. */
static void nintSpcEventMasterVolFade (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume Fade, length = %d, to = %d", arg1, arg2);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-mastervolfade");

    nintSpcAddVcmdToMML(seq, "VCMD_MASTER_VOL_FADE", ev, true);
}

/** vcmd: tempo. */
static void nintSpcEventTempo (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;
    double bpm;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;
    bpm = nintSpcTempo(seq);
    sprintf(ev->note, "Tempo, val = %d (%f bpm)", arg1, bpm);

    smfInsertTempoBPM(seq->smf, ev->tick, 0, bpm);
    //smfInsertTempoBPM(seq->smf, ev->tick, ev->track, bpm);
    strcat(ev->classStr, " ev-tempo");

    if (!seq->looped)
        sbprintf(tr->mml, "t%d\n", arg1);
    tr->mmlWritten = true;
}

/** vcmd: tempo fade. */
static void nintSpcEventTempoFade (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;
    double bpm;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    // FIXME: do fade
    seq->tempo = arg2;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, nintSpcTempo(seq));

    bpm = nintSpcTempoOf(arg2);

    sprintf(ev->note, "Tempo Fade, length = %d, to = %d (%f bpm)", arg1, arg2, bpm);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tempofade");

    nintSpcAddVcmdToMML(seq, "VCMD_TEMPO_FADE", ev, true);
}

/** vcmd: transpose (global). */
static void nintSpcEventKeyShift (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    seq->transpose = arg1;
    sprintf(ev->note, "Global Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    nintSpcAddVcmdToMML(seq, "VCMD_GLOBAL_TRANSPOSE", ev, true);
}

/** vcmd: transpose (local). */
static void nintSpcEventTranspose (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;
    sprintf(ev->note, "Channel Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose-ch");

    nintSpcAddVcmdToMML(seq, "VCMD_PERVOICE_TRANSPOSE", ev, true);
}

/** vcmd: tremolo on. */
static void nintSpcEventTremoloOn (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tremolo On, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tremoloon");

    nintSpcAddVcmdToMML(seq, "VCMD_TREMOLO_ON", ev, true);
}

/** vcmd: tremolo off. */
static void nintSpcEventTremoloOff (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Tremolo Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tremolooff");

    nintSpcAddVcmdToMML(seq, "VCMD_TREMOLO_OFF", ev, true);
}

/** vcmd: volume. */
static void nintSpcEventVolume (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, val = %d", arg1);
    //if (!nintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-volume");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, nintSpcMidiVolOf(arg1));

    if (!seq->looped)
        sbprintf(tr->mml, "v%d\n", arg1);
    tr->mmlWritten = true;
}

/** vcmd: volume fade. */
static void nintSpcEventVolumeFade (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Fade, length = %d, to = %d", arg1, arg2);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-volumefade");

    nintSpcAddVcmdToMML(seq, "VCMD_VOL_FADE", ev, true);
}

/** vcmd: repeat n times. */
static void nintSpcEventSubroutine (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int dest, count;
    int *p = &tr->pos;

    ev->size += 3;
    dest = mget2l(&seq->aRAM[*p]) + seq->ver.konamiAddrBase;
    (*p) += 2;
    count = seq->aRAM[*p];
    (*p)++;

    tr->retnAddr = *p;
    *p = dest;
    tr->loopStart = dest;
    tr->loopCount = count;

    sprintf(ev->note, "Call/Repeat, addr = $%04X, count = %d", dest, count);
    strcat(ev->classStr, " ev-call");

    nintSpcAddVcmdToMML(seq, "\n; VCMD_SUBROUTINE", ev, true);
}

/** vcmd: (Konami) repeat start. */
static void nintSpcEventKonamiRepeatStart (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    tr->konamiRepeatStart = *p;
    tr->konamiRepeatCount = 0;

    sprintf(ev->note, "Repeat Start");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-repeatstart");

    nintSpcAddVcmdToMML(seq, "\n; VCMD_KONAMI_REPEAT_START", ev, true);
}

/** vcmd: (Konami) repeat end. */
static void nintSpcEventKonamiRepeatEnd (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = utos1(seq->aRAM[*p]);
    (*p)++;
    arg3 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->konamiRepeatCount = (tr->konamiRepeatCount + 1) & 0xff;
    if (tr->konamiRepeatCount != arg1)
    {
        // repeat continue
        *p = tr->konamiRepeatStart;
        sprintf(ev->note, "Repeat Again, count = %d, velocity-diff = %d, tuning-diff = %d / 16 semitones", arg1, arg2, arg3);
    }
    else
    {
        // repeat end
        sprintf(ev->note, "Repeat End");
    }

    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-repeatend");
    nintSpcAddVcmdToMML(seq, "\n; VCMD_KONAMI_REPEAT_END", ev, true);
}

/** vcmd: set ADSR/GAIN value. */
static void nintSpcEventSetADSRGAIN (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set ADSR/GAIN, ADSR(1) = $%02X, ADSR(2) = $%02X, GAIN = $%02X", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-adsrgain");
    nintSpcAddVcmdToMML(seq, "\n; VCMD_ADSR_AND_GAIN", ev, true);
}

/** vcmd: vibrato fade(-in). */
static void nintSpcEventVibratoFade (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato Fade, length = %d", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratofade");

    nintSpcAddVcmdToMML(seq, "VCMD_VIBRATO_FADE", ev, true);
}

/** vcmd: pitch envelope (to). */
static void nintSpcEventPitchEnvTo (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Envelope (To), delay = %d, length = %d, key = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvto");

    nintSpcAddVcmdToMML(seq, "VCMD_PITCHENV_TO", ev, true);
}

/** vcmd: pitch envelope (from). */
static void nintSpcEventPitchEnvFrom (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Envelope (From), delay = %d, length = %d, key = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvfrom");

    nintSpcAddVcmdToMML(seq, "VCMD_PITCHENV_FROM", ev, true);
}

/** vcmd: pitch envelope off. */
static void nintSpcEventPitchEnvOff (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Pitch Envelope Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvoff");

    nintSpcAddVcmdToMML(seq, "VCMD_PITCHENV_OFF", ev, true);
}

/** vcmd: tuning. */
static void nintSpcEventTuning (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tuning, amount = %d/256", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tuning");

    nintSpcAddVcmdToMML(seq, "VCMD_TUNING", ev, true);
}

/** vcmd: set echo vbits, volume. */
static void nintSpcEventEchoVol (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;
    int vbit;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Volume, EON = $%02X, EVOL(L) = %d, EVOL(R) = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echovol");

    for (vbit = 0; vbit < 8; vbit++) {
        if (arg1 & (1 << vbit))
            smfInsertControl(seq->smf, ev->tick, vbit, vbit, SMF_CONTROL_REVERB, 60);
        else
            smfInsertControl(seq->smf, ev->tick, vbit, vbit, SMF_CONTROL_REVERB, 0);
    }

    nintSpcAddVcmdToMML(seq, "VCMD_ECHO_ON", ev, true);
}

/** vcmd: disable echo. */
static void nintSpcEventEchoOff (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echooff");

    nintSpcAddVcmdToMML(seq, "VCMD_ECHO_OFF", ev, true);
}

/** vcmd: set echo delay, feedback, filter. */
static void nintSpcEventEchoParam (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, EDL = %d, EFB = %d, FIR# = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echoparam");

    nintSpcAddVcmdToMML(seq, "VCMD_ECHO_PARAM", ev, true);
}

/** vcmd: echo volume fade. */
static void nintSpcEventEchoVolFade (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Volume Fade, length = %d, to L = %d, to R = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echovolfade");

    nintSpcAddVcmdToMML(seq, "VCMD_ECHO_VOL_FADE", ev, true);
}

/* vcmd: pitch slide. */
static void nintSpcEventPitchSlide (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    getNoteNameFromVbyte(argDumpStr, arg3, seq->ver.patchFix[tr->note.patch].key);
    sprintf(ev->note, "Pitch Slide, delay = %d, length = %d, key = %s", arg1, arg2, argDumpStr);
    strcat(ev->classStr, " ev-pitchslide");

    ev->tick += arg1;
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    tr->tick += arg1 + arg2;

    nintSpcAddVcmdToMML(seq, "VCMD_PITCH_SLIDE", ev, true);
}

/** vcmd: set perc base. */
static void nintSpcEventSetPercBase (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    NintSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    if (seq->ver.percBaseIsNYI)
        sprintf(ev->note, "Perc Base (NYI), arg1 = %d", arg1);
    else
        sprintf(ev->note, "Perc Base, arg1 = %d", arg1);

    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-percbase");

    nintSpcAddVcmdToMML(seq, "; VCMD_PERC_PATCH_BASE", ev, true);
}

/** vcmd: skip 2 bytes. */
static void nintSpcEventSkip2 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    (*p) += 2;

    sprintf(ev->note, "Skip 2 Bytes");
    strcat(ev->classStr, " ev-skip2");
}

/** vcmd: short jump (forward only). */
static void nintSpcEventShortJumpU8 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p]; // unsigned
    (*p)++;

    (*p) += arg1;
    ev->size += arg1;

    sprintf(ev->note, "Short Jump, arg1 = %d", arg1);
    strcat(ev->classStr, " ev-shortjump");
}


/** vcmd: Fire Emblem 3 vcmd f5. */
static void nintSpcEventFE3F5 (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    if (arg1 < 0) {
        // 
    }
    else {
        if (arg1 & 8)
            seq->fe3ByteCA |= (1 << (arg1 & 7));
        else
            seq->fe3ByteCA &= ~(1 << (arg1 & 7));
    }

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: Fire Emblem 3 vcmd fa. */
static void nintSpcEventFE3FA (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    if (arg1 < 0) {
        ev->size += 6;
        (*p) += 6;
    }
    else {
        ev->size += (arg1 * 4);
        (*p) += (arg1 * 4);
    }

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: Fire Emblem 4 vcmd fa. */
static void nintSpcEventFE4FA (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    if (arg1 < 0) {
        // no more args? it is complicated...
    }
    else {
        ev->size += (arg1 * 4);
        (*p) += (arg1 * 4);
    }

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}


/** vcmd: Fire Emblem 4 vcmd fc. */
static void nintSpcEventFE4FC (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int paramSize;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    paramSize = ((arg1 & 15) + 1) * 3;
    ev->size += paramSize;
    (*p) += paramSize;

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: Fire Emblem 4 vcmd fd. */
static void nintSpcEventFE4FD (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    switch (arg1) {
      case 0x01:
        ev->size++;
        (*p)++;
        break;
      case 0x02:
        ev->size++;
        (*p)++;
        break;
    }

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: Tetris Attack vcmd fc. */
static void nintSpcEventTAFC (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int n;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    n = ((arg1 & 15) + 1);
    ev->size += n * 3;
    (*p) += n * 3;

    nintSpcEventUnknownInline(seq, ev);
    //sprintf(argDumpStr, ", arg1 = %d", arg1);
    //strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: Tetris Attack vcmd fd. */
static void nintSpcEventTAFD (NintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    switch (arg1) {
      case 0x00:
        ev->size += 3;
        (*p) += 3;
        break;
      case 0x01:
        ev->size++;
        (*p)++;
        break;
      case 0x02:
        ev->size++;
        (*p)++;
        break;
      case 0x03:
        break;
      case 0x04:
        break;
      case 0x05:
        ev->size++;
        (*p)++;
        break;
    }

    nintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** set pointers of each event. */
static void nintSpcSetEventList (NintSpcSeqStat *seq)
{
    int code;
    NintSpcEvent *event = seq->ver.event;
    int vcmdStart;
    int vcmdLensAddr;
    const byte *aRAM = seq->aRAM;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (NintSpcEvent) nintSpcEventUnidentified;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    event[0x00] = (NintSpcEvent) nintSpcEventEndOfBlock;
    for(code = seq->ver.noteInfoByteMin; code <= seq->ver.noteInfoByteMax; code++) {
        event[code] = (NintSpcEvent) nintSpcEventNoteInfo;
    }
    for(code = seq->ver.noteByteMin; code <= seq->ver.noteByteMax; code++) {
        event[code] = (NintSpcEvent) nintSpcEventNote;
    }
    event[seq->ver.tieByte]  = (NintSpcEvent) nintSpcEventTie;
    event[seq->ver.restByte] = (NintSpcEvent) nintSpcEventRest;
    for(code = seq->ver.percByteMin; code <= seq->ver.percByteMax; code++) {
        event[code] = (NintSpcEvent) nintSpcEventPercNote;
    }

    vcmdStart = seq->ver.vcmdByteMin;
    vcmdLensAddr = seq->ver.vcmdLensAddr;

    // set basic vcmds
    switch (seq->ver.id) {
    // old version
    case SPC_VER_OLD:
        //for(code = 0xc8; code <= 0xcf; code++) {
        //    event[code] = (NintSpcEvent) nintSpcEventRest;
        //}
        event[vcmdStart+0x00] = (NintSpcEvent) nintSpcEventSetPatch;
        event[vcmdStart+0x01] = (NintSpcEvent) nintSpcEventPanpot;
        event[vcmdStart+0x02] = (NintSpcEvent) nintSpcEventPanFade;
        event[vcmdStart+0x03] = (NintSpcEvent) nintSpcEventPitchSlide;
        event[vcmdStart+0x04] = (NintSpcEvent) nintSpcEventVibratoOn;
        event[vcmdStart+0x05] = (NintSpcEvent) nintSpcEventVibratoOff;
        event[vcmdStart+0x06] = (NintSpcEvent) nintSpcEventMasterVolume;
        event[vcmdStart+0x07] = (NintSpcEvent) nintSpcEventMasterVolFade;
        event[vcmdStart+0x08] = (NintSpcEvent) nintSpcEventTempo;
        event[vcmdStart+0x09] = (NintSpcEvent) nintSpcEventTempoFade;
        event[vcmdStart+0x0a] = (NintSpcEvent) nintSpcEventKeyShift;
        event[vcmdStart+0x0b] = (NintSpcEvent) nintSpcEventTremoloOn;
        event[vcmdStart+0x0c] = (NintSpcEvent) nintSpcEventTremoloOff;
        event[vcmdStart+0x0d] = (NintSpcEvent) nintSpcEventVolume;
        event[vcmdStart+0x0e] = (NintSpcEvent) nintSpcEventVolumeFade;
        event[vcmdStart+0x0f] = (NintSpcEvent) nintSpcEventSubroutine;
        event[vcmdStart+0x10] = (NintSpcEvent) nintSpcEventVibratoFade;
        event[vcmdStart+0x11] = (NintSpcEvent) nintSpcEventPitchEnvTo;
        event[vcmdStart+0x12] = (NintSpcEvent) nintSpcEventPitchEnvFrom;
        //event[vcmdStart+0x13] = (NintSpcEvent) nintSpcEventReserved;
        event[vcmdStart+0x14] = (NintSpcEvent) nintSpcEventTuning;
        event[vcmdStart+0x15] = (NintSpcEvent) nintSpcEventEchoVol;
        event[vcmdStart+0x16] = (NintSpcEvent) nintSpcEventEchoOff;
        event[vcmdStart+0x17] = (NintSpcEvent) nintSpcEventEchoParam;
        event[vcmdStart+0x18] = (NintSpcEvent) nintSpcEventEchoVolFade;
        //for(code = vcmdStart+0x19; code <= 0xff; code++) {
        //    event[code] = (NintSpcEvent) nintSpcEventReserved;
        //}
        break;
    default:
        event[vcmdStart+0x00] = (NintSpcEvent) nintSpcEventSetPatch;
        event[vcmdStart+0x01] = (NintSpcEvent) nintSpcEventPanpot;
        event[vcmdStart+0x02] = (NintSpcEvent) nintSpcEventPanFade;
        event[vcmdStart+0x03] = (NintSpcEvent) nintSpcEventVibratoOn;
        event[vcmdStart+0x04] = (NintSpcEvent) nintSpcEventVibratoOff;
        event[vcmdStart+0x05] = (NintSpcEvent) nintSpcEventMasterVolume;
        event[vcmdStart+0x06] = (NintSpcEvent) nintSpcEventMasterVolFade;
        event[vcmdStart+0x07] = (NintSpcEvent) nintSpcEventTempo;
        event[vcmdStart+0x08] = (NintSpcEvent) nintSpcEventTempoFade;
        event[vcmdStart+0x09] = (NintSpcEvent) nintSpcEventKeyShift;
        event[vcmdStart+0x0a] = (NintSpcEvent) nintSpcEventTranspose;
        event[vcmdStart+0x0b] = (NintSpcEvent) nintSpcEventTremoloOn;
        event[vcmdStart+0x0c] = (NintSpcEvent) nintSpcEventTremoloOff;
        event[vcmdStart+0x0d] = (NintSpcEvent) nintSpcEventVolume;
        event[vcmdStart+0x0e] = (NintSpcEvent) nintSpcEventVolumeFade;
        event[vcmdStart+0x0f] = (NintSpcEvent) nintSpcEventSubroutine;
        event[vcmdStart+0x10] = (NintSpcEvent) nintSpcEventVibratoFade;
        event[vcmdStart+0x11] = (NintSpcEvent) nintSpcEventPitchEnvTo;
        event[vcmdStart+0x12] = (NintSpcEvent) nintSpcEventPitchEnvFrom;
        event[vcmdStart+0x13] = (NintSpcEvent) nintSpcEventPitchEnvOff;
        event[vcmdStart+0x14] = (NintSpcEvent) nintSpcEventTuning;
        event[vcmdStart+0x15] = (NintSpcEvent) nintSpcEventEchoVol;
        event[vcmdStart+0x16] = (NintSpcEvent) nintSpcEventEchoOff;
        event[vcmdStart+0x17] = (NintSpcEvent) nintSpcEventEchoParam;
        event[vcmdStart+0x18] = (NintSpcEvent) nintSpcEventEchoVolFade;
        event[vcmdStart+0x19] = (NintSpcEvent) nintSpcEventPitchSlide;
        event[vcmdStart+0x1a] = (NintSpcEvent) nintSpcEventSetPercBase;
        //for(code = vcmdStart+0x1b; code <= 0xff; code++) {
        //    event[code] = (NintSpcEvent) nintSpcEventReserved;
        //}
    }

    // extra vcmds
    switch (seq->ver.id) {
    case SPC_VER_EXT1:
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventSkip2;
        event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1e] = (NintSpcEvent) nintSpcEventUnknown0;
        break;

    case SPC_VER_KONAMI:
        event[vcmdStart+0x04] = (NintSpcEvent) nintSpcEventUnknown2;
        event[vcmdStart+0x05] = (NintSpcEvent) nintSpcEventKonamiRepeatStart;
        event[vcmdStart+0x06] = (NintSpcEvent) nintSpcEventKonamiRepeatEnd;
        //event[vcmdStart+0x08] = (NintSpcEvent) nintSpcEventNOP;
        //event[vcmdStart+0x09] = (NintSpcEvent) nintSpcEventNOP;
        event[vcmdStart+0x15] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x16] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x17] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x18] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventSetADSRGAIN;
        event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventNOP;
        event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventNOP;
        event[vcmdStart+0x1e] = (NintSpcEvent) nintSpcEventNOP;
        break;

    case SPC_VER_YSFR:
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventUnknown1; // write APU port
        //event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventNOP;
        //event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventNOP;
        //event[vcmdStart+0x1e] = (NintSpcEvent) nintSpcEventNOP;
        //event[vcmdStart+0x1f] = (NintSpcEvent) nintSpcEventNOP;
        break;

    case SPC_VER_LEM:
        event[vcmdStart+0x05] = (NintSpcEvent) nintSpcEventUnknown1; // master volume NYI?
        event[vcmdStart+0x06] = (NintSpcEvent) nintSpcEventUnknown2; // master volume fade?
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventUnknown2; // nop
        event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1e] = (NintSpcEvent) nintSpcEventUnknown0;
        break;

    case SPC_VER_TA:
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventUnknown0; // vcmd f5
        event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventUnknown2;
        event[vcmdStart+0x1e] = (NintSpcEvent) nintSpcEventUnknown2;
        event[vcmdStart+0x1f] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x20] = (NintSpcEvent) nintSpcEventFE3FA;
        event[vcmdStart+0x21] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x22] = (NintSpcEvent) nintSpcEventFE4FC;
        event[vcmdStart+0x23] = (NintSpcEvent) nintSpcEventTAFD;
        break;

    case SPC_VER_FE3:
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventUnknown0; // vcmd f1
        event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1e] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1f] = (NintSpcEvent) nintSpcEventFE3F5;
        event[vcmdStart+0x20] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x21] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x22] = (NintSpcEvent) nintSpcEventShortJumpU8;
        event[vcmdStart+0x23] = (NintSpcEvent) nintSpcEventUnknown36;
        event[vcmdStart+0x24] = (NintSpcEvent) nintSpcEventFE3FA;
        event[vcmdStart+0x25] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x26] = (NintSpcEvent) nintSpcEventUnknown2;
        event[vcmdStart+0x27] = (NintSpcEvent) nintSpcEventUnknown2;
        break;

    case SPC_VER_FE4:
        event[vcmdStart+0x1b] = (NintSpcEvent) nintSpcEventUnknown0; // vcmd f5
        event[vcmdStart+0x1c] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x1d] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x1e] = event[vcmdStart+0x1d];
        event[vcmdStart+0x1f] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x20] = (NintSpcEvent) nintSpcEventFE4FA;
        event[vcmdStart+0x21] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x22] = (NintSpcEvent) nintSpcEventFE4FC;
        event[vcmdStart+0x23] = (NintSpcEvent) nintSpcEventFE4FD;
        break;
    }

    // autoguess
    if ((seq->ver.id == SPC_VER_STD_AT_LEAST || seq->ver.id == SPC_VER_STD_MODIFIED)
        && vcmdLensAddr >= 0 && vcmdStart == 0xe0) {
        int sizeOfs = aRAM[vcmdLensAddr+0x00] - 1;
        int vcmdIndex;
        int vcmdLen;
        int vcmdIndexMin = (seq->ver.id == SPC_VER_STD_AT_LEAST) ? 0x1b : 0;

        for (vcmdIndex = vcmdIndexMin; vcmdStart + vcmdIndex <= 0xff; vcmdIndex++)
        {
            int vcmdCode = vcmdStart+vcmdIndex;

            vcmdLen = aRAM[vcmdLensAddr+vcmdIndex] - sizeOfs;
            if (vcmdIndex < countof(NINT_STD_EVT_LEN_TABLE) && vcmdLen == NINT_STD_EVT_LEN_TABLE[vcmdIndex])
            {
                continue;
            }

            if (vcmdLen < 0 || vcmdLen > 8)
            {
                fprintf(stderr, "Warning: Event %02X cannot be supported.\n", vcmdCode);
                event[vcmdCode] = (NintSpcEvent) nintSpcEventUnidentified;
                continue;
            }

            switch (vcmdLen) {
                case 0: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown0; break;
                case 1: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown1; break;
                case 2: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown2; break;
                case 3: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown3; break;
                case 4: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown4; break;
                case 5: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown5; break;
                case 6: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown6; break;
                case 7: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown7; break;
                case 8: event[vcmdCode] = (NintSpcEvent) nintSpcEventUnknown8; break;
            }
            fprintf(stderr, "Warning: Event Length Mismatch! Replaced Event %02X (%d bytes)\n", vcmdCode, vcmdLen);
        }
    }
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* nintSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    NintSpcSeqStat *seq;
    Smf* smf = NULL;
    int mmlTemp;
    int track;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME, mycssfile);

    seq = newNintSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = nintSpcCreateSmf(seq);

    printHtmlInfoListMore(seq);

    myprintf("          </ul></li>\n");
    myprintf("        </ul>\n");
    myprintf("      </div>\n\n");

    myprintf("      <h2>Data Dump</h2>\n");
    myprintf("      <div class=\"section\" id=\"data-dump\">\n");
    myprintf("        <p>You can filter output by using stylesheet. Write %s as you like!</p>\n", mycssfile);

    printEventTableHeader(seq);

    while (seq->active && !abortFlag) {

        SeqEventReport ev;

        for (ev.track = 0; ev.track < SPC_TRACK_MAX; ev.track++) {

            NintSpcTrackStat *evtr = &seq->track[ev.track];

            while (seq->active && evtr->active && evtr->tick <= seq->tick) {

                bool inSub;
                byte nextVcmd;

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
                inSub = (evtr->loopCount > 0);
                strcat(ev.classStr, inSub ? " sub" : "");

                if (seq->endBlock
                    && ev.code != seq->ver.endBlockByte
                    && ev.code != seq->ver.pitchSlideByte)
                    break; // ???

                evtr->mmlWritten = false;
                evtr->used = true;

                // dispatch event
                seq->ver.event[ev.code](seq, &ev);
                memset(&seq->aRAMRef[ev.addr], 0xff, ev.size);

                nextVcmd = (ev.code != seq->ver.endBlockByte) ? aRAM[evtr->pos] : seq->ver.endBlockByte;
                if (nextVcmd != seq->ver.pitchSlideByte)
                    evtr->tick = evtr->nextTick;

                if (!seq->looped && !evtr->mmlWritten) {
                    for (mmlTemp = 0; mmlTemp < ev.size; mmlTemp++) {
                        sbprintf(evtr->mml, mmlTemp == 0 ? "" : " ");
                        sbprintf(evtr->mml, "$%02x", aRAM[ev.addr + mmlTemp]);
                    }
                    sbprintf(evtr->mml, "\n");
                }

                if (nintSpcTextLoopMax == 0 || max(seq->looped, seq->blockLooped) < nintSpcTextLoopMax) {
                    printHtmlEventDump(seq, &ev);
                }

                if (seq->endBlock &&
                    ev.code == seq->ver.endBlockByte &&
                    nextVcmd == seq->ver.endBlockByte)
                {
                    break; // prevent overrun
                }

                if (ev.unidentified && !nintSpcParseForce) {
                    abortFlag = true;
                    goto quitConversion;
                }
            }
        }

        // enter to new block, or quit, if needed
        if (seq->endBlock) {
            // rewind tracks to end point
            for (track = 0; track < SPC_TRACK_MAX; track++) {
                seq->track[track].tick = seq->tick;
                seq->track[track].nextTick = seq->tick;
                smfSetEndTimingOfTrack(seq->smf, track, seq->tick);
            }
            if (nintSpcReadNewBlock(seq)) {
                // put new table
                if (nintSpcTextLoopMax == 0 || max(seq->looped, seq->blockLooped) < nintSpcTextLoopMax) {
                    printEventTableFooter(seq);
                    printEventTableHeader(seq);
                }

                // exception for rest byte
                nintSpcTruncateNoteAll(seq);
                nintSpcDequeueNoteAll(seq); // FIXME: not true
            }
        }
        else {
            nintSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= nintSpcTimeLimit) {
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    nintSpcTruncateNoteAll(seq);
    nintSpcDequeueNoteAll(seq);

    printMML(seq);

    printEventTableFooter(seq);
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
        if (nintSpcARAMRefLog)
            fwrite(seq->aRAMRef, SPC_ARAM_SIZE, 1, nintSpcARAMRefLog);
        delNintSpcSeq(&seq);
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
Smf* nintSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = nintSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* nintSpcToMidiFromFile (const char *filename)
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

    smf = nintSpcToMidi(data, size);

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

static char spcBasePath[PATH_MAX] = { '\0' };
static char midBasePath[PATH_MAX] = { '\0' };
static char htmlBasePath[PATH_MAX] = { '\0' };
static char mmlBasePath[PATH_MAX] = { '\0' };
static char refBasePath[PATH_MAX] = { '\0' };

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
static bool cmdOptCount (void);
static bool cmdOptSong (void);
static bool cmdOptNoPort (void);
static bool cmdOptForce (void);
static bool cmdOptSongList (void);
static bool cmdOptBlockPtr (void);
static bool cmdOptDurTbl (void);
static bool cmdOptVelTbl (void);
static bool cmdOptLoop (void);
static bool cmdOptVolLinear (void);
static bool cmdOptBendRange (void);
static bool cmdOptPatchFix (void);
static bool cmdOptGS (void);
static bool cmdOptXG (void);
static bool cmdOptGM1 (void);
static bool cmdOptGM2 (void);
static bool cmdOptMML (void);
static bool cmdOptMMLAbs (void);
static bool cmdOptNoqFix (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "count", '\0', 1, cmdOptCount, "<n>", "convert n songs continuously" },
    { "song", '\0', 1, cmdOptSong, "<index>", "force set song index" },
    { "np", '\0', 0, cmdOptNoPort, "", "disable reading song index from APU port" },
    { "force", 'f', 0, cmdOptForce, "", "force parse song even if unidentified event appears" },
    { "songlist", '\0', 1, cmdOptSongList, "<addr>", "force set song (list) address (advanced)" },
    { "blockptr", '\0', 1, cmdOptBlockPtr, "<addr>", "specify block pointer address (advanced)" },
    { "durtbl", '\0', 1, cmdOptDurTbl, "<addr>", "specify duration table address (advanced)" },
    { "veltbl", '\0', 1, cmdOptVelTbl, "<addr>", "specify velocity table address (advanced)" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "linear", '\0', 0, cmdOptVolLinear, "", "assume midi volume is linear" },
    { "bendrange", '\0', 1, cmdOptBendRange, "<N>", "pitch bend sensitivity (0:auto)" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm1", '\0', 0, cmdOptGM1, "", "Insert GM1 System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
    { NULL, '\0', 0, NULL, NULL, NULL },
    { "mml", '\0', 1, cmdOptMML, "<filename>", "Output mml log for addmusic (incomplete, not so smart)" },
    { "mmlabs", '\0', 0, cmdOptMMLAbs, "", "Express note length by tick count" },
    { "noqf", '\0', 0, cmdOptNoqFix, "", "No 'q' curve conversion for MML" },
};

//----

/** display how to use. */
void man (void)
{
    const char *cmdname = APPSHORTNAME;
    int op;

    fprintf(stderr, "%s - %s %s\n", APPSHORTNAME, APPNAME, VERSION);
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
    fprintf(stderr, "%s\n", WEBSITE);

    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    for (op = 0; op < countof(optDef); op++) {
        if (optDef[op].description) {
            if (optDef[op].dispatch) {
                fprintf(stderr, " %s%c  %s%-9s  %-14s  %s\n",
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
    fprintf(stderr, "Programmed by %s - %s\n", AUTHOR, WEBSITE);
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
}

//----

/** show usage. */
static bool cmdOptHelp (void)
{
    man();
    return true;
}

/** set loop song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    nintSpcSetSongIndex(songIndex);
    return true;
}

/** set number of songs to convert. */
static bool cmdOptCount (void)
{
    int count = strtol(gArgv[0], NULL, 0);
    nintSpcContConvNum = count;
    return true;
}

/** don't read song index from APU port. */
static bool cmdOptNoPort (void)
{
    nintSpcSetSongFromPort(false);
    return true;
}

/** force analyze unidentified event. */
static bool cmdOptForce (void)
{
    nintSpcParseForce = true;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    nintSpcForceSongListAddr = songListAddr;
    return true;
}

/** set block ptr address. */
static bool cmdOptBlockPtr (void)
{
    int blockPtrAddr = strtol(gArgv[0], NULL, 16);
    nintSpcForceBlockPtrAddr = blockPtrAddr;
    return true;
}

/** set duration table address. */
static bool cmdOptDurTbl (void)
{
    int tableAddr = strtol(gArgv[0], NULL, 16);
    nintSpcForceDurTableAddr = tableAddr;
    return true;
}

/** set velocity table address. */
static bool cmdOptVelTbl (void)
{
    int tableAddr = strtol(gArgv[0], NULL, 16);
    nintSpcForceVelTableAddr = tableAddr;
    return true;
}

/** set loop count. */
static bool cmdOptLoop (void)
{
    int loopCount = strtol(gArgv[0], NULL, 0);
    nintSpcSetLoopCount(loopCount);
    return true;
}

/** set linear midi volume conversion. */
static bool cmdOptVolLinear (void)
{
    nintSpcVolIsLinear = true;
    return true;
}

/** set midi bendrange. */
static bool cmdOptBendRange (void)
{
    nintSpcPitchBendSens = strtol(gArgv[0], NULL, 0);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (nintSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    nintSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    nintSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    nintSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    nintSpcMidiResetType = SMF_RESET_GM2;
    return true;
}

/** enable mml logging. */
static bool cmdOptMML (void)
{
    strcpy(mmlBasePath, gArgv[0]);
    return true;
}

/** disable convert tick to note conversion. */
static bool cmdOptMMLAbs (void)
{
    nintSpcMMLAbsTick = true;
    return true;
}

/** disable mml q curve fix. */
static bool cmdOptNoqFix (void)
{
    nintSpcAutoQFix = false;
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
    bool result;
    char tmpPath[PATH_MAX];
    char spcPath[PATH_MAX];
    char midPath[PATH_MAX];
    char htmlPath[PATH_MAX];
    char mmlPath[PATH_MAX];
    char refPath[PATH_MAX];

    // handle options
    gArgc = argc - 1;
    gArgv = argv + 1;
    result = handleCmdLineOpts();

    // too few or much args
    if (gArgc < 2 || gArgc > 4 || !result) {
        if (!manDisplayed) {
            about();
            fprintf(stderr, "Run with --help, for more details.\n");
            return (argc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        else
            return EXIT_SUCCESS;
    }

    strcpy(spcBasePath, gArgv[0]);
    strcpy(midBasePath, gArgv[1]);
    strcpy(htmlBasePath, (gArgc >= 3) ? gArgv[2] : "");
    strcpy(refBasePath, (gArgc >= 4) ? gArgv[3] : "");

    // convert input file
    for (nintSpcContConvCnt = 0; nintSpcContConvCnt < nintSpcContConvNum; nintSpcContConvCnt++) {
        strcpy(spcPath, spcBasePath);
        strcpy(midPath, midBasePath);
        strcpy(htmlPath, htmlBasePath);
        strcpy(mmlPath, mmlBasePath);
        strcpy(refPath, refBasePath);
        if (nintSpcContConvCnt) {
            sprintf(tmpPath, "%s-%03d.mid", removeExt(midPath), nintSpcContConvCnt + 1);
            strcpy(midPath, tmpPath);
            if (htmlPath[0] != '\0') {
                sprintf(tmpPath, "%s-%03d.html", removeExt(htmlPath), nintSpcContConvCnt + 1);
                strcpy(htmlPath, tmpPath);
            }
            if (mmlPath[0] != '\0') {
                sprintf(tmpPath, "%s-%03d.mml", removeExt(mmlPath), nintSpcContConvCnt + 1);
                strcpy(mmlPath, tmpPath);
            }
            if (refPath[0] != '\0') {
                sprintf(tmpPath, "%s-%03d.ref", removeExt(refPath), nintSpcContConvCnt + 1);
                strcpy(refPath, tmpPath);
            }
        }

        // set html handle if needed
        htmlFile = (htmlPath[0] != '\0') ? fopen(htmlPath, "w") : NULL;
        nintSpcSetLogStreamHandle(htmlFile);
        // set mml handle if needed
        nintSpcMMLLog = (mmlPath[0] != '\0') ? fopen(mmlPath, "w") : NULL;
        // set aram ref log if needed
        nintSpcARAMRefLog = (refPath[0] != '\0') ? fopen(refPath, "wb") : NULL;

        fprintf(stderr, "%s", spcPath);
        if (nintSpcContConvCnt)
            fprintf(stderr, "(%d)", nintSpcContConvCnt + 1);
        fprintf(stderr, ":\n");

        smf = nintSpcToMidiFromFile(spcPath);
        // then output result
        if (smf != NULL) {
            smfWriteFile(smf, midPath);
        }
        else {
            fprintf(stderr, "Error: Conversion failed.\n");
            result = false;
        }

        if (htmlFile != NULL) {
            fclose(htmlFile);
            htmlFile = NULL;
        }
        if (nintSpcMMLLog != NULL) {
            fclose(nintSpcMMLLog);
            nintSpcMMLLog = NULL;
        }
        if (nintSpcARAMRefLog != NULL) {
            fclose(nintSpcARAMRefLog);
            nintSpcARAMRefLog = NULL;
        }
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
