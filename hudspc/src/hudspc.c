/**
 * Hudson spc2midi. (Super Bomberman 3)
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
#define VERSION "[2013-05-23]"

static int hudsonSpcLoopMax = 2;            // maximum loop count of parser
static int hudsonSpcTextLoopMax = 1;        // maximum loop count of text output
static double hudsonSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool hudsonSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool hudsonSpcNoPatchChange = false; // XXX: hack, should be false for serious conversion
static bool hudsonSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int hudsonSpcTimeBase = 48;
static int hudsonSpcForceSongIndex = -1;
static int hudsonSpcForceSongListAddr = -1;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_SBM3,           // Super Bomberman 3
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

typedef struct TagHudsonSpcTrackStat HudsonSpcTrackStat;
typedef struct TagHudsonSpcSeqStat HudsonSpcSeqStat;
typedef void (*HudsonSpcEvent) (HudsonSpcSeqStat *, SeqEventReport *, Smf *);

typedef struct TagHudsonSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int seqHeaderAddr;
    HudsonSpcEvent event[256]; // vcmds
    PatchFixInfo patchFix[256];
} HudsonSpcVerInfo;

typedef struct TagHudsonNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int len;            // total length (tick)
    bool tied;          // if the note tied
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} HudsonNoteParam;

struct TagHudsonSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    HudsonNoteParam note;     // current note param
    HudsonNoteParam lastNote; // note params for last note
    byte quantize;          // quantize (0-8: qN, 9-: @qN)
    byte octave;            // octave
    int patch;              // patch number (for pitch fix)
    byte callStack[0x10];   // subroutine stack
    int callStackPtr;       // subroutine stack ptr
    int callStackSize;      // subroutine stack size
};

struct TagHudsonSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    HudsonSpcVerInfo ver;       // game version info
    HudsonSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
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

//----

/** returns version string of music engine. */
static const char *hudsonSpcVerToStrHtml (HudsonSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_SBM3:
        return "Super Bomberman 3";
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
    tr->octave = 2;
    tr->quantize = 8;
    tr->lastNote.active = false;
    tr->callStackPtr = 0;
    tr->callStackSize = 0x10; // Super Bomberman 3
//    tr->loopCount = 0;
}

/** reset before play/convert song. */
static void hudsonSpcResetParam (HudsonSpcSeqStat *seq)
{
    int track;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 120;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        HudsonSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        hudsonSpcResetTrackParam(seq, track);
    }
}

/** returns what version the sequence is, and sets individual info. */
static int hudsonSpcCheckVer (HudsonSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;

    seq->timebase = hudsonSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;

    if (hudsonSpcForceSongListAddr >= 0) {
        seq->ver.seqListAddr = hudsonSpcForceSongListAddr;
        version = SPC_VER_SBM3;
    }
    else {
        // TODO: more flexible autosearch
        // the following code targets Super Bomberman 3
        seq->ver.seqListAddr = mget2l(&aRAM[0x07c2]);
        version = SPC_VER_SBM3;
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

/** detect now playing and prepare for analyze. */
static bool hudsonSpcDetectSeq (HudsonSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int seqHeaderAddr;
    bool result;
    int tr;

    byte trActiveBits;
    int trHeaderOffset;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    result = false;
    seqHeaderAddr = seq->ver.seqHeaderAddr;

    trActiveBits = mget1(&aRAM[seqHeaderAddr]);
    trHeaderOffset = 1;
    // track list (reverse order, big-endian)
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        // if active, read more info
        if ((trActiveBits & (1 << tr)) != 0) {
            seq->track[tr].pos = mget2l(&aRAM[seqHeaderAddr + trHeaderOffset]);
            trHeaderOffset += 2;

            seq->track[tr].active = true;
            result = true;
        }
    }
    hudsonSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static HudsonSpcSeqStat *newHudsonSpcSeq (const byte *aRAM)
{
    HudsonSpcSeqStat *newSeq = (HudsonSpcSeqStat *) calloc(1, sizeof(HudsonSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        hudsonSpcCheckVer(newSeq);
        if (!hudsonSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
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

    myprintf("          <li>Version: %s</li>\n", hudsonSpcVerToStrHtml(seq));
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    myprintf(" (Song $%02x)", seq->ver.songIndex);
    myprintf("</li>\n");
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

/** insert program event. */
/*
static bool smfInsertHudsonSpcProgram (Smf* seq, int time, int channel, int track, int programNumber)
{
    bool result;

    programNumber &= 0xff;
    result  = smfInsertControl(seq, time, channel, track, SMF_CONTROL_BANKSELM, hudsonSpcPatchFixInfo[programNumber].bankSelM);
    result &= smfInsertControl(seq, time, channel, track, SMF_CONTROL_BANKSELL, hudsonSpcPatchFixInfo[programNumber].bankSelL);
    result &= smfInsertProgram(seq, time, channel, track, hudsonSpcPatchFixInfo[programNumber].patchNo);
    return result;
    //return false;
}
*/

/** create new smf object and link to spc seq. */
static Smf *hudsonSpcCreateSmf (HudsonSpcSeqStat *seq)
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
    smfInsertTempoBPM(smf, 0, 0, hudsonSpcTempo(seq));
    smfInsertGM1SystemOn(smf, 0, 0, 0);

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        // TODO: add code
    }
    return smf;
}

//----

static char argDumpStr[512];

/** returns note name from vbyte. */
/*
void hudsonSpcNoteNameFromVbyte (char *buf, int vbyte, int keyFix)
{
    int key = (vbyte & 0x7f) + SPC_NOTE_KEYSHIFT + keyFix;

    if (key >= 0x00 && key <= 0x7f)
        getNoteName(buf, key);
    else
        sprintf(buf, "Note %d", key);
}
*/

/** read duration rate from table. */
//static int hudsonSpcDurRateOf (HudsonSpcSeqStat *seq, int index)
//{
//    return mget1(&seq->aRAM[seq->ver.durTableAddr + index]);
//}

/** read velocity from table. */
//static int hudsonSpcVelRateOf (HudsonSpcSeqStat *seq, int index)
//{
//    return mget1(&seq->aRAM[seq->ver.velTableAddr + index]);
//}

/** truncate note. */
static void hudsonSpcTruncateNote (HudsonSpcSeqStat *seq, int track)
{
    HudsonSpcTrackStat *tr = &seq->track[track];

    if (tr->lastNote.active && tr->lastNote.len > 0) {
        int lastTick = tr->lastNote.tick + tr->lastNote.len;
        int diffTick = lastTick - seq->tick;

        if (diffTick > 0) {
            tr->lastNote.len -= diffTick;
            if (tr->lastNote.len == 0)
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
    // TODO
    return false;
#if 0
    HudsonSpcTrackStat *tr = &seq->track[track];
    HudsonSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        //if (lastNote->tied)
            dur = (lastNote->dur * lastNote->durRate) >> 8;
        //else
        //    dur = (lastNote->dur - lastNote->lastDur)
        //        + ((lastNote->lastDur * lastNote->durRate) >> 8);
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
#endif
/*
    int tick = seq->track[track].lastNoteTick;
    int note = seq->track[track].lastNote;
    int length = seq->track[track].lastNoteDur;
    int duration;
    int durRate = seq->track[track].lastDurRate;
    int velocity = seq->track[track].lastVel;
    int transpose = seq->track[track].lastTranspose;
    int keyFix = seq->track[track].lastKeyFix;
    bool perc = seq->track[track].lastIsPerc;
    bool tied = seq->track[track].tied;
    bool result = false;
    bool put = (note >= 0);

    if (put)
    {
        if (!tied)
            duration = (length * durRate) >> 8;
        else
            duration = (length - seq->track[track].lastLen)
                     + ((seq->track[track].lastLen * durRate) >> 8);

        if (duration == 0)
            duration++;

        result = smfInsertHudsonSpcNote(smf, tick, track, note + (perc ? 0 : transpose), keyFix, perc, velocity, duration);
    }
    seq->track[track].lastNote = -1;
    return result;
*/
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
static void hudsonSpcEventUnknownInline (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");
    if (!hudsonSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X\n", ev->code);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X\n", ev->code);
}

/** vcmds: unidentified event. */
static void hudsonSpcEventUnidentified (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    ev->unidentified = true;
    hudsonSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (no args). */
static void hudsonSpcEventUnknown0 (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    hudsonSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (1 byte arg). */
static void hudsonSpcEventUnknown1 (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    hudsonSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmds: unknown event (2 byte args). */
static void hudsonSpcEventUnknown2 (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    hudsonSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmd: unknown event (3 byte args). */
static void hudsonSpcEventUnknown3 (HudsonSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    hudsonSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
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
    bool xxxBit = (noteByte & 8) != 0;
    int keyBits = noteByte >> 4;

    if (lenBits != 0) {
        const byte lenTbl[8] = { 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01 };
        len = lenTbl[lenBits - 1];
    }
    else {
        ev->size++;
        len = mget1(&seq->aRAM[*p]);
        (*p)++;
    }

    if (!xxxBit) {
        if (tr->quantize <= 8) {
            dur = len * tr->quantize / 8;
        }
        else {
            dur = len - (tr->quantize - 8);
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

    if (rest) {
        sprintf(ev->note, "Rest, len = %d", len);
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, note + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key + SPC_NOTE_KEYSHIFT);
        sprintf(argDumpStr, ", len = %d", len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    // outputput old note first
    hudsonSpcDequeueNote(seq, ev->track);

    // set new note
//    if (!rest) {
//        tr->lastNote.tick = ev->tick;
//        tr->lastNote.dur = tr->note.dur;
//        tr->lastNote.key = note;
//        tr->lastNote.durRate = tr->note.durRate;
//        tr->lastNote.vel = tr->note.vel;
//        tr->lastNote.transpose = seq->transpose + tr->note.transpose;
//        tr->lastNote.patch = tr->note.patch;
//        tr->lastNote.tied = false;
//        tr->lastNote.active = true;
//    }
    tr->tick += len;
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
        *p = mget2l(&tr->callStack[tr->callStackPtr - 3]);
    }
}

/** vcmd ff: end subroutine / end of track. */
static void hudsonSpcEventEndSubroutine (HudsonSpcSeqStat *seq, SeqEventReport *ev)
{
    HudsonSpcTrackStat *tr = &seq->track[ev->track];

    if (tr->callStackPtr == 0) {
        sprintf(ev->note, "End of Track");
        strcat(ev->classStr, " ev-end");

        hudsonSpcInactiveTrack(seq, ev->track);
    }
    else {
        sprintf(ev->note, "End Subroutine (NYI)");
        strcat(ev->classStr, " ev-ret");

        hudsonSpcInactiveTrack(seq, ev->track);
    }
}

/** set pointers of each event. */
static void hudsonSpcSetEventList (HudsonSpcSeqStat *seq)
{
    int code;
    HudsonSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (HudsonSpcEvent) hudsonSpcEventUnidentified;
    }

    for (code = 0x00; code <= 0xcf; code++) {
        event[code] = (HudsonSpcEvent) hudsonSpcEventNote;
    }
    event[0xd0] = (HudsonSpcEvent) hudsonSpcEventNOP;
    event[0xd1] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xd2] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xd3] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xd4] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xd5] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xd6] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xd7] = (HudsonSpcEvent) hudsonSpcEventNOP2;
    event[0xd8] = (HudsonSpcEvent) hudsonSpcEventNOP2;
    event[0xd9] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xda] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xdb] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xdc] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xdd] = (HudsonSpcEvent) hudsonSpcEventLoopStart;
    event[0xde] = (HudsonSpcEvent) hudsonSpcEventLoopEnd;
    //event[0xdf] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
    //event[0xe0] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
    event[0xe1] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xe2] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
    event[0xe3] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xe4] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
    event[0xe5] = (HudsonSpcEvent) hudsonSpcEventUnknown3;
    event[0xe6] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xe7] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xe8] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xe9] = (HudsonSpcEvent) hudsonSpcEventUnknown3;
    event[0xea] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xeb] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xec] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xed] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xee] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xef] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
    event[0xf0] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xf1] = (HudsonSpcEvent) hudsonSpcEventUnknown2;
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
    event[0xfe] = (HudsonSpcEvent) hudsonSpcEventUnknown1;
    event[0xff] = (HudsonSpcEvent) hudsonSpcEventUnknown0;
    event[0xff] = (HudsonSpcEvent) hudsonSpcEventEndSubroutine;

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

    if (seq->ver.id == SPC_VER_UNKNOWN) {
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
                seq->ver.event[ev.code](seq, &ev, smf);

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

typedef void (*CmdDispatcher) (void);

typedef struct TagCmdOptDefs {
    char *name;
    char shortName;
    int numArgs;
    CmdDispatcher dispatch;
    char *syntax;
    char *description;
} CmdOptDefs;

static void cmdOptHelp (void);
static void cmdOptLoop (void);
static bool cmdOptSong (void);
static bool cmdOptSongList (void);

static CmdOptDefs optDef[] = {
    { "--help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "--loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
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
static void cmdOptHelp (void)
{
    man();
}

/** set loop count */
static void cmdOptLoop (void)
{
    int loopCount = strtol(gArgv[0], NULL, 0);
    hudsonSpcSetLoopCount(loopCount);
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

/** handle command-line options. */
static void handleCmdLineOpts (void)
{
    int op;

    // dispatch options
    while (gArgc > 0 && gArgv[0][0] == '-') {
        bool unknown = true;

        // match for each option
        for (op = 0; op < countof(optDef); op++) {
            if (strcmp(gArgv[0], optDef[op].name) == 0) {
                unknown = false;
                gArgc--;
                gArgv++;
                if (gArgc >= optDef[op].numArgs) {
                    optDef[op].dispatch();
                    gArgc -= optDef[op].numArgs;
                    gArgv += optDef[op].numArgs;
                }
                else {
                    fprintf(stderr, "Error: too few arguments for option \"%s\".\n", optDef[op].name);
                    gArgv += gArgc;
                    gArgc = 0;
                }
                break;
            }
        }
        if (unknown) {
            fprintf(stderr, "Error: unknown option \"%s\".\n", gArgv[0]);
            gArgc--;
            gArgv++;
        }
    }
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
