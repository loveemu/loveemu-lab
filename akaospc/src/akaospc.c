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
#define VERSION "[2013-06-10]"

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
static int akaoSpcMidiResetType = SMF_RESET_GM1;

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
#define SPC_NOTE_KEYSHIFT   24
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
    AkaoSpcEvent event[256];
    PatchFixInfo patchFix[256];
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
    int looped;             // how many times looped (internal)
    byte octave;            // octave
    int patch;              // patch number (for pitch fix)
    bool rhythmChannel;     // rhythm channel / melody channel
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
        akaoSpcPatchFix[patch].bankSelM = 0;
        akaoSpcPatchFix[patch].bankSelL = patch >> 7;
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
    tr->rhythmChannel = false;
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
        seq->ver.patchFix[patch].bankSelM = 0;
        seq->ver.patchFix[patch].bankSelL = patch >> 7;
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
    seq->ver.subId = SPC_SUBVER_UNKNOWN;

    // (Romancing SaGa 2)
    // mov   x,#$0e
    // div   ya,x
    // mov   x,$a7
    // mov   a,$1e7e+y
    if ((noteLenLdCodeAddr = indexOfHexPat(seq->aRAM, "\xcd\x0e\x9e\xf8.\xf6..", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.noteLenTableAddr = mget2l(&seq->aRAM[noteLenLdCodeAddr + 6]);
        seq->ver.noteLenTableLen = 14;
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
        seq->ver.noteLenTableLen = 14;
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
        seq->ver.noteLenTableLen = 15;
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

    // classify
    if (seq->ver.seqHeaderAddr != -1 &&
        seq->ver.noteLenTableAddr != -1 &&
        seq->ver.vcmdTableAddr != -1 &&
        seq->ver.vcmdLenTableAddr != -1)
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

        // detect vcmd mapping (silly...)
        if (seq->ver.vcmdFirstByte == 0x100 - countof(FF4_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], FF4_VCMD_LEN_TABLE, sizeof(FF4_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_FF4;
        }
        else if (seq->ver.vcmdFirstByte == 0x100 - countof(FF5_VCMD_LEN_TABLE) &&
            memcmp(&seq->aRAM[seq->ver.vcmdLenTableAddr], FF5_VCMD_LEN_TABLE, sizeof(FF5_VCMD_LEN_TABLE)) == 0)
        {
            seq->ver.subId = SPC_SUBVER_FF5;
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
        seq->apuAddressOffset = (seq->ver.apuAddressBase - mget2l(&seq->aRAM[seqHeaderReadOfs])) & 0xffff;
        seqHeaderReadOfs += 2;
        if (seq->ver.id == SPC_VER_REV3)
        {
            seqEndOffset = mget2l(&seq->aRAM[seqHeaderReadOfs + SPC_TRACK_MAX * 2]);
        }
        else
        {
            seqEndOffset = mget2l(&seq->aRAM[seqHeaderReadOfs]);
            seqHeaderReadOfs += 2;
        }
    }

    // track list
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
    myprintf("          <li>Note Length Table: $%04X (%d bytes)", seq->ver.noteLenTableAddr, seq->ver.noteLenTableLen);
    myprintf("          <li>Voice Command Table: $%04X", seq->ver.vcmdTableAddr);
    myprintf("          <li>Voice Command Length Table: $%04X", seq->ver.vcmdLenTableAddr);
    myprintf("          <li>First Voice Command Byte: $%02X", seq->ver.vcmdFirstByte);
    if (seq->ver.useROMAddress)
    {
        myprintf("          <li>APU RAM Address Base: $%04X", seq->ver.apuAddressBase);
    }
    //myprintf(" (Song $%02x)", seq->ver.songIndex);
    myprintf("</li>\n");
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
static double akaoSpcTempo (AkaoSpcSeqStat *seq)
{
    return (double) seq->tempo * 60000000 / 55296000; // 55296000 = (timer0) 4.5ms * 48 TPQN * 256
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
    if (value > 0x7f)
    {
        fprintf(stderr, "Warning: Panpot %d is too large\n", value); // older version, work on it later
    }
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
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
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
static void akaoSpcEventNOP (AkaoSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
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

    len = seq->aRAM[seq->ver.noteLenTableAddr + lenIndex];
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

    if (tie)
    {
        if (!akaoSpcLessTextInSMF)
           smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }

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
            //tr->lastNote.vel = tr->note.vel;
            tr->lastNote.transpose = seq->transpose + tr->note.transpose;
            tr->lastNote.patch = tr->note.patch;
        }
        tr->lastNote.tied = tie;
        tr->lastNote.active = true;
    }
    tr->tick += len;
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

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, akaoSpcMidiVolOf(arg1));
}

/** vcmd c5: set volume fade. */
static void akaoSpcEventVolumeFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vol");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, akaoSpcMidiVolOf(arg2)); // TODO: OH LAZY!
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

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, akaoSpcMidiPanOf(arg1));
}

/** vcmd c7: set volume fade. */
static void akaoSpcEventPanpotFade (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-pan");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, akaoSpcMidiPanOf(arg2)); // TODO: OH LAZY!
}

/** vcmd c8: portamento (one-shot pitch slide). */
static void akaoSpcEventPitchSlide (AkaoSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    AkaoSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Slide, speed = %d, key += %d", arg1, arg2);
    strcat(ev->classStr, " ev-pitchslide");

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

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Tuning, key += %d / 256", arg1); // TODO: details
    strcat(ev->classStr, " ev-tuning");

    if (!akaoSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNM, 0);
    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNL, 1);
    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYM, 64 + (arg1 / 4));
    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYL, 0);
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

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg2;

    sprintf(ev->note, "Set Tempo, fade speed = %d, tempo = %.1f", arg1, akaoSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, akaoSpcTempo(seq)); // OH LAZY!
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

/** set pointers of each event. */
static void akaoSpcSetEventList (AkaoSpcSeqStat *seq)
{
    int code;
    AkaoSpcEvent *event = seq->ver.event;
    byte vcmdFirst = seq->ver.vcmdFirstByte;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (AkaoSpcEvent) akaoSpcEventUnidentified;
    }

    for(code = 0x00; code < vcmdFirst; code++) {
        event[code] = (AkaoSpcEvent) akaoSpcEventNote;
    }

    if (seq->ver.id == SPC_VER_REV1)
    {
        event[0xd2] = akaoSpcEventUnknown3; // tempo
        //event[0xd3] = akaoSpcEventUnidentified;
        event[0xd4] = akaoSpcEventEchoVolume;
        event[0xd5] = akaoSpcEventUnknown2; // feedback, delay
        event[0xd6] = akaoSpcEventUnknown3; // pitch slide
        event[0xd7] = akaoSpcEventUnknown3; // volume
        event[0xd8] = akaoSpcEventVibratoOn;
        //event[0xd9] = akaoSpcEventUnknown1; // panpot?
        event[0xda] = akaoSpcEventSetOctave;
        event[0xdb] = akaoSpcEventInstrument;
        //event[0xdc] = akaoSpcEventUnidentified;
        //event[0xdd] = akaoSpcEventUnidentified;
        //event[0xde] = akaoSpcEventUnidentified;
        event[0xdf] = akaoSpcEventNoiseFreq;
        //event[0xe0] = akaoSpcEventUnidentified; // repeat
        event[0xe1] = akaoSpcEventOctaveUp;
        event[0xe2] = akaoSpcEventOctaveDown;
        //event[0xe3] = akaoSpcEventUnidentified;
        //event[0xe4] = akaoSpcEventUnidentified;
        //event[0xe5] = akaoSpcEventUnidentified;
        //event[0xe6] = akaoSpcEventUnidentified;
        //event[0xe7] = akaoSpcEventUnidentified;
        //event[0xe8] = akaoSpcEventUnidentified; // vibrato
        //event[0xe9] = akaoSpcEventUnidentified;
        event[0xea] = akaoSpcEventEchoOn;
        event[0xeb] = akaoSpcEventEchoOff;
        event[0xec] = akaoSpcEventNoiseOn; // d0
        event[0xed] = akaoSpcEventNoiseOff;
        event[0xee] = akaoSpcEventPitchModOn;
        event[0xef] = akaoSpcEventPitchModOff;
        //event[0xf0] = akaoSpcEventUnidentified; // repeat?
        //event[0xf1] = akaoSpcEventUnidentified;
        //event[0xf2] = akaoSpcEventUnidentified;
        //event[0xf3] = akaoSpcEventUnidentified;
        //event[0xf4] = akaoSpcEventUnidentified;
        //event[0xf5] = akaoSpcEventUnidentified;
        //event[0xf6] = akaoSpcEventUnidentified;
        //event[0xf7] = akaoSpcEventUnidentified;
        //event[0xf8] = akaoSpcEventUnidentified;
        //event[0xf9] = akaoSpcEventUnidentified;
        //event[0xfa] = akaoSpcEventUnidentified;
        //event[0xfb] = akaoSpcEventUnidentified;
        //event[0xfc] = akaoSpcEventUnidentified;
        //event[0xfd] = akaoSpcEventUnidentified;
        //event[0xfe] = akaoSpcEventUnidentified;
        //event[0xff] = akaoSpcEventUnidentified;
    }
    else if (seq->ver.id == SPC_VER_REV2)
    {
        event[0xd2] = akaoSpcEventSetTempo;
        event[0xd3] = akaoSpcEventUnknown1; // restore default tempo
        event[0xd4] = akaoSpcEventVolume; // c4
        event[0xd5] = akaoSpcEventVolumeFade;
        event[0xd6] = akaoSpcEventPanpot;
        event[0xd7] = akaoSpcEventPanpotFade;
        event[0xd8] = akaoSpcEventPitchSlide;
        //event[0xd9] = akaoSpcEventUnidentified;
        //event[0xda] = akaoSpcEventUnidentified;
        //event[0xdb] = akaoSpcEventUnidentified;
        //event[0xdc] = akaoSpcEventUnidentified;
        //event[0xdd] = akaoSpcEventUnidentified;
        //event[0xde] = akaoSpcEventUnidentified;
        //event[0xdf] = akaoSpcEventUnidentified;
        //event[0xe0] = akaoSpcEventUnidentified;
        event[0xe1] = akaoSpcEventNoiseFreq;
        event[0xe2] = akaoSpcEventNoiseOn; // d0
        event[0xe3] = akaoSpcEventNoiseOff;
        event[0xe4] = akaoSpcEventPitchModOn;
        event[0xe5] = akaoSpcEventPitchModOff;
        event[0xe6] = akaoSpcEventUnknown2; // feedback, delay
        event[0xe7] = akaoSpcEventEchoOn;
        event[0xe8] = akaoSpcEventEchoOff;
        //event[0xe9] = akaoSpcEventUnidentified;
        //event[0xea] = akaoSpcEventUnidentified;
        event[0xeb] = akaoSpcEventSetOctave;
        event[0xec] = akaoSpcEventOctaveUp;
        event[0xed] = akaoSpcEventOctaveDown;
        //event[0xee] = akaoSpcEventUnidentified;
        //event[0xef] = akaoSpcEventUnidentified;
        //event[0xf0] = akaoSpcEventUnidentified;
        event[0xf1] = akaoSpcEventUnknown3; // loop (old style)
        event[0xf2] = akaoSpcEventSlurOn;
        event[0xf3] = akaoSpcEventInstrument;
        event[0xf4] = akaoSpcEventUnknown1; // software envelope
        event[0xf5] = akaoSpcEventSlurOff;
        //event[0xf6] = akaoSpcEventUnidentified;
        event[0xf7] = akaoSpcEventDetune;
        //event[0xf8] = akaoSpcEventUnidentified;
        //event[0xf9] = akaoSpcEventUnidentified;
        //event[0xfa] = akaoSpcEventUnidentified;
        //event[0xfb] = akaoSpcEventUnidentified;
        //event[0xfc] = akaoSpcEventUnidentified;
        //event[0xfd] = akaoSpcEventUnidentified;
        //event[0xfe] = akaoSpcEventUnidentified;
        //event[0xff] = akaoSpcEventUnidentified;
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
        event[vcmdFirst + 0x1e] = akaoSpcEventUnknown1; // TODO repeat start
        event[vcmdFirst + 0x1f] = akaoSpcEventUnknown0; // TODO repeat end
        event[vcmdFirst + 0x20] = akaoSpcEventSlurOn;
        event[vcmdFirst + 0x21] = akaoSpcEventSlurOff; // TODO is that true?
        //event[vcmdFirst + 0x22] = akaoSpcEventUnidentified; // e6
        //event[vcmdFirst + 0x23] = akaoSpcEventUnidentified; // e7
        event[vcmdFirst + 0x24] = akaoSpcEventUnknown1;
        event[vcmdFirst + 0x25] = akaoSpcEventUnknown1;
        event[vcmdFirst + 0x26] = akaoSpcEventUnknown1;
        event[vcmdFirst + 0x27] = akaoSpcEventEndOfTrack;
        event[vcmdFirst + 0x28] = akaoSpcEventEndOfTrack; // duplicated
        event[vcmdFirst + 0x29] = akaoSpcEventEndOfTrack; // duplicated
        event[vcmdFirst + 0x2a] = akaoSpcEventEndOfTrack; // duplicated
        //event[vcmdFirst + 0x2b] = akaoSpcEventUnidentified; // ef
        event[vcmdFirst + 0x2c] = akaoSpcEventSetTempo; // f0
        event[vcmdFirst + 0x2d] = akaoSpcEventSetTempoFade;
        event[vcmdFirst + 0x2e] = akaoSpcEventEchoVolume;
        event[vcmdFirst + 0x2f] = akaoSpcEventEchoVolumeFade;

        switch (seq->ver.subId)
        {
        case SPC_SUBVER_RS3:
            event[0xf5] = akaoSpcEventUnknown3; // TODO repeat break
            event[0xf6] = akaoSpcEventJump;
            event[0xf7] = akaoSpcEventEchoFeedback;
            event[0xf8] = akaoSpcEventEchoFIR;
            event[0xf9] = akaoSpcEventUnidentified;
            //event[0xfa] = akaoSpcEventUnidentified;
            event[0xfb] = akaoSpcEventRhythmOn;
            event[0xfc] = akaoSpcEventRhythmOff;
            event[0xfd] = akaoSpcEventUnknown1;
            event[0xfe] = akaoSpcEventEndOfTrack; // duplicated
            event[0xff] = akaoSpcEventEndOfTrack; // duplicated
            break;
        }
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
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
static bool cmdOptGM2 (void);
static bool cmdOptSong (void);
static bool cmdOptSongList (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
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
