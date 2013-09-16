/**
 * Pandora Box spc2midi.
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
#include "pboxspc.h"

#define APPNAME "Pandora Box SPC2MIDI"
#define APPSHORTNAME "pboxspc"
#define VERSION "[2013-09-15]"

// from VS2008 math.h
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616

static int pboxSpcLoopMax = 2;            // maximum loop count of parser
static int pboxSpcTextLoopMax = 1;        // maximum loop count of text output
static double pboxSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool pboxSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool pboxSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int pboxSpcTimeBase = 48; // gkh common timebase

static bool pboxSpcPatchFixOverride = false;
static PatchFixInfo pboxSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int pboxSpcMidiResetType = SMF_RESET_GM1;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_V1,
    SPC_VER_V2,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

typedef struct TagPBoxSpcTrackStat PBoxSpcTrackStat;
typedef struct TagPBoxSpcSeqStat PBoxSpcSeqStat;
typedef void (*PBoxSpcEvent) (PBoxSpcSeqStat *, SeqEventReport *);

typedef struct TagPBoxSpcVerInfo {
    int id;
    int songIndex;
    int seqHeaderAddr;
    int seqHeaderTempoOffset;
    int seqHeaderScoreAddressOffset;
    PBoxSpcEvent event[256];
    PatchFixInfo patchFix[256];
    bool seqDetected;
} PBoxSpcVerInfo;

typedef struct TagPBoxSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int patch;          // instrument
} PBoxSpcNoteParam;

struct TagPBoxSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    PBoxSpcNoteParam note;     // current note param
    PBoxSpcNoteParam lastNote; // note params for last note
    int lastNoteLen;        // last note length ($0b+x)
    int looped;             // how many times looped (internal)
    int patch;              // patch number (for pitch fix)
    int octave;             // current octave
    int quantize;           // quantize (0-7)
    int volumeIndex;        // volume index/value
    int panpot;             // panpot (0-127)
    int transpose;          // transpose
    byte rptStack[0x100];   // repeat stack
    int rptStackPtr;        // repeat stack ptr
    int rptStackSize;       // repeat stack size
};

struct TagPBoxSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    PBoxSpcVerInfo ver;         // game version info
    PBoxSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void pboxSpcSetEventList (PBoxSpcSeqStat *seq);

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
FILE *pboxSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int pboxSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = pboxSpcLoopMax;
    pboxSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool pboxSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        pboxSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        pboxSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        pboxSpcPatchFix[patch].bankSelM = 0;
        pboxSpcPatchFix[patch].bankSelL = patch >> 7;
        pboxSpcPatchFix[patch].patchNo = patch & 0x7f;
        pboxSpcPatchFix[patch].key = 0;
        pboxSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        pboxSpcPatchFix[src].bankSelM = bankM & 0x7f;
        pboxSpcPatchFix[src].bankSelL = bankL & 0x7f;
        pboxSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        pboxSpcPatchFix[src].key = key;
        pboxSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    pboxSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *pboxSpcVerToStrHtml (PBoxSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_V1:
        return "Version 1 (Kishin Kourinden Oni)";
    case SPC_VER_V2:
        return "Version 2 (Traverse)";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void pboxSpcResetTrackParam (PBoxSpcSeqStat *seq, int track)
{
    PBoxSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->lastNote.active = false;
    tr->lastNoteLen = 1; // random value (> 0 for infinite loop safe)
    tr->octave = 3;
    tr->quantize = 0;
    tr->volumeIndex = 15;
    tr->panpot = 64;
    tr->transpose = 0;
    tr->rptStackPtr = 0;
    tr->rptStackSize = 40;
}

/** reset before play/convert song. */
static void pboxSpcResetParam (PBoxSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 120; // random value
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        PBoxSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        pboxSpcResetTrackParam(seq, track);
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
    if (pboxSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &pboxSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }

}

/** returns what version the sequence is, and sets individual info. */
static int pboxSpcCheckVer (PBoxSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int songLdCodeAddr = -1;

    seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.seqHeaderTempoOffset = 6;
    seq->ver.seqHeaderScoreAddressOffset = -1;
    seq->ver.seqDetected = false;

    // (Kishin Kourinden Oni)
    // f91d: 8d 10     mov   y,#$10
    // f91f: fc        inc   y
    // f920: f7 d3     mov   a,($d3)+y
    // f922: dc        dec   y
    // f923: 37 d3     and   a,($d3)+y
    // f925: 68 ff     cmp   a,#$ff
    // f927: f0 30     beq   $f959
    if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\x8d.\xfc\xf7.\xdc\x37.\x68\xff\xf0.", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[songLdCodeAddr + 4] == seq->aRAM[songLdCodeAddr + 7])
    {
        seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[seq->aRAM[songLdCodeAddr + 4]]);
        seq->ver.seqHeaderScoreAddressOffset = seq->aRAM[songLdCodeAddr + 1];
        version = SPC_VER_V1;
    }
    // (Traverse)
    // f96f: 8d 10     mov   y,#$10
    // f971: 7d        mov   a,x
    // f972: f0 45     beq   $f9b9
    // f974: 6d        push  y
    // f975: f7 08     mov   a,($08)+y
    // f977: fc        inc   y
    // f978: c4 00     mov   $00,a
    // f97a: f7 08     mov   a,($08)+y
    // f97c: fc        inc   y
    // f97d: c4 01     mov   $01,a
    // f97f: bc        inc   a
    // f980: f0 31     beq   $f9b3
    else if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\x8d.\x7d\xf0.\x6d\xf7.\xfc\xc4.\xf7.\xfc\xc4.\xbc\xf0.", SPC_ARAM_SIZE, NULL)) != -1 &&
        seq->aRAM[songLdCodeAddr + 7] == seq->aRAM[songLdCodeAddr + 12] &&
        seq->aRAM[songLdCodeAddr + 10] + 1 == seq->aRAM[songLdCodeAddr + 15])
    {
        seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[seq->aRAM[songLdCodeAddr + 7]]);
        seq->ver.seqHeaderScoreAddressOffset = seq->aRAM[songLdCodeAddr + 1];
        version = SPC_VER_V2;
    }

    seq->timebase = pboxSpcTimeBase;
    if (version != SPC_VER_UNKNOWN) {
        if (seq->ver.seqHeaderAddr != -1) {
            int timebaseCandidate = seq->aRAM[seq->ver.seqHeaderAddr + seq->ver.seqHeaderTempoOffset + 1];
            if (timebaseCandidate % 4 == 0) {
                seq->timebase = timebaseCandidate / 4;
            }
            else {
                fprintf(stderr, "Warning: Timebase must be multiple of 4. [%d]\n", timebaseCandidate);
            }
        }
    }

    seq->ver.id = version;
    pboxSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool pboxSpcDetectSeq (PBoxSpcSeqStat *seq)
{
    bool result = true;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    pboxSpcResetParam(seq);
    // initial tempo
    seq->tempo = seq->aRAM[seq->ver.seqHeaderAddr + seq->ver.seqHeaderTempoOffset];
    // track list
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        int trackOffset = mget2l(&seq->aRAM[tr * 2 + seq->ver.seqHeaderAddr + seq->ver.seqHeaderScoreAddressOffset]);
        int trackAddr = (trackOffset != 0xffff) ? (seq->ver.seqHeaderAddr + trackOffset) : 0xffff;
        if (trackAddr != 0xffff)
        {
            seq->track[tr].pos = trackAddr;
            seq->track[tr].active = true;
            result = true;
        }
    }

    return result;
}

/** create new spc2mid object. */
static PBoxSpcSeqStat *newPBoxSpcSeq (const byte *aRAM)
{
    PBoxSpcSeqStat *newSeq = (PBoxSpcSeqStat *) calloc(1, sizeof(PBoxSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        pboxSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = pboxSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delPBoxSpcSeq (PBoxSpcSeqStat **seq)
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
static void printHtmlInfoList (PBoxSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", pboxSpcVerToStrHtml(seq));

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    myprintf("</li>\n");
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (PBoxSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (PBoxSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (PBoxSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (PBoxSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double pboxSpcTempo (PBoxSpcSeqStat *seq)
{
    return (double) seq->tempo;
}

/** convert SPC channel volume into MIDI one. */
static int pboxSpcMidiVolOf (int value)
{
    const byte pboxVolumeTable[] = {
        0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
        0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
    };

    // read from table
    if (value < 0x10) {
        value = pboxVolumeTable[value];
    }
    // clipping
    if (value > 0x7f) {
        fprintf(stderr, "Warning: Volume too large than expected. [%d]\n", value);
        value = 0x7f;
    }

    if (pboxSpcVolIsLinear)
        return value * 2; // linear
    else
        return (int) floor(sqrt((double) value / 127) * 127 + 0.5); // closer to GM2 MIDI
}

/** convert SPC channel panpot into MIDI one. */
static int pboxSpcMidiPanOf (int value)
{
    double pan;
    int midiPan;

    if (value > 128)
        value = 128;

    pan = (value <= 127) ? value / 127.0 : 1.0;
    if (!pboxSpcVolIsLinear) {
        double panPI2 = atan2(pan, 1.0 - pan);
        pan = panPI2 / M_PI_2;
    }

    midiPan = (int) (pan * 126 + 0.5);
    if (midiPan != 0)
        midiPan++;
    return midiPan;
}

/** create new smf object and link to spc seq. */
static Smf *pboxSpcCreateSmf (PBoxSpcSeqStat *seq)
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

    switch (pboxSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, pboxSpcTempo(seq));

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

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, pboxSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void pboxSpcTruncateNote (PBoxSpcSeqStat *seq, int track)
{
    PBoxSpcTrackStat *tr = &seq->track[track];

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
static void pboxSpcTruncateNoteAll (PBoxSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        pboxSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool pboxSpcDequeueNote (PBoxSpcSeqStat *seq, int track)
{
    PBoxSpcTrackStat *tr = &seq->track[track];
    PBoxSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        dur = lastNote->dur;
        if (dur == 0)
            dur++;

        key = lastNote->key
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
static void pboxSpcDequeueNoteAll (PBoxSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        pboxSpcDequeueNote(seq, tr);
    }
}
