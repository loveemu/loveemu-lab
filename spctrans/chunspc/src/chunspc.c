/**
 * Chunsoft spc2midi.
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
#include "chunspc.h"

#define APPNAME "Chunsoft SPC2MIDI"
#define APPSHORTNAME "chunspc"
#define VERSION "[2014-02-15]"

static int chunSpcLoopMax = 2;            // maximum loop count of parser
static int chunSpcTextLoopMax = 1;        // maximum loop count of text output
static double chunSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool chunSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int chunSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool chunSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int chunSpcTimeBase = 48;
static int chunSpcForceSongIndex = -1;
static int chunSpcForceSongListAddr = -1;

static bool chunSpcPatchFixOverride = false;
static PatchFixInfo chunSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int chunSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_OTOGIRISOU,
    SPC_VER_DQ5,
    SPC_VER_TORNECO,
    SPC_VER_KAMAITACHI,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   23
#define SPC_ARAM_SIZE       0x10000

typedef struct TagChunSpcTrackStat ChunSpcTrackStat;
typedef struct TagChunSpcSeqStat ChunSpcSeqStat;
typedef void (*ChunSpcEvent) (ChunSpcSeqStat *, SeqEventReport *);

typedef struct TagChunSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int songIndexAddr;
    int seqHeaderAddr;
    int vcmdTableAddr;
    int songTableItemLen;
    int songTableMaxCount;
    int songTableSeqAddrOfs;
    int baseTempoVarAddr;
    int cpuControledVarAddr;
    ChunSpcEvent event[256];
    PatchFixInfo patchFix[256];
    bool seqDetected;
} ChunSpcVerInfo;

typedef struct TagChunSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} ChunSpcNoteParam;

struct TagChunSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    ChunSpcNoteParam note;     // current note param
    ChunSpcNoteParam lastNote; // note params for last note
    int looped;             // how many times looped (internal)
    int patch;              // patch number (for pitch fix)
    int noteLen;            // length of note (tick)
    int durRate;            // duration rate (0-255)
    int subRetnAddr[0x100]; // return address of subroutine vcmd
    int subNestLevel;       // current nest level of subroutine vcmd
    int subNestLevelMax;    // max nest level allowed of subroutine vcmd
    int volume;             // current volume
    int panpot;             // current panpot
    int expression;         // current expression (subvolume)
    int transpose;          // per-voice transpose
    int loopCount;          // repeat count for loop command
    int loopCountAlt;       // repeat count for alternative loop command
    bool refNoteLen;        // refer note length from prior channel
    int pitchBendSensMax;   // limit of pitch slide for MIDI output
    int lastPitchSlideTick; // indicates whether write bend=0 at note on
};

struct TagChunSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int baseTempo;              // base tempo
    int numOfTracks;            // number of tracks
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    byte cpuControledVar;       // cpu-controled jump interface
    ChunSpcVerInfo ver;         // game version info
    ChunSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void chunSpcSetEventList (ChunSpcSeqStat *seq);

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
FILE *chunSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int chunSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = chunSpcLoopMax;
    chunSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool chunSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        chunSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        chunSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            chunSpcPatchFix[patch].bankSelM = patch >> 7;
            chunSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            chunSpcPatchFix[patch].bankSelM = 0;
            chunSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        chunSpcPatchFix[patch].patchNo = patch & 0x7f;
        chunSpcPatchFix[patch].key = 0;
        chunSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        chunSpcPatchFix[src].bankSelM = bankM & 0x7f;
        chunSpcPatchFix[src].bankSelL = bankL & 0x7f;
        chunSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        chunSpcPatchFix[src].key = key;
        chunSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    chunSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *chunSpcVerToStrHtml (ChunSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_OTOGIRISOU:
        return "summer OTOGIRISOU version $Revision: 2.0.1.3";
    case SPC_VER_DQ5:
        return "winter DQ5 version $Revision: 1.44";
    case SPC_VER_TORNECO:
        return "winter F version $Revision: 2.3";
    case SPC_VER_KAMAITACHI:
        return "winter SN2 version $Revision: 3.32";
    default:
        return "Unknown Version / Unsupported";
    }
}

/* return if the engine version is summer */
static bool chunSpcIsVersionSummer(ChunSpcSeqStat *seq)
{
    return (seq->ver.id == SPC_VER_OTOGIRISOU);
}

/** reset for each track. */
static void chunSpcResetTrackParam (ChunSpcSeqStat *seq, int track)
{
    ChunSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->noteLen = 1;
    tr->durRate = 0xcc;
    tr->refNoteLen = false;
    tr->patch = 0;
    tr->volume = 0x60;
    tr->panpot = 0;
    tr->expression = 0x80;
    tr->subNestLevel = 0;
    tr->subNestLevelMax = 3;
    tr->loopCount = 0;
    tr->loopCountAlt = 0;
    tr->pitchBendSensMax = (chunSpcPitchBendSens == 0) ? SMF_PITCHBENDSENS_DEFAULT : chunSpcPitchBendSens;
    tr->lastPitchSlideTick = INT_MAX;
}

/** reset before play/convert song. */
static void chunSpcResetParam (ChunSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = seq->baseTempo = 120; // dummy, just in case
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;
    if (seq->ver.cpuControledVarAddr != 0)
    {
        seq->cpuControledVar = seq->aRAM[seq->ver.cpuControledVarAddr];
    }
    else
    {
        seq->cpuControledVar = 0;
    }

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        ChunSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        chunSpcResetTrackParam(seq, track);
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
    if (chunSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &chunSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int chunSpcCheckVer (ChunSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int songLdCodeAddr;
    int cpuCondJumpCodeAddr;
    int vcmdExecCodeAddr;
    int songTableReadPtr;
    int songFirstAddrDiffBest;
    //int songTempoDiffBest;
    int songIndex;

    seq->timebase = chunSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.songIndexAddr = -1;
    seq->ver.songTableItemLen = 6;
    seq->ver.songTableMaxCount = 32; // just a random choice
    seq->ver.songTableSeqAddrOfs = 1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.vcmdTableAddr = -1;
    seq->ver.baseTempoVarAddr = -1;
    seq->ver.cpuControledVarAddr = -1;
    seq->ver.seqDetected = false;

    // (Kamaitachi no Yoru)
    // mov   $0409,x
    // mov   y,a               ; Y = A = $03B7+X
    // mov   a,$04d8+y
    // mov   $a0,#$e6
    // mov   $a1,#$06
    // mov   y,#$06
    // mul   ya
    // addw  ya,$a0
    // movw  $a0,ya            ; $A0/1 = #$06E6 + ($04D8+Y * 6)
    // mov   y,#$05
    // mov   a,($a0)+y
    // or    a,#$08
    // mov   ($a0)+y,a
    // mov   y,#$01
    // mov   a,($a0)+y
    // push  a
    // inc   y
    // mov   a,($a0)+y
    // mov   $a1,a
    // pop   a
    // mov   $a0,a             ; $A0/1 = (WORD) [$A0]+1 (sequence header)
    // mov   a,#$00
    // mov   $03ef+x,a
    // mov   $03d7+x,a
    // mov   $03f7+x,a
    // mov   a,#$02
    // mov   $03e7+x,a
    // ; read the sequence header
    // mov   y,#$00
    // mov   a,($a0)+y         ; header+0: initial tempo
    // inc   y
    // mov   $03bf+x,a
    // mov   $03c7+x,a         ; base tempo
    if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xc9..\xfd\xf6..\x8f..\x8f..\x8d\x06\xcf\x7a.\xda.\x8d\x05\xf7.\x08\x08\xd7.\x8d\x01\xf7.\x2d\xfc\xf7.\xc4.\xae\xc4.\xe8\\\x00\xd5..\xd5..\xd5..\xe8\x02\xd5..\x8d\\\x00\xf7.\xfc\xd5..\xd5..", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[songLdCodeAddr + 9] + 1 == seq->aRAM[songLdCodeAddr + 12] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 17] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 19] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 23] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 27] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 31] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 35] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 40] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 60] &&
        seq->aRAM[songLdCodeAddr + 12] == seq->aRAM[songLdCodeAddr + 37])
    {
        seq->ver.seqListAddr = seq->aRAM[songLdCodeAddr + 8] | (seq->aRAM[songLdCodeAddr + 11] << 8);
        seq->ver.baseTempoVarAddr = mget2l(&seq->aRAM[songLdCodeAddr + 66]);
        version = SPC_VER_KAMAITACHI;
    }
    // (Dragon Quest 5)
    // mov   $03f8,x
    // mov   y,a               ; Y = A = $03AF+X
    // mov   a,$04c7+y
    // mov   $a0,#$d5
    // mov   $a1,#$05
    // mov   y,#$06
    // mul   ya
    // addw  ya,$a0
    // movw  $a0,ya            ; $A0/1 = #$5D5 + ($04C7+Y * 6)
    // mov   y,#$01
    // mov   a,($a0)+y
    // push  a
    // inc   y
    // mov   a,($a0)+y
    // mov   $a1,a
    // pop   a
    // mov   $a0,a             ; $A0/1 = (WORD) [$A0]+1 (sequence header)
    // mov   a,#$00
    // mov   $03df+x,a
    // mov   $03c7+x,a
    // mov   $03e7+x,a
    // mov   a,#$02
    // mov   $03d7+x,a
    // ; read the sequence header
    // mov   y,#$00
    // mov   a,($a0)+y         ; header+0: initial tempo
    // inc   y
    // mov   $03b7+x,a
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xc9..\xfd\xf6..\x8f..\x8f..\x8d\x06\xcf\x7a.\xda.\x8d\x01\xf7.\x2d\xfc\xf7.\xc4.\xae\xc4.\xe8\\\x00\xd5..\xd5..\xd5..\xe8\x02\xd5..\x8d\\\x00\xf7.\xfc\xd5..", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[songLdCodeAddr + 9] + 1 == seq->aRAM[songLdCodeAddr + 12] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 17] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 19] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 23] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 27] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 32] &&
        seq->aRAM[songLdCodeAddr + 9] == seq->aRAM[songLdCodeAddr + 52] &&
        seq->aRAM[songLdCodeAddr + 12] == seq->aRAM[songLdCodeAddr + 29])
    {
        seq->ver.seqListAddr = seq->aRAM[songLdCodeAddr + 8] | (seq->aRAM[songLdCodeAddr + 11] << 8);
        seq->ver.baseTempoVarAddr = mget2l(&seq->aRAM[songLdCodeAddr + 55]);
        version = SPC_VER_DQ5;
    }
    // (Otogirisou)
    // mov   $051d+x,a         ; $051D+X = A
    // mov   $0566,x
    // push  a
    // mov   a,$218f
    // mov   $c0,a
    // mov   a,$2190
    // mov   $c1,a             ; $C0/1 = $218F/90
    // pop   a
    // asl   a
    // bcc   $0ee1
    // inc   $c1
    // clrc
    // adc   a,$c0
    // mov   $c0,a             ; $C0/1 += ($051D+X * 2)
    // bcc   $0eea
    // inc   $c1
    // mov   y,#$00
    // mov   a,($c0)+y
    // push  a
    // inc   y
    // mov   a,($c0)+y
    // mov   $c1,a
    // pop   a
    // mov   $c0,a             ; $C0/1 = (WORD) [$C0] (sequence header)
    // or    a,$c1
    // beq   $0e95             ; no song
    // mov   a,#$ff
    // mov   $0515+x,a
    // mov   a,#$00
    // mov   $0545+x,a
    // mov   $054d+x,a
    // mov   $0535+x,a
    // mov   $0555+x,a
    // ; read the sequence header
    // mov   y,#$00
    // mov   a,($c0)+y         ; header+0: initial tempo
    // inc   y
    // mov   $0525+x,a
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xd5..\xc9..\x2d\xe5..\xc4.\xe5..\xc4.\xae\x1c\x90\x02\xab.\x60\x84.\xc4.\x90\x02\xab.\x8d\\\x00\xf7.\x2d\xfc\xf7.\xc4.\xae\xc4.\x04.\xf0.\xe8\xff\xd5..\xe8\\\x00\xd5..\xd5..\xd5..\xd5..\x8d\\\x00\xf7.\xfc\xd5..", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[songLdCodeAddr + 11] + 1 == seq->aRAM[songLdCodeAddr + 16] &&
        seq->aRAM[songLdCodeAddr + 11] == seq->aRAM[songLdCodeAddr + 25] &&
        seq->aRAM[songLdCodeAddr + 11] == seq->aRAM[songLdCodeAddr + 27] &&
        seq->aRAM[songLdCodeAddr + 11] == seq->aRAM[songLdCodeAddr + 35] &&
        seq->aRAM[songLdCodeAddr + 11] == seq->aRAM[songLdCodeAddr + 39] &&
        seq->aRAM[songLdCodeAddr + 11] == seq->aRAM[songLdCodeAddr + 44] &&
        seq->aRAM[songLdCodeAddr + 11] == seq->aRAM[songLdCodeAddr + 71] &&
        seq->aRAM[songLdCodeAddr + 16] == seq->aRAM[songLdCodeAddr + 22] &&
        seq->aRAM[songLdCodeAddr + 16] == seq->aRAM[songLdCodeAddr + 31] &&
        seq->aRAM[songLdCodeAddr + 16] == seq->aRAM[songLdCodeAddr + 41] &&
        seq->aRAM[songLdCodeAddr + 16] == seq->aRAM[songLdCodeAddr + 46] &&
        mget2l(&seq->aRAM[songLdCodeAddr + 8]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 13]))
    {
        int seqListAddrPtr = mget2l(&seq->aRAM[songLdCodeAddr + 8]);
        seq->ver.seqListAddr = mget2l(&seq->aRAM[seqListAddrPtr]);
        seq->ver.baseTempoVarAddr = mget2l(&seq->aRAM[songLdCodeAddr + 74]);
        version = SPC_VER_OTOGIRISOU;
    }

    // ; vcmd e0 - cpu-controled jump
    // call  $1c83
    // mov   $a0,a
    // call  $1c83
    // mov   $a1,a             ; set arg1/2 to $A0/1
    // mov   a,$02a3+x
    // mov   y,a
    // mov   a,$03f7+y
    // and   a,#$7f
    // mov   $a2,a
    // call  $1c83
    // cmp   a,$a2
    // bne   $1986
    // mov   a,$00+x
    // mov   y,$01+x
    // clrc
    // addw  ya,$a0            ; relative jump
    // mov   $00+x,a
    // mov   $01+x,y
    if ((cpuCondJumpCodeAddr = indexOfHexPat(seq->aRAM, "\x3f..\xc4.\x3f..\xc4.\xf5..\xfd\xf6..\x28\x7f\xc4.\x3f..\x64.\xd0.\xf4.\xfb.\x60\x7a.\xd4.\xdb.", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[cpuCondJumpCodeAddr + 4] + 1 == seq->aRAM[cpuCondJumpCodeAddr + 9] &&
        seq->aRAM[cpuCondJumpCodeAddr + 29] + 1 == seq->aRAM[cpuCondJumpCodeAddr + 31] &&
        seq->aRAM[cpuCondJumpCodeAddr + 4] == seq->aRAM[cpuCondJumpCodeAddr + 34] &&
        seq->aRAM[cpuCondJumpCodeAddr + 29] == seq->aRAM[cpuCondJumpCodeAddr + 36] &&
        seq->aRAM[cpuCondJumpCodeAddr + 31] == seq->aRAM[cpuCondJumpCodeAddr + 38])
    {
        seq->ver.cpuControledVarAddr = mget2l(&seq->aRAM[cpuCondJumpCodeAddr + 15]);
    }

    // push  x
    // asl   a
    // mov   x,a
    // mov   a,$07af+x
    // mov   $122e+1,a
    // mov   a,$07b0+x
    // mov   $122e+2,a         ; overwrite call addr
    // pop   x
    // call  $xxxx             ; do vcmd
    if ((vcmdExecCodeAddr = indexOfHexPat(seq->aRAM, "\x4d\x1c\\\x5d\xf5..\xc5..\xf5..\xc5..\xce\x3f..", SPC_ARAM_SIZE, NULL)) != -1 &&
        mget2l(&seq->aRAM[vcmdExecCodeAddr + 4]) + 1 == mget2l(&seq->aRAM[vcmdExecCodeAddr + 10]) &&
        mget2l(&seq->aRAM[vcmdExecCodeAddr + 7]) + 1 == mget2l(&seq->aRAM[vcmdExecCodeAddr + 13]) &&
        mget2l(&seq->aRAM[vcmdExecCodeAddr + 7]) == (vcmdExecCodeAddr + 17))
    {
        seq->ver.vcmdTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 4]);
    }

    // version detection by RCS string
    if (indexOfHexPat(seq->aRAM, "\\\x00ummer OTOGIRISOU version $Revision: 2.0.1.3 $l", SPC_ARAM_SIZE, NULL) != -1)
    {
        version = SPC_VER_OTOGIRISOU;
        seq->ver.songIndexAddr = 0x051d;
    }
    else if (indexOfHexPat(seq->aRAM, "winter DQ5 version $Revision: 1.44 $l", SPC_ARAM_SIZE, NULL) != -1)
    {
        version = SPC_VER_DQ5;
    }
    else if (indexOfHexPat(seq->aRAM, "winter F version $Revision: 2.3 $l", SPC_ARAM_SIZE, NULL) != -1)
    {
        version = SPC_VER_TORNECO;
    }
    else if (indexOfHexPat(seq->aRAM, "winter SN2 version $Revision: 3.32 $l", SPC_ARAM_SIZE, NULL) != -1)
    {
        version = SPC_VER_KAMAITACHI;
    }

    // summer has somewhat different table design from winter
    if (version == SPC_VER_OTOGIRISOU)
    {
        seq->ver.songTableItemLen = 2;
        seq->ver.songTableSeqAddrOfs = 0;
    }

    // overwrite song table address if needed
    if (chunSpcForceSongListAddr != -1)
    {
        seq->ver.seqListAddr = chunSpcForceSongListAddr;
    }

    // set song index
    if (chunSpcForceSongIndex != -1)
    {
        seq->ver.songIndex = chunSpcForceSongIndex;
    }
    else if (seq->ver.songIndexAddr != -1 && seq->aRAM[seq->ver.songIndexAddr] < seq->ver.songTableMaxCount)
    {
        seq->ver.songIndex = seq->aRAM[seq->ver.songIndexAddr];
    }
    else
    {
        seq->ver.songIndex = 0; // default

        if (seq->ver.seqListAddr != -1
#if 1
            && !(seq->ver.songIndexAddr != -1 && seq->aRAM[seq->ver.songIndexAddr] >= seq->ver.songTableMaxCount)
#endif
            )
        {
            int aramFirstAddr = mget2l(&seq->aRAM[0x0000]);

            // auto-detect current song
            if (aramFirstAddr != 0)
            {
                songTableReadPtr = seq->ver.seqListAddr;
                songFirstAddrDiffBest = INT_MAX;
                for (songIndex = 0; songIndex < seq->ver.songTableMaxCount; songIndex++)
                {
                    int seqHeaderAddr = mget2l(&seq->aRAM[songTableReadPtr + seq->ver.songTableSeqAddrOfs]);
                    int firstAddr = mget2l(&seq->aRAM[seqHeaderAddr + 2]) + (chunSpcIsVersionSummer(seq) ? 0 : seqHeaderAddr);
                    int firstAddrDiff = aramFirstAddr - firstAddr;
                    int numOfTracks = seq->aRAM[seqHeaderAddr + 1];

                    if (seqHeaderAddr != 0 && numOfTracks > 0 && numOfTracks <= SPC_TRACK_MAX)
                    {
                        if (firstAddrDiff >= 0 && firstAddrDiff <= songFirstAddrDiffBest)
                        {
                            songFirstAddrDiffBest = firstAddrDiff;
                            seq->ver.songIndex = songIndex;
                        }
                    }

                    songTableReadPtr += seq->ver.songTableItemLen;
                    if (songTableReadPtr + seq->ver.songTableItemLen > SPC_ARAM_SIZE)
                    {
                        break;
                    }
                }
            }
//            else
//            {
//                // search by tempo, for certain Otogirisou SPCs...
//                songTableReadPtr = seq->ver.seqListAddr;
//                songTempoDiffBest = INT_MAX;
//                for (songIndex = 0; songIndex < seq->ver.songTableMaxCount; songIndex++)
//                {
//                    int seqHeaderAddr = mget2l(&seq->aRAM[songTableReadPtr + seq->ver.songTableSeqAddrOfs]);
//                    int numOfTracks = seq->aRAM[seqHeaderAddr + 1];
//
//                    if (seqHeaderAddr != 0 && numOfTracks > 0 && numOfTracks <= SPC_TRACK_MAX)
//                    {
//                        if (seq->ver.baseTempoVarAddr != -1 && seq->aRAM[seq->ver.baseTempoVarAddr] != 0)
//                        {
//                            int tempoDiff = abs(seq->aRAM[seq->ver.baseTempoVarAddr] - seq->aRAM[seqHeaderAddr]);
//                            if (tempoDiff <= songTempoDiffBest)
//                            {
//                                songTempoDiffBest = tempoDiff;
//                                seq->ver.songIndex = songIndex;
//                            }
//                        }
//                        else
//                        {
//                            if (seq->aRAM[seqHeaderAddr] != 0 && mget2l(&seq->aRAM[seqHeaderAddr + 2]) != 0)
//                            {
//                                seq->ver.songIndex = songIndex;
//                                break;
//                            }
//                        }
//                    }
//
//                    songTableReadPtr += seq->ver.songTableItemLen;
//                    if (songTableReadPtr + seq->ver.songTableItemLen > SPC_ARAM_SIZE)
//                    {
//                        break;
//                    }
//                }
//            }
        }
    }

    // finally, determine the song header address
    if (seq->ver.seqListAddr != -1 && seq->ver.songIndex != -1)
    {
        seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[seq->ver.seqListAddr + seq->ver.songIndex * seq->ver.songTableItemLen + seq->ver.songTableSeqAddrOfs]);
    }

    seq->ver.id = version;
    chunSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool chunSpcDetectSeq (ChunSpcSeqStat *seq)
{
    bool result = true;
    int seqHeaderReadOfs;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN || seq->ver.seqHeaderAddr == -1)
        return false;

    chunSpcResetParam(seq);

    seqHeaderReadOfs = seq->ver.seqHeaderAddr;
    seq->tempo = seq->baseTempo = seq->aRAM[seqHeaderReadOfs];
    seqHeaderReadOfs++;
    seq->numOfTracks = seq->aRAM[seqHeaderReadOfs];
    seqHeaderReadOfs++;
    if (seq->numOfTracks == 0)
    {
        fprintf(stderr, "Error: No tracks available\n");
        return false;
    }
    else if (seq->numOfTracks > SPC_TRACK_MAX)
    {
        fprintf(stderr, "Error: Too many tracks [%d]\n", seq->numOfTracks);
        return false;
    }

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].active = false;
    }

    // track list
    result = true;
    for (tr = 0; tr < seq->numOfTracks; tr++) {
        int trackAddr = mget2l(&seq->aRAM[seqHeaderReadOfs]);
        if (seq->ver.id >= SPC_VER_DQ5)
        {
            // offset -> absolute address
            trackAddr += seq->ver.seqHeaderAddr;
        }
        seqHeaderReadOfs += 2;

        seq->track[tr].pos = trackAddr;
        seq->track[tr].active = true;
    }

    return result;
}

/** create new spc2mid object. */
static ChunSpcSeqStat *newChunSpcSeq (const byte *aRAM)
{
    ChunSpcSeqStat *newSeq = (ChunSpcSeqStat *) calloc(1, sizeof(ChunSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        chunSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = chunSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delChunSpcSeq (ChunSpcSeqStat **seq)
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
static void printHtmlInfoList (ChunSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", chunSpcVerToStrHtml(seq));
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    myprintf(" (from ($%02x*%d+%d))", seq->ver.songIndex, seq->ver.songTableItemLen, seq->ver.songTableSeqAddrOfs);
    myprintf("</li>\n");
    myprintf("          <li>Voice Command Dispatch Table: $%04X</li>\n", seq->ver.vcmdTableAddr);
    if (seq->ver.songIndexAddr != -1)
    {
        myprintf("          <li>Current Song Index Variable: $%04X</li>\n", seq->ver.songIndexAddr);
    }
    myprintf("          <li>CPU-controled Jump Variable: $%04X</li>\n", seq->ver.cpuControledVarAddr);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (ChunSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (ChunSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (ChunSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (ChunSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double chunSpcTempoOf (ChunSpcSeqStat *seq, int tempoValue)
{
    return (double) seq->tempo;
}

/** convert SPC tempo into bpm. */
static double chunSpcTempo (ChunSpcSeqStat *seq)
{
    return chunSpcTempoOf(seq, seq->tempo);
}

/** convert SPC velocity into MIDI one. */
static int chunSpcMidiVelOf (int value)
{
    if (chunSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int chunSpcMidiVolOf (int value)
{
    if (chunSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel panpot into MIDI one. */
static int chunSpcMidiPanOf (int value)
{
    return (value+0x80)/2; // linear (TODO: sine curve)
}

/** create new smf object and link to spc seq. */
static Smf *chunSpcCreateSmf (ChunSpcSeqStat *seq)
{
    static char songTitle[512];
    Smf* smf;
    int tr;

    smf = smfCreate(seq->timebase);
    if (!smf)
        return NULL;
    seq->smf = smf;

    sprintf(songTitle, "%s %s", APPNAME, VERSION);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, songTitle);

    switch (chunSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, chunSpcTempo(seq));

    // put track name first
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].pos);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, chunSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_EXPRESSION, chunSpcMidiVolOf(seq->track[tr].expression));
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
        if (chunSpcPitchBendSens != 0) {
            smfInsertPitchBendSensitivity(smf, 0, tr, tr, seq->track[tr].pitchBendSensMax);
        }
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void chunSpcTruncateNote (ChunSpcSeqStat *seq, int track)
{
    ChunSpcTrackStat *tr = &seq->track[track];

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
static void chunSpcTruncateNoteAll (ChunSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        chunSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool chunSpcDequeueNote (ChunSpcSeqStat *seq, int track)
{
    ChunSpcTrackStat *tr = &seq->track[track];
    ChunSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        dur = lastNote->dur;
        if (dur == 0)
            dur++;

        key = lastNote->key + lastNote->transpose
            + seq->ver.patchFix[tr->lastNote.patch].key
            + SPC_NOTE_KEYSHIFT;
        vel = lastNote->vel;
        if (vel == 0)
            vel++;

        result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void chunSpcDequeueNoteAll (ChunSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        chunSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void chunSpcInactiveTrack(ChunSpcSeqStat *seq, int track)
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
static void chunSpcAddTrackLoopCount(ChunSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (chunSpcLoopMax > 0) ? chunSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= chunSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void chunSpcSeqAdvTick(ChunSpcSeqStat *seq)
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
    seq->time += (double) 60 / chunSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void chunSpcEventUnknownInline (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
}

/** vcmds: unidentified event. */
static void chunSpcEventUnidentified (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    chunSpcEventUnknownInline(seq, ev);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void chunSpcEventUnknown0 (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    chunSpcEventUnknownInline(seq, ev);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void chunSpcEventUnknown1 (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void chunSpcEventUnknown2 (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void chunSpcEventUnknown3 (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void chunSpcEventUnknown4 (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 4;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void chunSpcEventUnknown5 (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

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

    chunSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void chunSpcEventNOP (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00-9f: note, rest, tie. */
static void chunSpcEventNote(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    bool rest;
    bool tie;
    bool slur = false;
    int key;
    int len;
    int dur;

    if (noteByte >= 0x50) {
        noteByte -= 0x50;

        ev->size++;
        tr->noteLen = seq->aRAM[*p];
        (*p)++;
    }

    rest = (noteByte == 0x00);
    tie = (noteByte == 0x4f);
    key = noteByte;
    len = tr->noteLen;

    // formula for duration is:
    //   dur = len * (durRate + 1) / 256
    // but there are a few of exceptions.
    //   durRate = 0   : full length (tie uses it)
    //   durRate = 254 : full length - 1 (tick)
    dur = len;
    if (tr->durRate == 254)
    {
        dur--;
    }
    else if (tr->durRate == 0)
    {
        slur = true;
    }
    else
    {
        dur = len * (tr->durRate + 1) / 256;
    }

    if (rest) {
        sprintf(ev->note, "Rest, len = %d", len);
        strcat(ev->classStr, " ev-rest");
    }
    else if (tie) {
        sprintf(ev->note, "Tie, len = %d", len);
        strcat(ev->classStr, " ev-tie");
    }
    else {
        getNoteName(ev->note, key + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key
            + SPC_NOTE_KEYSHIFT);
        sprintf(argDumpStr, ", len = %d, dur = %d", len, dur);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    if (tr->lastNote.active && tr->lastNote.tied &&
        !tie && !rest && (tr->lastNote.key != key))
    {
        if (!chunSpcLessTextInSMF)
           smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, "Slur");
    }

    // output old note first
    if (!tie)
    {
        chunSpcDequeueNote(seq, ev->track);
    }

    // set new note
    if (!rest) {
        if (tie) {
            tr->lastNote.dur += dur;
        }
        else {
            if (tr->lastPitchSlideTick < tr->tick)
            {
                smfInsertPitchBend(seq->smf, tr->tick, ev->track, ev->track, 0);
                tr->lastPitchSlideTick = INT_MAX;
            }

            tr->lastNote.tick = ev->tick;
            tr->lastNote.dur = dur;
            tr->lastNote.key = key;
            tr->lastNote.vel = 100;
            tr->lastNote.transpose = seq->transpose + tr->note.transpose;
            tr->lastNote.patch = tr->note.patch;
            tr->lastNote.active = true;
        }
        tr->lastNote.tied = slur;
    }
    tr->tick += len;
}

/** vcmd a0-b5: set duration rate from table. */
static void chunSpcEventDurFromTable(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int durIndex;

    const int durRateTable[] = {
        0x0d, 0x1a, 0x26, 0x33, 0x40, 0x4d, 0x5a, 0x66,
        0x73, 0x80, 0x8c, 0x99, 0xa6, 0xb3, 0xbf, 0xcc,
        0xd9, 0xe6, 0xf2, 0xfe, 0xff, 0x00
    };

    durIndex = ev->code - 0xa0;
    tr->durRate = durRateTable[durIndex];
    sprintf(ev->note, "Duration Rate From Table, index = %d, rate = %d/255%s", durIndex, tr->durRate,
        (tr->durRate == 0) ? " (tie/slur)" : ((tr->durRate == 254) ? " (full -1 tick)" : ""));
    strcat(ev->classStr, " ev-durrate");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e0: cpu-controled jump. */
static void chunSpcEventCPUControledJump(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int arg1;
    int arg2;
    int dest;
    bool doJump;

    ev->size += 3;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;
    arg2 = seq->aRAM[*p];
    (*p)++;

    doJump = false;
    if ((seq->cpuControledVar & 0x7f) == arg2)
    {
        doJump = true;
    }
    seq->cpuControledVar |= 0x80;

    // relative
    dest = *p + arg1;

    if (doJump)
    {
        // assumes backjump = loop
        if (dest < *p) {
            chunSpcAddTrackLoopCount(seq, ev->track, 1);
        }
        *p = dest;
    }

    sprintf(ev->note, "CPU-Controled Jump, dest = $%04X, jump = %s", dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-cpujump");

    if (!chunSpcLessTextInSMF)
       smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e6: set expression fade. */
static void chunSpcEventExpressionFade (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Expression Fade, vol = %d, step = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vol");

    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    if (arg2 == 0)
    {
        tr->expression = arg1;
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_EXPRESSION, chunSpcMidiVolOf(tr->expression));
    }
    else
    {
        valueFrom = tr->expression;
        valueTo = arg1;
        for (faderStep = 1; faderStep <= arg2; faderStep++)
        {
            faderPos = (double)faderStep / arg2;
            faderValue = (int)(valueTo * faderPos + valueFrom * (1.0 - faderPos)); // alphablend
            if (tr->expression != faderValue)
            {
                int lastMidiVal = chunSpcMidiVolOf(tr->expression);
                int currMidiVal = chunSpcMidiVolOf(faderValue);
                tr->expression = faderValue;
                if (currMidiVal != lastMidiVal)
                {
                    smfInsertControl(seq->smf, ev->tick + faderStep, ev->track, ev->track, SMF_CONTROL_EXPRESSION, currMidiVal);
                }
            }
        }
    }
}

/** vcmd e8: set panpot fade. */
static void chunSpcEventPanpotFade (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot Fade, pan = %d, step = %d", arg1, arg2);
    strcat(ev->classStr, " ev-pan");

    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    if (arg2 == 0)
    {
        tr->panpot = arg1;
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, chunSpcMidiPanOf(tr->panpot));
    }
    else
    {
        valueFrom = tr->panpot;
        valueTo = arg1;
        for (faderStep = 1; faderStep <= arg2; faderStep++)
        {
            faderPos = (double)faderStep / arg2;
            faderValue = (int)(valueTo * faderPos + valueFrom * (1.0 - faderPos)); // alphablend
            if (tr->panpot != faderValue)
            {
                int lastMidiVal = chunSpcMidiPanOf(tr->panpot);
                int currMidiVal = chunSpcMidiPanOf(faderValue);
                tr->panpot = faderValue;
                if (currMidiVal != lastMidiVal)
                {
                    smfInsertControl(seq->smf, ev->tick + faderStep, ev->track, ev->track, SMF_CONTROL_PANPOT, currMidiVal);
                }
            }
        }
    }
}

/** vcmd ea: jump. */
static void chunSpcEventJump(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int arg1;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;

    // relative
    dest = *p + arg1;

    // assumes backjump = loop
    // FIXME: not always true!
    // see Boss Battle of Dragon Quest 5
    if (dest < *p) {
        chunSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = dest;

    sprintf(ev->note, "Jump, dest = $%04X", dest);
    strcat(ev->classStr, " ev-jump");

    if (!chunSpcLessTextInSMF)
       smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd eb: set tempo. */
static void chunSpcEventTempo (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    if (seq->ver.id == SPC_VER_KAMAITACHI)
    {
        seq->tempo = seq->baseTempo * arg1 / 64;
        sprintf(ev->note, "Set Tempo, rate = %d/64, bpm = %.1f", arg1, chunSpcTempo(seq));
    }
    else
    {
        seq->tempo = arg1;
        sprintf(ev->note, "Set Tempo, bpm = %.1f", chunSpcTempo(seq));
    }
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, chunSpcTempo(seq));
}

/** vcmd ec: set duration rate. */
static void chunSpcEventDurRate(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int arg1;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->durRate = arg1;

    sprintf(ev->note, "Duration Rate, rate = %d/255%s", arg1,
        (tr->durRate == 0) ? " (tie/slur)" : ((tr->durRate == 254) ? " (full -1 tick)" : ""));
    strcat(ev->classStr, " ev-durrate");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f0: set instrument. */
static void chunSpcEventInstrument (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd f2: duration copy on. */
static void chunSpcEventCopyNoteLenOn(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];

    tr->refNoteLen = true;

    // refresh duration info promptly
    if (ev->track > 0) {
        tr->noteLen = seq->track[ev->track-1].noteLen;
        tr->durRate = seq->track[ev->track-1].durRate;
    }

    sprintf(ev->note, "Copy Note Length On");
    strcat(ev->classStr, " ev-copylenon");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f3: duration copy off. */
static void chunSpcEventCopyNoteLenOff(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];

    tr->refNoteLen = false;

    sprintf(ev->note, "Copy Note Length Off");
    strcat(ev->classStr, " ev-copylenoff");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd db: repeat break (alternative). */
static void chunSpcEventRepeatBreakAlt(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int arg1;
    int dest;
    bool doJump;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;

    dest = *p + arg1;

    doJump = (tr->loopCountAlt != 0);
    if (doJump)
    {
        *p = dest;
    }

    sprintf(ev->note, "Repeat Break (Alt), dest = $%04X, jump = %s", dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-loop");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dc: repeat again (alternative). */
static void chunSpcEventRepeatAgainAlt(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int times;
    int dest;
    bool doJump;

    ev->size += 2;
    times = 2;
    dest = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;
    dest += *p;

    if (tr->loopCountAlt == 0)
    {
        tr->loopCountAlt = times;
    }

    tr->loopCountAlt = (tr->loopCountAlt - 1) & 0xff;
    doJump = (tr->loopCountAlt != 0);

    if (doJump)
    {
        *p = dest;
    }

    sprintf(ev->note, "Repeat Again (Alt), dest = $%04X, jump = %s", dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-loop");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dd: set release rate. */
static void chunSpcEventSetRR (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int rr;

    ev->size++;
    rr = seq->aRAM[*p] & 0x1f;
    (*p)++;

    sprintf(ev->note, "Set Release Rate, RR = %d (%.1f%s)",
        rr, (spcSRTable[rr] >= 1) ? spcSRTable[rr] : spcSRTable[rr] * 1000, (spcSRTable[rr] >= 1) ? "s" : "ms");
    strcat(ev->classStr, " ev-adsr");

    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd de: set ADSR and release rate. */
static void chunSpcEventSetADSR_RR (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int adsr, ar, dr, sl, sr, rr;

    ev->size += 3;
    adsr = mget2b(&seq->aRAM[*p]);
    (*p) += 2;
    rr = seq->aRAM[*p] & 0x1f;
    (*p)++;

    ar = (adsr & 0x0f00) >> 8;
    dr = (adsr & 0x7000) >> 12;
    sl = (adsr & 0x00e0) >> 5;
    sr = (adsr & 0x001f);

    sprintf(ev->note, "Set ADSR + Release, ADSR1/2 = $%04X, AR = %d (%.1f%s), DR = %d (%.1f%s), SL = %d (%d/8), SR = %d (%.1f%s), RR = %d (%.1f%s)", adsr,
        ar, (spcARTable[ar] >= 1) ? spcARTable[ar] : spcARTable[ar] * 1000, (spcARTable[ar] >= 1) ? "s" : "ms",
        dr, (spcARTable[dr] >= 1) ? spcDRTable[dr] : spcDRTable[dr] * 1000, (spcDRTable[dr] >= 1) ? "s" : "ms",
        sl, sl + 1,
        sr, (spcSRTable[sr] >= 1) ? spcSRTable[sr] : spcSRTable[sr] * 1000, (spcSRTable[sr] >= 1) ? "s" : "ms",
        rr, (spcSRTable[rr] >= 1) ? spcSRTable[rr] : spcSRTable[rr] * 1000, (spcSRTable[rr] >= 1) ? "s" : "ms");
    strcat(ev->classStr, " ev-adsr");

    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ef: set ADSR. */
static void chunSpcEventSetADSR (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int adsr, ar, dr, sl, sr;

    ev->size += 2;
    adsr = mget2b(&seq->aRAM[*p]);
    (*p) += 2;

    ar = (adsr & 0x0f00) >> 8;
    dr = (adsr & 0x7000) >> 12;
    sl = (adsr & 0x00e0) >> 5;
    sr = (adsr & 0x001f);

    sprintf(ev->note, "Set ADSR, ADSR1/2 = $%04X, AR = %d (%.1f%s), DR = %d (%.1f%s), SL = %d (%d/8), SR = %d (%.1f%s)", adsr,
        ar, (spcARTable[ar] >= 1) ? spcARTable[ar] : spcARTable[ar] * 1000, (spcARTable[ar] >= 1) ? "s" : "ms",
        dr, (spcARTable[dr] >= 1) ? spcDRTable[dr] : spcDRTable[dr] * 1000, (spcDRTable[dr] >= 1) ? "s" : "ms",
        sl, sl + 1,
        sr, (spcSRTable[sr] >= 1) ? spcSRTable[sr] : spcSRTable[sr] * 1000, (spcSRTable[sr] >= 1) ? "s" : "ms");
    strcat(ev->classStr, " ev-adsr");

    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ed: set volume. */
static void chunSpcEventVolume (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!chunSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->volume = arg1;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, chunSpcMidiVolOf(tr->volume));
}

/** vcmd ee: set panpot. */
static void chunSpcEventPanpot (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Panpot, balance = %d", arg1);
    strcat(ev->classStr, " ev-pan");

    //if (!chunSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->panpot = arg1;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, chunSpcMidiPanOf(tr->panpot));
}

/** vcmd f4: repeat again. */
static void chunSpcEventRepeatAgain(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int times;
    int dest;
    bool doJump;

    ev->size += 2;
    times = 2;
    dest = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;
    dest += *p;

    if (tr->loopCount == 0)
    {
        tr->loopCount = times;
    }

    tr->loopCount = (tr->loopCount - 1) & 0xff;
    doJump = (tr->loopCount != 0);

    if (doJump)
    {
        *p = dest;
    }

    sprintf(ev->note, "Repeat Again, dest = $%04X, jump = %s", dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-loop");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f5: repeat until. */
static void chunSpcEventRepeatUntil(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int times;
    int dest;
    bool doJump;

    ev->size += 3;
    times = seq->aRAM[*p];
    (*p)++;
    dest = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;
    dest += *p;

    if (tr->loopCount == 0)
    {
        tr->loopCount = times;
    }

    tr->loopCount = (tr->loopCount - 1) & 0xff;
    doJump = (tr->loopCount != 0);

    if (doJump)
    {
        *p = dest;
    }

    sprintf(ev->note, "Repeat Until, times = %d, dest = $%04X, jump = %s", times, dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-loop");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f6: set expression. */
static void chunSpcEventExpression (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Expression, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!chunSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->expression = arg1;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_EXPRESSION, chunSpcMidiVolOf(tr->expression));
}

/** vcmd f8: call subroutine. */
static void chunSpcEventCallSubroutine(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int arg1;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;

    dest = *p + arg1;

    if (tr->subNestLevel < tr->subNestLevelMax) {
        tr->subRetnAddr[tr->subNestLevel] = *p;
        tr->subNestLevel++;
        *p = dest;
    }

    sprintf(ev->note, "Call, dest = $%04X", dest);
    strcat(ev->classStr, " ev-call");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f9: end subroutine. */
static void chunSpcEventEndSubroutine(ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    bool doJump = false;

    if (tr->subNestLevel > 0) {
        tr->subNestLevel--;
        *p = tr->subRetnAddr[tr->subNestLevel];
        doJump = true;
    }

    sprintf(ev->note, "Return");
    if (doJump) {
        sprintf(argDumpStr, ", dest = $%04X", *p);
        strcat(ev->note, argDumpStr);
    }
    else
    {
        //strcat(ev->note, " (ignored)");
    }
    strcat(ev->classStr, " ev-ret");

    //if (!chunSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fa: transpose (absolute). */
static void chunSpcEventTransposeAbs (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;

    sprintf(ev->note, "Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!chunSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fb: pitch slide. */
static void chunSpcEventPitchSlide (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int lastPitchBendSens;
    double valueFrom, valueTo;
    int faderStep, faderIntValue;
    int faderLastIntValue;
    double faderValue;
    double faderPos;

    ev->size += 2;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    if (chunSpcPitchBendSens == 0) {
        // try updating pitch bend range
        lastPitchBendSens = tr->pitchBendSensMax;
        tr->pitchBendSensMax = min(abs(arg1), SMF_PITCHBENDSENS_MAX);
        if (tr->pitchBendSensMax != lastPitchBendSens) {
            smfInsertPitchBendSensitivity(seq->smf, ev->tick, ev->track, ev->track, tr->pitchBendSensMax);
        }
    }
    // MIDI bend range check
    if (abs(arg1) > tr->pitchBendSensMax) {
        fprintf(stderr, "Warning: Pitch Slide out of range (%d) at $%04X, track %d, tick %d\n", arg1, ev->addr, ev->track, ev->tick);
    }

    sprintf(ev->note, "Pitch Slide, key += %d, step = %d", arg1, arg2);
    strcat(ev->classStr, " ev-pitchslide");

    //if (!chunSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    if (arg2 == 0)
    {
        faderIntValue = (int)((double)arg1 / tr->pitchBendSensMax * 8192);
        if (faderIntValue >= -8192 && faderIntValue <= 8191)
        {
            smfInsertPitchBend(seq->smf, tr->tick, ev->track, ev->track, faderIntValue);
        }
    }
    else
    {
        valueFrom = 0.0;
        valueTo = arg1;
        faderLastIntValue = 0;
        for (faderStep = 1; faderStep <= arg2; faderStep++)
        {
            faderPos = (double)faderStep / arg2;
            faderValue = valueTo * faderPos + valueFrom * (1.0 - faderPos); // alphablend
            faderIntValue = (int)(faderValue / tr->pitchBendSensMax * 8192);
            if (faderIntValue == 8192)
            {
                faderIntValue--;
            }
            if (faderIntValue != faderLastIntValue)
            {
                faderLastIntValue = faderIntValue;
                if (faderIntValue >= -8192 && faderIntValue <= 8191)
                {
                    smfInsertPitchBend(seq->smf, tr->tick + faderStep, ev->track, ev->track, faderIntValue);
                }
            }
        }
    }
    tr->lastPitchSlideTick = ev->tick;
}

/** vcmd ff: end of track. */
static void chunSpcEventEndOfTrack (ChunSpcSeqStat *seq, SeqEventReport *ev)
{
    ChunSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    chunSpcInactiveTrack(seq, ev->track);

    //if (!chunSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** set pointers of each event. */
static void chunSpcSetEventList (ChunSpcSeqStat *seq)
{
    int code;
    ChunSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (ChunSpcEvent) chunSpcEventUnidentified;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    for(code = 0x00; code <= 0x9f; code++) {
        event[code] = (ChunSpcEvent) chunSpcEventNote;
    }
    for(code = 0xa0; code <= 0xdc; code++) {
        event[code] = (ChunSpcEvent) chunSpcEventNOP; // DQ5 Bridal March uses it
    }
    event[0xdd] = (ChunSpcEvent) chunSpcEventSetRR;
    event[0xde] = (ChunSpcEvent) chunSpcEventSetADSR_RR;
    event[0xdf] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xe0] = (ChunSpcEvent) chunSpcEventCPUControledJump;
    event[0xe1] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xe2] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xe3] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xe4] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xe5] = (ChunSpcEvent) chunSpcEventUnknown2;
    event[0xe6] = (ChunSpcEvent) chunSpcEventExpressionFade;
    event[0xe7] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xe8] = (ChunSpcEvent) chunSpcEventPanpotFade;
    event[0xe9] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xea] = (ChunSpcEvent) chunSpcEventJump;
    event[0xeb] = (ChunSpcEvent) chunSpcEventTempo;
    event[0xec] = (ChunSpcEvent) chunSpcEventDurRate;
    event[0xed] = (ChunSpcEvent) chunSpcEventVolume;
    event[0xee] = (ChunSpcEvent) chunSpcEventPanpot;
    event[0xef] = (ChunSpcEvent) chunSpcEventSetADSR;
    event[0xf0] = (ChunSpcEvent) chunSpcEventInstrument;
    event[0xf1] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xf2] = (ChunSpcEvent) chunSpcEventCopyNoteLenOn;
    event[0xf3] = (ChunSpcEvent) chunSpcEventCopyNoteLenOff;
    event[0xf4] = (ChunSpcEvent) chunSpcEventRepeatAgain;
    event[0xf5] = (ChunSpcEvent) chunSpcEventRepeatUntil;
    event[0xf6] = (ChunSpcEvent) chunSpcEventExpression;
    event[0xf7] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xf8] = (ChunSpcEvent) chunSpcEventCallSubroutine;
    event[0xf9] = (ChunSpcEvent) chunSpcEventEndSubroutine;
    event[0xfa] = (ChunSpcEvent) chunSpcEventTransposeAbs;
    event[0xfb] = (ChunSpcEvent) chunSpcEventPitchSlide;
    event[0xfc] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xfd] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xfe] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xff] = (ChunSpcEvent) chunSpcEventEndOfTrack;

    if (!chunSpcIsVersionSummer(seq))
    {
        for(code = 0xa0; code <= 0xb5; code++) {
            event[code] = (ChunSpcEvent) chunSpcEventDurFromTable;
        }
        // b5-da - not used
        event[0xdb] = (ChunSpcEvent) chunSpcEventRepeatBreakAlt;
        event[0xdc] = (ChunSpcEvent) chunSpcEventRepeatAgainAlt;
        event[0xf1] = (ChunSpcEvent) chunSpcEventCopyNoteLenOn;
        event[0xf7] = (ChunSpcEvent) chunSpcEventNOP;
    }
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* chunSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    ChunSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newChunSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = chunSpcCreateSmf(seq);

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

        // [driver-specific]
        if ((seq->cpuControledVar & 0x80) != 0)
        {
            seq->cpuControledVar = 0;
        }

        for (ev.track = 0; ev.track < SPC_TRACK_MAX; ev.track++) {

            ChunSpcTrackStat *evtr = &seq->track[ev.track];

            // [driver-specific] copy note length from prior channel
            if (evtr->active && evtr->refNoteLen && ev.track > 0) {
                evtr->noteLen = seq->track[ev.track-1].noteLen;
                evtr->durRate = seq->track[ev.track-1].durRate;
            }

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
                inSub = false; // TODO NYI
                strcat(ev.classStr, inSub ? " sub" : "");

                //if (ev.code != seq->ver.pitchSlideByte)
                //    evtr->prevTick = evtr->tick;
                evtr->used = true;
                // dispatch event
                seq->ver.event[ev.code](seq, &ev);

                // dump event report
                if (chunSpcTextLoopMax == 0 || seq->looped < chunSpcTextLoopMax)
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
            chunSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= chunSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, chunSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    chunSpcTruncateNoteAll(seq);
    chunSpcDequeueNoteAll(seq);

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
        delChunSpcSeq(&seq);
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
Smf* chunSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = chunSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* chunSpcToMidiFromFile (const char *filename)
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

    smf = chunSpcToMidi(data, size);

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
static bool cmdOptBendRange (void);

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
    { "bendrange", '\0', 0, cmdOptBendRange, "<range>", "set pitch bend sensitivity (0:auto)" },
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
                fprintf(stderr, " %s%c  %s%-10s  %-15s  %s\n",
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
    chunSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    chunSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    chunSpcForceSongListAddr = songListAddr;
    return true;
}

/** set pitchbend range. */
static bool cmdOptBendRange (void)
{
    int range = strtol(gArgv[0], NULL, 0);
    chunSpcPitchBendSens = range;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (chunSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    chunSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    chunSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    chunSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    chunSpcMidiResetType = SMF_RESET_GM2;
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
            chunSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = chunSpcToMidiFromFile(gArgv[0]);
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
