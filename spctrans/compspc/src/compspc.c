/**
 * "Puyo Puyo" Compile spc2midi.
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
#include "compspc.h"

#define APPNAME         "Compile SPC2MIDI"
#define APPSHORTNAME    "compspc"
#define VERSION         "[2014-02-15]"
#define AUTHOR          "loveemu"
#define WEBSITE         "http://loveemu.yh.land.to/"

static int compSpcLoopMax = 2;            // maximum loop count of parser
static int compSpcTextLoopMax = 1;        // maximum loop count of text output
static double compSpcTimeLimit = 2400;    // time limit of conversion (for safety)
static bool compSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int compSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool compSpcNoPatchChange = false; // XXX: hack, should be false for serious conversion

static int compSpcTimeBase = 12;
static bool compSpcForcePAL = false;
static int compSpcForceSongTableAddr = -1;

static bool compSpcPatchFixOverride = false;
static PatchFixInfo compSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int compSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_ALESTE,     // oldest
    SPC_VER_JAKICRUSH,
    SPC_VER_PUYOPUYO,
    SPC_VER_NAZOPUYO,
    SPC_VER_PUYOPUYO2,  // newest
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_SONG_MAX        16
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   0
#define SPC_PERC_KEYSHIFT   (127 - 30)
#define SPC_ARAM_SIZE       0x10000
#define SPC_LOOPCOUNT_NUM   256

typedef struct TagCompSpcTrackStat CompSpcTrackStat;
typedef struct TagCompSpcSeqStat CompSpcSeqStat;
typedef void (*CompSpcEvent) (CompSpcSeqStat *, SeqEventReport *);

typedef struct TagCompSpcVerInfo {
    int id;
    bool isPAL;
    int songIndex;
    int songTableAddr;
    int seqHeaderAddr;
    CompSpcEvent event[256];  // vcmds
    PatchFixInfo patchFix[256];
} CompSpcVerInfo;

typedef struct TagCompSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // duration (tick)
    int vel;            // velocity
    int durRate;        // duration rate (n/256)
    bool perc;          // if the note is percussion
    bool portamento;    // portamento flag
    bool insPtmnt;      // insert portamento event
    int ptmntTime;      // portamento time
    int key;            // key
    int patch;          // instrument
    int transpose;      // transpose
} CompSpcNoteParam;

struct TagCompSpcTrackStat {
    bool active;        // if the channel is still active
    bool used;          // if the channel used once or not
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    CompSpcNoteParam note;     // current note param
    CompSpcNoteParam lastNote; // note params for last note
    int retnAddr;       // return address for loop command
    int loopCount[SPC_LOOPCOUNT_NUM]; // repeat count for loop command
    int looped;         // how many times looped (internal)
    int tempo;          // tempo (bpm)
    int a80;            // 
};

struct TagCompSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    CompSpcVerInfo ver;         // game version info
    CompSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
    int numTracks;              // count of tracks in seq
};

static void compSpcSetEventList (CompSpcSeqStat *seq);

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
FILE *compSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int compSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = compSpcLoopMax;
    compSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool compSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        compSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        compSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            compSpcPatchFix[patch].bankSelM = patch >> 7;
            compSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            compSpcPatchFix[patch].bankSelM = 0;
            compSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        compSpcPatchFix[patch].patchNo = patch & 0x7f;
        compSpcPatchFix[patch].key = 0;
        compSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        compSpcPatchFix[src].bankSelM = bankM & 0x7f;
        compSpcPatchFix[src].bankSelL = bankL & 0x7f;
        compSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        compSpcPatchFix[src].key = key;
        compSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    compSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *compSpcVerToStrHtml (CompSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_ALESTE:
        return "Space Megaforce";
    case SPC_VER_JAKICRUSH:
        return "Jaki Crush";
    case SPC_VER_PUYOPUYO:
        return "Super Puyo Puyo";
    case SPC_VER_NAZOPUYO:
        return "Super Nazo Puyo";
    case SPC_VER_PUYOPUYO2:
        return "Super Puyo Puyo 2";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void compSpcResetTrackParam (CompSpcSeqStat *seq, int track)
{
    CompSpcTrackStat *tr = &seq->track[track];
    int index;

    tr->used = false;
    tr->note.vel = 100; // FIXME
    tr->note.durRate = 0xff; // FIXME
    //tr->note.patch = 0;
    //tr->note.transpose = 0;
    tr->note.portamento = false;
    tr->note.insPtmnt = false;
    tr->lastNote.active = false;

    for (index = 0; index < SPC_LOOPCOUNT_NUM; index++)
        tr->loopCount[index] = 0;
}

/** reset before play/convert song. */
static void compSpcResetParam (CompSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        CompSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        compSpcResetTrackParam(seq, track);
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
    if (compSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &compSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int compSpcCheckVer (CompSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;
    int pos1;

    seq->timebase = compSpcTimeBase;
    seq->ver.isPAL = compSpcForcePAL;
    seq->ver.songIndex = -1;
    seq->ver.songTableAddr = -1;
    seq->ver.seqHeaderAddr = -1;

    // TODO: FIXME: implement
    if (compSpcForceSongTableAddr >= 0) {
        seq->ver.songTableAddr = compSpcForceSongTableAddr;
        seq->ver.songIndex = 1;
        version = SPC_VER_PUYOPUYO2;
    }
    else {
        pos1 = indexOfHexPat(aRAM, (const byte *) "\x8f\x6c\\\xf2\x8f\x60\\\xf3\xe4.\x28\x4c\xc4.\xe5..\xc4.\xe5..\xc4.", SPC_ARAM_SIZE, NULL);
        if (pos1 >= 0 && (aRAM[pos1 + 21] - aRAM[pos1 + 16] == 1)) {
            seq->ver.songTableAddr = mget2l(&aRAM[aRAM[pos1 + 16]]);
            seq->ver.songIndex = 1;
            version = SPC_VER_PUYOPUYO2;
        }
    }

    if (seq->ver.songTableAddr >= 0 && seq->ver.songIndex >= 0) {
        int songAddr = (seq->ver.songTableAddr + (seq->ver.songIndex * 2)) & 0xffff;
        seq->ver.seqHeaderAddr = mget2l(&aRAM[songAddr]);
    }
    else
        version = SPC_VER_UNKNOWN;

    seq->ver.id = version;
    compSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool compSpcDetectSeq (CompSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int seqHeaderAddr;
    bool result;
    int track;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    result = false;
    seqHeaderAddr = seq->ver.seqHeaderAddr;

    seq->numTracks = aRAM[seqHeaderAddr];
    if (seq->numTracks == 0) {
        fprintf(stderr, "Error: no tracks detected\n");
        return false;
    }
    else if (seq->numTracks > SPC_TRACK_MAX) {
        fprintf(stderr, "Error: too much tracks [%d]\n", seq->numTracks);
        return false;
    }

    // track list
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        CompSpcTrackStat *tr = &seq->track[track];
        int trackInfoAddr = seqHeaderAddr + 1 + (track * 14);

        tr->active = (track < seq->numTracks);
        if (!tr->active)
            continue;

        //tr->voiceNumber = aRAM[trackInfoAddr];
        tr->a80 = aRAM[trackInfoAddr + 1];
        tr->note.transpose = utos1(aRAM[trackInfoAddr + 5]);
        seq->tempo = tr->tempo = aRAM[trackInfoAddr + 6];
        //tr->channel = aRAM[trackInfoAddr + 7];
        tr->pos = mget2l(&aRAM[trackInfoAddr + 8]);
        tr->note.patch = aRAM[trackInfoAddr + 10];
        result = true;
    }
    compSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static CompSpcSeqStat *newCompSpcSeq (const byte *aRAM)
{
    CompSpcSeqStat *newSeq = (CompSpcSeqStat *) calloc(1, sizeof(CompSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        compSpcCheckVer(newSeq);
        if (!compSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delCompSpcSeq (CompSpcSeqStat **seq)
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
static void printHtmlInfoList (CompSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", compSpcVerToStrHtml(seq));
    myprintf("          <li>Song Table: $%04X</li>\n", seq->ver.songTableAddr);
    myprintf("          <li>Sequence Header: $%04X (Song $%02X)</li>\n", seq->ver.seqHeaderAddr, seq->ver.songIndex);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (CompSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>TODO: Header Dump</li>\n");
}

/** output other seq detail for valid seq. */
static void printHtmlInfoOthers (CompSpcSeqStat *seq)
{
    int track;
    int index;

    if (seq == NULL)
        return;

    // track list
    myprintf("        <table>\n");
    for (track = 0; track < seq->numTracks; track++) {
        int trackInfoAddr = seq->ver.seqHeaderAddr + 1 + (track * 14);

        myprintf("          <tr><td>");
        for (index = 0; index < 14; index++) {
            myprintf("%s%02X\n", index ? " " : "", seq->aRAM[trackInfoAddr+index]);
        }
        myprintf("</td></tr>\n");
    }
    myprintf("        </table>\n");
}

/** output event dump. */
static void printHtmlEventDump (CompSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (CompSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("        <h3>Sequence</h3>\n");
    myprintf("        <div class=\"section\">\n");
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

/** output event table footer. */
static void printEventTableFooter (CompSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

//----

/** convert SPC tempo into bpm. */
static double compSpcTempo (CompSpcSeqStat *seq)
{
    double tempoFactor;
    int tempo;

    tempo = seq->tempo ? seq->tempo : 256;
    tempoFactor = 16000 * 256 * seq->timebase;
    if (seq->ver.isPAL)
        tempoFactor = tempoFactor * 60 / 50;
    return (double) 60000000 / tempoFactor * tempo;
}

/** create new smf object and link to spc seq. */
static Smf *compSpcCreateSmf (CompSpcSeqStat *seq)
{
    static char songTitle[512];
    Smf* smf;
    int track;

    smf = smfCreate(seq->timebase);
    if (!smf)
        return NULL;
    seq->smf = smf;

    sprintf(songTitle, "%s %s", APPNAME, VERSION);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, songTitle);

    switch (compSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, compSpcTempo(seq));

    // put initial info for each track
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        CompSpcTrackStat *tr = &seq->track[track];

        if (!tr->active)
            continue;

        smfInsertControl(smf, 0, track, track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[tr->note.patch].bankSelM);
        smfInsertControl(smf, 0, track, track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[tr->note.patch].bankSelL);
        smfInsertProgram(smf, 0, track, track, seq->ver.patchFix[tr->note.patch].patchNo);
        //smfInsertControl(smf, 0, track, track, SMF_CONTROL_VOLUME, compSpcMidiVolOf(tr->volume));
        smfInsertControl(smf, 0, track, track, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, track, track, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, track, track, SMF_CONTROL_MONO, 127);

        sprintf(songTitle, "Track %d - $%04X", track + 1, tr->pos);
        smfInsertMetaText(seq->smf, 0, track, SMF_META_TRACKNAME, songTitle);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void compSpcTruncateNote (CompSpcSeqStat *seq, int track)
{
    CompSpcTrackStat *tr = &seq->track[track];

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
static void compSpcTruncateNoteAll (CompSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        compSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool compSpcDequeueNote (CompSpcSeqStat *seq, int track)
{
    CompSpcTrackStat *tr = &seq->track[track];
    CompSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        //if (lastNote->tied)
        //    dur = (lastNote->dur * lastNote->durRate) >> 8;
        //else
        //    dur = (lastNote->dur - lastNote->lastDur)
        //        + ((lastNote->lastDur * lastNote->durRate) >> 8);
            dur = lastNote->dur;
        if (dur == 0)
            dur++;

        if (tr->lastNote.perc)
            key = lastNote->key + SPC_PERC_KEYSHIFT;
        else
            key = lastNote->key + lastNote->transpose
                + seq->ver.patchFix[tr->lastNote.patch].key
                + SPC_NOTE_KEYSHIFT;

        vel = lastNote->vel;
        if (vel == 0)
            vel++;

        result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        if (lastNote->insPtmnt) {
            smfInsertControl(seq->smf, lastNote->tick, track, track, SMF_CONTROL_PORTAMENTO, 127);
            lastNote->insPtmnt = false;
        }
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void compSpcDequeueNoteAll (CompSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        compSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void compSpcInactiveTrack(CompSpcSeqStat *seq, int track)
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
static void compSpcAddTrackLoopCount(CompSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (compSpcLoopMax > 0) ? compSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= compSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void compSpcSeqAdvTick(CompSpcSeqStat *seq)
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
    seq->time += (double) 60 / compSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void compSpcEventUnknownInline (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X [Track %d]\n", ev->code, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X [Track %d]\n", ev->code, ev->track + 1);
}

/** vcmds: unidentified event. */
static void compSpcEventUnidentified (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    compSpcEventUnknownInline(seq, ev);
    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void compSpcEventUnknown0 (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    compSpcEventUnknownInline(seq, ev);
    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void compSpcEventUnknown1 (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    compSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void compSpcEventUnknown2 (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    compSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void compSpcEventUnknown3 (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    compSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: nop (1 byte arg). */
static void compSpcEventNOP1 (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    (*p)++;

    sprintf(ev->note, "Event %02X (NOP)", ev->code);
    strcat(ev->classStr, " ev-nop");
}

/** vcmd de: set duration directly. */
static void compSpcEventDurImmInline (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int dur;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    dur = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, ", dur = %d", dur);
    strcat(ev->note, argDumpStr);
    tr->note.dur = dur;
}

/** vcmd de: set duration directly. */
static void compSpcEventDurImm (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Duration (Imm)");
    strcat(ev->classStr, " ev-dur");
    compSpcEventDurImmInline(seq, ev);
}

/** vcmd df-ee: set duration from table. */
static void compSpcEventDurInline (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int dur;
    // TODO: FIXME: read from ARAM
    const int durTable[16] = {
        0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0c, 0x10, 
        0x18, 0x20, 0x30, 0x09, 0x12, 0x1e, 0x24, 0x2a,
    };

    dur = durTable[ev->code - 0xdf];
    sprintf(argDumpStr, ", dur = %d", dur);
    strcat(ev->note, argDumpStr);
    tr->note.dur = dur;
}

/** vcmd df-ee: set duration from table. */
static void compSpcEventDur (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Duration");
    strcat(ev->classStr, " ev-dur");
    compSpcEventDurInline(seq, ev);
}

/** read duration if it exists. */
static void compSpcReadDurByte (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte evCode = ev->code;
    bool dispatched;

    do {
        dispatched = false;
        ev->code = seq->aRAM[*p];
        if (ev->code == 0xde) {
            (*p)++;
            dispatched = true;
            compSpcEventDurImmInline(seq, ev);
        }
        else if (ev->code >= 0xdf && ev->code <= 0xee) {
            (*p)++;
            dispatched = true;
            compSpcEventDurInline(seq, ev);
        }
    } while (dispatched);
    ev->code = evCode;
}

/** vcmd 00,01-7f: rest/note. */
static void compSpcEventNote (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int note = (ev->code - 1);
    bool rest = (ev->code == 0x00);
    CompSpcTrackStat *tr = &seq->track[ev->track];
    bool oldPtmnt = tr->lastNote.active ? tr->lastNote.portamento : false;
    bool curPtmnt = tr->note.portamento;
    bool ptmntEnd = oldPtmnt && !curPtmnt;

    if (rest) {
        sprintf(ev->note, "Rest");
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, note + seq->transpose + tr->note.transpose
            + seq->ver.patchFix[tr->note.patch].key + SPC_NOTE_KEYSHIFT);
        strcat(ev->classStr, " ev-note");
    }
    compSpcReadDurByte(seq, ev);

    // outputput old note first
    compSpcDequeueNote(seq, ev->track);

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
        tr->lastNote.portamento = tr->note.portamento;
        tr->lastNote.insPtmnt = !oldPtmnt && curPtmnt;
        tr->lastNote.ptmntTime = tr->note.ptmntTime;
        tr->lastNote.perc = false;
        tr->lastNote.active = true;
    }
    tr->tick += tr->note.dur;

    if (ptmntEnd)
        smfInsertControl(seq->smf, tr->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 0);
}

/** vcmd c0-dd: percussion note. */
static void compSpcEventPercNote (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int note = ev->code - 0xc0;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    bool oldPtmnt = tr->lastNote.active ? tr->lastNote.portamento : false;
    bool curPtmnt = tr->note.portamento;
    bool ptmntEnd = oldPtmnt && !curPtmnt;

    sprintf(ev->note, "Perc %d", note + 1);
    strcat(ev->classStr, " ev-perc ev-note");
    compSpcReadDurByte(seq, ev);

    // outputput old note first
    compSpcDequeueNote(seq, ev->track);

    // set new note
    tr->lastNote.tick = ev->tick;
    tr->lastNote.dur = tr->note.dur;
    tr->lastNote.key = note;
    tr->lastNote.durRate = tr->note.durRate;
    tr->lastNote.vel = tr->note.vel;
    tr->lastNote.transpose = seq->transpose + tr->note.transpose;
    tr->lastNote.patch = tr->note.patch;
    tr->lastNote.portamento = tr->note.portamento;
    tr->lastNote.insPtmnt = !oldPtmnt && curPtmnt;
    tr->lastNote.ptmntTime = tr->note.ptmntTime;
    tr->lastNote.perc = true;
    tr->lastNote.active = true;
    tr->tick += tr->note.dur;

    if (ptmntEnd)
        smfInsertControl(seq->smf, tr->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 0);
}

/** vcmd 80: jump. */
static void compSpcEventJump (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    // assumes backjump = loop
    if (arg1 < *p) {
        compSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = arg1;

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");
}

/** vcmd 81: loop. */
static void compSpcEventLoop (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    if (--tr->loopCount[arg1])
        *p = arg2;

    sprintf(ev->note, "Loop, id = %d, dest = $%04X", arg1, arg2);
    strcat(ev->classStr, " ev-loop");
}

/** vcmd 82: end of track. */
static void compSpcEventEndOfTrack (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    tr->a80 = 0;
    compSpcInactiveTrack(seq, ev->track);
}

/** vcmd 83: set vibrato (1 byte arg). */
static void compSpcEventVibrato (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato, index = %d", arg1);
    strcat(ev->classStr, " ev-vibrato");

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 84: portamento time. */
static void compSpcEventPortamentoTime (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Portamento Time, rate = %d", arg1);
    strcat(ev->classStr, " ev-ptmnttime");

    tr->note.ptmntTime = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTOTIME, min(tr->note.ptmntTime, 127));
}

/** vcmd 87: set volume (1 byte arg). */
static void compSpcEventVolume (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-volume");

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 88: set software volume envelope (1 byte arg). */
static void compSpcEventSoftVolEnvelope (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Envelope, index = %d", arg1);
    strcat(ev->classStr, " ev-adsr");

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 89: per-voice transpose (relative). */
static void compSpcEventTranspose (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Voice Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-vtranspose");

    tr->note.transpose += arg1;
}

/** vcmd 8a: set volume (1 byte arg). */
static void compSpcEventVolumeAdd (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Add Volume, vol += %d", arg1);
    strcat(ev->classStr, " ev-volume");

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 8d: set repeat count. */
static void compSpcEventSetLoopCnt (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Loop Count, id = %d, count = %d", arg1, arg2);
    tr->loopCount[arg1] = arg2;
}

/** vcmd 90: ?. */
static void compSpcEvent90 (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set $80+x, value = %d", arg1);
    tr->a80 = arg1;
}

/** vcmd 96: set tempo. */
static void compSpcEventTempo (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = tr->tempo = arg1;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, compSpcTempo(seq));

    sprintf(ev->note, "Tempo, bpm = %f", compSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    //if (!compSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 97: tuning. */
static void compSpcEventTuning (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    if (seq->ver.id <= SPC_VER_JAKICRUSH) {
        // byte
        ev->size++;
        arg1 = utos1(seq->aRAM[*p]);
        (*p)++;
    }
    else {
        // word
        ev->size += 2;
        arg1 = utos2(mget2l(&seq->aRAM[*p]));
        (*p) += 2;
    }

    sprintf(ev->note, "Tuning, pitch = %d", arg1);
    strcat(ev->classStr, " ev-tuning");

    // FIXME: the real sound code add tuning value to pitch register
    // directly, but it cannot be emulated easily on SMF conversion.
    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 9a: call subroutine. */
static void compSpcEventSubroutine (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    sprintf(ev->note, "Call, addr = $%04X", arg1);
    strcat(ev->classStr, " ev-subroutine");

    tr->retnAddr = *p;
    *p = arg1;
}

/** vcmd 9b: return from subroutine. */
static void compSpcEventReturn (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Return From Subroutine, addr = $%04X", tr->retnAddr);
    strcat(ev->classStr, " ev-ret");

    *p = tr->retnAddr;
}

/** vcmd a0: set instrument. */
static void compSpcEventInstrument (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd 9f: set ADSR. */
static void compSpcEventSetADSR (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set ADSR, index = %d", arg1);
    strcat(ev->classStr, " ev-adsr");

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd a1: portamento on. */
static void compSpcEventPortamentoOn (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Portamento On");
    strcat(ev->classStr, " ev-portamento-on");

    tr->note.portamento = true;

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 127);
}

/** vcmd a2: portamento off. */
static void compSpcEventPortamentoOff (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    CompSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Portamento Off");
    strcat(ev->classStr, " ev-portamento-off");

    tr->note.portamento = false;

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 0);
}

/** vcmd ab: set panpot (1 byte arg). */
static void compSpcEventPanpot (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Panpot, pan = %d", arg1);
    strcat(ev->classStr, " ev-panpot");

    if (!compSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ad: loop break. */
static void compSpcEventLoopBreak (CompSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    CompSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    if (!--tr->loopCount[arg1])
        *p = arg2;

    sprintf(ev->note, "Loop Break, id = %d, dest = $%04X", arg1, arg2);
    strcat(ev->classStr, " ev-loopbreak");
}

/** set pointers of each event. */
static void compSpcSetEventList (CompSpcSeqStat *seq)
{
    int code;
    CompSpcEvent *event = seq->ver.event;

    // disable them all first
    for (code = 0x00; code <= 0xff; code++) {
        event[code] = (CompSpcEvent) compSpcEventUnidentified;
    }

    for (code = 0x00; code <= 0x7f; code++) {
        event[code] = (CompSpcEvent) compSpcEventNote;
    }
    event[0x80] = (CompSpcEvent) compSpcEventJump;
    event[0x81] = (CompSpcEvent) compSpcEventLoop;
    event[0x82] = (CompSpcEvent) compSpcEventEndOfTrack;
    event[0x83] = (CompSpcEvent) compSpcEventVibrato;
    event[0x84] = (CompSpcEvent) compSpcEventPortamentoTime;
    // vcmd 85 - sa/jc | others
    // vcmd 86 - Unknown0? no version differences
    event[0x87] = (CompSpcEvent) compSpcEventVolume;
    event[0x88] = (CompSpcEvent) compSpcEventSoftVolEnvelope;
    event[0x89] = (CompSpcEvent) compSpcEventTranspose;
    event[0x8a] = (CompSpcEvent) compSpcEventVolumeAdd;
    event[0x8b] = (CompSpcEvent) compSpcEventUnknown2;
    event[0x8c] = (CompSpcEvent) compSpcEventNOP1;
    event[0x8d] = (CompSpcEvent) compSpcEventSetLoopCnt;
    event[0x8e] = (CompSpcEvent) compSpcEventUnknown1;
    event[0x8f] = (CompSpcEvent) compSpcEventUnknown1;
    event[0x90] = (CompSpcEvent) compSpcEvent90;
    event[0x91] = (CompSpcEvent) compSpcEventUnknown1;
    event[0x92] = (CompSpcEvent) compSpcEventUnknown1;
    event[0x93] = (CompSpcEvent) compSpcEventUnknown2;
    event[0x94] = (CompSpcEvent) compSpcEventUnknown1;
    // 95 no version differences
    event[0x96] = (CompSpcEvent) compSpcEventTempo;
    event[0x97] = (CompSpcEvent) compSpcEventTuning;
    event[0x98] = (CompSpcEvent) compSpcEventUnknown1;
    event[0x99] = (CompSpcEvent) compSpcEventUnknown0;
    event[0x9a] = (CompSpcEvent) compSpcEventSubroutine;
    event[0x9b] = (CompSpcEvent) compSpcEventReturn;
    // vcmd 9c -  no version differences
    event[0x9d] = (CompSpcEvent) compSpcEventUnknown1;
    // vcmd 9e -  no version differences
    event[0x9f] = (CompSpcEvent) compSpcEventSetADSR;
    event[0xa0] = (CompSpcEvent) compSpcEventInstrument;
    event[0xa1] = (CompSpcEvent) compSpcEventPortamentoOn;
    event[0xa2] = (CompSpcEvent) compSpcEventPortamentoOff;
    event[0xa3] = (CompSpcEvent) compSpcEventUnknown1;
//    event[0xa4] = (CompSpcEvent) compSpcEventUnknown1; // conditional do (channel match), for delay
//    event[0xa5] = (CompSpcEvent) compSpcEventUnknown3; // conditional jump
    event[0xa6] = (CompSpcEvent) compSpcEventUnknown1;
    event[0xa7] = (CompSpcEvent) compSpcEventUnknown1;
    event[0xab] = (CompSpcEvent) compSpcEventPanpot;
    event[0xac] = (CompSpcEvent) compSpcEventUnknown1;
    event[0xad] = (CompSpcEvent) compSpcEventLoopBreak;
    event[0xae] = (CompSpcEvent) compSpcEventUnknown0;
    event[0xaf] = (CompSpcEvent) compSpcEventUnknown0;
    for (code = 0xc0; code <= 0xdd; code++) {
        event[code] = (CompSpcEvent) compSpcEventPercNote;
    }
    event[0xde] = (CompSpcEvent) compSpcEventDurImm;
    for (code = 0xdf; code <= 0xee; code++) {
        event[code] = (CompSpcEvent) compSpcEventDur;
    }

    switch (seq->ver.id) {
      case SPC_VER_ALESTE:
        event[0xa4] = (CompSpcEvent) compSpcEventUnknown1;
        event[0xa5] = (CompSpcEvent) compSpcEventUnknown1;
        event[0xa6] = (CompSpcEvent) compSpcEventUnknown1;
        break;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* compSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    CompSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newCompSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = compSpcCreateSmf(seq);

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

            CompSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (compSpcTextLoopMax == 0 || seq->looped < compSpcTextLoopMax)
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
            compSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= compSpcTimeLimit) {
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    compSpcTruncateNoteAll(seq);
    compSpcDequeueNoteAll(seq);

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
        delCompSpcSeq(&seq);
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
Smf* compSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = compSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* compSpcToMidiFromFile (const char *filename)
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

    smf = compSpcToMidi(data, size);

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
static bool cmdOptGM1 (void);
static bool cmdOptGM2 (void);
static bool cmdOptTimeBase (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm1", '\0', 0, cmdOptGM1, "", "Insert GM1 System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
    { "timebase", '\0', 0, cmdOptTimeBase, "", "Set SMF timebase (tick count for quarter note)" },
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
    compSpcSetLoopCount(loopCount);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (compSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    compSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    compSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    compSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    compSpcMidiResetType = SMF_RESET_GM2;
    return true;
}

/** set SMF division. */
static bool cmdOptTimeBase (void)
{
    int timebase = strtol(gArgv[0], NULL, 0);
    compSpcTimeBase = timebase;
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
        compSpcSetLogStreamHandle(htmlFile);

        fprintf(stderr, "%s:\n", spcPath);

        smf = compSpcToMidiFromFile(spcPath);
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
