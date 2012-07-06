/**
 * Capcom spc2midi, based on Nintendo SPC.
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
#include "capspc.h"

#define APPNAME "Capcom SPC2MIDI"
#define APPSHORTNAME "capspc"
#define VERSION "[2008-12-11 dev]"

static int nintSpcLoopMax = 2;              // maximum loop count of parser
static int nintSpcTextLoopMax = 1;          // maximum loop count of text output
static double nintSpcTimeLimit = 1200;      // time limit of conversion (for safety)
static bool nintSpcLessTextInSMF = false;   // decreases amount of texts in SMF output

static bool nintSpcNoPatchChange = false;   // XXX: hack, should be false for serious conversion
static bool nintSpcVolIsLinear = false;     // assumes volume curve between SPC and MIDI is linear

static int nintSpcForceSongAddr = -1;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_MMX,            // Mega Man X
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

typedef struct TagNintSpcTrackStat NintSpcTrackStat;
typedef struct TagNintSpcSeqStat NintSpcSeqStat;
typedef void (*NintSpcEvent) (NintSpcSeqStat *, SeqEventReport *, Smf *);

typedef struct TagNintSpcVerInfo {
    int id;
    int seqHeaderAddr;
    int durTableAddr;
    int velTableAddr;
    NintSpcEvent event[256]; // vcmds
    PatchFixInfo patchFix[256];
} NintSpcVerInfo;

struct TagNintSpcTrackStat {
    bool active;        // if the channel is still active
    bool used;          // if the channel used once or not
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    int prevTick;       // previous timing (for pitch slide)
    NoteParam note;     // current note param
    NoteParam lastNote; // note params for last note
    int patch;          // patch number (for pitch fix)
    int loopStart;      // loop start address for loop command
    int loopCount;      // repeat count for loop command
    int retnAddr;       // return address for loop command
};

struct TagNintSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool end;                   // flag if the parse reaches the end
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

//----

/** returns version string of music engine. */
static const char *nintSpcVerToStrHtml (NintSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_MMX:
        return "Mega Man X";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void nintSpcResetTrackParam (NintSpcSeqStat *seq, int track)
{
    NintSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->lastNote.active = false;
    tr->loopCount = 0;
}

/** reset before play/convert song. */
static void nintSpcResetParam (NintSpcSeqStat *seq)
{
    int track;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x20; // FIXME: proper value, not correct
    seq->transpose = 0;
    seq->looped = 0;
    seq->end = false;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        NintSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        nintSpcResetTrackParam(seq, track);
    }
}

/** returns what version the sequence is, and sets individual info. */
static int nintSpcCheckVer (NintSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;

    if (nintSpcForceSongAddr >= 0) {
        seq->ver.seqHeaderAddr = nintSpcForceSongAddr;
        version= SPC_VER_MMX;
    }

    seq->ver.id = version;
    nintSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool nintSpcDetectSeq (NintSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int seqHeaderAddr;
    bool result;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    result = false;
    seqHeaderAddr = seq->ver.seqHeaderAddr;
    // track list (reverse order, big-endian)
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].pos = mget2b(&aRAM[seqHeaderAddr + (7 - tr) * 2]);
        if (seq->track[tr].pos) {
            seq->track[tr].active = true;
            result = true;
        }
    }
    nintSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static NintSpcSeqStat *newNintSpcSeq (const byte *aRAM)
{
    NintSpcSeqStat *newSeq = (NintSpcSeqStat *) calloc(1, sizeof(NintSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        nintSpcCheckVer(newSeq);
        if (!nintSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delNintSpcSeq (NintSpcSeqStat **seq)
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
static void printHtmlInfoList (NintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", nintSpcVerToStrHtml(seq));
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (NintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
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

/** output event table header. */
static void printEventTableHeader (NintSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (NintSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC timebase. */
static int nintSpcTimebase (NintSpcSeqStat *seq)
{
    return 48;
}

/** convert SPC tempo into bpm. */
static double nintSpcTempo (NintSpcSeqStat *seq)
{
    return (double) seq->tempo * 60000000 / 24576000; // 24576000 = (timer0) 2ms * 48 * 256?
}

/** convert SPC velocity into MIDI one. */
static int nintSpcMidiVelOf (int value)
{
    if (nintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int nintSpcMidiVolOf (int value)
{
    if (nintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** insert program event. */
/*
static bool smfInsertNintSpcProgram (Smf* seq, int time, int channel, int track, int programNumber)
{
    bool result;

    programNumber &= 0xff;
    result  = smfInsertControl(seq, time, channel, track, SMF_CONTROL_BANKSELM, nintSpcPatchFixInfo[programNumber].bankSelM);
    result &= smfInsertControl(seq, time, channel, track, SMF_CONTROL_BANKSELL, nintSpcPatchFixInfo[programNumber].bankSelL);
    result &= smfInsertProgram(seq, time, channel, track, nintSpcPatchFixInfo[programNumber].patchNo);
    return result;
    //return false;
}
*/

/** create new smf object and link to spc seq. */
static Smf *nintSpcCreateSmf (NintSpcSeqStat *seq)
{
    static char songTitle[512];
    Smf* smf;
    int tr;

    smf = smfCreate(nintSpcTimebase(seq));
    if (!smf)
        return NULL;
    seq->smf = smf;

    sprintf(songTitle, "%s %s", APPNAME, VERSION);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, songTitle);
    smfInsertTempoBPM(smf, 0, 0, nintSpcTempo(seq));
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
void nintSpcNoteNameFromVbyte (char *buf, int vbyte, int keyFix)
{
    int key = (vbyte & 0x7f) + SPC_NOTE_KEYSHIFT + keyFix;

    if (key >= 0x00 && key <= 0x7f)
        getNoteName(buf, key);
    else
        sprintf(buf, "Note %d", key);
}
*/

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

/** truncate note. */
static void nintSpcTruncateNote (NintSpcSeqStat *seq, int track)
{
    NintSpcTrackStat *tr = &seq->track[track];

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
static void nintSpcTruncateNoteAll (NintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        nintSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool nintSpcDequeueNote (NintSpcSeqStat *seq, int track)
{
    return false;
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

        result = smfInsertNintSpcNote(smf, tick, track, note + (perc ? 0 : transpose), keyFix, perc, velocity, duration);
    }
    seq->track[track].lastNote = -1;
    return result;
*/
}

/** finalize note for each track. */
static void nintSpcDequeueNoteAll (NintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        nintSpcDequeueNote(seq, tr);
    }
}

/** vcmds: unknown event (without status change). */
static void nintSpcEventUnknownInline (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X\n", ev->code);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X\n", ev->code);
}

/** vcmds: unidentified event. */
static void nintSpcEventUnidentified (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    ev->unidentified = true;
    nintSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (no args). */
static void nintSpcEventUnknown0 (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    nintSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (1 byte arg). */
static void nintSpcEventUnknown1 (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmds: unknown event (2 byte args). */
static void nintSpcEventUnknown2 (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmd: unknown event (3 byte args). */
static void nintSpcEventUnknown3 (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
}

/** vcmds: no operation. */
static void nintSpcEventNOP (NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** set pointers of each event. */
static void nintSpcSetEventList (NintSpcSeqStat *seq)
{
    int code;
    NintSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (NintSpcEvent) nintSpcEventUnidentified;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* nintSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    NintSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

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

    printEventTableHeader(seq, smf);

    while (!seq->end && !abortFlag) {

        SeqEventReport ev;

        for (ev.track = 0; ev.track < SPC_TRACK_MAX; ev.track++) {

            while (!seq->end && seq->track[ev.track].pos && seq->track[ev.track].tick <= seq->tick) {

                bool inSub;

                // init event report
                ev.tick = seq->tick;
                ev.addr = seq->track[ev.track].pos;
                ev.size = 0;
                ev.unidentified = false;
                strcpy(ev.note, "");

                // read first byte
                ev.size++;
                ev.code = aRAM[ev.addr];
                sprintf(ev.classStr, "ev%02X", ev.code);
                seq->track[ev.track].pos++;
                // in subroutine?
                inSub = (seq->track[ev.track].loopCount > 0);
                strcat(ev.classStr, inSub ? " sub" : "");

                //if (ev.code != seq->ver.pitchSlideByte)
                //    seq->track[ev.track].prevTick = seq->track[ev.track].tick;
                seq->track[ev.track].used = true;
                // dispatch event
                seq->ver.event[ev.code](seq, &ev, smf);

                // dump event report
                if (nintSpcTextLoopMax == 0 || seq->looped < nintSpcTextLoopMax)
                    printHtmlEventDump(seq, &ev);

                if (ev.unidentified) {
                    abortFlag = true;
                    goto quitConversion;
                }
            }
        }

        // end of seq, quit
        if (seq->end) {
            // rewind tracks to end point
            for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                seq->track[tr].tick = seq->tick;
                seq->track[tr].prevTick = seq->tick;
                smfSetEndTimingOfTrack(smf, tr, seq->tick);
            }
        }
        else {
            // step
            int minTickStep = 0;
            for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                if (seq->track[tr].pos) {
                    if (minTickStep == 0)
                        minTickStep = seq->track[tr].tick - seq->tick;
                    else
                        minTickStep = min(minTickStep, seq->track[tr].tick - seq->tick);
                }
            }

            seq->tick += minTickStep;
            seq->time += (double) 60 / nintSpcTempo(seq) * minTickStep / nintSpcTimebase(seq);

            // check time limit
            if (seq->time >= nintSpcTimeLimit) {
                abortFlag = true;
            }
        }
    }

quitConversion:

    // finalize for all notes
    nintSpcTruncateNoteAll(seq);
    nintSpcDequeueNoteAll(seq);

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

static int gArgc;
static char **gArgv;
static bool manDisplayed = false;

typedef void (*CmdDispatcher) (void);

typedef struct TagCmdOptDefs {
    char *name;
    int numArgs;
    CmdDispatcher dispatch;
    char *syntax;
    char *description;
} CmdOptDefs;

static void cmdOptHelp (void);
static void cmdOptLoop (void);
static void cmdOptSong (void);

static CmdOptDefs optDef[] = {
    { "--help", 0, cmdOptHelp, "", "show usage" },
    { "--loop", 1, cmdOptLoop, "<times>", "set loop count" },
    { "--song", 1, cmdOptSong, "<addr>", "force set song address" },
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
        fprintf(stderr, " %-8s  %-12s  %s\n", optDef[op].name, optDef[op].syntax, optDef[op].description);
    }

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
    nintSpcSetLoopCount(loopCount);
}

/** set loop song address */
static void cmdOptSong (void)
{
    int songAddr = strtol(gArgv[0], NULL, 16);
    nintSpcForceSongAddr = songAddr;
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
            nintSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = nintSpcToMidiFromFile(gArgv[0]);
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
