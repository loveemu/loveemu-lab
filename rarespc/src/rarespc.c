/**
 * "Donkey Kong Country" Rare spc2midi.
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
#include "rarespc.h"

#define APPNAME         "Rare SPC2MIDI"
#define APPSHORTNAME    "rarespc"
#define VERSION         "[2013-05-20]"
#define AUTHOR          "loveemu"
#define WEBSITE         "http://loveemu.yh.land.to/"

// from VS2008 math.h
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616

static int rareSpcLoopMax = 2;            // maximum loop count of parser
static int rareSpcTextLoopMax = 1;        // maximum loop count of text output
static double rareSpcTimeLimit = 2400;    // time limit of conversion (for safety)
static bool rareSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int rareSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool rareSpcNoPatchChange = false; // XXX: hack, should be false for serious conversion

static int rareSpcTimeBase = 32;
static bool rareSpcDoFineTuning = false;
static int rareSpcForceSongHeaderAddr = -1;

static bool rareSpcPatchFixOverride = false;
static PatchFixInfo rareSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int rareSpcMidiResetType = SMF_RESET_GM1;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_DKC1,            // Donkey Kong Country
    SPC_VER_DKC2,            // Donkey Kong Country 2 (and DKC3)
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_SONG_MAX        16
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   36
#define SPC_ARAM_SIZE       0x10000
#define SPC_LOOPCOUNT_NUM   8

typedef struct TagRareSpcTrackStat RareSpcTrackStat;
typedef struct TagRareSpcSeqStat RareSpcSeqStat;
typedef void (*RareSpcEvent) (RareSpcSeqStat *, SeqEventReport *);

typedef struct TagRareSpcVerInfo {
    int id;
    int seqHeaderAddr;
    RareSpcEvent event[256];  // vcmds
    PatchFixInfo patchFix[256];
} RareSpcVerInfo;

typedef struct TagRareSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int defDur;         // default length for note vcmd (tick)
    int dur;            // duration (tick)
    int vel;            // velocity
    int durRate;        // duration rate (n/256)
    bool tied;          // if the note tied
    bool portamento;    // portamento flag
    int key;            // key
    int patch;          // instrument
    int transpose;      // transpose
} RareSpcNoteParam;

struct TagRareSpcTrackStat {
    bool active;        // if the channel is still active
    bool used;          // if the channel used once or not
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    RareSpcNoteParam note;     // current note param
    RareSpcNoteParam lastNote; // note params for last note
    int loopLevel;      // next count for loop command
    int loopStart[SPC_LOOPCOUNT_NUM]; // loop start address for loop command
    int loopCount[SPC_LOOPCOUNT_NUM]; // repeat count for loop command
    int retnAddr[SPC_LOOPCOUNT_NUM];  // return address for loop command
    int looped;         // how many times looped (internal)
    byte volL;          // left volume
    byte volR;          // right volume
    bool longDur;       // duration byte/word
};

struct TagRareSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int timerFreq;              // timer0 freq
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    byte tempoScale;            // tempo scalar (DKC1=$27, DKC2=$1f)
    byte a2a;
    RareSpcVerInfo ver;         // game version info
    RareSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void rareSpcSetEventList (RareSpcSeqStat *seq);

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
FILE *rareSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int rareSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = rareSpcLoopMax;
    rareSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool rareSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        rareSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        rareSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        rareSpcPatchFix[patch].bankSelM = 0;
        rareSpcPatchFix[patch].bankSelL = patch >> 7;
        rareSpcPatchFix[patch].patchNo = patch & 0x7f;
        rareSpcPatchFix[patch].key = 0;
        rareSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        rareSpcPatchFix[src].bankSelM = bankM & 0x7f;
        rareSpcPatchFix[src].bankSelL = bankL & 0x7f;
        rareSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        rareSpcPatchFix[src].key = key;
        rareSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    rareSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *rareSpcVerToStrHtml (RareSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_DKC1:
        return "Donkey Kong Country";
    case SPC_VER_DKC2:
        return "Donkey Kong Country 2";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void rareSpcResetTrackParam (RareSpcSeqStat *seq, int track)
{
    RareSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->volL = 0x7f;
    tr->volR = 0x7f;
    tr->note.vel = 127;
    tr->note.defDur = 0;
    tr->note.durRate = 0xff; // FIXME
    tr->note.patch = 0; // just in case
    tr->note.transpose = 0;
    tr->note.portamento = false;
    tr->lastNote.active = false;
    tr->loopLevel = 0;
    tr->longDur = false;
}

/** reset before play/convert song. */
static void rareSpcResetParam (RareSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    switch(seq->ver.id)
    {
    case SPC_VER_DKC1:
        seq->timerFreq = 0x3c;
        break;
    case SPC_VER_DKC2:
        seq->timerFreq = 0x64;
        break;
    }

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        RareSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        rareSpcResetTrackParam(seq, track);
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
    if (rareSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &rareSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int rareSpcCheckVer (RareSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;
    int pos1;

    seq->timebase = rareSpcTimeBase;
    seq->ver.seqHeaderAddr = -1;

    if (rareSpcForceSongHeaderAddr >= 0) {
        seq->ver.seqHeaderAddr = rareSpcForceSongHeaderAddr;
        version = SPC_VER_DKC1;
    }
    else {
    	// search for song pointer set routine
        pos1 = indexOfHexPat(aRAM, (const byte *) "\xe8\x01\xd4.\xd5..\\\xf6..\xd4.\\\xf6..\xd4.", SPC_ARAM_SIZE, NULL); // DKC1
        if (pos1 >= 0 && (aRAM[pos1 + 13] - aRAM[pos1 + 8] == 1)) {
            seq->ver.seqHeaderAddr = mget2l(&aRAM[pos1 + 8]);
            version = SPC_VER_DKC1;
        }
        else {
            pos1 = indexOfHexPat(aRAM, (const byte *) "\xe8\x01\xd4.\xd5..\\\xf7.\xd4.\\\xfc\\\xf7.\xd4.", SPC_ARAM_SIZE, NULL); // DKC2
            if (pos1 >= 0) {
                seq->ver.seqHeaderAddr = aRAM[pos1 + 8];
                seq->ver.seqHeaderAddr = mget2l(&aRAM[aRAM[pos1 + 8]]);
                version = SPC_VER_DKC2;
            }
        }
    }

    seq->ver.id = version;
    rareSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool rareSpcDetectSeq (RareSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int seqHeaderAddr;
    bool result;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    // track list
    result = false;
    seqHeaderAddr = seq->ver.seqHeaderAddr;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].active = false;
        seq->track[tr].pos = mget2l(&aRAM[seqHeaderAddr + tr * 2]);
        if (!seq->track[tr].pos)
            continue;
        seq->track[tr].active = true;
        result = true;
    }
    seq->tempoScale = mget2l(&aRAM[seqHeaderAddr + 0x10]);
    seq->a2a = mget2l(&aRAM[seqHeaderAddr + 0x11]);
    rareSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static RareSpcSeqStat *newRareSpcSeq (const byte *aRAM)
{
    RareSpcSeqStat *newSeq = (RareSpcSeqStat *) calloc(1, sizeof(RareSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        rareSpcCheckVer(newSeq);
        if (!rareSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delRareSpcSeq (RareSpcSeqStat **seq)
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
static void printHtmlInfoList (RareSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", rareSpcVerToStrHtml(seq));
    myprintf("          <li>Sequence Header: $%04X</li>\n", seq->ver.seqHeaderAddr);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (RareSpcSeqStat *seq)
{
    int track;

    if (seq == NULL)
        return;

    myprintf("          <li>Voices:");
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        myprintf(" %d:$%04X", track + 1, seq->track[track].active ? seq->track[track].pos : 0x0000);
    }
    myprintf("</li>\n");
    myprintf("          <li>More 2 Bytes: %02x %02x</li>\n", seq->tempoScale, seq->a2a);
}

/** output other seq detail for valid seq. */
static void printHtmlInfoOthers (RareSpcSeqStat *seq)
{
}

/** output event dump. */
static void printHtmlEventDump (RareSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (RareSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("        <h3>Sequence</h3>\n");
    myprintf("        <div class=\"section\">\n");
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

/** output event table footer. */
static void printEventTableFooter (RareSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

//----

/** convert SPC tempo into bpm. */
static double rareSpcTempo (RareSpcSeqStat *seq)
{
    return (double) 60000000 / (seq->timebase * (125 * seq->timerFreq)) * ((double) seq->tempoScale / 256); // 1tick = (timer0) (125*t) us
//  return (double) 60000000 / (seq->timebase * (125 * seq->timerFreq)); // 1tick = (timer0) (125*t) us
}

/** insert volume/panpot from current volume L/R. */
static bool rareSpcInsertVolPan (RareSpcSeqStat *seq, int track)
{
    RareSpcTrackStat *tr = &seq->track[track];
    bool result = true;
    byte trVolL, trVolR;
    byte volByte, panByte;
    double vol, pan, panPI2;

    // GM2 vol => dB: pow(2) curve
    // GM2 pan => dB: sin/cos curve
    // SPC vol => linear

    trVolL = tr->volL & 0x7f;
    trVolR = tr->volR & 0x7f;
    panPI2 = atan2(trVolR, trVolL);
    pan = panPI2 / M_PI_2;
    vol = ((double) trVolR / 0x7f) / sin(panPI2) * sin(M_PI_4);
    vol = sqrt(vol);

    volByte = (byte) floor(vol * 127 + 0.5);
    panByte = (byte) ((trVolL == trVolR) ? 64 : (pan * 127));
    tr->note.vel = volByte;
    //result &= smfInsertControl(seq->smf, tr->tick, track, track, SMF_CONTROL_VOLUME, volByte);
    result &= smfInsertControl(seq->smf, tr->tick, track, track, SMF_CONTROL_PANPOT, panByte);
    return result;
}

/** create new smf object and link to spc seq. */
static Smf *rareSpcCreateSmf (RareSpcSeqStat *seq)
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

    switch (rareSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, rareSpcTempo(seq));

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, rareSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);

        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].pos);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void rareSpcTruncateNote (RareSpcSeqStat *seq, int track)
{
    RareSpcTrackStat *tr = &seq->track[track];

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
static void rareSpcTruncateNoteAll (RareSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        rareSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool rareSpcDequeueNote (RareSpcSeqStat *seq, int track)
{
    RareSpcTrackStat *tr = &seq->track[track];
    RareSpcNoteParam *lastNote = &tr->lastNote;
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
}

/** finalize note for each track. */
static void rareSpcDequeueNoteAll (RareSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        rareSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void rareSpcInactiveTrack(RareSpcSeqStat *seq, int track)
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
static void rareSpcAddTrackLoopCount(RareSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (rareSpcLoopMax > 0) ? rareSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= rareSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void rareSpcSeqAdvTick(RareSpcSeqStat *seq)
{
    int minTickStep = 0;
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active) {
            if (minTickStep == 0)
                minTickStep = seq->track[tr].tick - seq->tick;
            else
                minTickStep = min(minTickStep, seq->track[tr].tick - seq->tick);
        }
    }
    seq->tick += minTickStep;
    seq->time += (double) 60 / rareSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void rareSpcEventUnknownInline (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X [Track %d]\n", ev->code, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X [Track %d]\n", ev->code, ev->track + 1);
}

/** vcmds: unidentified event. */
static void rareSpcEventUnidentified (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    rareSpcEventUnknownInline(seq, ev);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void rareSpcEventUnknown0 (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    rareSpcEventUnknownInline(seq, ev);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void rareSpcEventUnknown1 (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void rareSpcEventUnknown2 (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void rareSpcEventUnknown3 (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void rareSpcEventUnknown4 (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (7 byte args). */
static void rareSpcEventUnknown7 (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5, arg6, arg7;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

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

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d, arg6 = %d, arg7 = %d", arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 00: end of track. */
static void rareSpcEventEndOfTrack (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    rareSpcInactiveTrack(seq, ev->track);
}

/** vcmd 01: set instrument. */
static void rareSpcEventInstrument (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd 02: set L/R volume. */
static void rareSpcVolumeLR (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, L = %d, R = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vol");

    tr->volL = arg1;
    tr->volR = arg2;
    rareSpcInsertVolPan(seq, ev->track);
}

/** vcmd 23: set volume (center). (DKC2) */
static void rareSpcVolumeCenter (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume (Center), L = %d, R = %d", arg1, arg1);
    strcat(ev->classStr, " ev-vol");

    tr->volL = arg1;
    tr->volR = arg1;
    rareSpcInsertVolPan(seq, ev->track);
}

/** vcmd 03: jump. */
static void rareSpcEventJump (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    // assumes backjump = loop
    if (arg1 < *p) {
        rareSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = arg1;

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");
}

/** vcmd 04: call subroutine for xx times. */
static void rareSpcEventSubroutine (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    tr->retnAddr[tr->loopLevel] = *p;
    tr->loopCount[tr->loopLevel] = arg1;
    tr->loopStart[tr->loopLevel] = arg2;
    tr->loopLevel++;
    *p = arg2;

    sprintf(ev->note, "Call, count = %d, dest = $%04X", arg1, arg2);
    strcat(ev->classStr, " ev-call");
}

/** vcmd 21: call subroutine once. (DKC2) */
static void rareSpcEventSubroutineOnce (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    tr->retnAddr[tr->loopLevel] = *p;
    tr->loopCount[tr->loopLevel] = 1;
    tr->loopStart[tr->loopLevel] = arg1;
    tr->loopLevel++;
    *p = arg1;

    sprintf(ev->note, "Call, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-call");
}

/** vcmd 05: end of subroutine. */
static void rareSpcEventEndSubroutine (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    bool cont;

    tr->loopLevel--;
    tr->loopCount[tr->loopLevel]--;
    tr->loopCount[tr->loopLevel] &= 0xff;
    cont = (tr->loopCount[tr->loopLevel] != 0);
    if (cont) {
        // loop
        *p = tr->loopStart[tr->loopLevel];
        tr->loopLevel++;
    }
    else {
        // return
        *p = tr->retnAddr[tr->loopLevel];
    }

    sprintf(ev->note, "End Subroutine (%s)", cont ? "Loop" : "Return");
    strcat(ev->classStr, " ev-ret");
}

/** vcmd 06: default duration on. */
static void rareSpcEventDefDurOn (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int dur;
    int *p = &tr->pos;

    if (tr->longDur) {
        ev->size += 2;
        dur = mget2b(&seq->aRAM[*p]);
        (*p) += 2;
    }
    else {
        ev->size++;
        dur = seq->aRAM[*p];
        (*p)++;
    }

    sprintf(ev->note, "Default Duration On, dur = %d", dur);
    strcat(ev->classStr, " ev-defduron");

    tr->note.defDur = dur;
}

/** vcmd 07: default duration off. */
static void rareSpcEventDefDurOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Default Duration Off");
    strcat(ev->classStr, " ev-defduroff");

    tr->note.defDur = 0;
}

/** vcmd 08: pitch slide up. */
static void rareSpcEventPitchSlideUp (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
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

    sprintf(ev->note, "Pitch Slide Up, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->classStr, " ev-pitchslideup");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 09: pitch slide down. */
static void rareSpcEventPitchSlideDown (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
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

    sprintf(ev->note, "Pitch Slide Down, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->classStr, " ev-pitchslidedown");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 0a: pitch slide off. */
static void rareSpcEventPitchSlideOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Pitch Slide Off");
    strcat(ev->classStr, " ev-pitchslideoff");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 0b: set tempo. */
static void rareSpcEventSetTempo (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Tempo, scale = %d / 256", ev->code, arg1);
    strcat(ev->classStr, " ev-settempo");
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    seq->tempoScale = arg1;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, rareSpcTempo(seq));
}

/** vcmd 0c: add tempo. */
static void rareSpcEventAddTempo (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Add Tempo, scale = %d / 256", ev->code, arg1);
    strcat(ev->classStr, " ev-addtempo");
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    seq->tempoScale += arg1;
    seq->tempoScale &= 0xff;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, rareSpcTempo(seq));
}

/** vcmd 0e: vibrato off. */
static void rareSpcEventVibratoOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Vibrato Off");
    strcat(ev->classStr, " ev-vibratooff");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 0f: set vibrato. */
static void rareSpcEventVibrato (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    RareSpcTrackStat *tr = &seq->track[ev->track];
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

    sprintf(ev->note, "Vibrato, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->classStr, " ev-vibrato");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 10: set ADSR. */
static void rareSpcEventADSR (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
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
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-adsr");
}

/** vcmd 11: master volume. */
static void rareSpcEventMasterVolume (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume, L = %d, R = %d", arg1, arg2);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-mastervol");
}

/** vcmd 12: ?. */
static void rareSpcEvent12Microtune (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, " (Tuning?), arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 13: fine tuning. */
static void rareSpcEventFineTuning (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Fine Tuning, key = %d", arg1);
    strcat(ev->classStr, " ev-finetune");

    if (rareSpcDoFineTuning)
        tr->note.transpose = arg1;
    else {
        //tr->note.transpose = (arg1 / 12) * 12;
        tr->note.transpose = 0;

        if (!rareSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }
}

/** vcmd 14: relative transpose */
static void rareSpcEventTransposeRel (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Relative Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-noteshift");

    tr->note.transpose += arg1;
}

/** vcmd 15: set echo param. */
static void rareSpcEventEchoParam (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, feedback = %d, L = %d, R = %d", arg1, arg2, arg3);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echoparam");
}

/** vcmd 16: echo on. */
static void rareSpcEventEchoOn (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd 17: echo off. */
static void rareSpcEventEchoOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo Off");
    strcat(ev->classStr, " ev-echooff");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 0);
}

/** vcmd 18: set echo FIR. */
static void rareSpcEventEchoFIR (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int i;

    ev->size += 8;

    sprintf(ev->note, "Echo FIR, filter =");
    for (i = 0; i < 8; i++) {
        sprintf(argDumpStr, " %02X", seq->aRAM[*p]);
        strcat(ev->note, argDumpStr);
        (*p)++;
    }
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echofir");
}

/** vcmd 19: set noise clock. */
static void rareSpcEventSetNoiseClock (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Noise Clock, NCK = %d", arg1);
    strcat(ev->classStr, " ev-noiseclock");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 1b: noise off. */
static void rareSpcEventNoiseOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Noise Off");
    strcat(ev->classStr, " ev-noiseoff");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 1f: set echo delay. (DKC2) */
static void rareSpcEventEchoDelay (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 1;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Delay, delay = %d / 2", arg1);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echodelay");
}

/** vcmd 28: set instrument and volume. */
static void rareSpcEventInstVol (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Instrument &amp; Volume, patch = %d, L = %d, R = %d", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-patchvol");

    tr->note.patch = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[arg1].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[arg1].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[arg1].patchNo);

    tr->volL = arg2;
    tr->volR = arg3;
    rareSpcInsertVolPan(seq, ev->track);
}

/** vcmd 2a: set timer0 freq. */
static void rareSpcEventTimerFreq (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->timerFreq = arg1;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, rareSpcTempo(seq));

    sprintf(ev->note, "Timer#0 Frequency, bpm = %f", rareSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");
}

/** vcmd 2b: long duration on. */
static void rareSpcEventLongDurOn (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Long Duration Mode On");
    strcat(ev->classStr, " ev-longduron");

    tr->longDur = true;
}

/** vcmd 2c: long duration off. */
static void rareSpcEventLongDurOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Long Duration Mode Off");
    strcat(ev->classStr, " ev-longduroff");

    tr->longDur = false;
}

/** vcmd 2e: ?. */
static void rareSpcEvent2E (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    rareSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, " (Tremolo Param?), arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 2f: set tremolo. */
static void rareSpcEventTremolo (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    RareSpcTrackStat *tr = &seq->track[ev->track];
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

    sprintf(ev->note, "Tremolo, arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->classStr, " ev-tremolo");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 30: tremolo off. */
static void rareSpcEventTremoloOff (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    RareSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Tremolo Off");
    strcat(ev->classStr, " ev-tremolooff");

    if (!rareSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 80-ff: note. */
static void rareSpcEventNote (RareSpcSeqStat *seq, SeqEventReport *ev)
{
    int note = (ev->code - 0x81);
    bool rest = (ev->code == 0x80);
    RareSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    if (!tr->note.defDur) {
        if (tr->longDur) {
            ev->size += 2;
            tr->note.dur = mget2b(&seq->aRAM[*p]);
            (*p) += 2;
        }
        else {
            ev->size++;
            tr->note.dur = seq->aRAM[*p];
            (*p)++;
        }
    }
    else
        tr->note.dur = tr->note.defDur;

    if (rest) {
        sprintf(ev->note, "Rest, len = %d", tr->note.dur);
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, note + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key + SPC_NOTE_KEYSHIFT);
        sprintf(argDumpStr, ", len = %d", tr->note.dur);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    if (seq->ver.id == SPC_VER_DKC2)
    {
        if (ev->code == 0xe1)
        {
            char stemp[256];
            sprintf(stemp, "Unknown Event %02X", ev->code);
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, stemp);
        }
        else if (ev->code >= 0xe0)
        {
            char stemp[256];
            sprintf(stemp, "Unknown Event %02X", ev->code);
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, stemp);
        }
        else
        {
            //if (!rareSpcLessTextInSMF)
            //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
        }
    }

    // outputput old note first
    rareSpcDequeueNote(seq, ev->track);

    // set new note
    if (!rest) {
        // FIXME: 
        tr->lastNote.tick = ev->tick;
        tr->lastNote.dur = tr->note.dur;
        tr->lastNote.key = note;
        tr->lastNote.durRate = tr->note.durRate;
        tr->lastNote.vel = tr->note.vel;
        tr->lastNote.transpose = seq->transpose + tr->note.transpose;
        tr->lastNote.patch = tr->note.patch;
        tr->lastNote.tied = false;
        tr->lastNote.active = true;
    }
    tr->tick += tr->note.dur;
}

/** set pointers of each event. */
static void rareSpcSetEventList (RareSpcSeqStat *seq)
{
    int code;
    RareSpcEvent *event = seq->ver.event;

    // disable them all first
    for (code = 0x00; code <= 0xff; code++) {
        event[code] = (RareSpcEvent) rareSpcEventUnidentified;
    }

    switch(seq->ver.id)
    {
    case SPC_VER_DKC1:
        event[0x00] = (RareSpcEvent) rareSpcEventEndOfTrack;
        event[0x01] = (RareSpcEvent) rareSpcEventInstrument;
        event[0x02] = (RareSpcEvent) rareSpcVolumeLR;
        event[0x03] = (RareSpcEvent) rareSpcEventJump;
        event[0x04] = (RareSpcEvent) rareSpcEventSubroutine;
        event[0x05] = (RareSpcEvent) rareSpcEventEndSubroutine;
        event[0x06] = (RareSpcEvent) rareSpcEventDefDurOn;
        event[0x07] = (RareSpcEvent) rareSpcEventDefDurOff;
        event[0x08] = (RareSpcEvent) rareSpcEventPitchSlideUp;
        event[0x09] = (RareSpcEvent) rareSpcEventPitchSlideDown;
        event[0x0a] = (RareSpcEvent) rareSpcEventPitchSlideOff;
        event[0x0b] = (RareSpcEvent) rareSpcEventSetTempo;
        event[0x0c] = (RareSpcEvent) rareSpcEventAddTempo;
        event[0x0d] = (RareSpcEvent) rareSpcEventUnknown3;
        event[0x0e] = (RareSpcEvent) rareSpcEventVibratoOff;
        event[0x0f] = (RareSpcEvent) rareSpcEventVibrato;
        event[0x10] = (RareSpcEvent) rareSpcEventADSR;
        event[0x11] = (RareSpcEvent) rareSpcEventMasterVolume;
        event[0x12] = (RareSpcEvent) rareSpcEvent12Microtune;
        event[0x13] = (RareSpcEvent) rareSpcEventFineTuning;
        event[0x14] = (RareSpcEvent) rareSpcEventTransposeRel;
        event[0x15] = (RareSpcEvent) rareSpcEventEchoParam;
        event[0x16] = (RareSpcEvent) rareSpcEventEchoOn;
        event[0x17] = (RareSpcEvent) rareSpcEventEchoOff;
        event[0x18] = (RareSpcEvent) rareSpcEventEchoFIR;
        event[0x19] = (RareSpcEvent) rareSpcEventSetNoiseClock;
        event[0x1a] = (RareSpcEvent) rareSpcEventUnknown0;
        event[0x1b] = (RareSpcEvent) rareSpcEventNoiseOff;
        event[0x1c] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x1d] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x1e] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x1f] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x20] = (RareSpcEvent) rareSpcEventUnknown4;
        //event[0x21] = (RareSpcEvent) rareSpcEventUnidentified;
        //event[0x22] = (RareSpcEvent) rareSpcEventUnidentified;
        //event[0x23] = (RareSpcEvent) rareSpcEventUnidentified;
        //event[0x24] = (RareSpcEvent) rareSpcEventUnidentified;
        //event[0x25] = (RareSpcEvent) rareSpcEventUnidentified;
        event[0x26] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x27] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x28] = (RareSpcEvent) rareSpcEventInstVol;
        event[0x29] = (RareSpcEvent) rareSpcEventUnknown1;
        event[0x2a] = (RareSpcEvent) rareSpcEventTimerFreq;
        event[0x2b] = (RareSpcEvent) rareSpcEventLongDurOn;
        event[0x2c] = (RareSpcEvent) rareSpcEventLongDurOff;
        //event[0x2d] = (RareSpcEvent) rareSpcEventUnidentified;
        event[0x2e] = (RareSpcEvent) rareSpcEvent2E;
        event[0x2f] = (RareSpcEvent) rareSpcEventTremolo;
        event[0x30] = (RareSpcEvent) rareSpcEventTremoloOff;
        for (code = 0x80; code <= 0xff; code++) {
            event[code] = (RareSpcEvent) rareSpcEventNote;
        }
    case SPC_VER_DKC2:
        event[0x00] = (RareSpcEvent) rareSpcEventEndOfTrack;
        event[0x01] = (RareSpcEvent) rareSpcEventInstrument;
        //event[0x01] = (RareSpcEvent) rareSpcEventUnknown1;
        event[0x02] = (RareSpcEvent) rareSpcVolumeLR;
        //event[0x02] = (RareSpcEvent) rareSpcEventUnknown2;
        event[0x03] = (RareSpcEvent) rareSpcEventJump;
        event[0x04] = (RareSpcEvent) rareSpcEventSubroutine;
        event[0x05] = (RareSpcEvent) rareSpcEventEndSubroutine;
        event[0x06] = (RareSpcEvent) rareSpcEventDefDurOn;
        event[0x07] = (RareSpcEvent) rareSpcEventDefDurOff;
        event[0x08] = (RareSpcEvent) rareSpcEventPitchSlideUp;
        event[0x09] = (RareSpcEvent) rareSpcEventPitchSlideDown;
        event[0x0a] = (RareSpcEvent) rareSpcEventPitchSlideOff;
        event[0x0b] = (RareSpcEvent) rareSpcEventSetTempo;
        event[0x0c] = (RareSpcEvent) rareSpcEventAddTempo;
        event[0x0d] = (RareSpcEvent) rareSpcEventUnknown3;
        event[0x0e] = (RareSpcEvent) rareSpcEventVibratoOff;
        event[0x0f] = (RareSpcEvent) rareSpcEventVibrato;
        event[0x10] = (RareSpcEvent) rareSpcEventADSR;
        event[0x11] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x12] = (RareSpcEvent) rareSpcEventUnknown1;
        event[0x13] = (RareSpcEvent) rareSpcEventFineTuning;
        event[0x14] = (RareSpcEvent) rareSpcEventTransposeRel;
        event[0x15] = (RareSpcEvent) rareSpcEventEchoParam;
        event[0x16] = (RareSpcEvent) rareSpcEventEchoOn;
        event[0x17] = (RareSpcEvent) rareSpcEventEchoOff;
        event[0x18] = (RareSpcEvent) rareSpcEventEchoFIR;
        event[0x19] = (RareSpcEvent) rareSpcEventUnknown1;
        event[0x1a] = (RareSpcEvent) rareSpcEventUnknown0;
        event[0x1b] = (RareSpcEvent) rareSpcEventUnknown0;
        event[0x1c] = (RareSpcEvent) rareSpcEventUnknown1; // reuse note
        event[0x1d] = (RareSpcEvent) rareSpcEventUnknown1;
        event[0x1e] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x1f] = (RareSpcEvent) rareSpcEventEchoDelay;
        event[0x20] = (RareSpcEvent) rareSpcEventUnknown0;
        event[0x21] = (RareSpcEvent) rareSpcEventSubroutineOnce;
        event[0x22] = (RareSpcEvent) rareSpcEventUnknown7;
        event[0x23] = (RareSpcEvent) rareSpcVolumeCenter;
        event[0x24] = (RareSpcEvent) rareSpcEventUnknown1;
        event[0x25] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x26] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x27] = (RareSpcEvent) rareSpcEventUnknown4;
        event[0x28] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x29] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x2a] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x2b] = (RareSpcEvent) rareSpcEventLongDurOn;
        event[0x2c] = (RareSpcEvent) rareSpcEventLongDurOff;
        event[0x2d] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x2e] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x2f] = (RareSpcEvent) rareSpcEventUnidentified; // null
        event[0x30] = event[0x17];
        event[0x31] = (RareSpcEvent) rareSpcEventUnknown0;
        event[0x32] = event[0x17];
        for (code = 0x80; code <= 0xff; code++) {
            event[code] = (RareSpcEvent) rareSpcEventNote;
        }
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* rareSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    RareSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newRareSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = rareSpcCreateSmf(seq);

    printHtmlInfoListMore(seq);

    myprintf("          </ul></li>\n");
    myprintf("        </ul>\n");

    printHtmlInfoOthers(seq);

    myprintf("      </div>\n\n");

    myprintf("      <h2>Data Dump</h2>\n");
    myprintf("      <div class=\"section\" id=\"data-dump\">\n");
    myprintf("        <p>You can filter output by using stylesheet. Write %s as you like!</p>\n", mycssfile);

    printEventTableHeader(seq);

    while (seq->active && !abortFlag) {

        SeqEventReport ev;

        for (ev.track = 0; ev.track < SPC_TRACK_MAX; ev.track++) {

            RareSpcTrackStat *evtr = &seq->track[ev.track];

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

                evtr->used = true;
                // dispatch event
                seq->ver.event[ev.code](seq, &ev);

                // dump event report
                if (rareSpcTextLoopMax == 0 || seq->looped < rareSpcTextLoopMax)
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
            rareSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= rareSpcTimeLimit) {
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    rareSpcTruncateNoteAll(seq);
    rareSpcDequeueNoteAll(seq);

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
        delRareSpcSeq(&seq);
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
Smf* rareSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = rareSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* rareSpcToMidiFromFile (const char *filename)
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

    smf = rareSpcToMidi(data, size);

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
static bool cmdOptTimeBase (void);
static bool cmdOptFineTune (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
    { "timebase", '\0', 0, cmdOptTimeBase, "", "Set SMF timebase (tick count for quarter note)" },
    { "finetune", '\0', 0, cmdOptFineTune, "", "Emulate Fine Tuning accurately" },
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

/** set loop count. */
static bool cmdOptLoop (void)
{
    int loopCount = strtol(gArgv[0], NULL, 0);
    rareSpcSetLoopCount(loopCount);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (rareSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    rareSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    rareSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    rareSpcMidiResetType = SMF_RESET_GM2;
    return true;
}

/** set SMF division. */
static bool cmdOptTimeBase (void)
{
    int timebase = strtol(gArgv[0], NULL, 0);
    rareSpcTimeBase = timebase;
    return true;
}

/** emulate fine tuning accurately. */
static bool cmdOptFineTune (void)
{
    rareSpcDoFineTuning = true;
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
    char spcPath[PATH_MAX];
    char midPath[PATH_MAX];
    char htmlPath[PATH_MAX];

    // handle options
    gArgc = argc - 1;
    gArgv = argv + 1;
    result = handleCmdLineOpts();

    // too few or much args
    if (gArgc < 2 || gArgc > 3 || !result) {
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

    // convert input file
    {
        strcpy(spcPath, spcBasePath);
        strcpy(midPath, midBasePath);
        strcpy(htmlPath, htmlBasePath);

        // set html handle if needed
        htmlFile = (htmlPath[0] != '\0') ? fopen(htmlPath, "w") : NULL;
        rareSpcSetLogStreamHandle(htmlFile);

        fprintf(stderr, "%s:\n", spcPath);

        smf = rareSpcToMidiFromFile(spcPath);
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
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
