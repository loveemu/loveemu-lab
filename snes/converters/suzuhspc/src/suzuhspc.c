/**
 * Square SUZUKI Hidenori spc2midi.
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
#include "suzuhspc.h"

#define APPNAME "Square SUZUKI Hidenori SPC2MIDI"
#define APPSHORTNAME "suzuhspc"
#define VERSION "[2014-02-15]"

static int suzuhSpcLoopMax = 2;            // maximum loop count of parser
static int suzuhSpcTextLoopMax = 1;        // maximum loop count of text output
static double suzuhSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool suzuhSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool suzuhSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int suzuhSpcTimeBase = 48;
static int suzuhSpcForceSongIndex = -1;
static int suzuhSpcForceSongListAddr = -1;

static bool suzuhSpcPatchFixOverride = false;
static PatchFixInfo suzuhSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int suzuhSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_REV1,
    SPC_VER_REV2,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   0
#define SPC_ARAM_SIZE       0x10000

typedef struct TagSuzuhSpcTrackStat SuzuhSpcTrackStat;
typedef struct TagSuzuhSpcSeqStat SuzuhSpcSeqStat;
typedef void (*SuzuhSpcEvent) (SuzuhSpcSeqStat *, SeqEventReport *);

typedef struct TagSuzuhSpcVerInfo {
    int id;
    int seqHeaderAddr;
    int vcmdTableAddr;
    int vcmdLenTableAddr;
    int sfxBaseAddr;
    SuzuhSpcEvent event[256];
    PatchFixInfo patchFix[256];
    bool seqDetected;
} SuzuhSpcVerInfo;

typedef struct TagSuzuhSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} SuzuhSpcNoteParam;

struct TagSuzuhSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    SuzuhSpcNoteParam note;     // current note param
    SuzuhSpcNoteParam lastNote; // note params for last note
    int lastNoteLen;        // last note length ($0230+x)
    int looped;             // how many times looped (internal)
    int octave;             // octave
    int patch;              // patch number (for pitch fix)
    int repHeadAddr[0x100]; // start address of repeat vcmd
    int repTailAddr[0x100]; // end address of repeat vcmd
    int repDecCount[0x100]; // decremental repeat counter
    int repOctave[0x100];   // octave at repeat start
    int repNestLevel;       // current nest level of repeat vcmd
    int repNestLevelMax;    // max nest level allowed of repeat vcmd
    bool rhythmChannel;     // rhythm channel / melody channel
    int volume;             // current volume (used for fade control)
    int panpot;             // current panpot (used for fade control)
    int loopPointAddr;      // song loop point (0=no loop)
    int sfxReturnAddr;      // sfx return address (0=not used)
    int transpose;          // per-voice transpose (4=semitone +1)
};

struct TagSuzuhSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    int noiseFreqReg;       // noise clock reg value
    bool active;                // if the seq is still active
    SuzuhSpcVerInfo ver;        // game version info
    SuzuhSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void suzuhSpcSetEventList (SuzuhSpcSeqStat *seq);

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
FILE *suzuhSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int suzuhSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = suzuhSpcLoopMax;
    suzuhSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool suzuhSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        suzuhSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        suzuhSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            suzuhSpcPatchFix[patch].bankSelM = patch >> 7;
            suzuhSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            suzuhSpcPatchFix[patch].bankSelM = 0;
            suzuhSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        suzuhSpcPatchFix[patch].patchNo = patch & 0x7f;
        suzuhSpcPatchFix[patch].key = 0;
        suzuhSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        suzuhSpcPatchFix[src].bankSelM = bankM & 0x7f;
        suzuhSpcPatchFix[src].bankSelL = bankL & 0x7f;
        suzuhSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        suzuhSpcPatchFix[src].key = key;
        suzuhSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    suzuhSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *suzuhSpcVerToStrHtml (SuzuhSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_REV1:
        return "Revision 1 (Seiken Densetsu 3)";
    case SPC_VER_REV2:
        return "Revision 2 (Bahamut Lagoon)";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void suzuhSpcResetTrackParam (SuzuhSpcSeqStat *seq, int track)
{
    SuzuhSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->octave = 6;
    tr->transpose = 0;
    tr->lastNote.active = false;
    tr->lastNoteLen = 0;
    tr->patch = 0;
    tr->volume = 0x3c;
    tr->panpot = 0x80;
    tr->rhythmChannel = false;
    tr->repNestLevel = 0;
    tr->repNestLevelMax = 0xff; // I don't really care
    tr->loopPointAddr = 0;
    tr->sfxReturnAddr = 0;
}

/** reset before play/convert song. */
static void suzuhSpcResetParam (SuzuhSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x81; // I dunno
    seq->transpose = 0;
    seq->looped = 0;
    seq->noiseFreqReg = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        SuzuhSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        suzuhSpcResetTrackParam(seq, track);
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
    if (suzuhSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &suzuhSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int suzuhSpcCheckVer (SuzuhSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int songLdCodeAddr = -1;
    int vcmdExecCodeAddr = -1;
    int vcmdLenLdCodeAddr = -1;
    int sfxBaseLdCodeAddr = -1;

    seq->timebase = suzuhSpcTimeBase;
    //seq->ver.seqListAddr = -1;
    //seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.vcmdTableAddr = -1;
    seq->ver.vcmdLenTableAddr = -1;
    seq->ver.sfxBaseAddr = -1;
    seq->ver.seqDetected = false;

    // (Seiken Densetsu 3)
    // mov   ($5c),($f5)
    // mov   ($f5),($5c)
    // call  $0a0f
    // mov   x,#$00
    // mov   a,$1a
    // asl   a
    // mov   y,a
    // mov   a,$2000+x
    // mov   $1b79+y,a
    // mov   a,$2001+x
    // mov   $1b7a+y,a
    // inc   x
    // inc   x
    if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xfa..\xfa..\x3f..\xcd\\\x00\xe4.\x1c\xfd\xf5..\xd6..\xf5..\xd6..\x3d\x3d", SPC_ARAM_SIZE, NULL)) != -1 &&
        mget2l(&seq->aRAM[songLdCodeAddr + 16]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 22]) &&
        mget2l(&seq->aRAM[songLdCodeAddr + 19]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 25]))
    {
        seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[songLdCodeAddr + 16]);
        version = SPC_VER_REV1;
    }
    // (Bahamut Lagoon, Super Mario RPG)
    // mov   ($5f),($f5)
    // call  $09fe
    // call  $048a
    // mov   $06,#$08
    // mov   a,$1d
    // asl   a
    // mov   x,a
    // mov   a,$2000+y
    // mov   $1b4c+x,a
    // mov   a,$2001+y
    // mov   $1b4d+x,a
    // inc   x
    // inc   x
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\xfa..\x3f..\x3f..\x8f..\xe4.\x1c\\\x5d\xf6..\xd5..\xf6..\xd5..\x3d\x3d", SPC_ARAM_SIZE, NULL)) != -1 &&
        mget2l(&seq->aRAM[songLdCodeAddr + 17]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 23]) &&
        mget2l(&seq->aRAM[songLdCodeAddr + 20]) + 1 == mget2l(&seq->aRAM[songLdCodeAddr + 26]))
    {
        seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[songLdCodeAddr + 17]);
        version = SPC_VER_REV2;
    }

    // (Seiken Densetsu 3)
    // setc
    // sbc   a,#$c4
    // asl   a
    // mov   x,a
    // clrc
    // mov   a,#$00
    // jmp   ($16ed+x)
    if ((vcmdExecCodeAddr = indexOfHexPat(seq->aRAM, "\x80\xa8\xc4\x1c\\\x5d\x60\xe8\\\x00\x1f..", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.vcmdTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 9]);
    }
    // (Bahamut Dragoon)
    // setc
    // sbc   a,#$c4
    // push  a
    // mov   x,a
    // mov   a,$1721+x
    // and   a,#$07
    // mov   $06,a
    // mov   y,#$00
    // mov   x,#$00
    // dec   $06
    // beq   $1006
    // mov   a,($29)+y
    // mov   $0e+x,a
    // incw  $29
    // inc   x
    // bra   $0ff9
    // pop   a
    // asl   a
    // mov   x,a
    // clrc
    // mov   y,$1e
    // jmp   ($16a9+x)
    else if ((vcmdExecCodeAddr = indexOfHexPat(seq->aRAM, "\x80\xa8\xc4\x2d\\\x5d\xf5..\x28\x07\xc4.\x8d\\\x00\xcd\\\x00\x8b.\xf0.\xf7.\xd4.\x3a.\x3d\x2f.\xae\x1c\x5d\x60\xeb.\x1f..", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.vcmdTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 36]);
        seq->ver.vcmdLenTableAddr = mget2l(&seq->aRAM[vcmdExecCodeAddr + 6]);
    }

    if (seq->ver.vcmdLenTableAddr == -1)
    {
        // (Seiken Densetsu 3)
        // setc
        // sbc   a,#$c4
        // mov   x,a
        // mov   a,$1765+x         ; oplen table
        // bmi   $0ed1
        // beq   $0ed6
        // clrc
        // mov   a,y
        // adc   a,$1765+x         ; oplen table
        // mov   y,a
        // bra   $0ebb
        if ((vcmdLenLdCodeAddr = indexOfHexPat(seq->aRAM, "\x80\xa8\xc4\\\x5d\xf5..\x30.\xf0.\x60\xdd\x95..\xfd\x2f.", SPC_ARAM_SIZE, NULL)) != -1 &&
            mget2l(&seq->aRAM[vcmdLenLdCodeAddr + 5]) == mget2l(&seq->aRAM[vcmdLenLdCodeAddr + 14]))
        {
            seq->ver.vcmdLenTableAddr = mget2l(&seq->aRAM[vcmdLenLdCodeAddr + 5]);
        }
    }

    // (Seiken Densetsu 3)
    // mov   y,#$04
    // mul   ya
    // push  a
    // mov   a,y
    // clrc
    // adc   a,#$40
    // mov   y,a
    // pop   a
    // ret
    if ((sfxBaseLdCodeAddr = indexOfHexPat(seq->aRAM, "\x8d\x04\xcf\x2d\xdd\x60\x88.\xfd\xae\x6f", SPC_ARAM_SIZE, NULL)) != -1)
    {
        seq->ver.sfxBaseAddr = seq->aRAM[sfxBaseLdCodeAddr + 7] << 8;
    }

    if (seq->ver.seqHeaderAddr == -1 ||
        seq->ver.vcmdTableAddr == -1 ||
        seq->ver.vcmdLenTableAddr == -1 ||
        seq->ver.sfxBaseAddr == -1)
    {
        version = SPC_VER_UNKNOWN;
    }

    seq->ver.id = version;
    suzuhSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool suzuhSpcDetectSeq (SuzuhSpcSeqStat *seq)
{
    bool result = true;
    int seqHeaderReadOfs;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    suzuhSpcResetParam(seq);

    seqHeaderReadOfs = seq->ver.seqHeaderAddr;
    if (seq->ver.id != SPC_VER_REV1)
    {
        while (seq->aRAM[seqHeaderReadOfs] < 0x80)
        {
            seqHeaderReadOfs += 5;
            if (seqHeaderReadOfs >= SPC_ARAM_SIZE)
            {
                return false;
            }
        }
        seqHeaderReadOfs++;
    }

    // track list
    result = false;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        int trackAddr = mget2l(&seq->aRAM[seqHeaderReadOfs]);
        seqHeaderReadOfs += 2;

        if (trackAddr != 0)
        {
            seq->track[tr].pos = trackAddr;
            seq->track[tr].active = true;
            result = true;
        }
    }

    return result;
}

/** create new spc2mid object. */
static SuzuhSpcSeqStat *newSuzuhSpcSeq (const byte *aRAM)
{
    SuzuhSpcSeqStat *newSeq = (SuzuhSpcSeqStat *) calloc(1, sizeof(SuzuhSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        suzuhSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = suzuhSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delSuzuhSpcSeq (SuzuhSpcSeqStat **seq)
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
static void printHtmlInfoList (SuzuhSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", suzuhSpcVerToStrHtml(seq));
    //myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    //myprintf(" (Song $%02x)", seq->ver.songIndex);
    myprintf("</li>\n");
    myprintf("          <li>Voice Command Table: $%04X</li>", seq->ver.vcmdTableAddr);
    myprintf("          <li>Voice Command Length Table: $%04X</li>", seq->ver.vcmdLenTableAddr);
    myprintf("          <li>SFX Base Address: $%04X</li>", seq->ver.sfxBaseAddr);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (SuzuhSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (SuzuhSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (SuzuhSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double suzuhSpcTempoOf (SuzuhSpcSeqStat *seq, int tempoValue)
{
    return (double) 60000000 / (125 * seq->tempo * 48);
}

/** convert SPC tempo into bpm. */
static double suzuhSpcTempo (SuzuhSpcSeqStat *seq)
{
    return suzuhSpcTempoOf(seq, seq->tempo);
}

/** convert SPC velocity into MIDI one. */
static int suzuhSpcMidiVelOf (int value)
{
    if (suzuhSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int suzuhSpcMidiVolOf (int value)
{
    if (suzuhSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel panpot into MIDI one. */
static int suzuhSpcMidiPanOf (int value)
{
    return value/2; // linear (TODO: sine curve)
}

/** create new smf object and link to spc seq. */
static Smf *suzuhSpcCreateSmf (SuzuhSpcSeqStat *seq)
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

    switch (suzuhSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, suzuhSpcTempo(seq));

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

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, suzuhSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void suzuhSpcTruncateNote (SuzuhSpcSeqStat *seq, int track)
{
    SuzuhSpcTrackStat *tr = &seq->track[track];

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
static void suzuhSpcTruncateNoteAll (SuzuhSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        suzuhSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool suzuhSpcDequeueNote (SuzuhSpcSeqStat *seq, int track)
{
    SuzuhSpcTrackStat *tr = &seq->track[track];
    SuzuhSpcNoteParam *lastNote = &tr->lastNote;
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
static void suzuhSpcDequeueNoteAll (SuzuhSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        suzuhSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void suzuhSpcInactiveTrack(SuzuhSpcSeqStat *seq, int track)
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
static void suzuhSpcAddTrackLoopCount(SuzuhSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (suzuhSpcLoopMax > 0) ? suzuhSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= suzuhSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void suzuhSpcSeqAdvTick(SuzuhSpcSeqStat *seq)
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
    seq->time += (double) 60 / suzuhSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void suzuhSpcEventUnknownInline (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
}

/** vcmds: unidentified event. */
static void suzuhSpcEventUnidentified (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    suzuhSpcEventUnknownInline(seq, ev);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void suzuhSpcEventUnknown0 (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    suzuhSpcEventUnknownInline(seq, ev);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void suzuhSpcEventUnknown1 (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    suzuhSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void suzuhSpcEventUnknown2 (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    suzuhSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void suzuhSpcEventUnknown3 (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    suzuhSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void suzuhSpcEventUnknown4 (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
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

    suzuhSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void suzuhSpcEventUnknown5 (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
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

    suzuhSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void suzuhSpcEventNOP (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00-c3: note. */
static void suzuhSpcEventNote (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int keyIndex = noteByte % 14;
    int lenIndex = noteByte / 14;
    int note;
    int len;
    int dur;
    bool rest;
    bool tie;
    const int noteLenTable[] = { 0xc0, 0x90, 0x60, 0x48, 0x30, 0x24, 0x20, 0x18, 0x10, 0x0c, 0x08, 0x06, 0x03 };

    rest = (keyIndex == 12);
    tie = (keyIndex == 13);

    if (lenIndex == 13)
    {
        ev->size++;
        len = seq->aRAM[*p];
        (*p)++;
    }
    else
    {
        len = noteLenTable[lenIndex];
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
        getNoteName(ev->note, note + seq->transpose + (tr->transpose / 4)
            + seq->ver.patchFix[tr->note.patch].key
            + (tr->rhythmChannel ? 0 : SPC_NOTE_KEYSHIFT));
        sprintf(argDumpStr, ", len = %d", len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    //if (!suzuhSpcLessTextInSMF)
    //   smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // output old note first
    if (!tie)
    {
        suzuhSpcDequeueNote(seq, ev->track);
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
            tr->lastNote.transpose = seq->transpose + (tr->transpose / 4);
            tr->lastNote.patch = tr->note.patch;
            tr->lastNote.active = true;
        }
        tr->lastNote.tied = tie;
    }
    tr->tick += len;
}

/** vcmd c4: increase octave. */
static void suzuhSpcEventOctaveUp (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    tr->octave++;

    sprintf(ev->note, "Octave Up, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octaveup");
}

/** vcmd c5: decrease octave. */
static void suzuhSpcEventOctaveDown (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    tr->octave--;

    sprintf(ev->note, "Octave Down, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octavedown");
}

/** vcmd c6: set octave. */
static void suzuhSpcEventSetOctave (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Octave, octave = %d", arg1);
    strcat(ev->classStr, " ev-octave");

    tr->octave = arg1;
}

/** vcmd c8: set noise clock. */
static void suzuhSpcEventNoiseFreq (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->noiseFreqReg = arg1 & 0x1f;

    sprintf(ev->note, "Set Noise Frequency, NCK = %d", seq->noiseFreqReg);
    strcat(ev->classStr, " ev-noisefreq");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd c9: noise on. */
static void suzuhSpcEventNoiseOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Noise On");
    strcat(ev->classStr, " ev-noiseon");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ca: noise off. */
static void suzuhSpcEventNoiseOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Noise Off");
    strcat(ev->classStr, " ev-noiseoff");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cb: pitchmod on. */
static void suzuhSpcEventPitchModOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Mod On");
    strcat(ev->classStr, " ev-pitchmodon");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cc: pitchmod off. */
static void suzuhSpcEventPitchModOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Mod Off");
    strcat(ev->classStr, " ev-pitchmodoff");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cd: jump to SFX (lo). */
static void suzuhSpcEventJumpToSFXLo (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    dest = seq->ver.sfxBaseAddr + (arg1 * 4);

    sprintf(ev->note, "Jump to SFX (Lo), index = %d, dest = $%04X", arg1, dest);
    strcat(ev->classStr, " ev-jumpsfx");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    *p = dest;
}

/** vcmd ce: jump to SFX (hi). */
static void suzuhSpcEventJumpToSFXHi (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    dest = seq->ver.sfxBaseAddr + 2 + (arg1 * 4);

    sprintf(ev->note, "Jump to SFX (Hi), index = %d, dest = $%04X", arg1, dest);
    strcat(ev->classStr, " ev-jumpsfx");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    *p = dest;
}

/** vcmd cf: detune. */
static void suzuhSpcEventDetune (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    //int midiScaled;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Tuning, key += %.1f cents?", arg1 * 100 / 16.0);
    strcat(ev->classStr, " ev-tuning");

    if (!suzuhSpcLessTextInSMF)
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

/** vcmd d0: song repeat / end of track. */
static void suzuhSpcEventEndOfTrack (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    if ((tr->loopPointAddr & 0xff00) != 0)
    {
        sprintf(ev->note, "Jump To Loop Point, dest = $%04X", tr->loopPointAddr);
        strcat(ev->classStr, " ev-loop");

        suzuhSpcAddTrackLoopCount(seq, ev->track, 1);
        *p = tr->loopPointAddr;
    }
    else if ((tr->sfxReturnAddr & 0xff00) != 0)
    {
        sprintf(ev->note, "Return From SFX, dest = $%04X", tr->sfxReturnAddr);
        strcat(ev->classStr, " ev-ret");

        *p = tr->sfxReturnAddr;
        tr->sfxReturnAddr = 0;
    }
    else
    {
        sprintf(ev->note, "End of Track");
        strcat(ev->classStr, " ev-end");

        suzuhSpcInactiveTrack(seq, ev->track);
    }

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d1: set tempo. */
static void suzuhSpcEventSetTempo (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;

    sprintf(ev->note, "Set Tempo, tempo = %.1f", suzuhSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, suzuhSpcTempo(seq));
}

/** vcmd f2: change tempo. */
static void suzuhSpcEventAddTempo (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    seq->tempo = (seq->tempo + arg1) & 0xff;

    sprintf(ev->note, "Add/Subtract Tempo, tempo = %.1f", suzuhSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, suzuhSpcTempo(seq));
}

/** vcmd d2: set timer1 freq. */
static void suzuhSpcEventSetTimer1Freq (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Timer 1 Frequency, freq = %.2f ms", 0.125 * arg1);
    strcat(ev->classStr, " ev-timer1freq");
}

/** vcmd d3: change timer1 freq. */
static void suzuhSpcEventAddTimer1Freq (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Add/Subtract Timer 1 Frequency, freq += %.2f ms", 0.125 * arg1);
    strcat(ev->classStr, " ev-timer1freq");
}

/** vcmd d4: loop start. */
static void suzuhSpcEventLoopStart (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int repCount;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    if (arg1 == 0)
    {
        repCount = 256;
    }
    else
    {
        repCount = arg1;
    }

    sprintf(ev->note, "Loop Start, count = %d", repCount);
    strcat(ev->classStr, " ev-loopstart");

    if (tr->repNestLevel + 1 > tr->repNestLevelMax) {
        fprintf(stderr, "Repeat Stack Access Violation, addr = $%04X\n", ev->addr);
        suzuhSpcInactiveTrack(seq, ev->track);
        return;
    }
    tr->repHeadAddr[tr->repNestLevel] = *p;
    //tr->repTailAddr[tr->repNestLevel] = 0;
    tr->repDecCount[tr->repNestLevel] = repCount - 1;
    tr->repOctave[tr->repNestLevel] = tr->octave;
    tr->repNestLevel++;

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d5: loop end. */
static void suzuhSpcEventLoopEnd (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool repeatAgain;
    int nestIndex;

    sprintf(ev->note, "Loop End/Continue");
    strcat(ev->classStr, " ev-loopend");

    nestIndex = (tr->repNestLevel - 1);
    if (nestIndex < 0)
    {
        fprintf(stderr, "Repeat Stack Access Violation, addr = $%04X\n", ev->addr);
        suzuhSpcInactiveTrack(seq, ev->track);
        return;
    }

    if (tr->repDecCount[nestIndex] == 0)
    {
        repeatAgain = false;
    }
    else
    {
        tr->repDecCount[nestIndex]--;
        repeatAgain = true;
    }

    if (!repeatAgain) {
        // repeat end, fall through
        sprintf(ev->note, "Loop End");
        tr->repNestLevel = nestIndex;
    }
    else {
        // repeat again
        sprintf(ev->note, "Loop Continue, count = %d", tr->repDecCount[nestIndex]);
        tr->octave = tr->repOctave[nestIndex];
        tr->repTailAddr[nestIndex] = *p;
        *p = tr->repHeadAddr[nestIndex];
    }

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d6: loop break. */
static void suzuhSpcEventLoopBreak (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool doJump;
    int nestIndex;

    nestIndex = (tr->repNestLevel - 1);
    if (nestIndex < 0)
    {
        fprintf(stderr, "Repeat Stack Access Violation, addr = $%04X\n", ev->addr);
        suzuhSpcInactiveTrack(seq, ev->track);
        return;
    }

    doJump = (tr->repDecCount[nestIndex] == 0);

    sprintf(ev->note, "Loop Break, jump = %s", doJump ? "true" : "false");
    strcat(ev->classStr, " ev-loopbreak");
    if (doJump)
    {
        sprintf(argDumpStr, ", dest = $%04X", tr->repTailAddr[nestIndex]);
        strcat(ev->note, argDumpStr);

        *p = tr->repTailAddr[nestIndex];
        tr->repNestLevel = nestIndex;
    }

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d7: set loop point. */
static void suzuhSpcEventSetLoopPoint (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "Set Loop Point, dest = $%04X", *p);
    strcat(ev->classStr, " ev-looppoint");

    tr->loopPointAddr = *p;

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d8: set default ADSR. */
static void suzuhSpcEventSetDefaultADSR (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "Set Default ADSR");
    strcat(ev->classStr, " ev-setdefaultADSR");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d9: set attack rate (AR). */
static void suzuhSpcEventSetAR (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Attack Rate, AR = %d", arg1 & 15);
    strcat(ev->classStr, " ev-setAR");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd da: set decay rate (DR). */
static void suzuhSpcEventSetDR (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Decay Rate, DR = %d", arg1 & 7);
    strcat(ev->classStr, " ev-setDR");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd db: set sustain level (SL). */
static void suzuhSpcEventSetSL (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Sustain Level, SL = %d", arg1 & 7);
    strcat(ev->classStr, " ev-setSL");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dc: set sustain rate (SR). */
static void suzuhSpcEventSetSR (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Sustain Rate, SR = %d", arg1 & 31);
    strcat(ev->classStr, " ev-setSR");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dd: set duration rate. */
static void suzuhSpcEventDurationRate (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Duration Rate, rate = %d", arg1);
    strcat(ev->classStr, " ev-durrate");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd de: set instrument. */
static void suzuhSpcEventInstrument (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd df: change noise clock. */
static void suzuhSpcEventAddNoiseFreq (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 1;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    seq->noiseFreqReg = (seq->noiseFreqReg + arg1) & 0x1f;

    sprintf(ev->note, "Add/Subtract Noise Frequency, NCK = %d", seq->noiseFreqReg);
    strcat(ev->classStr, " ev-noisefreq");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e0: set volume. */
static void suzuhSpcEventVolume (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->volume = arg1;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, suzuhSpcMidiVolOf(tr->volume));
}

/** vcmd e3: change volume. */
static void suzuhSpcEventAddVolume (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->volume = (tr->volume + arg1) & 0x7f;

    sprintf(ev->note, "Add/Subtract Volume, vol = %d", tr->volume);
    strcat(ev->classStr, " ev-vol");

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, suzuhSpcMidiVolOf(tr->volume));
}

/** vcmd e4: set volume fade. */
static void suzuhSpcEventVolumeFade (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vol");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    // (for instance, Chrono Trigger Title)
    if (arg1 == 0)
    {
        tr->volume = arg2;
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, suzuhSpcMidiVolOf(tr->volume));
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
                smfInsertControl(seq->smf, ev->tick + faderStep, ev->track, ev->track, SMF_CONTROL_VOLUME, suzuhSpcMidiVolOf(tr->volume));
            }
        }
    }
}

/** vcmd e5: portamento. */
static void suzuhSpcEventPortamento (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Portamento, arg1 = %d, arg2 = %d", arg1, arg2);
    strcat(ev->classStr, " ev-portamento");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e6: portamento toggle. */
static void suzuhSpcEventPortamentoToggle (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Portamento Toggle");
    strcat(ev->classStr, " ev-portamento");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e7: set panpot. */
static void suzuhSpcEventPanpot (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot, balance = %d", arg1);
    strcat(ev->classStr, " ev-pan");

    //if (!suzuhSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->panpot = arg1;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, suzuhSpcMidiPanOf(tr->panpot));
}

/** vcmd e8: set panpot fade. */
static void suzuhSpcEventPanpotFade (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int valueFrom, valueTo;
    int faderStep, faderValue;
    double faderPos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot Fade, speed = %d, vol = %d", arg1, arg2);
    strcat(ev->classStr, " ev-pan");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // lazy fader, hope it won't be canceled by other vcmds
    if (arg1 == 0)
    {
        tr->panpot = arg2;
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, suzuhSpcMidiPanOf(tr->panpot));
    }
    else
    {
        valueFrom = tr->panpot;
        valueTo = arg2;
        for (faderStep = 1; faderStep <= arg1; faderStep++)
        {
            faderPos = (double)faderStep / arg1;
            faderValue = (int)(valueTo * faderPos + valueFrom * (1.0 - faderPos)); // alphablend
            if (tr->panpot != faderValue)
            {
                tr->panpot = faderValue;
                smfInsertControl(seq->smf, ev->tick + faderStep, ev->track, ev->track, SMF_CONTROL_PANPOT, suzuhSpcMidiPanOf(tr->panpot));
            }
        }
    }
}

/** vcmd e9: set panpot LFO on. */
static void suzuhSpcEventPanLFOOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot LFO, depth = %d, rate = %d", arg1, arg2);
    strcat(ev->classStr, " ev-panLFO");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ea: restart panpot LFO. */
static void suzuhSpcEventPanLFORestart (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Panpot LFO Restart");
    strcat(ev->classStr, " ev-panLFOrestart");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd eb: set panpot LFO off. */
static void suzuhSpcEventPanLFOOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Panpot LFO Off");
    strcat(ev->classStr, " ev-panLFOoff");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ec: transpose (absolute). */
static void suzuhSpcEventTransposeAbs (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int oldTranspose = tr->transpose;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->transpose = arg1;

    sprintf(ev->note, "Transpose, key = %d/4", arg1);
    strcat(ev->classStr, " ev-transpose");

    if (oldTranspose % 4 != 0 || tr->transpose % 4 != 0)
    {
        if (!suzuhSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }
}

/** vcmd ed: transpose (relative). */
static void suzuhSpcEventTransposeRel (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int oldTranspose = tr->transpose;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->transpose += arg1;

    sprintf(ev->note, "Transpose, key += %d/4", arg1);
    strcat(ev->classStr, " ev-transpose");

    if (oldTranspose % 4 != 0 || tr->transpose % 4 != 0)
    {
        if (!suzuhSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }
}

/** vcmd ee: rhythm channel on. */
static void suzuhSpcEventRhythmOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    strcpy(ev->note, "Rhythm Channel On");
    strcat(ev->classStr, " ev-rhythmon");
    tr->rhythmChannel = true;

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // put program change to SMF (better than nothing)
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[255].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[255].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[255].patchNo);
}

/** vcmd ef: rhythm channel off. */
static void suzuhSpcEventRhythmOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    strcpy(ev->note, "Rhythm Channel Off");
    strcat(ev->classStr, " ev-rhythmoff");
    tr->rhythmChannel = false;

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f0: set vibrato on. */
static void suzuhSpcEventVibratoOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato, rate = %d, depth = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vibrato");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f1: set vibrato on (w/delay). */
static void suzuhSpcEventVibratoOnWithDelay (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato, rate = %d, depth = %d, delay = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-vibrato");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f3: set vibrato off. */
static void suzuhSpcEventVibratoOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Vibrato Off");
    strcat(ev->classStr, " ev-vibratooff");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f4: set tremolo on. */
static void suzuhSpcEventTremoloOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tremolo, rate = %d, depth = %d", arg1, arg2);
    strcat(ev->classStr, " ev-tremolo");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f5: set tremolo on (w/delay). */
static void suzuhSpcEventTremoloOnWithDelay (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tremolo, rate = %d, depth = %d, delay = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-tremolo");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f7: set tremolo off. */
static void suzuhSpcEventTremoloOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Tremolo Off");
    strcat(ev->classStr, " ev-tremolooff");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f8: slur on. */
static void suzuhSpcEventSlurOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Slur On");
    strcat(ev->classStr, " ev-sluron");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f9: slur off. */
static void suzuhSpcEventSlurOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Slur Off");
    strcat(ev->classStr, " ev-sluroff");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fa: echo on. */
static void suzuhSpcEventEchoOn (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd fb: echo off. */
static void suzuhSpcEventEchoOff (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo Off");
    strcat(ev->classStr, " ev-echooff");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd fc: call SFX (lo). */
static void suzuhSpcEventCallSFXLo (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    dest = seq->ver.sfxBaseAddr + (arg1 * 4);

    sprintf(ev->note, "Call SFX (Lo), index = %d, dest = $%04X", arg1, dest);
    strcat(ev->classStr, " ev-jumpsfx");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->sfxReturnAddr = *p;
    *p = dest;
}

/** vcmd fd: call SFX (hi). */
static void suzuhSpcEventCallSFXHi (SuzuhSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SuzuhSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    dest = seq->ver.sfxBaseAddr + 2 + (arg1 * 4);

    sprintf(ev->note, "Call SFX (Hi), index = %d, dest = $%04X", arg1, dest);
    strcat(ev->classStr, " ev-jumpsfx");

    if (!suzuhSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->sfxReturnAddr = *p;
    *p = dest;
}

/** set pointers of each event. */
static void suzuhSpcSetEventList (SuzuhSpcSeqStat *seq)
{
    int code;
    const byte vcmdFirst = 0xc4;
    bool needsAutoEventAssign = true;
    SuzuhSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (SuzuhSpcEvent) suzuhSpcEventUnidentified;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    for(code = 0x00; code < vcmdFirst; code++) {
        event[code] = (SuzuhSpcEvent) suzuhSpcEventNote;
    }
    event[0xc4] = suzuhSpcEventOctaveUp;
    event[0xc5] = suzuhSpcEventOctaveDown;
    event[0xc6] = suzuhSpcEventSetOctave;
    event[0xc7] = suzuhSpcEventNOP;
    event[0xc8] = suzuhSpcEventNoiseFreq;
    event[0xc9] = suzuhSpcEventNoiseOn;
    event[0xca] = suzuhSpcEventNoiseOff;
    event[0xcb] = suzuhSpcEventPitchModOn;
    event[0xcc] = suzuhSpcEventPitchModOff;
    event[0xcd] = suzuhSpcEventJumpToSFXLo;
    event[0xce] = suzuhSpcEventJumpToSFXHi;
    event[0xcf] = suzuhSpcEventDetune;
    event[0xd0] = suzuhSpcEventEndOfTrack;
    event[0xd1] = suzuhSpcEventSetTempo;
    if (seq->ver.id == SPC_VER_REV1)
    {
        //event[0xd2] = suzuhSpcEventLoopStart;
        //event[0xd3] = suzuhSpcEventLoopStart;
    }
    else
    {
        event[0xd2] = suzuhSpcEventSetTimer1Freq;
        event[0xd3] = suzuhSpcEventAddTimer1Freq;
    }
    event[0xd4] = suzuhSpcEventLoopStart;
    event[0xd5] = suzuhSpcEventLoopEnd;
    event[0xd6] = suzuhSpcEventLoopBreak;
    event[0xd7] = suzuhSpcEventSetLoopPoint;
    event[0xd8] = suzuhSpcEventSetDefaultADSR;
    event[0xd9] = suzuhSpcEventSetAR;
    event[0xda] = suzuhSpcEventSetDR;
    event[0xdb] = suzuhSpcEventSetSL;
    event[0xdc] = suzuhSpcEventSetSR;
    event[0xdd] = suzuhSpcEventDurationRate;
    event[0xde] = suzuhSpcEventInstrument;
    event[0xdf] = suzuhSpcEventAddNoiseFreq;
    event[0xe0] = suzuhSpcEventVolume;
    event[0xe1] = suzuhSpcEventUnidentified;
    event[0xe2] = suzuhSpcEventVolume; // duplicated
    event[0xe3] = suzuhSpcEventAddVolume;
    event[0xe4] = suzuhSpcEventVolumeFade;
    event[0xe5] = suzuhSpcEventPortamento;
    event[0xe6] = suzuhSpcEventPortamentoToggle;
    event[0xe7] = suzuhSpcEventPanpot;
    event[0xe8] = suzuhSpcEventPanpotFade;
    event[0xe9] = suzuhSpcEventPanLFOOn;
    event[0xea] = suzuhSpcEventPanLFORestart;
    event[0xeb] = suzuhSpcEventPanLFOOff;
    event[0xec] = suzuhSpcEventTransposeAbs;
    event[0xed] = suzuhSpcEventTransposeRel;
    event[0xee] = suzuhSpcEventRhythmOn;
    event[0xef] = suzuhSpcEventRhythmOff;
    event[0xf0] = suzuhSpcEventVibratoOn;
    event[0xf1] = suzuhSpcEventVibratoOnWithDelay;
    event[0xf2] = suzuhSpcEventAddTempo;
    event[0xf3] = suzuhSpcEventVibratoOff;
    event[0xf4] = suzuhSpcEventTremoloOn;
    event[0xf5] = suzuhSpcEventTremoloOnWithDelay;
    if (seq->ver.id == SPC_VER_REV1)
    {
        //event[0xf6] = suzuhSpcEventOctaveUp; // duplicated
    }
    else
    {
        event[0xf6] = suzuhSpcEventUnknown1;
    }
    event[0xf7] = suzuhSpcEventTremoloOff;
    event[0xf8] = suzuhSpcEventSlurOn;
    event[0xf9] = suzuhSpcEventSlurOff;
    event[0xfa] = suzuhSpcEventEchoOn;
    event[0xfb] = suzuhSpcEventEchoOff;
    if (seq->ver.id == SPC_VER_REV1)
    {
        event[0xfc] = suzuhSpcEventCallSFXLo;
        event[0xfd] = suzuhSpcEventCallSFXHi;
        //event[0xfe] = suzuhSpcEventOctaveUp; // duplicated
        //event[0xff] = suzuhSpcEventOctaveUp; // duplicated
    }
    else
    {
        if (seq->aRAM[seq->ver.vcmdLenTableAddr + (0xfc - 0xc4)] == 4)
        {
            // Super Mario RPG
            event[0xfc] = suzuhSpcEventUnknown3;
            //event[0xfd] = suzuhSpcEventOctaveUp; // duplicated
            event[0xfe] = suzuhSpcEventUnknown0;
            //event[0xff] = suzuhSpcEventOctaveUp; // duplicated
        }
        else
        {
            // Bahamut Lagoon
            //event[0xfc] = suzuhSpcEventOctaveUp; // duplicated
            //event[0xfd] = suzuhSpcEventOctaveUp; // duplicated
            event[0xfe] = suzuhSpcEventUnknown0;
            event[0xff] = suzuhSpcEventUnknown0;
        }
    }

    if (needsAutoEventAssign)
    {
        // assign events by using length table
        // (some variable-length vcmds and jumps can be wrong)
        int vcmdIndex;

        for (vcmdIndex = vcmdFirst; vcmdIndex <= 0xff; vcmdIndex++)
        {
            int vcmdAddr = mget2l(&seq->aRAM[seq->ver.vcmdTableAddr + ((vcmdIndex - vcmdFirst) * 2)]);
            if (event[vcmdIndex] == suzuhSpcEventUnidentified)
            {
                if (vcmdAddr == 0)
                {
                    event[vcmdIndex] = suzuhSpcEventUnidentified;
                }
                else
                {
                    int len = seq->aRAM[seq->ver.vcmdLenTableAddr + vcmdIndex - vcmdFirst];
                    switch(len) // &7
                    {
                        case 1: event[vcmdIndex] = suzuhSpcEventUnknown0; break;
                        case 2: event[vcmdIndex] = suzuhSpcEventUnknown1; break;
                        case 3: event[vcmdIndex] = suzuhSpcEventUnknown2; break;
                        case 4: event[vcmdIndex] = suzuhSpcEventUnknown3; break;
                        case 5: event[vcmdIndex] = suzuhSpcEventUnknown4; break;
                    }
                }
            }
        }
    }
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* suzuhSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    SuzuhSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newSuzuhSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = suzuhSpcCreateSmf(seq);

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

            SuzuhSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (suzuhSpcTextLoopMax == 0 || seq->looped < suzuhSpcTextLoopMax)
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
            suzuhSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= suzuhSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, suzuhSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    suzuhSpcTruncateNoteAll(seq);
    suzuhSpcDequeueNoteAll(seq);

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
        delSuzuhSpcSeq(&seq);
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
Smf* suzuhSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = suzuhSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* suzuhSpcToMidiFromFile (const char *filename)
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

    smf = suzuhSpcToMidi(data, size);

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
    suzuhSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    suzuhSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    suzuhSpcForceSongListAddr = songListAddr;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (suzuhSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    suzuhSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    suzuhSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    suzuhSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    suzuhSpcMidiResetType = SMF_RESET_GM2;
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
            suzuhSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = suzuhSpcToMidiFromFile(gArgv[0]);
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
