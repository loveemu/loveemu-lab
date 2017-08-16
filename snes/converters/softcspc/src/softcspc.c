/**
 * Software Creations spc2midi.
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
#include "softcspc.h"

#define APPNAME "Software Creations SPC2MIDI"
#define APPSHORTNAME "softcspc"
#define VERSION "[2014-02-15]"

static int softcSpcLoopMax = 2;            // maximum loop count of parser
static int softcSpcTextLoopMax = 1;        // maximum loop count of text output
static double softcSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool softcSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int softcSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool softcSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int softcSpcTimeBase = 48;
static int softcSpcForceSongIndex = -1;
static int softcSpcForceSongListAddr = -1;

static bool softcSpcPatchFixOverride = false;
static PatchFixInfo softcSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int softcSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_OK,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   0
#define SPC_ARAM_SIZE       0x10000

typedef struct TagSoftcSpcTrackStat SoftcSpcTrackStat;
typedef struct TagSoftcSpcSeqStat SoftcSpcSeqStat;
typedef void (*SoftcSpcEvent) (SoftcSpcSeqStat *, SeqEventReport *);

typedef struct TagSoftcSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int songIndexMax;
    SoftcSpcEvent event[256];
    PatchFixInfo patchFix[256];
    bool seqDetected;
} SoftcSpcVerInfo;

typedef struct TagSoftcSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} SoftcSpcNoteParam;

struct TagSoftcSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    SoftcSpcNoteParam note;     // current note param
    SoftcSpcNoteParam lastNote; // note params for last note
    int looped;             // how many times looped (internal)
    int patch;              // patch number (for pitch fix)
    int transpose;          // per-voice transpose
    byte callStack[0x100];  // subroutine stack
    int callStackPtr;       // subroutine stack ptr
    int callStackSize;      // subroutine stack size
    int defNoteLen;         // default note length (0:unused)
    int noteDurDir;         // duration (direct)
    int noteDurSub;         // duration (subtract)
};

struct TagSoftcSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    byte cpuControledVar;       // cpu-controled jump interface
    SoftcSpcVerInfo ver;         // game version info
    SoftcSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void softcSpcSetEventList (SoftcSpcSeqStat *seq);

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
FILE *softcSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int softcSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = softcSpcLoopMax;
    softcSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool softcSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        softcSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        softcSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            softcSpcPatchFix[patch].bankSelM = patch >> 7;
            softcSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            softcSpcPatchFix[patch].bankSelM = 0;
            softcSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        softcSpcPatchFix[patch].patchNo = patch & 0x7f;
        softcSpcPatchFix[patch].key = 0;
        softcSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        softcSpcPatchFix[src].bankSelM = bankM & 0x7f;
        softcSpcPatchFix[src].bankSelL = bankL & 0x7f;
        softcSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        softcSpcPatchFix[src].key = key;
        softcSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    softcSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *softcSpcVerToStrHtml (SoftcSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_OK:
        return "Accepted (Plok!)";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void softcSpcResetTrackParam (SoftcSpcSeqStat *seq, int track)
{
    SoftcSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->patch = 0;
    tr->callStackPtr = 0;
    tr->callStackSize = 0x10;
    tr->defNoteLen = 0;
    tr->noteDurDir = 0;
    tr->noteDurSub = 0;
}

/** reset before play/convert song. */
static void softcSpcResetParam (SoftcSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x85;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        SoftcSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        softcSpcResetTrackParam(seq, track);
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
    if (softcSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &softcSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int softcSpcCheckVer (SoftcSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int songLdCodeAddr;

    seq->timebase = softcSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.songIndexMax = 0;
    seq->ver.seqDetected = false;

    // (Plok!)
    // ; read voice ptr for song x
    // mov   a,x
    // cmp   a,#$05
    // bcs   $0588
    // mov   y,a               ; y = song select (0-4)
    // mov   x,#$00
    // mov   a,$1385+y
    // beq   $05a0             ; $00xx = unused channel
    // mov   $31,a
    // mov   a,$1380+y
    // mov   $30,a             ; channel 1 voice ptr
    // call  $0638
    // inc   x
    // inc   x
    if ((songLdCodeAddr = indexOfHexPat(seq->aRAM, "\x7d\x68.\xb0.\xfd\xcd\\\x00\xf6..\xf0\x0a\xc4.\xf6..\xc4.\x3f..\x3d\x3d", SPC_ARAM_SIZE, NULL)) != -1 &&
        mget2l(&seq->aRAM[songLdCodeAddr + 16]) + seq->aRAM[songLdCodeAddr + 2] == mget2l(&seq->aRAM[songLdCodeAddr + 9]) &&
        seq->aRAM[songLdCodeAddr + 14] == seq->aRAM[songLdCodeAddr + 19] + 1)
    {
        seq->ver.seqListAddr = mget2l(&seq->aRAM[songLdCodeAddr + 16]);
        seq->ver.songIndexMax = seq->aRAM[songLdCodeAddr + 2];
        seq->ver.songIndex = 1;
    }

    // overwrite loader params
    if (softcSpcForceSongListAddr != -1)
    {
        seq->ver.seqListAddr = softcSpcForceSongListAddr;
    }
    if (softcSpcForceSongIndex != -1)
    {
        seq->ver.songIndex = softcSpcForceSongIndex;
        if (seq->ver.songIndexMax == 0)
        {
            seq->ver.songIndexMax = 5;
        }
    }

    // branch version
    if (seq->ver.seqListAddr != -1 && seq->ver.songIndexMax > 0)
    {
        version = SPC_VER_OK;
    }

    seq->ver.id = version;
    softcSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool softcSpcDetectSeq (SoftcSpcSeqStat *seq)
{
    bool result = true;
    int seqHeaderReadOfs;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN || seq->ver.seqListAddr == -1 || seq->ver.songIndexMax < 0 || seq->ver.songIndex >= seq->ver.songIndexMax)
        return false;

    softcSpcResetParam(seq);

    result = false;
    seqHeaderReadOfs = seq->ver.seqListAddr + seq->ver.songIndex;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++)
    {
        int trackAddr = seq->aRAM[seqHeaderReadOfs] | (seq->aRAM[seqHeaderReadOfs + seq->ver.songIndexMax] << 8);
        if ((trackAddr & 0xff00) != 0)
        {
            seq->track[tr].pos = trackAddr;
            seq->track[tr].active = true;
            result = true;
        }
        seqHeaderReadOfs += (seq->ver.songIndexMax * 2);
    }

    return result;
}

/** create new spc2mid object. */
static SoftcSpcSeqStat *newSoftcSpcSeq (const byte *aRAM)
{
    SoftcSpcSeqStat *newSeq = (SoftcSpcSeqStat *) calloc(1, sizeof(SoftcSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        softcSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = softcSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delSoftcSpcSeq (SoftcSpcSeqStat **seq)
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
static void printHtmlInfoList (SoftcSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", softcSpcVerToStrHtml(seq));
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Index: %d</li>", seq->ver.songIndex);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (SoftcSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (SoftcSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (SoftcSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (SoftcSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double softcSpcTempoOf (SoftcSpcSeqStat *seq, int tempoValue)
{
    return (double) 60000000 / (seq->tempo * 125 * softcSpcTimeBase / 2);
}

/** convert SPC tempo into bpm. */
static double softcSpcTempo (SoftcSpcSeqStat *seq)
{
    return softcSpcTempoOf(seq, seq->tempo);
}

/** convert SPC velocity into MIDI one. */
static int softcSpcMidiVelOf (int value)
{
    if (softcSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int softcSpcMidiVolOf (int value)
{
    if (softcSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel panpot into MIDI one. */
static int softcSpcMidiPanOf (int value)
{
    return (value+0x80)/2; // linear (TODO: sine curve)
}

/** create new smf object and link to spc seq. */
static Smf *softcSpcCreateSmf (SoftcSpcSeqStat *seq)
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

    switch (softcSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, softcSpcTempo(seq));

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

        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void softcSpcTruncateNote (SoftcSpcSeqStat *seq, int track)
{
    SoftcSpcTrackStat *tr = &seq->track[track];

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
static void softcSpcTruncateNoteAll (SoftcSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        softcSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool softcSpcDequeueNote (SoftcSpcSeqStat *seq, int track)
{
    SoftcSpcTrackStat *tr = &seq->track[track];
    SoftcSpcNoteParam *lastNote = &tr->lastNote;
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
static void softcSpcDequeueNoteAll (SoftcSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        softcSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void softcSpcInactiveTrack(SoftcSpcSeqStat *seq, int track)
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
static void softcSpcAddTrackLoopCount(SoftcSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (softcSpcLoopMax > 0) ? softcSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= softcSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void softcSpcSeqAdvTick(SoftcSpcSeqStat *seq)
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
    seq->time += (double) 60 / softcSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void softcSpcEventUnknownInline (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, ev->addr, ev->track + 1);
}

/** vcmds: unidentified event. */
static void softcSpcEventUnidentified (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    softcSpcEventUnknownInline(seq, ev);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void softcSpcEventUnknown0 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    softcSpcEventUnknownInline(seq, ev);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void softcSpcEventUnknown1 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    softcSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void softcSpcEventUnknown2 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    softcSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void softcSpcEventUnknown3 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    softcSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void softcSpcEventUnknown4 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
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

    softcSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void softcSpcEventUnknown5 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
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

    softcSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (7 byte args). */
static void softcSpcEventUnknown7 (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5, arg6, arg7;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
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
    arg6 = seq->aRAM[*p];
    (*p)++;
    arg7 = seq->aRAM[*p];
    (*p)++;

    softcSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d, arg6 = %d, arg7 = %d", arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    strcat(ev->note, argDumpStr);
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void softcSpcEventNOP (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00-7f: note. */
static void softcSpcEventNote (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int key;
    int len;
    int dur;
    bool rest = (noteByte == 0);
    bool tie = false;

    if (tr->defNoteLen != 0)
    {
        len = tr->defNoteLen;
    }
    else
    {
        ev->size++;
        len = seq->aRAM[*p];
        (*p)++;
    }

    key = noteByte;
    if (tr->noteDurDir != 0)
    {
        // duration (direct)
        dur = tr->noteDurDir;
        if (dur > len)
        {
            fprintf(stderr, "Warning: Note duration %d exceeds its length %d.\n", dur, len);
            dur = len;
        }
    }
    else
    {
        // duration (subtract)
        dur = len - tr->noteDurSub;
        if (dur <= 0)
        {
            fprintf(stderr, "Warning: Note duration %d is less than 0.\n", dur);
            dur = 1;
        }
    }

    if (rest) {
        sprintf(ev->note, "Rest, len = %d", len);
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, key + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key
            + SPC_NOTE_KEYSHIFT);
        sprintf(argDumpStr, ", len = %d", len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    // output old note first
    if (!tie)
    {
        softcSpcDequeueNote(seq, ev->track);
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
            tr->lastNote.vel = 100;
            tr->lastNote.transpose = seq->transpose + tr->note.transpose;
            tr->lastNote.patch = tr->note.patch;
            tr->lastNote.active = true;
        }
        tr->lastNote.tied = tie;
    }
    tr->tick += len;
}

/** vcmd 80: end of track. */
static void softcSpcEventEndOfTrack (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    softcSpcInactiveTrack(seq, ev->track);

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 81: jump. */
static void softcSpcEventJump (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");

    // assumes backjump = loop
    if (arg1 < *p) {
        softcSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = arg1;

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 82: call subroutine. */
static void softcSpcEventSubroutine (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);

    sprintf(ev->note, "Call Subroutine, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-call");

    if (tr->callStackPtr + 2 > tr->callStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        softcSpcInactiveTrack(seq, ev->track);
        return;
    }

    tr->callStack[tr->callStackPtr++] = (byte)(*p);
    tr->callStack[tr->callStackPtr++] = (byte)((*p) >> 8);
    *p = arg1;
}

/** vcmd 83: end subroutine. */
static void softcSpcEventEndSubroutine (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "End Subroutine");
    strcat(ev->classStr, " ev-ret");

    if (tr->callStackPtr < 2) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        softcSpcInactiveTrack(seq, ev->track);
        return;
    }

    tr->callStackPtr -= 2;
    *p = mget2l(&tr->callStack[tr->callStackPtr]) + 2;

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 84: loop start. */
static void softcSpcEventLoopStart (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];

    sprintf(ev->note, "Loop Start, count = %d", arg1);
    strcat(ev->classStr, " ev-loopstart");

    if (tr->callStackPtr + 3 > tr->callStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        softcSpcInactiveTrack(seq, ev->track);
        return;
    }

    tr->callStack[tr->callStackPtr++] = (byte)(*p);
    tr->callStack[tr->callStackPtr++] = (byte)((*p) >> 8);
    tr->callStack[tr->callStackPtr++] = arg1;
    (*p)++;

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 85: loop end. */
static void softcSpcEventLoopEnd (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    byte loopCount;

    sprintf(ev->note, "Loop End/Continue");
    strcat(ev->classStr, " ev-loopend");

    if (tr->callStackPtr == 0) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        softcSpcInactiveTrack(seq, ev->track);
        return;
    }

    loopCount = tr->callStack[tr->callStackPtr - 1];
    if (--loopCount == 0) {
        // repeat end, fall through
        sprintf(ev->note, "Loop End");
        if (tr->callStackPtr < 3) {
            fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
            softcSpcInactiveTrack(seq, ev->track);
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

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 86: set default note length. */
static void softcSpcEventSetDefNoteLen (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->defNoteLen = arg1;

    sprintf(ev->note, "Default Note Length, len = %d", arg1);
    strcat(ev->classStr, " ev-notelen");

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 88: transpose (absolute). */
static void softcSpcEventTransposeAbs (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;

    sprintf(ev->note, "Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 89: set instrument. */
static void softcSpcEventInstrument (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd 92: set duration (direct). */
static void softcSpcEventSetDurationDir (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->noteDurDir = arg1;
    tr->noteDurSub = 0;

    sprintf(ev->note, "Duration (Direct), len = %d", arg1);
    strcat(ev->classStr, " ev-notedur");

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 93: set duration (subtract). */
static void softcSpcEventSetDurationSub (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->noteDurSub = arg1;
    tr->noteDurDir = 0;

    sprintf(ev->note, "Duration (Subtract), len = %d", arg1);
    strcat(ev->classStr, " ev-notedur");

    //if (!softcSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd af: set echo FIR. */
static void softcSpcEventEchoFIR (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int i;

    ev->size += 8;

    sprintf(ev->note, "Echo FIR, filter =");
    for (i = 0; i < 8; i++) {
        sprintf(argDumpStr, " %02X", seq->aRAM[*p]);
        strcat(ev->note, argDumpStr);
        (*p)++;
    }
    if (!softcSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echofir");
}

/** vcmd b6: set tempo. */
static void softcSpcEventSetTempo (SoftcSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    SoftcSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;

    sprintf(ev->note, "Set Tempo, tempo = %.1f", softcSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, softcSpcTempo(seq));
}

/** set pointers of each event. */
static void softcSpcSetEventList (SoftcSpcSeqStat *seq)
{
    int code;
    SoftcSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (SoftcSpcEvent) softcSpcEventUnidentified;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    for(code = 0x00; code <= 0x7f; code++) {
        event[code] = (SoftcSpcEvent) softcSpcEventNote;
    }
    event[0x80] = (SoftcSpcEvent) softcSpcEventEndOfTrack;
    event[0x81] = (SoftcSpcEvent) softcSpcEventJump;
    event[0x82] = (SoftcSpcEvent) softcSpcEventSubroutine;
    event[0x83] = (SoftcSpcEvent) softcSpcEventEndSubroutine;
    event[0x84] = (SoftcSpcEvent) softcSpcEventLoopStart;
    event[0x85] = (SoftcSpcEvent) softcSpcEventLoopEnd;
    event[0x86] = (SoftcSpcEvent) softcSpcEventSetDefNoteLen;
    event[0x87] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x88] = (SoftcSpcEvent) softcSpcEventTransposeAbs;
    event[0x89] = (SoftcSpcEvent) softcSpcEventInstrument;
    event[0x8a] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x8b] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x8c] = (SoftcSpcEvent) softcSpcEventUnknown3;
    event[0x8d] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x8e] = (SoftcSpcEvent) softcSpcEventUnknown3;
    event[0x8f] = (SoftcSpcEvent) softcSpcEventUnknown3;
    event[0x90] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x91] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x92] = (SoftcSpcEvent) softcSpcEventSetDurationDir;
    event[0x93] = (SoftcSpcEvent) softcSpcEventSetDurationSub;
    event[0x94] = (SoftcSpcEvent) softcSpcEventUnknown2;
    event[0x95] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x96] = (SoftcSpcEvent) softcSpcEventUnknown3;
    event[0x97] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x98] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x99] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x9a] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0x9b] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x9c] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x9d] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x9e] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0x9f] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0xa0] = (SoftcSpcEvent) softcSpcEventUnknown0;
    //event[0xa1] = (SoftcSpcEvent) softcSpcEventUnidentified; // start another song?
    event[0xa2] = (SoftcSpcEvent) softcSpcEventUnknown7;
    //event[0xa3] = (SoftcSpcEvent) softcSpcEventUnidentified; // complexed jump?
    //event[0xa4] = (SoftcSpcEvent) softcSpcEventUnidentified; // complexed jump?
    event[0xa5] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0xa6] = (SoftcSpcEvent) softcSpcEventUnknown1;
    //event[0xa7] = (SoftcSpcEvent) softcSpcEventUnidentified; // complexed jump?
    //event[0xa8] = (SoftcSpcEvent) softcSpcEventUnidentified; // complexed jump?
    //event[0xa9] = (SoftcSpcEvent) softcSpcEventUnidentified; // complexed jump?
    event[0xaa] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0xab] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0xac] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0xad] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0xae] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0xaf] = (SoftcSpcEvent) softcSpcEventEchoFIR;
    event[0xb0] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0xb1] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0xb2] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0xb3] = (SoftcSpcEvent) softcSpcEventUnknown2;
    event[0xb4] = (SoftcSpcEvent) softcSpcEventUnknown1;
    //event[0xb5] = (SoftcSpcEvent) softcSpcEventUnknown1;
    event[0xb6] = (SoftcSpcEvent) softcSpcEventSetTempo;
    event[0xb7] = (SoftcSpcEvent) softcSpcEventUnknown0;
    event[0xb8] = (SoftcSpcEvent) softcSpcEventUnknown1;
    //event[0xb9] = (SoftcSpcEvent) softcSpcEventUnidentified;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* softcSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    SoftcSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newSoftcSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = softcSpcCreateSmf(seq);

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

            SoftcSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (softcSpcTextLoopMax == 0 || seq->looped < softcSpcTextLoopMax)
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
            softcSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= softcSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, softcSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    softcSpcTruncateNoteAll(seq);
    softcSpcDequeueNoteAll(seq);

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
        delSoftcSpcSeq(&seq);
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
Smf* softcSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = softcSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* softcSpcToMidiFromFile (const char *filename)
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

    smf = softcSpcToMidi(data, size);

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
//  { "bendrange", '\0', 0, cmdOptBendRange, "<range>", "set pitch bend sensitivity (0:auto)" },
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
    softcSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    softcSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    softcSpcForceSongListAddr = songListAddr;
    return true;
}

/** set pitchbend range. */
static bool cmdOptBendRange (void)
{
    int range = strtol(gArgv[0], NULL, 0);
    softcSpcPitchBendSens = range;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (softcSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    softcSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    softcSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    softcSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    softcSpcMidiResetType = SMF_RESET_GM2;
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
            softcSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = softcSpcToMidiFromFile(gArgv[0]);
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
