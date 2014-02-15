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
#define VERSION "[2014-02-15]"

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
static int pboxSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

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
        if (preferBankMSB)
        {
            pboxSpcPatchFix[patch].bankSelM = patch >> 7;
            pboxSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            pboxSpcPatchFix[patch].bankSelM = 0;
            pboxSpcPatchFix[patch].bankSelL = patch >> 7;
        }
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
        smfInsertGM1SystemOn(smf, 0, 0, 0);
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

/** inactivate track. */
static void pboxSpcInactiveTrack(PBoxSpcSeqStat *seq, int track)
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
static void pboxSpcAddTrackLoopCount(PBoxSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (pboxSpcLoopMax > 0) ? pboxSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= pboxSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void pboxSpcSeqAdvTick(PBoxSpcSeqStat *seq)
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
    seq->time += (double) 60 / pboxSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void pboxSpcEventUnknownInline (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
}

/** vcmds: unidentified event. */
static void pboxSpcEventUnidentified (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    pboxSpcEventUnknownInline(seq, ev);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void pboxSpcEventUnknown0 (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    pboxSpcEventUnknownInline(seq, ev);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void pboxSpcEventUnknown1 (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    pboxSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void pboxSpcEventUnknown2 (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    pboxSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void pboxSpcEventUnknown3 (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    pboxSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void pboxSpcEventUnknown4 (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
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

    pboxSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void pboxSpcEventUnknown5 (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
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

    pboxSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void pboxSpcEventNOP (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00-3f: note */
static void pboxSpcEventNote (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int keyBits = noteByte & 15;
    int key = -1;
    int len;
    int dur;
    bool rest = (keyBits == 0);

    bool tieBit = (noteByte & 0x10) != 0;
    bool hasLength = (noteByte & 0x20) == 0;
    bool slur;
    bool tie;

    // take length byte
    if (hasLength) {
        ev->size++;
        tr->lastNoteLen = seq->aRAM[*p];
        (*p)++;
    }

    // determine note number
    if (!rest) {
        key = (tr->octave * 12) + (keyBits - 1) + seq->transpose + tr->transpose;
    }

    // slur, or tie?
    slur = (!rest && tr->lastNote.active && tr->lastNote.tied &&
        tr->lastNote.key != key);
    tie = (!rest && tr->lastNote.active && tr->lastNote.tied &&
        tr->lastNote.key == key);

    // determine the note length
    len = tr->lastNoteLen;
    if (tr->quantize == 0 || rest || slur || tie) {
        // full length

        // the condition is somewhat different from the actual engine.
        // tie/slur of actual engine does not work very well if quantize != 0.
        // (Gakkou de atta kowai hanashi does not, at least)
        // I guess the behavior is simply unexpected, so I decided to ignore it.

        dur = len;
    }
    else {
        dur = (len * tr->quantize / 8) & 0xff;
    }

    if (rest) {
        sprintf(ev->note, "Rest, len = %d", len);
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, key
            + seq->ver.patchFix[tr->note.patch].key
            + SPC_NOTE_KEYSHIFT);
        sprintf(argDumpStr, "%s, len = %d", tieBit ? (slur ? " (Slur)" : " (Tied)") : "", len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    if (!rest && slur)
    {
        if (!pboxSpcLessTextInSMF)
           smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }

    // output old note first
    if (!tie)
    {
        pboxSpcDequeueNote(seq, ev->track);
    }

    // set new note
    if (!rest) {
        if (tie) {
            tr->lastNote.dur += dur;
        }
        else {
            tr->lastNote.tick = ev->tick;
            tr->lastNote.dur = dur;
            tr->lastNote.key = key;
            tr->lastNote.vel = 127;
            tr->lastNote.patch = tr->note.patch;
        }
        tr->lastNote.tied = tieBit;
        tr->lastNote.active = true;
    }
    tr->tick += len;
}

/** vcmd 40-47: set octave */
static void pboxSpcEventSetOctaveByOp (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->octave = (ev->code - 0x40);

    sprintf(ev->note, "Set Octave, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-octave");
}

/** vcmd 48-4f: set octave */
static void pboxSpcEventSetQuantizeByOp (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->quantize = (ev->code - 0x48);

    sprintf(ev->note, "Set Quantize, quantize = %d", tr->quantize);
    strcat(ev->classStr, " ev-quantize");
}

/** vcmd 50-5f: set octave */
static void pboxSpcEventSetVolumeByOp (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int *p = &seq->track[ev->track].pos;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];

    tr->volumeIndex = (ev->code - 0x50);

    sprintf(ev->note, "Volume, vol = %d", tr->volumeIndex);
    strcat(ev->classStr, " ev-vol");

    //if (!hudsonSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, pboxSpcMidiVolOf(tr->volumeIndex));
}

/** vcmd 60-df: set instrument */
static void pboxSpcEventSetInstrumentByOp (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int patch;

    patch = (ev->code - 0x60);

    tr->note.patch = patch;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[patch].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[patch].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[patch].patchNo);

    sprintf(ev->note, "Set Instrument, patch = %d", patch);
    strcat(ev->classStr, " ev-patch");
}

/** vcmd e0: set tempo. */
static void pboxSpcEventSetTempo (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;

    sprintf(ev->note, "Set Tempo, tempo = %.1f", pboxSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, pboxSpcTempo(seq));
}

/** vcmd e1: tuning. */
static void pboxSpcEventTuning (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Tuning, Hz += %d", arg1);
    strcat(ev->classStr, " ev-tuning");

    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e2: transpose (absolute). */
static void pboxSpcEventTransposeAbs (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    tr->transpose = arg1;

    //if (!pboxSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e3: set panpot. */
static void pboxSpcEventPanpot (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->panpot = (arg1 <= 128) ? arg1 : 128;

    sprintf(ev->note, "panpot, pan = %d", tr->panpot);
    strcat(ev->classStr, " ev-pan");

    //if (!pboxSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, pboxSpcMidiPanOf(tr->panpot));
}

/** vcmd e4: increase octave. */
static void pboxSpcEventIncreaseOctave (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->octave = (tr->octave + 1) & 0xff;

    sprintf(ev->note, "Increase Octave, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-incoctave");
}

/** vcmd e5: decrease octave. */
static void pboxSpcEventDecreaseOctave (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->octave = (tr->octave - 1) & 0xff;

    sprintf(ev->note, "Decrease Octave, octave = %d", tr->octave);
    strcat(ev->classStr, " ev-decoctave");
}

/** vcmd e6: increase volume. */
static void pboxSpcEventIncreaseVolume (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    if (tr->volumeIndex < 0x0f) {
        tr->volumeIndex++;
    }

    sprintf(ev->note, "Increase Volume, volume = %d", tr->volumeIndex);
    strcat(ev->classStr, " ev-incvolume");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, pboxSpcMidiVolOf(tr->volumeIndex));
}

/** vcmd e7: decrease volume. */
static void pboxSpcEventDecreaseVolume (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    if (tr->volumeIndex > 0) {
        tr->volumeIndex--;
    }

    sprintf(ev->note, "Decrease Volume, volume = %d", tr->volumeIndex);
    strcat(ev->classStr, " ev-decvolume");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, pboxSpcMidiVolOf(tr->volumeIndex));
}

/** vcmd e8: vibrato params. */
static void pboxSpcEventVibratoParams (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
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

    // for researchers :)
    fprintf(stderr, "Warning: Vibrato %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);

    sprintf(ev->note, "Vibrato, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->classStr, " ev-vibrato");

    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e9: vibrato on/off. */
static void pboxSpcEventVibratoSwitch (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    bool doSlide;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    doSlide = (arg1 != 0);

    sprintf(ev->note, "Vibrato On/Off, enabled = %s", doSlide ? "true" : "false");
    strcat(ev->classStr, " ev-vibratosw");
}

/** vcmd ea: echo off. */
static void pboxSpcEventEchoOff (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo Off");
    strcat(ev->classStr, " ev-echooff");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 0);
}

/** vcmd eb: echo on. */
static void pboxSpcEventEchoOn (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 40);
}

/** vcmd ec: repeat start. */
static void pboxSpcEventRepeatStart (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Repeat Start, count = %d", arg1);
    strcat(ev->classStr, " ev-repeatstart");

    if (tr->rptStackPtr + 5 > tr->rptStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->rptStackPtr);
        pboxSpcInactiveTrack(seq, ev->track);
        return;
    }

    // repeat start address
    tr->rptStack[tr->rptStackPtr++] = (byte)(*p);
    tr->rptStack[tr->rptStackPtr++] = (byte)((*p) >> 8);
    // repeat end address
    tr->rptStack[tr->rptStackPtr++] = 0;
    tr->rptStack[tr->rptStackPtr++] = 0;
    // repeat count
    tr->rptStack[tr->rptStackPtr++] = arg1;
}

/** vcmd ed: repeat end. */
static void pboxSpcEventRepeatEnd (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool doJump = false;

    sprintf(ev->note, "Repeat End");
    strcat(ev->classStr, " ev-repeatstart");

    if (tr->rptStackPtr < 5) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->rptStackPtr);
        pboxSpcInactiveTrack(seq, ev->track);
        return;
    }

    if (tr->rptStack[tr->rptStackPtr - 5 + 4] == 0xff) {
        // infinite loop
        doJump = true;
        pboxSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    else {
        // decrease repeat count (0 becomes an infinite loop, as a result)
        tr->rptStack[tr->rptStackPtr - 5 + 4] = (tr->rptStack[tr->rptStackPtr - 5 + 4] - 1) & 0xff;
        doJump = (tr->rptStack[tr->rptStackPtr - 5 + 4] != 0);
    }

    if (doJump) {
        // set repeat end address
        tr->rptStack[tr->rptStackPtr - 5 + 2] = (byte)(*p);
        tr->rptStack[tr->rptStackPtr - 5 + 3] = (byte)((*p) >> 8);
        // jump to start address
        *p = (tr->rptStack[tr->rptStackPtr - 5 + 1] << 8) | tr->rptStack[tr->rptStackPtr - 5 + 0];
    }
    else {
        // repeat end
        tr->rptStackPtr -= 5;
    }

    sprintf(argDumpStr, ", again = %s", doJump ? "true" : "false");
    strcat(ev->note, argDumpStr);
}

/** vcmd ee: repeat break. */
static void pboxSpcEventRepeatBreak (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    bool doJump = false;

    sprintf(ev->note, "Repeat Break");
    strcat(ev->classStr, " ev-repeatbreak");

    if (tr->rptStackPtr < 5) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->rptStackPtr);
        pboxSpcInactiveTrack(seq, ev->track);
        return;
    }

    doJump = (tr->rptStack[tr->rptStackPtr - 5 + 4] == 1);
    if (doJump) {
        // jump to end address
        *p = (tr->rptStack[tr->rptStackPtr - 5 + 3] << 8) | tr->rptStack[tr->rptStackPtr - 5 + 2];
        // repeat end
        tr->rptStackPtr -= 5;
    }

    sprintf(argDumpStr, ", break = %s", doJump ? "true" : "false");
    strcat(ev->note, argDumpStr);
}

/** vcmd f1: write dsp reg. */
static void pboxSpcEventWriteDSP (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int reg, value;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    reg = seq->aRAM[*p];
    (*p)++;
    value = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Write DSP, reg = $%02x, value = $%02x (%d)", reg, value, value);
    strcat(ev->classStr, " ev-writedsp");

    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f2: set noise params. */
static void pboxSpcEventSetNoiseParams (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int target = 0;
    int freq = -1;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    target = (arg1 & 3);
    if ((arg1 & 0x80) == 0) {
        freq = (arg1 >> 2) & 15;
    }

    sprintf(ev->note, "Set Noise Params");
    if (target == 0) {
        sprintf(argDumpStr, ", target = keep current");
    }
    else {
        sprintf(argDumpStr, ", target = %d", target);
    }
    strcat(ev->note, argDumpStr);
    if (freq < 0) {
        sprintf(argDumpStr, ", frequency = keep current");
    }
    else {
        sprintf(argDumpStr, ", frequency = %d (%d Hz)", freq, spcNCKTable[freq]);
    }
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-noiseparam");

    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f3: set ADSR. */
static void pboxSpcEventSetADSR (PBoxSpcSeqStat *seq, SeqEventReport *ev)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int arPb, drPb, srPb, slPb, xxPb;
    int ar, dr, sl, sr;

    ev->size += 5;
    arPb = seq->aRAM[*p];
    (*p)++;
    drPb = seq->aRAM[*p];
    (*p)++;
    srPb = seq->aRAM[*p];
    (*p)++;
    slPb = seq->aRAM[*p];
    (*p)++;
    xxPb = seq->aRAM[*p];
    (*p)++;

    ar = (arPb * 0x0f) / 255;
    dr = (drPb * 0x07) / 255;
    sl = (slPb * 0x07) / 255;
    sr = (srPb * 0x1f) / 255;

    sprintf(ev->note, "Set ADSR, AR = %d (%d: %.1f%s), DR = %d (%d: %.1f%s), SR = %d (%d: %.1f%s), SL = %d (%d/8), arg5 = %d",
        arPb, ar, (spcARTable[ar] >= 1) ? spcARTable[ar] : spcARTable[ar] * 1000, (spcARTable[ar] >= 1) ? "s" : "ms",
        drPb, dr, (spcARTable[dr] >= 1) ? spcDRTable[dr] : spcDRTable[dr] * 1000, (spcDRTable[dr] >= 1) ? "s" : "ms",
        srPb, sr, (spcSRTable[sr] >= 1) ? spcSRTable[sr] : spcSRTable[sr] * 1000, (spcSRTable[sr] >= 1) ? "s" : "ms",
        slPb, sl + 1,
        xxPb);
    if (!pboxSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-adsr");
}

/** vcmd f5: end of track. */
static void pboxSpcEventEndOfTrack (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    PBoxSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    pboxSpcInactiveTrack(seq, ev->track);

    //if (!pboxSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f6: set volume. */
static void pboxSpcEventVolume (PBoxSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    PBoxSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->volumeIndex = arg1;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!pboxSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, pboxSpcMidiVolOf(tr->volumeIndex));
}

/** set pointers of each event. */
static void pboxSpcSetEventList (PBoxSpcSeqStat *seq)
{
    int code;
    PBoxSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (PBoxSpcEvent) pboxSpcEventUnidentified;
    }

    for (code = 0x00; code <= 0x3f; code++) {
        event[code] = (PBoxSpcEvent) pboxSpcEventNote;
    }
    for (code = 0x40; code <= 0x47; code++) {
        event[code] = (PBoxSpcEvent) pboxSpcEventSetOctaveByOp;
    }
    for (code = 0x48; code <= 0x4f; code++) {
        event[code] = (PBoxSpcEvent) pboxSpcEventSetQuantizeByOp;
    }
    for (code = 0x50; code <= 0x5f; code++) {
        event[code] = (PBoxSpcEvent) pboxSpcEventSetVolumeByOp;
    }
    for (code = 0x60; code <= 0xdf; code++) {
        event[code] = (PBoxSpcEvent) pboxSpcEventSetInstrumentByOp;
    }
    event[0xe0] = (PBoxSpcEvent) pboxSpcEventSetTempo;
    event[0xe1] = (PBoxSpcEvent) pboxSpcEventTuning;
    event[0xe2] = (PBoxSpcEvent) pboxSpcEventTransposeAbs;
    event[0xe3] = (PBoxSpcEvent) pboxSpcEventPanpot;
    event[0xe4] = (PBoxSpcEvent) pboxSpcEventIncreaseOctave;
    event[0xe5] = (PBoxSpcEvent) pboxSpcEventDecreaseOctave;
    event[0xe6] = (PBoxSpcEvent) pboxSpcEventIncreaseVolume;
    event[0xe7] = (PBoxSpcEvent) pboxSpcEventDecreaseVolume;
    event[0xe8] = (PBoxSpcEvent) pboxSpcEventVibratoParams;
    event[0xe9] = (PBoxSpcEvent) pboxSpcEventVibratoSwitch;
    event[0xea] = (PBoxSpcEvent) pboxSpcEventEchoOff;
    event[0xeb] = (PBoxSpcEvent) pboxSpcEventEchoOn;
    event[0xec] = (PBoxSpcEvent) pboxSpcEventRepeatStart;
    event[0xed] = (PBoxSpcEvent) pboxSpcEventRepeatEnd;
    event[0xee] = (PBoxSpcEvent) pboxSpcEventRepeatBreak;
    event[0xef] = (PBoxSpcEvent) pboxSpcEventNOP;
    event[0xf0] = (PBoxSpcEvent) pboxSpcEventNOP;
    event[0xf1] = (PBoxSpcEvent) pboxSpcEventWriteDSP;
    event[0xf2] = (PBoxSpcEvent) pboxSpcEventSetNoiseParams;
    event[0xf3] = (PBoxSpcEvent) pboxSpcEventSetADSR;
    event[0xf4] = (PBoxSpcEvent) pboxSpcEventUnknown1;
    event[0xf5] = (PBoxSpcEvent) pboxSpcEventEndOfTrack;
    event[0xf6] = (PBoxSpcEvent) pboxSpcEventVolume;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* pboxSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    PBoxSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newPBoxSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = pboxSpcCreateSmf(seq);

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

            PBoxSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (pboxSpcTextLoopMax == 0 || seq->looped < pboxSpcTextLoopMax)
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
            pboxSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= pboxSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, pboxSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    pboxSpcTruncateNoteAll(seq);
    pboxSpcDequeueNoteAll(seq);

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
        delPBoxSpcSeq(&seq);
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
Smf* pboxSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = pboxSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* pboxSpcToMidiFromFile (const char *filename)
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

    smf = pboxSpcToMidi(data, size);

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

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm1", '\0', 0, cmdOptGM1, "", "Insert GM1 System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
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
    pboxSpcSetLoopCount(loopCount);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (pboxSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    pboxSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    pboxSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    pboxSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    pboxSpcMidiResetType = SMF_RESET_GM2;
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
            pboxSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = pboxSpcToMidiFromFile(gArgv[0]);
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
