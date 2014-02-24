/**
 * Square AKAO spc2midi.
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
#include "akaospc.h"

#define APPNAME "Square AKAO SPC2MIDI"
#define APPSHORTNAME "akaospc"
#define VERSION "[2014-02-15]"

static int akaoSpcLoopMax = 2;            // maximum loop count of parser
static int akaoSpcTextLoopMax = 1;        // maximum loop count of text output
static double akaoSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool akaoSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool akaoSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int akaoSpcTimeBase = 48;
static int akaoSpcForceSongIndex = -1;
static int akaoSpcForceSongListAddr = -1;

static bool akaoSpcPatchFixOverride = false;
static PatchFixInfo akaoSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int akaoSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_REV1,
    SPC_VER_REV2,
    SPC_VER_REV3,
    SPC_VER_REV4,
};

enum {
    SPC_SUBVER_UNKNOWN = 0,

    //SPC_VER_REV1,
        SPC_SUBVER_FF4,

    //SPC_VER_REV2,
        SPC_SUBVER_RS1,

    //SPC_VER_REV3,
        SPC_SUBVER_FF5, // and HAHE
        SPC_SUBVER_SD2,
        SPC_SUBVER_FFMQ,

    //SPC_VER_REV4,
        SPC_SUBVER_RS2,
        SPC_SUBVER_FF6,
        SPC_SUBVER_LAL,
        SPC_SUBVER_FM, // and CT
        SPC_SUBVER_RS3,
        SPC_SUBVER_GH,
        SPC_SUBVER_BSGAME,
};

byte FF4_VCMD_LEN_TABLE[] = {
                0x03, 0x03, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x03, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
byte RS1_VCMD_LEN_TABLE[] = {
                0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00,
    0x03, 0x02, 0x00, 0x01, 0x01, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
byte FF5_VCMD_LEN_TABLE[] = {
                0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x01, 0x02, 0x01, 0x02, 0x02, 0x01, 0x03, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,
};
byte SD2_VCMD_LEN_TABLE[] = {
                0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x01, 0x02, 0x01, 0x02, 0x02, 0x01, 0x03, 0x02, 0x02, 0x00, 0x01, 0x00, 0x00,
};
byte RS2_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x02, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
byte FF6_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
};
byte LAL_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x02, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
};
byte FM_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x02, 0x02, 0x02, 0x01, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00,
};
byte RS3_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x02, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00,
};
byte GH_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x02, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
};
byte BSGAME_VCMD_LEN_TABLE[] = {
                            0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x02, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   0
#define SPC_ARAM_SIZE       0x10000

typedef struct TagAkaoSpcTrackStat AkaoSpcTrackStat;
typedef struct TagAkaoSpcSeqStat AkaoSpcSeqStat;
typedef void (*AkaoSpcEvent) (AkaoSpcSeqStat *, SeqEventReport *);

typedef struct TagAkaoSpcVerInfo {
    int id;
    int subId;
    //int seqListAddr;
    //int songIndex;
    int seqHeaderAddr;
    int noteLenTableLen;
    int noteLenTableAddr;
    int vcmdTableAddr;
    int vcmdLenTableAddr;
    byte vcmdFirstByte;
    bool useROMAddress;
    int apuAddressBase; // ROM -> ARAM
    int timer0Freq;
    AkaoSpcEvent event[256];
    PatchFixInfo patchFix[256];
    int cpuCtledJumpFlgAddr;
    int tempoMultiplier;
    bool assumeTrackZeroIsAtZero;
    bool panpotIs7bit;
    bool seqDetected;
} AkaoSpcVerInfo;

typedef struct TagAkaoSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} AkaoSpcNoteParam;

struct TagAkaoSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    AkaoSpcNoteParam note;     // current note param
    AkaoSpcNoteParam lastNote; // note params for last note
    int lastNoteLen;        // last note length ($0230+x)
    int forceNextNoteLen;   // force next note length (0=inactive)
    int looped;             // how many times looped (internal)
    byte octave;            // octave
    int patch;              // patch number (for pitch fix)
    bool rhythmChannel;     // rhythm channel / melody channel
    int repRetAddr[0x100];  // return address for repeat vcmd
    int repIncCount[0x100]; // incremental repeat counter
    int repDecCount[0x100]; // decremental repeat counter
    int repNestLevel;       // current nest level of repeat vcmd
    int repNestLevelMax;    // max nest level allowed of repeat vcmd
    int volume;             // current volume (used for fade control)
    int panpot;             // current panpot (used for fade control)
};

struct TagAkaoSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    int apuAddressOffset;       // ROM -> APU RAM offset
    bool active;                // if the seq is still active
    AkaoSpcVerInfo ver;       // game version info
    AkaoSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void akaoSpcSetEventList (AkaoSpcSeqStat *seq);

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
FILE *akaoSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int akaoSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = akaoSpcLoopMax;
    akaoSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool akaoSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        akaoSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        akaoSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            akaoSpcPatchFix[patch].bankSelM = patch >> 7;
            akaoSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            akaoSpcPatchFix[patch].bankSelM = 0;
            akaoSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        akaoSpcPatchFix[patch].patchNo = patch & 0x7f;
        akaoSpcPatchFix[patch].key = 0;
        akaoSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        akaoSpcPatchFix[src].bankSelM = bankM & 0x7f;
        akaoSpcPatchFix[src].bankSelL = bankL & 0x7f;
        akaoSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        akaoSpcPatchFix[src].key = key;
        akaoSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    akaoSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *akaoSpcVerToStrHtml (AkaoSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_REV1:
        return "Revision 1 (Final Fantasy 4)";
    case SPC_VER_REV2:
        return "Revision 2 (Romancing SaGa)";
    case SPC_VER_REV3:
        return "Revision 3 (Final Fantasy 5, etc.)";
    case SPC_VER_REV4:
        return "Revision 4 (Romancing SaGa 2, etc.)";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void akaoSpcResetTrackParam (AkaoSpcSeqStat *seq, int track)
{
    AkaoSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->octave = 6;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->lastNoteLen = 0;
    tr->patch = 0;
    tr->volume = 0x46; // depends on title, but nobody cares
    tr->panpot = 0x80;
    tr->rhythmChannel = false;
    tr->repNestLevel = 0;
    tr->repNestLevelMax = 4;
    tr->forceNextNoteLen = 0;
}

/** reset before play/convert song. */
static void akaoSpcResetParam (AkaoSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 1;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        AkaoSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        akaoSpcResetTrackParam(seq, track);
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
    if (akaoSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &akaoSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }

}

static int akaoGetARAMAddress(AkaoSpcSeqStat *seq, int address)
{
    return (address + seq->apuAddressOffset) & 0xffff;
}

/** returns what version the sequence is, and sets individual info. */
static int akaoSpcCheckVer (AkaoSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int noteLenLdCodeAddr;
    int noteLenLdCodeVer = SPC_VER_UNKNOWN;
    int vcmdExecCodeAddr;
    int vcmdExecCodeVer = SPC_VER_UNKNOWN;
    int songLdCodeAddr;
    int timerSetCodeAddr;
    int cpuCtledJumpVCmdAddr;

    seq->timebase = akaoSpcTimeBase;
    //seq->ver.seqListAddr = -1;
    //seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.noteLenTableAddr = -1;
    seq->ver.noteLenTableLen = 0;
    seq->ver.vcmdTableAddr = -1;
    seq->ver.vcmdLenTableAddr = -1;
    seq->ver.vcmdFirstByte = -1;
    seq->ver.useROMAddress = false;
    seq->ver.apuAddressBase = 0;
    seq->ver.seqDetected = false;
    seq->ver.timer0Freq = -1;
    seq->ver.subId = SPC_SUBVER_UNKNOWN;
    seq->ver.tempoMultiplier = 0;
    seq->ver.cpuCtledJumpFlgAddr = -1;
    seq->ver.panpotIs7bit = false;
    seq->ver.assumeTrackZeroIsAtZero = false;

    // (Romancing SaGa 2)
    // mov   x,#$0e
    // div   ya,x
    // mov   x,$a7
    // mov   a,$1e7e+y
    if ((noteLenLdCodeAddr = indexOfHexPat(seq->aRAM, "\xcd\x0e\x9e\xf8.\xf6..", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.noteLenTableAddr = mget2l(&seq->aRAM[noteLenLdCodeAddr + 6]);
        noteLenLdCodeVer = SPC_VER_REV4;
    }
    // (Romacing SaGa)
    // mov   y,#$00
    // mov   x,#$0f
    // div   ya,x
    // mov   x,$06
    // mov   a,$19f4+y
    else if ((noteLenLdCodeAddr = indexOfHexPat(seq->aRAM, "\x8d\\\x00\xcd\x0f\x9e\xf8.\xf6..", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.noteLenTableAddr = mget2l(&seq->aRAM[noteLenLdCodeAddr + 8]);
        noteLenLdCodeVer = SPC_VER_REV2;
    }
    // (Final Fantasy 4)
    // mov   x,#$0f
    // mov   y,#$00
    // div   ya,x
    // mov   x,$..
    // mov   a,$....+y
    else if ((noteLenLdCodeAddr = indexOfHexPat(seq->aRAM, "\xcd\x0f\x8d\\\x00\x9e\xf8.\xf6..", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.noteLenTableAddr = mget2l(&seq->aRAM[noteLenLdCodeAddr + 8]);
        noteLenLdCodeVer = SPC_VER_REV1;
    }

    // (Romancing SaGa 3)
    // sbc   a,#$c4
    // mov   $a6,a
    // asl   a
    // mov   y,a
    // mov   a,$1656+y
    // push  a
    // mov   a,$1655+y
    // push  a
    // mov   y,$a6
    // mov   a,$16cd+y
    // bne   $0747
    // ret
    if ((vcmdExecCodeAddr = indexOfHexPat(seq->aRAM, "\xa8.\xc4.\x1c\xfd\xf6..\x2d\xf6..\x2d\xeb.\xf6....", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if (seq->aRAM[vcmdExecCodeAddr + 3] == seq->aRAM[vcmdExecCodeAddr + 15] &&
            mget2l(&seq->aRAM[vcmdExecCodeAddr + 7]) == mget2l(&seq->aRAM[vcmdExecCodeAddr + 11]) + 1 &&
            (seq->aRAM[vcmdExecCodeAddr + 19] == 0xf0 || seq->aRAM[vcmdExecCodeAddr + 19] == 0xd0)) // beq/bne
        {
            seq->ver.vcmdFirstByte = seq->aRAM[vcmdExecCodeAddr + 1];
            seq->ver.vcmdTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 11]);
            seq->ver.vcmdLenTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 17]);
            vcmdExecCodeVer = SPC_VER_REV4;
        }
    }
    // (Final Fantasy 5)
    // sbc   a,#$d2
    // asl   a
    // mov   y,a
    // mov   a,$182c+y
    // push  a
    // mov   a,$182b+y
    // push  a
    // mov   a,y
    // lsr   a
    // mov   y,a
    // mov   a,$1887+y
    // beq   $05a6
    else if ((vcmdExecCodeAddr = indexOfHexPat(seq->aRAM, "\xa8.\x1c\xfd\xf6..\x2d\xf6..\x2d\xdd\\\x5c\xfd\xf6....", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if (mget2l(&seq->aRAM[vcmdExecCodeAddr + 5]) == mget2l(&seq->aRAM[vcmdExecCodeAddr + 9]) + 1 &&
            (seq->aRAM[vcmdExecCodeAddr + 18] == 0xf0 || seq->aRAM[vcmdExecCodeAddr + 18] == 0xd0)) // beq/bne
        {
            seq->ver.vcmdFirstByte = seq->aRAM[vcmdExecCodeAddr + 1];
            seq->ver.vcmdTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 9]);
            seq->ver.vcmdLenTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 16]);
            vcmdExecCodeVer = SPC_VER_REV1;
        }
    }

    // (Romancing SaGa 3)
    // mov   a,$2200           ; $2200 - header start address
    // mov   $00,a
    // mov   a,$2201
    // mov   $01,a             ; header 00/1 - ROM address base
    // mov   a,#$24
    // mov   y,#$22            ; $2224 - ARAM address base
    // subw  ya,$00
    // movw  $00,ya            ; $00/1 - offset from ROM to ARAM address
    if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xe5..\xc4.\xe5..\xc4.\xe8.\x8d.\x9a.\xda.", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if (mget2l(&seq->aRAM[songLdCodeAddr + 1]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 6]) &&
            seq->aRAM[songLdCodeAddr + 4] + 1 == seq->aRAM[songLdCodeAddr + 9] &&
            seq->aRAM[songLdCodeAddr + 4] == seq->aRAM[songLdCodeAddr + 15] &&
            seq->aRAM[songLdCodeAddr + 15] == seq->aRAM[songLdCodeAddr + 17])
        {
            seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[songLdCodeAddr + 1]);
            seq->ver.useROMAddress = true;
            seq->ver.apuAddressBase = (seq->aRAM[songLdCodeAddr + 13] << 8) | seq->aRAM[songLdCodeAddr + 11];
        }
    }
    // (Final Fantasy: Mystic Quest)
    // mov   x,#$10
    // mov   a,$1bff+x
    // mov   $0d+x,a
    // dec   x
    // bne   $0dfb
    // mov   a,#$12
    // mov   y,#$1c
    // subw  ya,$0e
    // movw  $08,ya
    // mov   x,#$0e
    // mov   $c1,#$80
    // mov   a,$1c10
    // mov   y,$1c11
    // movw  $36,ya
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xcd\x10\xf5..\xd4.\x1d\xd0\xf8\xe8.\x8d.\x9a.\xda.\xcd\x0e\x8f\x80.\xe5..\xec..\xda.", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if (mget2l(&seq->aRAM[songLdCodeAddr + 3]) + 0x11 == mget2l(&seq->aRAM[songLdCodeAddr + 24]) &&
            mget2l(&seq->aRAM[songLdCodeAddr + 24]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 27]) &&
            seq->aRAM[songLdCodeAddr + 6] + 1 == seq->aRAM[songLdCodeAddr + 15])
        {
            seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[songLdCodeAddr + 3]) + 1;
            seq->ver.useROMAddress = true;
            seq->ver.apuAddressBase = (seq->aRAM[songLdCodeAddr + 13] << 8) | seq->aRAM[songLdCodeAddr + 11];
            seq->ver.assumeTrackZeroIsAtZero = true;
            seq->ver.subId = SPC_SUBVER_FFMQ;
        }
    }
    // (Romancing SaGa)
    // mov   x,#$00
    // mov   y,#$00
    // mov   $93,#$01
    // mov   a,$2001+x
    // beq   $14d8
    // or    ($8e),($93)
    // mov   $08+x,a
    // mov   a,$2000+x
    // mov   $07+x,a
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xcd\\\x00\x8d\\\x00\x8f\x01.\xf5..\xf0.\x09..\xd4.\xf5..\xd4.", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if (mget2l(&seq->aRAM[songLdCodeAddr + 8]) == mget2l(&seq->aRAM[songLdCodeAddr + 18]) + 1 &&
            seq->aRAM[songLdCodeAddr + 6] == seq->aRAM[songLdCodeAddr + 13] &&
            seq->aRAM[songLdCodeAddr + 16] == seq->aRAM[songLdCodeAddr + 21] + 1)
        {
            seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[songLdCodeAddr + 18]);
            seq->ver.useROMAddress = false;
            seq->ver.apuAddressBase = 0;
        }
    }
    // (Final Fantasy 4)
    // mov   y,#$01
    // mov   $8d,y
    // mov   x,#$00
    // mov   a,$2000+x
    // mov   $02+x,a
    // mov   a,$2001+x
    // mov   $03+x,a
    // beq   $154c
    // mov   $48+x,y
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\x8d\x01\xcb.\xcd\\\x00\xf5..\xd4.\xf5..\xd4.\xf0.\xdb.", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if (mget2l(&seq->aRAM[songLdCodeAddr + 7]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 12]) &&
            seq->aRAM[songLdCodeAddr + 10] + 1 == seq->aRAM[songLdCodeAddr + 15])
        {
            seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[songLdCodeAddr + 7]);
            seq->ver.useROMAddress = false;
            seq->ver.apuAddressBase = 0;
        }
    }
    if (akaoSpcForceSongListAddr != -1)
    {
        seq->ver.seqHeaderAddr = akaoSpcForceSongListAddr;
    }

    // (Final Fantasy 5)
    // mov   $f1,#$f0
    // mov   $fa,#$24
    // mov   $fb,#$80
    // mov   $f1,#$03
    if ((timerSetCodeAddr = indexOfHexPat(seq->aRAM, "\x8f.\xf1\x8f.\xfa\x8f.\xfb\x8f\x03\xf1", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if ((seq->aRAM[timerSetCodeAddr + 1] & 0x07) == 0 &&
            seq->aRAM[timerSetCodeAddr + 10] == 0x03)
        {
            seq->ver.timer0Freq = seq->aRAM[timerSetCodeAddr + 4];
        }
    }
    // (Live A Live)
    // mov   $f1,#$f0
    // mov   $fa,#$24
    // mov   $fb,#$80
    // mov   $fc,#$05
    // mov   $f1,#$03
    else if ((timerSetCodeAddr = indexOfHexPat(seq->aRAM, "\x8f.\xf1\x8f.\xfa\x8f.\xfb\x8f.\xfc\x8f\x07\xf1", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if ((seq->aRAM[timerSetCodeAddr + 1] & 0x07) == 0)
        {
            seq->ver.timer0Freq = seq->aRAM[timerSetCodeAddr + 4];
        }
    }
    // (Seiken Densetsu 2)
    // mov   $f1,#$f0
    // mov   $fa,#$24
    // mov   $f1,#$01
    else if ((timerSetCodeAddr = indexOfHexPat(seq->aRAM, "\x8f.\xf1\x8f.\xfa\x8f\x01\xf1", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if ((seq->aRAM[timerSetCodeAddr + 1] & 0x07) == 0)
        {
            seq->ver.timer0Freq = seq->aRAM[timerSetCodeAddr + 4];
        }
    }
    // (Final Fantasy 4)
    // mov   a,#$f0
    // mov   $f1,a             ; clear ports, stop timers
    // mov   a,#$24
    // mov   $fa,a             ; set timer 0 latch to #$24 (222.2 Hz)
    // mov   a,#$80
    // mov   $fb,a             ; set timer 1 latch to #$80 (62.5 Hz)
    // mov   a,#$03
    // mov   $f1,a             ; start timers 0 and 1
    else if ((timerSetCodeAddr = indexOfHexPat(seq->aRAM, "\xe8.\xc4\xf1\xe8.\xc4\xfa\xe8.\xc4\xfb\xe8\x03\xc4\xf1", SPC_ARAM_SIZE, NULL)) != -1)
    {
        if ((seq->aRAM[timerSetCodeAddr + 1] & 0x07) == 0)
        {
            seq->ver.timer0Freq = seq->aRAM[timerSetCodeAddr + 5];
        }
    }

    // (Final Fantasy 5)
    // ; vcmd fb - goto if vbit set in D1
    // mov   y,a
    // call  $059c
    // cmp   x,#$10
    // bcs   $0a41
    // mov   a,$bf
    // and   a,$d1
    // beq   $0a41
    // tclr1 $00d1
    // mov   a,y
    // mov   y,$04
    // addw  ya,$06
    // mov   $0c+x,a
    // mov   $0d+x,y
    // ret
    if ((cpuCtledJumpVCmdAddr = indexOfHexPat(seq->aRAM, "\xfd\x3f..\xc8\x10\xb0\x12\xe4.\x24.\xf0\x0c\x4e..\xdd\xeb.\x7a.\xd4.\xdb.\x6f", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[cpuCtledJumpVCmdAddr + 23] + 1 == seq->aRAM[cpuCtledJumpVCmdAddr + 25])
    {
        seq->ver.cpuCtledJumpFlgAddr = mget2l(&seq->aRAM[cpuCtledJumpVCmdAddr + 15]);
    }
    // (Live A Live)
    // ; vcmd fb - branch if voice bit in $dd set
    // mov   y,a
    // call  $0590
    // mov   a,$8f
    // and   a,$dd
    // beq   $17ec
    // mov   a,y
    // mov   y,$a2
    // addw  ya,$00
    // mov   $02+x,a
    // mov   $03+x,y
    // ret
    else if ((cpuCtledJumpVCmdAddr = indexOfHexPat(seq->aRAM, "\xfd\x3f..\xe4.\x24.\xf0\x09\xdd\xeb.\x7a.\xd4.\xdb.\x6f", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[cpuCtledJumpVCmdAddr + 16] + 1 == seq->aRAM[cpuCtledJumpVCmdAddr + 18])
    {
        seq->ver.cpuCtledJumpFlgAddr = seq->aRAM[cpuCtledJumpVCmdAddr + 7];
    }
    // (Final Fantasy 6)
    // ; vcmd fc - branch if voice bit in $dd set
    // mov   y,a
    // call  $05c9
    // mov   a,$8f
    // and   a,$dd
    // beq   $176f
    // tclr1 $00dd
    // mov   a,y
    // mov   y,$a2
    // addw  ya,$00
    // mov   $02+x,a
    // mov   $03+x,y
    // ret
    else if ((cpuCtledJumpVCmdAddr = indexOfHexPat(seq->aRAM, "\xfd\x3f..\xe4.\x24.\xf0\x0c\x4e..\xdd\xeb.\x7a.\xd4.\xdb.\x6f", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[cpuCtledJumpVCmdAddr + 19] + 1 == seq->aRAM[cpuCtledJumpVCmdAddr + 21])
    {
        seq->ver.cpuCtledJumpFlgAddr = mget2l(&seq->aRAM[cpuCtledJumpVCmdAddr + 11]);
    }

    if (seq->ver.vcmdFirstByte % 14 == 0)
    {
        seq->ver.noteLenTableLen = seq->ver.vcmdFirstByte / 14;
    }

    // classify
    if (seq->ver.seqHeaderAddr != -1 &&
        seq->ver.noteLenTableAddr != -1 &&
        seq->ver.noteLenTableLen > 0 &&
        seq->ver.vcmdTableAddr != -1 &&
        seq->ver.vcmdLenTableAddr != -1 &&
        seq->ver.timer0Freq != -1)
    {
        if (noteLenLdCodeVer == SPC_VER_REV1 &&
            seq->ver.vcmdFirstByte == 0xd2 &&
            !seq->ver.useROMAddress)
        {
            version = SPC_VER_REV1;
        }
        else if (noteLenLdCodeVer == SPC_VER_REV2 &&
            seq->ver.vcmdFirstByte == 0xd2 &&
            !seq->ver.useROMAddress)
        {
            version = SPC_VER_REV2;
        }
        else if (noteLenLdCodeVer == SPC_VER_REV2 &&
            seq->ver.vcmdFirstByte == 0xd2 &&
            seq->ver.useROMAddress)
        {
            version = SPC_VER_REV3;
        }
        else if (noteLenLdCodeVer == SPC_VER_REV4 &&
            seq->ver.vcmdFirstByte == 0xc4 &&
            seq->ver.useROMAddress)
        {
            version = SPC_VER_REV4;
        }

        if (version == SPC_VER_REV2)
        {
            seq->ver.panpotIs7bit = true;
        }
        else if (version == SPC_VER_REV4)
        {
            int panVCmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + 0x02 * 2]);
            if (seq->aRAM[panVCmdAddr] == 0x1c) // asl
            {
                seq->ver.panpotIs7bit = true;
            }
        }

        // detect vcmd mapping (silly...)
        if (seq->ver.vcmdFirstByte == 0x100 - countof(FF4_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], FF4_VCMD_LEN_TABLE, sizeof(FF4_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_FF4;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(FF5_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], FF5_VCMD_LEN_TABLE, sizeof(FF5_VCMD_LEN_TABLE)) == 0)
        {
            if (seq->ver.subId != SPC_SUBVER_FFMQ)
            {
                seq->ver.subId = SPC_SUBVER_FF5;
            }
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(FF6_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], FF6_VCMD_LEN_TABLE, sizeof(FF6_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_FF6;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(RS1_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], RS1_VCMD_LEN_TABLE, sizeof(RS1_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_RS1;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(RS2_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], RS2_VCMD_LEN_TABLE, sizeof(RS2_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_RS2;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(RS3_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], RS3_VCMD_LEN_TABLE, sizeof(RS3_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_RS3;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(LAL_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], LAL_VCMD_LEN_TABLE, sizeof(LAL_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_LAL;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(FM_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], FM_VCMD_LEN_TABLE, sizeof(FM_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_FM;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(BSGAME_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], BSGAME_VCMD_LEN_TABLE, sizeof(BSGAME_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_BSGAME;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(GH_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], GH_VCMD_LEN_TABLE, sizeof(GH_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_GH;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(SD2_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], SD2_VCMD_LEN_TABLE, sizeof(SD2_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_SD2;
        }
    }

    if (seq->ver.vcmdFirstByte == 0xc4)
    {
        int tempoVcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0xf0 - seq->ver.vcmdFirstByte) * 2]);

        // (Chrono Trigger)
        // ; vcmd f0 - set tempo
        // mov   y,#$14
        // mul   ya
        if (seq->aRAM[tempoVcmdAddr] == 0x8d && seq->aRAM[tempoVcmdAddr + 2] == 0xcf)
        {
            seq->ver.tempoMultiplier = seq->aRAM[tempoVcmdAddr + 1];
        }
    }

    seq->ver.id = version;
    akaoSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool akaoSpcDetectSeq (AkaoSpcSeqStat *seq)
{
    bool result = true;
    int seqHeaderReadOfs;
    int seqEndOffset;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    seq->apuAddressOffset = 0;
    seqEndOffset = 0;
    akaoSpcResetParam(seq);

    seqHeaderReadOfs = seq->ver.seqHeaderAddr;
    if (seq->ver.id != SPC_VER_REV1 && seq->ver.id != SPC_VER_REV2)
    {
        if (seq->ver.id == SPC_VER_REV3)
        {
            if (seq->ver.assumeTrackZeroIsAtZero)
            {
                seq->apuAddressOffset = (seq->ver.apuAddressBase - mget2l(&seq->aRAM[seqHeaderReadOfs])) & 0xffff;
                seqEndOffset = mget2l(&seq->aRAM[seqHeaderReadOfs + SPC_TRACK_MAX * 2]);
            }
            else
            {
                seq->apuAddressOffset = (seq->ver.apuAddressBase - mget2l(&seq->aRAM[seqHeaderReadOfs])) & 0xffff;
                seqHeaderReadOfs += 2;
                seqEndOffset = mget2l(&seq->aRAM[seqHeaderReadOfs + SPC_TRACK_MAX * 2]);
            }
        }
        else
        {
            seq->apuAddressOffset = (seq->ver.apuAddressBase - mget2l(&seq->aRAM[seqHeaderReadOfs])) & 0xffff;
            seqHeaderReadOfs += 2;
            seqEndOffset = mget2l(&seq->aRAM[seqHeaderReadOfs]);
            seqHeaderReadOfs += 2;
        }
    }

    // track list
    result = false;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        int trackAddr = mget2l(&seq->aRAM[seqHeaderReadOfs]);
        seqHeaderReadOfs += 2;

        if (trackAddr == seqEndOffset)
        {
            continue;
        }

        seq->track[tr].pos = akaoGetARAMAddress(seq, trackAddr);
        seq->track[tr].active = true;
        result = true;
    }

    return result;
}

/** create new spc2mid object. */
static AkaoSpcSeqStat *newAkaoSpcSeq (const byte *aRAM)
{
    AkaoSpcSeqStat *newSeq = (AkaoSpcSeqStat *) calloc(1, sizeof(AkaoSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        akaoSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = akaoSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delAkaoSpcSeq (AkaoSpcSeqStat **seq)
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
static void printHtmlInfoList (AkaoSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", akaoSpcVerToStrHtml(seq));
    //myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    //myprintf(" (Song $%02x)", seq->ver.songIndex);
    myprintf("</li>\n");
    myprintf("          <li>Note Length Table: $%04X (%d bytes)</li>", seq->ver.noteLenTableAddr, seq->ver.noteLenTableLen);
    myprintf("          <li>Voice Command Table: $%04X</li>", seq->ver.vcmdTableAddr);
    myprintf("          <li>Voice Command Length Table: $%04X</li>", seq->ver.vcmdLenTableAddr);
    myprintf("          <li>First Voice Command Byte: $%02X</li>", seq->ver.vcmdFirstByte);
    if (seq->ver.useROMAddress)
    {
        myprintf("          <li>APU RAM Address Base: $%04X</li>", seq->ver.apuAddressBase);
    }
    myprintf("          <li>Timer 0 Frequency: $%02X", seq->ver.timer0Freq);
    if (seq->ver.tempoMultiplier != 0)
    {
        myprintf(" (tempo multiplier $%02X)", seq->ver.tempoMultiplier);
    }
    myprintf("</li>\n");
    myprintf("          <li>Panpot Range: %d bits</li>", seq->ver.panpotIs7bit ? 7 : 8);
    if (seq->ver.cpuCtledJumpFlgAddr >= 0)
    {
        myprintf("          <li>CPU Controled Jump Flag Address: $%04X</li>", seq->ver.cpuCtledJumpFlgAddr);
    }
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (AkaoSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (AkaoSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (AkaoSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (AkaoSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double akaoSpcTempoOf (AkaoSpcSeqStat *seq, int tempoValue)
{
    if (seq->ver.tempoMultiplier != 0)
    {
        tempoValue = (tempoValue * seq->ver.tempoMultiplier) / 0x100 + tempoValue;
    }
    return (double) tempoValue * 60000000 / (125 * seq->ver.timer0Freq * 48 * 256); // 55296000 = (timer0) 4.5ms * 48 TPQN * 256
}

/** convert SPC tempo into bpm. */
static double akaoSpcTempo (AkaoSpcSeqStat *seq)
{
    return akaoSpcTempoOf(seq, seq->tempo);
}

/** convert SPC velocity into MIDI one. */
static int akaoSpcMidiVelOf (int value)
{
    if (akaoSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int akaoSpcMidiVolOf (int value)
{
    if (akaoSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel panpot into MIDI one. */
static int akaoSpcMidiPanOf (int value)
{
    return value/2; // linear (TODO: sine curve)
}

/** create new smf object and link to spc seq. */
static Smf *akaoSpcCreateSmf (AkaoSpcSeqStat *seq)
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

    switch (akaoSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, akaoSpcTempo(seq));

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

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, akaoSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void akaoSpcTruncateNote (AkaoSpcSeqStat *seq, int track)
{
    AkaoSpcTrackStat *tr = &seq->track[track];

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
static void akaoSpcTruncateNoteAll (AkaoSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        akaoSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool akaoSpcDequeueNote (AkaoSpcSeqStat *seq, int track)
{
    AkaoSpcTrackStat *tr = &seq->track[track];
    AkaoSpcNoteParam *lastNote = &tr->lastNote;
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
static void akaoSpcDequeueNoteAll (AkaoSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        akaoSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void akaoSpcInactiveTrack(AkaoSpcSeqStat *seq, int track)
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
static void akaoSpcAddTrackLoopCount(AkaoSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (akaoSpcLoopMax > 0) ? akaoSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= akaoSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void akaoSpcSeqAdvTick(AkaoSpcSeqStat *seq)
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
    seq->time += (double) 60 / akaoSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void akaoSpcEventUnknownInline (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
}

/** vcmds: unidentified event. */
static void akaoSpcEventUnidentified (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    akaoSpcEventUnknownInline(seq, ev);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void akaoSpcEventUnknown0 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    akaoSpcEventUnknownInline(seq, ev);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void akaoSpcEventUnknown1 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    akaoSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void akaoSpcEventUnknown2 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    akaoSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void akaoSpcEventUnknown3 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    akaoSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void akaoSpcEventUnknown4 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
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

    akaoSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void akaoSpcEventUnknown5 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
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

    akaoSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void akaoSpcEventNOP (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "NOP");
}

/** vcmds: no operation (2 bytes). */
static void akaoSpcEventNOP2 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->size++;
    sprintf(ev->note, "NOP");
}

/** vcmd 00-c3: note. */
static void akaoSpcEventNote (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int keyIndex = noteByte / seq->ver.noteLenTableLen;
    int lenIndex = noteByte % seq->ver.noteLenTableLen;
    int note;
    int len;
    int dur;
    bool rest;
    bool tie;

    tie = (keyIndex == 12);
    rest = (keyIndex == 13);
    if (seq->ver.id == SPC_VER_REV1)
    {
        // swap
        bool bSwap = tie;
        tie = rest;
        rest = bSwap;
    }

    len = seq->aRAM[seq->ver.noteLenTableAddr + lenIndex];
    if (tr->forceNextNoteLen != 0)
    {
        len = tr->forceNextNoteLen;
        tr->forceNextNoteLen = 0;
    }
    dur = len;
    if (!tie && !rest)
    {
        note = (tr->octave * 12) + keyIndex;
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
        getNoteName(ev->note, note + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key
            + (tr->rhythmChannel ? 0 : SPC_NOTE_KEYSHIFT));
        sprintf(argDumpStr, ", len = %d", len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    //if (!akaoSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // output old note first
    if (!tie)
    {
        akaoSpcDequeueNote(seq, ev->track);
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
            tr->lastNote.vel = 100;
            tr->lastNote.transpose = seq->transpose + tr->note.transpose;
            tr->lastNote.patch = tr->note.patch;
            tr->lastNote.active = true;
        }
        tr->lastNote.tied = tie;
    }
    tr->tick += len;
}

/** vcmd xx: set master volume. */
static void akaoSpcEventMasterVolume (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-mastervol");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd c4: set volume. */
static void akaoSpcEventVolume (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->volume = arg1;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, akaoSpcMidiVolOf(tr->volume));
}

/** vcmd xx: set alternate volume. */
static void akaoSpcEventAltVolume (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume (Alternate), vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_EXPRESSION, akaoSpcMidiVolOf(tr->volume));
}

/** vcmd c5: set volume fade. */
static void akaoSpcEventVolumeFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    if (seq->ver.id == SPC_VER_REV1)
    {
        ev->size++;
        arg1 = mget2l(&seq->aRAM[*p]);
        (*p) += 2;
    }
    else
    {
        arg1 = seq->aRAM[*p];
        (*p)++;
    }
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vol");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    // (for instance, Chrono Trigger Title)
    if (arg1 == 0)
    {
        tr->volume = arg2;
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, akaoSpcMidiVolOf(tr->volume));
    }
    else
    {
        valueFrom = tr->volume;
        valueTo = arg2;
        for (faderStep = 1; faderStep <= arg1; faderStep++)
        {
            faderPos = (double)faderStep / arg1;
            faderValue = (int)(valueTo * faderPos + valueFrom * (1.0 - faderPos)); // alphablend
            if (tr->volume != faderValue)
            {
                tr->volume = faderValue;
                smfInsertControl(seq->smf, ev->tick + faderStep, ev->track, ev->track, SMF_CONTROL_VOLUME, akaoSpcMidiVolOf(tr->volume));
            }
        }
    }
}

/** vcmd c6: set panpot. */
static void akaoSpcEventPanpot (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot, balance = %d", arg1);
    strcat(ev->classStr, " ev-pan");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->panpot = arg1 * (seq->ver.panpotIs7bit ? 2 : 1);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, akaoSpcMidiPanOf(tr->panpot));
}

/** vcmd c7: set panpot fade. */
static void akaoSpcEventPanpotFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    if (seq->ver.id == SPC_VER_REV1)
    {
        ev->size++;
        arg1 = mget2l(&seq->aRAM[*p]);
        (*p) += 2;
    }
    else
    {
        arg1 = seq->aRAM[*p];
        (*p)++;
    }
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-pan");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    if (arg1 == 0)
    {
        tr->panpot = arg2 * (seq->ver.panpotIs7bit ? 2 : 1);
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, akaoSpcMidiPanOf(tr->panpot));
    }
    else
    {
        valueFrom = tr->panpot;
        valueTo = arg2 * (seq->ver.panpotIs7bit ? 2 : 1);
        for (faderStep = 1; faderStep <= arg1; faderStep++)
        {
            faderPos = (double)faderStep / arg1;
            faderValue = (int)(valueTo * faderPos + valueFrom * (1.0 - faderPos)); // alphablend
            if (tr->panpot != faderValue)
            {
                tr->panpot = faderValue;
                smfInsertControl(seq->smf, ev->tick + faderStep, ev->track, ev->track, SMF_CONTROL_PANPOT, akaoSpcMidiPanOf(tr->panpot));
            }
        }
    }
}

/** vcmd c8: pitch slide (one-shot). */
static void akaoSpcEventPitchSlide (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Slide, speed = %d, key += %d", arg1, arg2);
    strcat(ev->classStr, " ev-pitchslide");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d6: pitch envelope on (old). */
static void akaoSpcEventPitchEnvelopeOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    if (seq->ver.id == SPC_VER_REV1)
    {
        arg3 = utos1(arg3);
        sprintf(ev->note, "Pitch Envelope, delay = %d, speed = %d, depth = %d semitones", arg1, arg2, arg3);
    }
    else // (SPC_VER_REV2 - Romancing SaGa)
    {
        arg1 = utos1(arg1);
        sprintf(ev->note, "Pitch Envelope, depth = %d semitones, delay = %d, speed = %d", arg1, arg2, arg3);
    }
    strcat(ev->classStr, " ev-pitchenv");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd xx: pitch envelope off. */
static void akaoSpcEventPitchEnvelopeOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Envelope Off");
    strcat(ev->classStr, " ev-pitchenv");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd c9: set vibrato on. */
static void akaoSpcEventVibratoOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-vibrato");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ca: set vibrato off. */
static void akaoSpcEventVibratoOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Vibrato Off");
    strcat(ev->classStr, " ev-vibratooff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cb: set tremolo on. */
static void akaoSpcEventTremoloOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tremolo, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-tremolo");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cc: set tremolo off. */
static void akaoSpcEventTremoloOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Tremolo Off");
    strcat(ev->classStr, " ev-tremolooff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cd: set panpot LFO on. */
static void akaoSpcEventPanLFOOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot LFO, depth = %d, rate = %d", arg1, arg2);
    strcat(ev->classStr, " ev-panLFO");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d9: set panpot LFO on (old). */
static void akaoSpcEventPanLFOOn3 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot LFO, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-panLFO");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ce: set panpot LFO off. */
static void akaoSpcEventPanLFOOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Panpot LFO Off");
    strcat(ev->classStr, " ev-panLFOoff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cf: set noise clock. */
static void akaoSpcEventNoiseFreq (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Noise Frequency, NCK = %d", arg1 & 0x1f);
    strcat(ev->classStr, " ev-noisefreq");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d0: noise on. */
static void akaoSpcEventNoiseOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Noise On");
    strcat(ev->classStr, " ev-noiseon");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d1: noise off. */
static void akaoSpcEventNoiseOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Noise Off");
    strcat(ev->classStr, " ev-noiseoff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d2: pitchmod on. */
static void akaoSpcEventPitchModOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Mod On");
    strcat(ev->classStr, " ev-pitchmodon");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d3: pitchmod off. */
static void akaoSpcEventPitchModOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Mod Off");
    strcat(ev->classStr, " ev-pitchmodoff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d4: echo on. */
static void akaoSpcEventEchoOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd d5: echo off. */
static void akaoSpcEventEchoOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo Off");
    strcat(ev->classStr, " ev-echooff");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd d6: set octave. */
static void akaoSpcEventSetOctave (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Octave, octave = %d", arg1);
    strcat(ev->classStr, " ev-octave");

    tr->octave = arg1;
}

/** vcmd d7: increase octave. */
static void akaoSpcEventOctaveUp (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    tr->octave++; // wrong emulation, but I think it's good enough

    sprintf(ev->note, "Octave Up, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octaveup");
}

/** vcmd d8: decrease octave. */
static void akaoSpcEventOctaveDown (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    tr->octave--; // wrong emulation, but I think it's good enough

    sprintf(ev->note, "Octave Down, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octavedown");
}

/** vcmd d9: transpose (absolute). */
static void akaoSpcEventTransposeAbs (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;

    sprintf(ev->note, "Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd da: transpose (relative). */
static void akaoSpcEventTransposeRel (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose += arg1;

    sprintf(ev->note, "Transpose, key += %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd db: detune. */
static void akaoSpcEventDetune (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    //int midiScaled;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Tuning, key += %.1f cents?", arg1 * 100 / 16.0);
    strcat(ev->classStr, " ev-tuning");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    //midiScaled = arg1 * 64 / 16;
    //if (midiScaled >= -64 && midiScaled <= 64)
    //{
    //    if (midiScaled == 64)
    //    {
    //        midiScaled = 63; // just a hack!
    //    }
    //    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNM, 0);
    //    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNL, 1);
    //    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYM, 64 + midiScaled);
    //    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYL, 0);
    //}
}

/** vcmd dc: set instrument. */
static void akaoSpcEventInstrument (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd dd: set attack rate (AR). */
static void akaoSpcEventSetAR (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Attack Rate, AR = %d", arg1 & 15);
    strcat(ev->classStr, " ev-setAR");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd de: set decay rate (DR). */
static void akaoSpcEventSetDR (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Decay Rate, DR = %d", arg1 & 7);
    strcat(ev->classStr, " ev-setDR");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd df: set sustain level (SL). */
static void akaoSpcEventSetSL (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Sustain Level, SL = %d", arg1 & 7);
    strcat(ev->classStr, " ev-setSL");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e0: set sustain rate (SR). */
static void akaoSpcEventSetSR (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Sustain Rate, SR = %d", arg1 & 31);
    strcat(ev->classStr, " ev-setSR");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e1: set default ADSR. */
static void akaoSpcEventSetDefaultADSR (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "Set Default ADSR");
    strcat(ev->classStr, " ev-setdefaultADSR");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e2: loop start. */
static void akaoSpcEventLoopStart (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int repCount;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    if (arg1 == 0)
    {
        repCount = 0;
    }
    else
    {
        repCount = (arg1 + 1) & 0xff;
    }

    sprintf(ev->note, "Loop Start, count = %d", repCount);
    strcat(ev->classStr, " ev-loopstart");

    if (tr->repNestLevel + 1 > tr->repNestLevelMax) {
        //fprintf(stderr, "Warning: Repeat Nest Level Overflow at $%04X\n", ev->addr);
    }
    tr->repRetAddr[tr->repNestLevel] = *p;
    switch (seq->ver.id)
    {
    case SPC_VER_REV1:
    case SPC_VER_REV2:
    case SPC_VER_REV3:
        tr->repIncCount[tr->repNestLevel] = 0;
        break;
    default:
        tr->repIncCount[tr->repNestLevel] = 1;
        break;
    }
    tr->repDecCount[tr->repNestLevel] = repCount;
    tr->repNestLevel = (tr->repNestLevel + 1) % tr->repNestLevelMax;

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e3: loop end. */
static void akaoSpcEventLoopEnd (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool repeatAgain;
    int nestIndex;

    nestIndex = (tr->repNestLevel - 1);
    if (nestIndex < 0)
    {
        //fprintf(stderr, "Warning: Repeat Nest Level Overflow at $%04X\n", ev->addr);
        nestIndex += tr->repNestLevelMax;
    }
    nestIndex %= tr->repNestLevelMax;

    sprintf(ev->note, "Loop End/Continue");
    strcat(ev->classStr, " ev-loopend");

    switch (seq->ver.id)
    {
    case SPC_VER_REV1:
    case SPC_VER_REV2:
    case SPC_VER_REV3:
        // do nothing
        break;
    default:
        tr->repIncCount[nestIndex]++;
        break;
    }

    if (tr->repDecCount[nestIndex] == 0)
    {
        akaoSpcAddTrackLoopCount(seq, ev->track, 1);
        repeatAgain = true;
    }
    else
    {
        if (tr->repDecCount[nestIndex] - 1 == 0)
        {
            repeatAgain = false;
        }
        else
        {
            tr->repDecCount[nestIndex]--;
            repeatAgain = true;
        }
    }

    if (!repeatAgain) {
        // repeat end, fall through
        sprintf(ev->note, "Loop End");
        tr->repNestLevel = nestIndex;
    }
    else {
        // repeat again
        sprintf(ev->note, "Loop Continue, count = %d%s", tr->repDecCount[nestIndex], tr->repDecCount[nestIndex] == 0 ? " (infinite)" : "");
        *p = tr->repRetAddr[nestIndex];
    }

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f5: conditional jump in repeat. */
static void akaoSpcEventConditionalJump (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int nestIndex;
    bool doJump;
    int dest;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    dest = (arg2 + seq->apuAddressOffset) & 0xffff;
    nestIndex = (tr->repNestLevel - 1);
    if (nestIndex < 0)
    {
        //fprintf(stderr, "Warning: Repeat Nest Level Overflow at $%04X\n", ev->addr);
        nestIndex += tr->repNestLevelMax;
    }
    nestIndex %= tr->repNestLevelMax;

    switch (seq->ver.id)
    {
    case SPC_VER_REV1:
    case SPC_VER_REV2:
    case SPC_VER_REV3:
        tr->repIncCount[nestIndex]++;
        break;
    default:
        // do nothing
        break;
    }

    doJump = (arg1 == tr->repIncCount[nestIndex]);
    if (doJump)
    {
        if (tr->repDecCount[nestIndex] - 1 == 0)
        {
            // last time, decrement stack ptr (finish loop)
            tr->repNestLevel = nestIndex;
        }
        *p = dest;
    }

    sprintf(ev->note, "Conditional Jump, count = %d, dest = $%04X, jump = %s", arg1, dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-loopbranch");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fc: reset repeat conditional counter. */
static void akaoSpcEventResetRepeatIncCount (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int nestIndex;

    nestIndex = (tr->repNestLevel - 1);
    if (nestIndex < 0)
    {
        //fprintf(stderr, "Warning: Repeat Nest Level Overflow at $%04X\n", ev->addr);
        nestIndex += tr->repNestLevelMax;
    }
    nestIndex %= tr->repNestLevelMax;

    tr->repIncCount[nestIndex] = 0;

    sprintf(ev->note, "Reset Repeat Conditional Count");
    strcat(ev->classStr, " ev-loopetc");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fb: cpu-controled jump. */
static void akaoSpcEventCPUControledJump (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool doJump;
    int dest;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    dest = (arg1 + seq->apuAddressOffset) & 0xffff;

    doJump = false;
    if (seq->ver.cpuCtledJumpFlgAddr >= 0)
    {
        int flag = seq->aRAM[seq->ver.cpuCtledJumpFlgAddr];
        int mask = 1 << ev->track;
        if (mask & flag)
        {
            doJump = true; // TODO: really?
        }
    }
    if (doJump)
    {
        *p = dest;
    }

    sprintf(ev->note, "CPU Controled Jump, dest = $%04X, jump = %s", dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-cpucontroledjump");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e4: slur on. */
static void akaoSpcEventSlurOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Slur On");
    strcat(ev->classStr, " ev-sluron");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e5: slur off. */
static void akaoSpcEventSlurOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Slur Off");
    strcat(ev->classStr, " ev-sluroff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e6: legato on. */
static void akaoSpcEventLegatoOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Legato On");
    strcat(ev->classStr, " ev-legatoon");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e7: legato off. */
static void akaoSpcEventLegatoOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Legato Off");
    strcat(ev->classStr, " ev-legatooff");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e8: force next note lenth. */
static void akaoSpcEventForceNextNoteLen (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Force Next Note Length, tick = %d", arg1);
    strcat(ev->classStr, " ev-notelen");

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->forceNextNoteLen = arg1;
}

/** vcmd e9: play SFX #1. */
static void akaoSpcEventPlaySFX1 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Redirect to SFX #1, index = %d", arg1);
    strcat(ev->classStr, " ev-playsfx");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // actually, this command will move voice pointer to a certain address,
    // the destination address is determined by a table and index (arg1).
    // it's probably not used in usual songs, and it will never return to
    // the main score data (perhaps), thus, the tool ends conversion here.
    fprintf(stderr, "Redirect to SFX #1 (index = %d) at $%04X [Track %d]\n", arg1, ev->addr, ev->track + 1);
    akaoSpcInactiveTrack(seq, ev->track);
}

/** vcmd ea: play SFX #2. */
static void akaoSpcEventPlaySFX2 (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Redirect to SFX #2, index = %d\n", arg1);
    strcat(ev->classStr, " ev-playsfx");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // actually, this command will move voice pointer to a certain address,
    // the destination address is determined by a table and index (arg1).
    // it's probably not used in usual songs, and it will never return to
    // the main score data (perhaps), thus, the tool ends conversion here.
    fprintf(stderr, "Redirect to SFX #2 (index = %d) at $%04X [Track %d]\n", arg1, ev->addr, ev->track + 1);
    akaoSpcInactiveTrack(seq, ev->track);
}

/** vcmd eb: end of track. */
static void akaoSpcEventEndOfTrack (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    akaoSpcInactiveTrack(seq, ev->track);

    //if (!akaoSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd xx: end of track (duplicated). */
static void akaoSpcEventEndOfTrackDup (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "End of Track (Duplicated)");
    strcat(ev->classStr, " ev-end");

    akaoSpcInactiveTrack(seq, ev->track);
    fprintf(stderr, "Warning: End of Track $%02X (duplicated) at $%04X (possible unknown event) [Track %d]\n", ev->code, ev->addr, ev->track + 1);

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f0: set tempo. */
static void akaoSpcEventSetTempo (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;

    sprintf(ev->note, "Set Tempo, tempo = %.1f", akaoSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, akaoSpcTempo(seq));
}

/** vcmd f1: set tempo fade. */
static void akaoSpcEventSetTempoFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    if (seq->ver.id == SPC_VER_REV1)
    {
        ev->size++;
        arg1 = mget2l(&seq->aRAM[*p]);
        (*p) += 2;
    }
    else
    {
        arg1 = seq->aRAM[*p];
        (*p)++;
    }
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tempo Fade, fade speed = %d, tempo = %.1f", arg1, akaoSpcTempoOf(seq, arg2));
    strcat(ev->classStr, " ev-tempo");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    if (arg1 == 0)
    {
        seq->tempo = arg2;
        smfInsertTempoBPM(seq->smf, ev->tick, 0, akaoSpcTempo(seq));
    }
    else
    {
        valueFrom = seq->tempo;
        valueTo = arg2;
        for (faderStep = 1; faderStep <= arg1; faderStep++)
        {
            faderPos = (double)faderStep / arg1;
            faderValue = (int)(valueTo * faderPos + valueFrom * (1.0 - faderPos)); // alphablend
            if (seq->tempo != faderValue)
            {
                seq->tempo = faderValue;
                smfInsertTempoBPM(seq->smf, ev->tick + faderStep, 0, akaoSpcTempo(seq));
            }
        }
    }
}

/** vcmd f1: set echo volume. */
static void akaoSpcEventEchoVolume (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-echovol");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f2: set echo volume fade. */
static void akaoSpcEventEchoVolumeFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Volume Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echovolfade");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f6: jump. */
static void akaoSpcEventJump (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);

    dest = (arg1 + seq->apuAddressOffset) & 0xffff;

    sprintf(ev->note, "Jump, dest = $%04X", dest);
    strcat(ev->classStr, " ev-jump");

    // assumes backjump = loop
    if (dest < *p) {
        akaoSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = dest;

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f7: set echo feedback. */
static void akaoSpcEventEchoFeedback (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Feedback, vol = %d", arg1);
    strcat(ev->classStr, " ev-echoparam");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd xx: set echo feedback fade. */
static void akaoSpcEventEchoFeedbackFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Feedback Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echoparam");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f8: set echo FIR. */
static void akaoSpcEventEchoFIR (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo FIR, index = %d", arg1);
    strcat(ev->classStr, " ev-echoparam");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd xx: set echo FIR fade. */
static void akaoSpcEventEchoFIRFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo FIR Fade, speed = %d, index = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echoparam");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd xx: set echo feedback, FIR. */
static void akaoSpcEventEchoFeedbackFIR (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Feedback/FIR, feedback = %d, FIR = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echoparam");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fb: rhythm channel on. */
static void akaoSpcEventRhythmOn (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    strcpy(ev->note, "Rhythm Channel On");
    strcat(ev->classStr, " ev-rhythmon");
    tr->rhythmChannel = true;

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // put program change to SMF (better than nothing)
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[255].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[255].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[255].patchNo);
}

/** vcmd fc: rhythm channel off. */
static void akaoSpcEventRhythmOff (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    strcpy(ev->note, "Rhythm Channel Off");
    strcat(ev->classStr, " ev-rhythmoff");
    tr->rhythmChannel = false;

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dc: set software envelope (old). */
static void akaoSpcEventSetSoftEnvPattern (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Envelope (Software), pattern = %d", arg1);
    strcat(ev->classStr, " ev-softenv");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dd: set GAIN-based release rate (old). */
static void akaoSpcEventSetGAINReleaseRate (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Release Rate, GAIN = %d", arg1 & 0x1f);
    strcat(ev->classStr, " ev-softenv");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd de: set GAIN-based sustain rate (old). */
static void akaoSpcEventSetGAINSustainRate (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int rate;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    rate = arg1;
    if (rate == 0 || rate > 100)
        rate = 100;

    sprintf(ev->note, "Set Duration Rate, rate = %d %%", rate);
    strcat(ev->classStr, " ev-softenv");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** set pointers of each event. */
static void akaoSpcSetEventList (AkaoSpcSeqStat *seq)
{
    int code;
    AkaoSpcEvent *event = seq->ver.event;
    byte vcmdFirst = seq->ver.vcmdFirstByte;
    bool needsAutoEventAssign = true;
    byte autoAssignFirstVCmdOfs = 0x30;
    int endOfTrackVcmdAddr = -1;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (AkaoSpcEvent) akaoSpcEventUnidentified;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    for(code = 0x00; code < vcmdFirst; code++) {
        event[code] = (AkaoSpcEvent) akaoSpcEventNote;
    }

    if (seq->ver.id == SPC_VER_REV1)
    {
        event[0xd2] = akaoSpcEventSetTempoFade; // tempo
        event[0xd3] = akaoSpcEventNOP2;
        event[0xd4] = akaoSpcEventEchoVolume;
        event[0xd5] = akaoSpcEventEchoFeedbackFIR;
        event[0xd6] = akaoSpcEventPitchEnvelopeOn;
        event[0xd7] = akaoSpcEventTremoloOn;
        event[0xd8] = akaoSpcEventVibratoOn;
        event[0xd9] = akaoSpcEventPanLFOOn3;
        event[0xda] = akaoSpcEventSetOctave;
        event[0xdb] = akaoSpcEventInstrument;
        event[0xdc] = akaoSpcEventSetSoftEnvPattern;
        event[0xdd] = akaoSpcEventSetGAINReleaseRate;
        event[0xde] = akaoSpcEventSetGAINSustainRate;
        event[0xdf] = akaoSpcEventNoiseFreq;
        event[0xe0] = akaoSpcEventLoopStart;
        event[0xe1] = akaoSpcEventOctaveUp;
        event[0xe2] = akaoSpcEventOctaveDown;
        event[0xe3] = akaoSpcEventNOP;
        event[0xe4] = akaoSpcEventNOP;
        event[0xe5] = akaoSpcEventNOP;
        event[0xe6] = akaoSpcEventPitchEnvelopeOff;
        event[0xe7] = akaoSpcEventTremoloOff;
        event[0xe8] = akaoSpcEventVibratoOff;
        event[0xe9] = akaoSpcEventPanLFOOff;
        event[0xea] = akaoSpcEventEchoOn;
        event[0xeb] = akaoSpcEventEchoOff;
        event[0xec] = akaoSpcEventNoiseOn; // d0
        event[0xed] = akaoSpcEventNoiseOff;
        event[0xee] = akaoSpcEventPitchModOn;
        event[0xef] = akaoSpcEventPitchModOff;
        event[0xf0] = akaoSpcEventLoopEnd;
        event[0xf1] = akaoSpcEventEndOfTrack;
        endOfTrackVcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((0xf1 - vcmdFirst) * 2)]);
        event[0xf2] = akaoSpcEventVolumeFade;
        event[0xf3] = akaoSpcEventPanpotFade;
        event[0xf4] = akaoSpcEventJump;
        event[0xf5] = akaoSpcEventConditionalJump;
        event[0xf6] = akaoSpcEventUnknown0; // cpu-controled jump?
        event[0xf7] = akaoSpcEventEndOfTrackDup;
        event[0xf8] = akaoSpcEventEndOfTrackDup;
        event[0xf9] = akaoSpcEventEndOfTrackDup;
        event[0xfa] = akaoSpcEventEndOfTrackDup;
        event[0xfb] = akaoSpcEventEndOfTrackDup;
        event[0xfc] = akaoSpcEventEndOfTrackDup;
        event[0xfd] = akaoSpcEventEndOfTrackDup;
        event[0xfe] = akaoSpcEventEndOfTrackDup;
        event[0xff] = akaoSpcEventEndOfTrackDup;
        autoAssignFirstVCmdOfs = 0;
    }
    else if (seq->ver.id == SPC_VER_REV2)
    {
        event[0xd2] = akaoSpcEventSetTempo;
        event[0xd3] = akaoSpcEventSetTempoFade;
        event[0xd4] = akaoSpcEventVolume;
        event[0xd5] = akaoSpcEventVolumeFade;
        event[0xd6] = akaoSpcEventPanpot;
        event[0xd7] = akaoSpcEventPanpotFade;
        event[0xd8] = akaoSpcEventEchoVolume;
        event[0xd9] = akaoSpcEventEchoVolumeFade;
        event[0xda] = akaoSpcEventTransposeAbs;
        event[0xdb] = akaoSpcEventPitchEnvelopeOn;
        event[0xdc] = akaoSpcEventPitchEnvelopeOff;
        event[0xdd] = akaoSpcEventTremoloOn;
        event[0xde] = akaoSpcEventTremoloOff;
        event[0xdf] = akaoSpcEventVibratoOn;
        event[0xe0] = akaoSpcEventVibratoOff;
        event[0xe1] = akaoSpcEventNoiseFreq;
        event[0xe2] = akaoSpcEventNoiseOn;
        event[0xe3] = akaoSpcEventNoiseOff;
        event[0xe4] = akaoSpcEventPitchModOn;
        event[0xe5] = akaoSpcEventPitchModOff;
        event[0xe6] = akaoSpcEventEchoFeedbackFIR;
        event[0xe7] = akaoSpcEventEchoOn;
        event[0xe8] = akaoSpcEventEchoOff;
        event[0xe9] = akaoSpcEventPanLFOOn;
        event[0xea] = akaoSpcEventPanLFOOff;
        event[0xeb] = akaoSpcEventSetOctave;
        event[0xec] = akaoSpcEventOctaveUp;
        event[0xed] = akaoSpcEventOctaveDown;
        event[0xee] = akaoSpcEventLoopStart;
        event[0xef] = akaoSpcEventLoopEnd;
        event[0xf0] = akaoSpcEventConditionalJump;
        event[0xf1] = akaoSpcEventJump;
        event[0xf2] = akaoSpcEventSlurOn;
        event[0xf3] = akaoSpcEventInstrument;
        event[0xf4] = akaoSpcEventSetSoftEnvPattern;
        event[0xf5] = akaoSpcEventSlurOff;
        event[0xf6] = akaoSpcEventUnknown2; // cpu-controled jump?
        event[0xf7] = akaoSpcEventDetune;
        event[0xf8] = akaoSpcEventEndOfTrack;
        endOfTrackVcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((0xf8 - vcmdFirst) * 2)]);
        event[0xf9] = akaoSpcEventEndOfTrackDup;
        event[0xfa] = akaoSpcEventEndOfTrackDup;
        event[0xfb] = akaoSpcEventEndOfTrackDup;
        event[0xfc] = akaoSpcEventEndOfTrackDup;
        event[0xfd] = akaoSpcEventEndOfTrackDup;
        event[0xfe] = akaoSpcEventEndOfTrackDup;
        event[0xff] = akaoSpcEventEndOfTrackDup;
        autoAssignFirstVCmdOfs = 0;
    }
    else
    {
        event[vcmdFirst + 0x00] = akaoSpcEventVolume; // c4
        event[vcmdFirst + 0x01] = akaoSpcEventVolumeFade;
        event[vcmdFirst + 0x02] = akaoSpcEventPanpot;
        event[vcmdFirst + 0x03] = akaoSpcEventPanpotFade;
        event[vcmdFirst + 0x04] = akaoSpcEventPitchSlide;
        event[vcmdFirst + 0x05] = akaoSpcEventVibratoOn;
        event[vcmdFirst + 0x06] = akaoSpcEventVibratoOff;
        event[vcmdFirst + 0x07] = akaoSpcEventTremoloOn;
        event[vcmdFirst + 0x08] = akaoSpcEventTremoloOff;
        event[vcmdFirst + 0x09] = akaoSpcEventPanLFOOn;
        event[vcmdFirst + 0x0a] = akaoSpcEventPanLFOOff;
        event[vcmdFirst + 0x0b] = akaoSpcEventNoiseFreq;
        event[vcmdFirst + 0x0c] = akaoSpcEventNoiseOn; // d0
        event[vcmdFirst + 0x0d] = akaoSpcEventNoiseOff;
        event[vcmdFirst + 0x0e] = akaoSpcEventPitchModOn;
        event[vcmdFirst + 0x0f] = akaoSpcEventPitchModOff;
        event[vcmdFirst + 0x10] = akaoSpcEventEchoOn;
        event[vcmdFirst + 0x11] = akaoSpcEventEchoOff;
        event[vcmdFirst + 0x12] = akaoSpcEventSetOctave;
        event[vcmdFirst + 0x13] = akaoSpcEventOctaveUp;
        event[vcmdFirst + 0x14] = akaoSpcEventOctaveDown;
        event[vcmdFirst + 0x15] = akaoSpcEventTransposeAbs;
        event[vcmdFirst + 0x16] = akaoSpcEventTransposeRel;
        event[vcmdFirst + 0x17] = akaoSpcEventDetune;
        event[vcmdFirst + 0x18] = akaoSpcEventInstrument;
        event[vcmdFirst + 0x19] = akaoSpcEventSetAR;
        event[vcmdFirst + 0x1a] = akaoSpcEventSetDR;
        event[vcmdFirst + 0x1b] = akaoSpcEventSetSL;
        event[vcmdFirst + 0x1c] = akaoSpcEventSetSR; // e0
        event[vcmdFirst + 0x1d] = akaoSpcEventSetDefaultADSR;
        event[vcmdFirst + 0x1e] = akaoSpcEventLoopStart;
        event[vcmdFirst + 0x1f] = akaoSpcEventLoopEnd;

        if (seq->ver.id == SPC_VER_REV3)
        {
            event[vcmdFirst + 0x20] = akaoSpcEventEndOfTrack; // f2
            endOfTrackVcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0x2d * 2)]);
            event[vcmdFirst + 0x21] = akaoSpcEventSetTempo;
            event[vcmdFirst + 0x22] = akaoSpcEventSetTempoFade;
            event[vcmdFirst + 0x23] = akaoSpcEventEchoVolume; // Seiken Densetsu 2 - NOP xx
            event[vcmdFirst + 0x24] = akaoSpcEventEchoVolumeFade; // Seiken Densetsu 2 - NOP xx yy
            event[vcmdFirst + 0x25] = akaoSpcEventEchoFeedbackFIR; // Seiken Densetsu 2 - NOP xx yy
            event[vcmdFirst + 0x26] = akaoSpcEventMasterVolume;
            event[vcmdFirst + 0x27] = akaoSpcEventConditionalJump;
            event[vcmdFirst + 0x28] = akaoSpcEventJump;
            event[vcmdFirst + 0x29] = akaoSpcEventCPUControledJump;
            autoAssignFirstVCmdOfs = 0x20;
        }
        else
        {
            event[vcmdFirst + 0x20] = akaoSpcEventSlurOn;
            event[vcmdFirst + 0x21] = akaoSpcEventSlurOff;
            event[vcmdFirst + 0x22] = akaoSpcEventLegatoOn;
            event[vcmdFirst + 0x23] = akaoSpcEventLegatoOff;
            event[vcmdFirst + 0x24] = akaoSpcEventForceNextNoteLen;
            event[vcmdFirst + 0x25] = akaoSpcEventPlaySFX1;
            event[vcmdFirst + 0x26] = akaoSpcEventPlaySFX2;
            //event[vcmdFirst + 0x27] = akaoSpcEventEndOfTrack; // use autodetect, there is a difference between RS2 and Gun Hazard
            //endOfTrackVcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0x27 * 2)]);
            //event[vcmdFirst + 0x28] = akaoSpcEventEndOfTrackDup;
            //event[vcmdFirst + 0x29] = akaoSpcEventEndOfTrackDup;
            //event[vcmdFirst + 0x2a] = akaoSpcEventEndOfTrackDup;
            //event[vcmdFirst + 0x2b] = akaoSpcEventEndOfTrackDup;
            event[vcmdFirst + 0x2c] = akaoSpcEventSetTempo; // f0
            event[vcmdFirst + 0x2d] = akaoSpcEventSetTempoFade;
            event[vcmdFirst + 0x2e] = akaoSpcEventEchoVolume;
            event[vcmdFirst + 0x2f] = akaoSpcEventEchoVolumeFade;
            autoAssignFirstVCmdOfs = 0x28;
        }

        switch (seq->ver.subId)
        {
        case SPC_SUBVER_SD2:
            event[0xfc] = akaoSpcEventResetRepeatIncCount;
            break;
        }
    }

    if (needsAutoEventAssign)
    {
        // assign events by using length table
        // (some variable-length vcmds and jumps can be wrong)
        int vcmdIndex;

        if (endOfTrackVcmdAddr == -1)
        {
            if (seq->ver.id == SPC_VER_REV4 &&
                mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0x27 * 2)]) == mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0x28 * 2)])) // rev.4 common
            {
                event[vcmdFirst + 0x27] = akaoSpcEventEndOfTrack;
            }
            else if (seq->ver.id == SPC_VER_REV4 &&
                mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0x28 * 2)]) == mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + (0x29 * 2)])) // Gun Hazard, BS Games
            {
                event[vcmdFirst + 0x28] = akaoSpcEventEndOfTrack;
            }
            // assume: last 2 events are end of track (duplicated)
            else if (mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((0xff - vcmdFirst) * 2)]) == mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((0xfe - vcmdFirst) * 2)]) &&
                seq->aRAM[seq->ver.vcmdLenTableAddr + (0xff - vcmdFirst)] == seq->aRAM[seq->ver.vcmdLenTableAddr + (0xfe - vcmdFirst)] &&
                seq->aRAM[seq->ver.vcmdLenTableAddr + (0xff - vcmdFirst)] == 0)
            {
                endOfTrackVcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((0xff - vcmdFirst) * 2)]);
                //fprintf(stderr, "Info: Auto detected an event \"End of Track\" - $%04X\n", endOfTrackVcmdAddr);

            }
        }

        for (vcmdIndex = vcmdFirst + autoAssignFirstVCmdOfs; vcmdIndex <= 0xff; vcmdIndex++)
        {
            int vcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((vcmdIndex - vcmdFirst) * 2)]);
            if (event[vcmdIndex] == akaoSpcEventUnidentified)
            {
                int len = seq->aRAM[seq->ver.vcmdLenTableAddr + vcmdIndex - vcmdFirst];
                switch(len)
                {
                    case 0:
                        if (endOfTrackVcmdAddr != -1 &&
                            vcmdAddr == endOfTrackVcmdAddr)
                        {
                            event[vcmdIndex] = akaoSpcEventEndOfTrackDup;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"End of Track (duplicated)\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        else if (seq->aRAM[vcmdAddr] == 0x6f) // ret
                        {
                            event[vcmdIndex] = akaoSpcEventNOP;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"NOP\" (it sometimes can have an effect though) - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Seiken Densetsu 2)
                        // ; vcmd fc
                        // cmp   x,#$10
                        // bcs   $0943
                        // mov   a,#$00
                        // mov   $fea0+x,a
                        // ret
                        else if (vcmdAddr + 10 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc8\x10\xb0\x05\xe8\\\x00\xd5..\x6f", 10, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown0;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Reset Repeat Conditional Counter\" (mov $%04x+x,#0)? Apparently it is not a channel message - $%04X\n", vcmdIndex, mget2l(&seq->aRAM[vcmdAddr + 7]), vcmdAddr);
                        }
                        // (Live A Live)
                        // ; vcmd f8 - increment cpu-shared counter
                        // inc   $66
                        // ret
                        else if (vcmdAddr + 3 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xab.\x6f", 3, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown0;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Increment CPU-shared Counter\" (inc $%02x)? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 1], vcmdAddr);
                        }
                        // (Live A Live)
                        // ; vcmd f9 - zero cpu-shared counter
                        // mov   $66,#$00
                        // ret
                        else if (vcmdAddr + 4 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\x8f\\\x00.\x6f", 4, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown0;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Zero CPU-shared Counter\" (mov $%02x,#0)? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 2], vcmdAddr);
                        }
                        // (Romancing SaGa 3)
                        // ; vcmd fb - rhythm kit on
                        // or    ($7c),($92)
                        // ret
                        // ; vcmd fc - rhythm kit off
                        // mov   a,$92
                        // tclr1 $007c
                        // ret
                        else if (vcmdAddr + 10 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\x09..\x6f\xe4.\x4e.\\\x00\x6f", 10, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 1] == seq->aRAM[vcmdAddr + 5] &&
                            seq->aRAM[vcmdAddr + 2] == seq->aRAM[vcmdAddr + 7])
                        {
                            event[vcmdIndex] = akaoSpcEventRhythmOn;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Rhythm Kit On\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Romancing SaGa 3)
                        // ; vcmd fb - rhythm kit on
                        // or    ($7c),($92)
                        // ret
                        // ; vcmd fc - rhythm kit off
                        // mov   a,$92
                        // tclr1 $007c
                        // ret
                        else if (vcmdAddr >= 4 && vcmdAddr + 6 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr - 4], "\x09..\x6f\xe4.\x4e.\\\x00\x6f", 10, NULL) != -1 &&
                            seq->aRAM[vcmdAddr - 4 + 1] == seq->aRAM[vcmdAddr - 4 + 5] &&
                            seq->aRAM[vcmdAddr - 4 + 2] == seq->aRAM[vcmdAddr - 4 + 7])
                        {
                            event[vcmdIndex] = akaoSpcEventRhythmOff;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Rhythm Kit Off\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Live A Live)
                        // ; vcmd fa - ignore master volume (per channel)
                        // or    ($61),($8f)
                        // ret
                        else if (vcmdAddr + 4 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\x09..\x6f", 4, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown0;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Ignore Master Volume\" (or ($%02x),($%02x))? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 2], seq->aRAM[vcmdAddr + 1], vcmdAddr);
                        }
                        // (Romancing SaGa 2)
                        // ; vcmd fa - mute channel
                        // cmp   x,#$10
                        // bcs   $14b1
                        // or    ($61),($8e)
                        // ret
                        else if (vcmdAddr + 8 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc8\x10\xb0\x03\x09..\x6f", 8, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown0;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Mute Channel\" (or ($%02x),($%02x))? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 6], seq->aRAM[vcmdAddr + 5], vcmdAddr);
                        }
                        else
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown0;
                        }
                        break;
                    case 1:
                        // (Final Fantasy 5)
                        // ; vcmd f5 - echo volume
                        // cmp   x,#$10
                        // bcs   $06f6
                        // lsr   a
                        // mov   $b4,a             ; echo volume shadow = op1/2
                        // mov   $b3,#$00
                        // mov   $b7,a
                        // ret
                        if (vcmdAddr + 13 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc8\x10\xb0\x08\\\x5c\xc4.\x8f\\\x00.\xc4.\x6f", 13, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 6] == seq->aRAM[vcmdAddr + 9] + 1)
                        {
                            event[vcmdIndex] = akaoSpcEventEchoVolume;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Volume\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 5)
                        // ; vcmd f8 - master volume
                        // mov   $b8,a
                        // ret
                        else if (vcmdAddr + 3 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x6f", 3, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown1;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Master Volume\" (mov $%02x,a)? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 1], vcmdAddr);
                        }
                        // (Chrono Trigger)
                        // ; vcmd f4
                        // mov   $4d,a
                        // or    ($d0),($53)
                        // ret
                        else if (vcmdAddr + 6 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x09..\x6f", 6, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown1;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Master Volume\" (mov $%02x,a)? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 1], vcmdAddr);
                        }
                        // (Chrono Trigger)
                        // ; vcmd fd - expression
                        // asl   a
                        // mov   $f220+x,a
                        // or    ($d0),($91)
                        // ret
                        else if (vcmdAddr + 8 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\x1c\xd5..\x09..\x6f", 8, NULL) != -1)
                        {
                            //event[vcmdIndex] = akaoSpcEventAltVolume;
                            event[vcmdIndex] = akaoSpcEventUnknown1;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Volume (Alternate)\" (mov $%04x,a*2)? Apparently it is not a channel message - $%04X\n", vcmdIndex, mget2l(&seq->aRAM[vcmdAddr + 2]), vcmdAddr);
                        }
                        // (Romancing SaGa 3)
                        // ; vcmd f7 - echo feedback
                        // bbs7  $89,$18d9
                        // mov   a,#$30
                        // mov   y,$00e8
                        // mov   a,#$00
                        // mov   $78,a
                        // mov   $8f,a
                        // mov   a,$a6
                        // mov   y,$78
                        // beq   $18f8
                        // eor   a,#$80
                        // not1  $0076,7
                        // setc
                        // sbc   a,$76
                        // not1  $0076,7
                        // call  $0f38
                        // mov   x,$a7
                        // movw  $79,ya
                        // ret
                        else if (vcmdAddr + 37 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xe3.\x03\xe8\x30\xec\xe8\\\x00\xc4.\xc4.\xe4.\xeb.\xf0\x13\x48\x80\xea.\xe0\x80\xa4.\xea.\xe0\x3f..\xf8.\xda.\x6f", 37, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 21] == seq->aRAM[vcmdAddr + 25] &&
                            seq->aRAM[vcmdAddr + 21] == seq->aRAM[vcmdAddr + 27])
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFeedback;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Feedback\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Romancing SaGa 3)
                        // ; vcmd f8 - echo FIR
                        // bbs7  $89,$1901
                        // mov   a,#$30
                        // mov   y,$00e8
                        // mov   a,#$00
                        // mov   $77,a
                        // mov   $8f,a
                        // mov   a,$a6
                        // and   a,#$03
                        // inc   a
                        // asl   a
                        // asl   a
                        // asl   a
                        // mov   y,a
                        // mov   x,#$10
                        else if (vcmdAddr + 23 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xe3.\x03\xe8\x30\xec\xe8\\\x00\xc4.\xc4.\xe4.\x28\x03\xbc\x1c\x1c\x1c\xfd\xcd\x10", 23, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFIR;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo FIR\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Chrono Trigger, Romancing SaGa 3)
                        // ; vcmd f9 - (related to vcmd fa)
                        // and   a,#$0f
                        // mov   $7b,a
                        // ret
                        else if (vcmdAddr + 5 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\x28\x0f\xc4.\x6f", 5, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown1;
                            fprintf(stderr, "Info: Event $%02X is possibly \"Set CPU-shared Var Value (Rev.4 New)\" (mov $%02x,a&#15)? Apparently it is not a channel message - $%04X\n", vcmdIndex, seq->aRAM[vcmdAddr + 3], vcmdAddr);
                        }
                        else if (seq->aRAM[vcmdAddr] == 0x6f) // ret
                        {
                            event[vcmdIndex] = akaoSpcEventNOP;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"NOP (2 bytes)\" (it sometimes can have an effect though) - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        else
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown1;
                        }
                        break;

                    case 2:
                        // (Romancing SaGa 3)
                        // mov   y,a
                        // call  $0747
                        // mov   a,y
                        // mov   y,$a6
                        // addw  ya,$00
                        // mov   $02+x,a
                        // mov   $03+x,y
                        // ret
                        if (vcmdAddr + 14 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xdd\xeb.\x7a.\xd4.\xdb.\x6f", 14, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 10] + 1 == seq->aRAM[vcmdAddr + 12])
                        {
                            event[vcmdIndex] = akaoSpcEventJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Jump\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 5)
                        // ; vcmd fa - goto
                        // mov   y,a
                        // call  $059c
                        // cmp   x,#$10
                        // bcs   $09a7
                        // mov   a,y
                        // mov   y,$04
                        // addw  ya,$06            ; add voffset
                        // mov   $0c+x,a
                        // mov   $0d+x,y
                        // ret
                        else if (vcmdAddr + 18 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xc8\x10\xb0.\xdd\xeb.\x7a.\xd4.\xdb.\x6f", 18, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 14] + 1 == seq->aRAM[vcmdAddr + 16])
                        {
                            event[vcmdIndex] = akaoSpcEventJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Jump\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 4)
                        // ; vcmd f4 - goto
                        // mov   y,a
                        // call  $0c85
                        // mov   $02+x,y
                        // mov   $03+x,a
                        // ret
                        else if (vcmdAddr + 9 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xdb.\xd4.\x6f", 9, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 5] + 1 == seq->aRAM[vcmdAddr + 7])
                        {
                            event[vcmdIndex] = akaoSpcEventJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Jump\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 5)
                        // ; vcmd f6 - fade echo volume
                        // mov   $b7,a
                        // call  $059c
                        // cmp   x,#$10
                        // bcs   $072f
                        // mov   y,$b7
                        // beq   $06ea
                        // lsr   a
                        // setc
                        // sbc   a,$b4
                        // beq   $06f4
                        else if (vcmdAddr + 19 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc8\x10\xb0.\xeb.\xf0.\\\x5c\x80\xa4.\xf0.", 19, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 1] == seq->aRAM[vcmdAddr + 10])
                        {
                            event[vcmdIndex] = akaoSpcEventEchoVolumeFade;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Volume Fade\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Romancing SaGa 2)
                        // ; vcmd f4 - echo feedback, FIR filter
                        // mov   $97,a
                        // call  $05d8
                        // cmp   x,#$10
                        // bcc   $129b
                        // ret
                        // mov   ($65),($97)
                        // mov   $64,a
                        // mov   a,$64
                        // and   a,#$03
                        // asl   a
                        // asl   a
                        // asl   a
                        // mov   y,a
                        // mov   x,#$0f
                        // mov   a,$168a+y
                        // mov   $f2,x
                        // mov   $f3,a
                        else if (vcmdAddr + 32 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc8\x10\x90\x01\x6f\xfa..\xc4.\xe4.\x28\x03\x1c\x1c\x1c\xfd\xcd\x0f\xf6..\xd8\xf2\xc4\xf3", 32, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 14] == seq->aRAM[vcmdAddr + 16])
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFeedbackFIR;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Feedback/FIR\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Live A Live)
                        // ; vcmd f4 - echo feedback, FIR filter
                        // mov   $65,a
                        // call  $0590
                        // mov   $64,a
                        // mov1  c,$1010,5
                        // bcc   $133f
                        // ret
                        // mov   a,$64
                        // and   a,#$03
                        // asl   a
                        // asl   a
                        // asl   a
                        // mov   y,a
                        // mov   x,#$0f
                        // mov   a,$1826+y
                        // mov   $f2,x
                        // mov   $f3,a
                        else if (vcmdAddr + 30 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc4.\xaa..\x90\x01\x6f\xe4.\x28\x03\x1c\x1c\x1c\xfd\xcd\x0f\xf6..\xd8\xf2\xc4\xf3", 30, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 6] == seq->aRAM[vcmdAddr + 14])
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFeedbackFIR;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Feedback/FIR\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 5)
                        // ; vcmd f7 - set echo feedback, filter
                        // mov   $d5,a             ; echo feedback shadow
                        // call  $059c
                        // cmp   x,#$10
                        // bcc   $0795
                        // ret
                        // and   a,#$03
                        // mov   $d4,a             ; filter shadow
                        // asl   a
                        // asl   a
                        // asl   a
                        // mov   y,a
                        // mov   x,#$0f
                        // mov   a,$190f+y         ; echo filter coeff
                        // mov   $f2,x
                        // mov   $f3,a
                        else if (vcmdAddr + 27 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc8\x10\x90\x01\x6f\x28\x03\xc4.\x1c\x1c\x1c\xfd\xcd\x0f\xf6..\xd8\xf2\xc4\xf3", 27, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFeedbackFIR;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Feedback/FIR\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 5)
                        // ; vcmd fb - goto if vbit set in D1
                        // mov   y,a
                        // call  $059c
                        // cmp   x,#$10
                        // bcs   $0a41
                        // mov   a,$bf
                        // and   a,$d1
                        // beq   $0a41
                        // tclr1 $00d1
                        // mov   a,y
                        // mov   y,$04
                        // addw  ya,$06
                        // mov   $0c+x,a
                        // mov   $0d+x,y
                        // ret
                        else if (vcmdAddr + 27 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xc8\x10\xb0\x12\xe4.\x24.\xf0\x0c\x4e..\xdd\xeb.\x7a.\xd4.\xdb.\x6f", 27, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 23] + 1 == seq->aRAM[vcmdAddr + 25])
                        {
                            event[vcmdIndex] = akaoSpcEventCPUControledJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"CPU Controled Jump\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 6)
                        // ; vcmd fc - branch if voice bit in $dd set
                        // mov   y,a
                        // call  $05c9
                        // mov   a,$8f
                        // and   a,$dd
                        // beq   $176f
                        // tclr1 $00dd
                        // mov   a,y
                        // mov   y,$a2
                        // addw  ya,$00
                        // mov   $02+x,a
                        // mov   $03+x,y
                        // ret
                        else if (vcmdAddr + 23 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xe4.\x24.\xf0\x0c\x4e..\xdd\xeb.\x7a.\xd4.\xdb.\x6f", 23, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 19] + 1 == seq->aRAM[vcmdAddr + 21])
                        {
                            event[vcmdIndex] = akaoSpcEventCPUControledJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"CPU Controled Jump\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Live A Live)
                        // ; vcmd fb - branch if voice bit in $dd set
                        // mov   y,a
                        // call  $0590
                        // mov   a,$8f
                        // and   a,$dd
                        // beq   $17ec
                        // mov   a,y
                        // mov   y,$a2
                        // addw  ya,$00
                        // mov   $02+x,a
                        // mov   $03+x,y
                        // ret
                        else if (vcmdAddr + 20 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xe4.\x24.\xf0\x09\xdd\xeb.\x7a.\xd4.\xdb.\x6f", 20, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 16] + 1 == seq->aRAM[vcmdAddr + 18])
                        {
                            event[vcmdIndex] = akaoSpcEventCPUControledJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"CPU Controled Jump\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 6, Chrono Trigger (slightly different))
                        // ; vcmd f7 - set/fade echo feedback
                        // mov   $78,a
                        // mov   $8c,a
                        // call  $05c9
                        // mov   y,$8c
                        // beq   $1298
                        // eor   a,#$80
                        // not1  $1c0e,6
                        // setc
                        // sbc   a,$76             ; echo feedback shadow
                        // not1  $1c0e,6
                        // call  $0cc5
                        // mov   x,$a3
                        // movw  $79,ya
                        else if (vcmdAddr + 29 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\xc4.\x3f..\xeb.\xf0.\x48\x80\xea.\xe0\x80\xa4.\xea.\xe0\x3f..\xf8.\xda.", 29, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFeedbackFade;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Feedback Fade\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        else if (vcmdAddr + 37 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xe3.\x03\xe8\x30\xec\xe8\\\x00\xc4.\xc4.\xe4.\xeb.\xf0\x13\x48\x80\xea.\xe0\x80\xa4.\xea.\xe0\x3f..\xf8.\xda.\x6f", 37, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 21] == seq->aRAM[vcmdAddr + 25] &&
                            seq->aRAM[vcmdAddr + 21] == seq->aRAM[vcmdAddr + 27])
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFeedbackFade;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo Feedback Fade\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }

                        // (Final Fantasy 6)
                        // ; vcmd f8 - set/fade echo FIR filter
                        // mov   $77,a
                        // mov   $8c,a
                        // call  $05c9
                        // and   a,#$03
                        // inc   a
                        // asl   a
                        // asl   a
                        // asl   a
                        // mov   y,a
                        // mov   x,#$10
                        // mov   a,$8c
                        // beq   $12d6
                        // mov   a,#$00
                        // mov   $63+x,a
                        // mov   a,$64+x
                        // eor   a,#$80
                        // mov   $98,a
                        // mov   a,$17a8+y
                        // eor   a,#$80
                        // setc
                        // sbc   a,$98
                        else if (vcmdAddr + 38 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\xc4.\x3f..\x28\x03\xbc\x1c\x1c\x1c\xfd\xcd\x10\xe4.\xf0.\xe8\x00\xd4.\xf4.\x48\x80\xc4.\xf6..\x48\x80\x80\xa4.", 38, NULL) != -1)
                        {
                            event[vcmdIndex] = akaoSpcEventEchoFIRFade;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Echo FIR Fade\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        else
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown2;
                        }
                        break;

                    case 3:
                        // (Romancing SaGa 3)
                        // ; vcmd f5 - conditional jump in repeat
                        // mov   $9e,a             ; arg1 - target repeat count
                        // call  $0747
                        // mov   $9c,a
                        // call  $0747
                        // mov   $9d,a             ; arg2/3 - target address
                        // mov   y,$27+x
                        // mov   a,$f520+y
                        // cbne  $9e,$1ccd         ; do nothing if repeat count doesn't match
                        if (vcmdAddr + 20 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc4.\x3f..\xc4.\xfb.\xf6..\x2e..", 20, NULL) != -1 &&
                            mget2l(&seq->aRAM[vcmdAddr + 3]) == mget2l(&seq->aRAM[vcmdAddr + 8]) &&
                            seq->aRAM[vcmdAddr + 6] + 1 == seq->aRAM[vcmdAddr + 11])
                        {
                            event[vcmdIndex] = akaoSpcEventConditionalJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Conditional Jump (Rev.4)\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 5)
                        // ; vcmd f9 - conditional goto
                        // mov   $36,a
                        // call  $059c
                        // mov   $34,a
                        // call  $059c
                        // mov   $35,a
                        // cmp   x,#$10
                        // bcs   $09cc
                        // mov   y,$5c+x
                        // mov   a,$fca0+y
                        // inc   a
                        // mov   $fca0+y,a         ; inc current rpt ctr
                        // cbne  $36,$09cc         ; return unless rpt == op1
                        else if (vcmdAddr + 28 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc4.\x3f..\xc4.\xc8\x10\xb0.\xfb.\xf6..\xbc\xd6..\x2e..", 28, NULL) != -1 &&
                            mget2l(&seq->aRAM[vcmdAddr + 3]) == mget2l(&seq->aRAM[vcmdAddr + 8]) &&
                            seq->aRAM[vcmdAddr + 6] + 1 == seq->aRAM[vcmdAddr + 11])
                        {
                            event[vcmdIndex] = akaoSpcEventConditionalJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Conditional Jump (Rev.3)\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Seiken Densetsu 2 - slightly different from FF5)
                        // mov   $36,a
                        // call  $0562
                        // mov   $34,a
                        // call  $0562
                        // mov   $35,a
                        // cmp   x,#$10
                        // bcs   $0939
                        // mov   a,$fea0+x
                        // inc   a
                        // mov   $fea0+x,a
                        // cbne  $36,$0939
                        else if (vcmdAddr + 26 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xc4.\x3f..\xc4.\x3f..\xc4.\xc8\x10\xb0.\xf5..\xbc\xd5..\x2e..", 26, NULL) != -1 &&
                            mget2l(&seq->aRAM[vcmdAddr + 3]) == mget2l(&seq->aRAM[vcmdAddr + 8]) &&
                            seq->aRAM[vcmdAddr + 6] + 1 == seq->aRAM[vcmdAddr + 11])
                        {
                            event[vcmdIndex] = akaoSpcEventConditionalJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Conditional Jump (Rev.3 SD2)\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }

                        // (Romancing SaGa)
                        // mov   y,a
                        // call  $0c1f
                        // mov   $4f,a
                        // call  $0c1f
                        // mov   $50,a
                        // cmp   x,#$10
                        // bcs   $1041
                        // mov   a,$03e1+x
                        // inc   a
                        // mov   $03e1+x,a
                        else if (vcmdAddr + 22 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xfd\x3f..\xc4.\x3f..\xc4.\xc8\x10\xb0.\xf5..\xbc\xd5..", 22, NULL) != -1 &&
                            mget2l(&seq->aRAM[vcmdAddr + 2]) == mget2l(&seq->aRAM[vcmdAddr + 7]) &&
                            seq->aRAM[vcmdAddr + 5] + 1 == seq->aRAM[vcmdAddr + 10] &&
                            mget2l(&seq->aRAM[vcmdAddr + 16]) == mget2l(&seq->aRAM[vcmdAddr + 20]))
                        {
                            event[vcmdIndex] = akaoSpcEventConditionalJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Conditional Jump (Rev.2)\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Final Fantasy 4)
                        // mov   a,$0681+x
                        // mov   y,a
                        // mov   a,$0780+y
                        // inc   a
                        // mov   $0780+y,a
                        // cmp   a,$26
                        // beq   $0fd4
                        // call  $0c89
                        // jmp   $0c89
                        else if (vcmdAddr + 21 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\xf5..\xfd\xf6..\xbc\xd6..\x64.\xf0\x06\x3f..\x5f..", 21, NULL) != -1 &&
                            mget2l(&seq->aRAM[vcmdAddr + 16]) == mget2l(&seq->aRAM[vcmdAddr + 19]))
                        {
                            event[vcmdIndex] = akaoSpcEventConditionalJump;
                            fprintf(stderr, "Info: Auto assigned an event $%02X \"Conditional Jump (Rev.1)\" - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        // (Chrono Trigger, Romancing SaGa 3)
                        // ; vcmd fa - CPU-controled branch (used for Magical Tank Battle SFX)
                        // and   a,#$0f
                        // mov   $a6,a
                        // cmp   a,$d4
                        // bcc   $1d9a
                        // mov   $d4,a
                        // mov   a,$d3
                        // cmp   a,$a6
                        // bcs   $1dad
                        // inc   $02+x
                        // bne   $1da6
                        // inc   $03+x
                        // inc   $02+x
                        // bne   $1dac
                        // inc   $03+x
                        else if (vcmdAddr + 28 <= SPC_ARAM_SIZE &&
                            indexOfHexPat(&seq->aRAM[vcmdAddr], "\x28\x0f\xc4.\x64.\x90\x02\xc4.\xe4.\x64.\xb0.\xbb.\xd0\x02\xbb.\xbb.\xd0.\xbb.", 28, NULL) != -1 &&
                            seq->aRAM[vcmdAddr + 17] + 1 == seq->aRAM[vcmdAddr + 21] && seq->aRAM[vcmdAddr + 17] == seq->aRAM[vcmdAddr + 23] && seq->aRAM[vcmdAddr + 21] == seq->aRAM[vcmdAddr + 27])
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown3;
                            fprintf(stderr, "Info: Event $%02X is possibly \"CPU-controled Jump (Rev.4 New)\"? Apparently it is not a channel message - $%04X\n", vcmdIndex, vcmdAddr);
                        }
                        else
                        {
                            event[vcmdIndex] = akaoSpcEventUnknown3;
                        }
                        break;

                    case 4: event[vcmdIndex] = akaoSpcEventUnknown4; break;
                    case 5: event[vcmdIndex] = akaoSpcEventUnknown5; break;
                }
            }
        }
    }
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* akaoSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    AkaoSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newAkaoSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = akaoSpcCreateSmf(seq);

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

            AkaoSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (akaoSpcTextLoopMax == 0 || seq->looped < akaoSpcTextLoopMax)
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
            akaoSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= akaoSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, akaoSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    akaoSpcTruncateNoteAll(seq);
    akaoSpcDequeueNoteAll(seq);

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
        delAkaoSpcSeq(&seq);
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
Smf* akaoSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = akaoSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* akaoSpcToMidiFromFile (const char *filename)
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

    smf = akaoSpcToMidi(data, size);

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
    akaoSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    akaoSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    akaoSpcForceSongListAddr = songListAddr;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (akaoSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    akaoSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    akaoSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset only. */
static bool cmdOptGM1 (void)
{
    akaoSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    akaoSpcMidiResetType = SMF_RESET_GM2;
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
            akaoSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = akaoSpcToMidiFromFile(gArgv[0]);
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
