/**
 * Mint spc2midi.
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
#include "mintspc.h"

#define APPNAME "Mint SPC2MIDI"
#define APPSHORTNAME "mintspc"
#define VERSION "[2014-04-13]"

static int mintSpcLoopMax = 2;            // maximum loop count of parser
static int mintSpcTextLoopMax = 1;        // maximum loop count of text output
static double mintSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool mintSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool mintSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int mintSpcTimeBase = 48;
static int mintSpcForceSongIndex = -1;
static int mintSpcForceSongListAddr = -1;

static bool mintSpcPatchFixOverride = false;
static PatchFixInfo mintSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int mintSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_GBT,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   12
#define SPC_ARAM_SIZE       0x10000

#define MINTSPC_TRACK_MAX   10

typedef struct TagMintSpcTrackStat MintSpcTrackStat;
typedef struct TagMintSpcSeqStat MintSpcSeqStat;
typedef void (*MintSpcEvent) (MintSpcSeqStat *, SeqEventReport *);

typedef struct TagMintSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int seqHeaderAddr;
    MintSpcEvent event[256];
    PatchFixInfo patchFix[256];
    bool seqDetected;
} MintSpcVerInfo;

typedef struct TagMintSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} MintSpcNoteParam;

struct TagMintSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    MintSpcNoteParam note;     // current note param
    MintSpcNoteParam lastNote; // note params for last note
    int looped;             // how many times looped (internal)
    int patch;              // patch number (for pitch fix)
    byte callStack[0x100];  // subroutine stack
    int callStackPtr;       // subroutine stack ptr
    int callStackSize;      // subroutine stack size
    byte noteKey;           // key of note
    byte noteDur;           // duration of note
    byte noteVel;           // velocity of note
    byte noteLen;           // note length
    byte noteWaitLen;       // actual wait amount (tick)
    bool viaNoteParam;      // previous vcmd is note param
    byte volume;            // voice volume
};

struct TagMintSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    MintSpcVerInfo ver;         // game version info
    MintSpcTrackStat track[MINTSPC_TRACK_MAX]; // status of each tracks
    bool timebaseHiRes;         // internal timebase quality (1x/0.5x)
};

static void mintSpcSetEventList (MintSpcSeqStat *seq);

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
FILE *mintSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int mintSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = mintSpcLoopMax;
    mintSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool mintSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        mintSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        mintSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            mintSpcPatchFix[patch].bankSelM = patch >> 7;
            mintSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            mintSpcPatchFix[patch].bankSelM = 0;
            mintSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        mintSpcPatchFix[patch].patchNo = patch & 0x7f;
        mintSpcPatchFix[patch].key = 0;
        mintSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        mintSpcPatchFix[src].bankSelM = bankM & 0x7f;
        mintSpcPatchFix[src].bankSelL = bankL & 0x7f;
        mintSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        mintSpcPatchFix[src].key = key;
        mintSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    mintSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *mintSpcVerToStrHtml (MintSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_GBT:
        return "Gokinjo Bouken Tai";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void mintSpcResetTrackParam (MintSpcSeqStat *seq, int track)
{
    MintSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->noteKey = 0;
    tr->noteDur = 0;
    tr->noteVel = 0;
    tr->volume = 200;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->noteLen = 0;
    tr->noteWaitLen = 0;
    tr->viaNoteParam = false;
    tr->callStackPtr = 0;
    tr->callStackSize = 0x0a;
}

/** reset before play/convert song. */
static void mintSpcResetParam (MintSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x20;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    seq->timebaseHiRes = false;

    // reset each track as well
    for (track = 0; track < MINTSPC_TRACK_MAX; track++) {
        MintSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        mintSpcResetTrackParam(seq, track);
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
    if (mintSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &mintSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }

}

/** returns what version the sequence is, and sets individual info. */
static int mintSpcCheckVer (MintSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int songLdCodeAddr;

    seq->timebase = mintSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.seqDetected = false;

    if (mintSpcForceSongListAddr != -1)
    {
        seq->ver.seqListAddr = mintSpcForceSongListAddr;
    }
    else
    {
        // (Gokinjo Bouken Tai)
        // asl   a
        // mov   y,a
        // mov   a,TBL+y
        // mov   PTR,a
        // mov   a,(TBL+1)+y
        // mov   PTR+1,a           ; set song header ptr
        // mov   y,#$00
        // mov   a,(PTR)+y         ; read first byte
        songLdCodeAddr = indexOfHexPat(seq->aRAM, "\x1c\xfd\xf6..\xc4.\xf6..\xc4.\x8d\\\x00\xf7.", SPC_ARAM_SIZE, NULL);
        if (songLdCodeAddr != -1 &&
            ((mget2l(&seq->aRAM[songLdCodeAddr + 3]) + 1) & 0xffff) == mget2l(&seq->aRAM[songLdCodeAddr + 8]) &&
            ((seq->aRAM[songLdCodeAddr + 6] + 1) & 0xff) == seq->aRAM[songLdCodeAddr + 11] &&
            seq->aRAM[songLdCodeAddr + 6] == seq->aRAM[songLdCodeAddr + 15])
        {
            seq->ver.seqListAddr = mget2l(&seq->aRAM[songLdCodeAddr + 3]);
        }
    }
/*
	static int mintSpcForceSongIndex = -1;
static int mintSpcForceSongListAddr = -1;
*/
    if (seq->ver.seqListAddr != -1)
    {
        if (mintSpcForceSongIndex != -1)
        {
            if (seq->ver.seqListAddr + (mintSpcForceSongIndex * 2) + 1 < SPC_ARAM_SIZE)
            {
                seq->ver.songIndex = mintSpcForceSongIndex;
            }
        }
        else
        {
            int songSearchIndexMax = 4;

            // lazy song search
            for (seq->ver.songIndex = 0; seq->ver.songIndex < songSearchIndexMax; seq->ver.songIndex++)
            {
                int seqHeaderAddrPtr = seq->ver.seqListAddr + (seq->ver.songIndex * 2);
                int seqHeaderAddrCandidate;

                if (seqHeaderAddrPtr >= SPC_ARAM_SIZE)
                {
                    break;
                }
                seqHeaderAddrCandidate = mget2l(&seq->aRAM[seqHeaderAddrPtr]);

                if (seqHeaderAddrCandidate != 0 && seqHeaderAddrCandidate != (SPC_ARAM_SIZE - 1))
                {
                    break;
                }
            }
            if (seq->ver.songIndex == songSearchIndexMax)
            {
                seq->ver.songIndex = -1;
            }
        }
    }

    if (seq->ver.songIndex != -1)
    {
        seq->ver.seqHeaderAddr = mget2l(&seq->aRAM[seq->ver.seqListAddr + (seq->ver.songIndex * 2)]);
        version = SPC_VER_GBT;
    }

    seq->ver.id = version;
    mintSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool mintSpcDetectSeq (MintSpcSeqStat *seq)
{
    bool result = true;
    bool hasValidTrack = false;
    int seqHeadReadPtr = seq->ver.seqHeaderAddr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    mintSpcResetParam(seq);
    while (seq->aRAM[seqHeadReadPtr] != 0xff)
    {
        if (seq->aRAM[seqHeadReadPtr] >= 0x80)
        {
            fprintf(stderr, "Error: Unknown track index $%02X at $%04X (NYI)\n", seq->aRAM[seqHeadReadPtr], seqHeadReadPtr); // not analyzed yet
            result = false;
            break;
        }
        else
        {
            byte trackIndex = seq->aRAM[seqHeadReadPtr];
            if (trackIndex >= MINTSPC_TRACK_MAX)
            {
                fprintf(stderr, "Error: Unsupported track index $%02X at $%04X\n", trackIndex, seqHeadReadPtr);
                result = false;
                break;
            }
            else
            {
                int scoreOffset;
                int scoreAddr;

                seqHeadReadPtr++;
                if (seqHeadReadPtr + 2 >= SPC_ARAM_SIZE)
                {
                    fprintf(stderr, "Error: Address $%04X is out of range\n", seqHeadReadPtr + 2);
                    result = false;
                    break;
                }

                scoreOffset = mget2l(&seq->aRAM[seqHeadReadPtr]);
                scoreAddr = (seqHeadReadPtr + 2) + scoreOffset;
                if (scoreAddr >= SPC_ARAM_SIZE)
                {
                    fprintf(stderr, "Error: Score offset $%04X at $%04X is out of range\n", scoreOffset, seqHeadReadPtr);
                    result = false;
                    break;
                }

                seq->track[trackIndex].pos = scoreAddr;
                seq->track[trackIndex].active = true;
                hasValidTrack = true;
                seqHeadReadPtr += 2;
            }
        }
    }
    if (!hasValidTrack)
    {
    	result = false;
    }

    return result;
}

/** create new spc2mid object. */
static MintSpcSeqStat *newMintSpcSeq (const byte *aRAM)
{
    MintSpcSeqStat *newSeq = (MintSpcSeqStat *) calloc(1, sizeof(MintSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        mintSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = mintSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delMintSpcSeq (MintSpcSeqStat **seq)
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
static void printHtmlInfoList (MintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", mintSpcVerToStrHtml(seq));

    //if (seq->ver.id == SPC_VER_UNKNOWN)
    //    return;

    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    myprintf(" (Song $%02x)", seq->ver.songIndex);
    myprintf("</li>\n");
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (MintSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (MintSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (MintSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (MintSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double mintSpcTempo (MintSpcSeqStat *seq)
{
    return (double) seq->tempo * 60000000 / (121344000 / (seq->timebaseHiRes ? 2 : 1)); // 121344000 = (timer0) 9.875ms * 48 TPQN * 256
}

/** convert SPC velocity into MIDI one. */
static int mintSpcMidiVelOf (int value)
{
    if (mintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int mintSpcMidiVolOf (int value)
{
    if (mintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** create new smf object and link to spc seq. */
static Smf *mintSpcCreateSmf (MintSpcSeqStat *seq)
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

    switch (mintSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, mintSpcTempo(seq));

    // put track name first
    for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].pos);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }

    // put initial info for each track
    for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, mintSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void mintSpcTruncateNote (MintSpcSeqStat *seq, int track)
{
    MintSpcTrackStat *tr = &seq->track[track];

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
static void mintSpcTruncateNoteAll (MintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
        mintSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool mintSpcDequeueNote (MintSpcSeqStat *seq, int track)
{
    MintSpcTrackStat *tr = &seq->track[track];
    MintSpcNoteParam *lastNote = &tr->lastNote;
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
        vel = mintSpcMidiVelOf(lastNote->vel);
        if (vel == 0)
            vel++;

        if (dur != 0)
        {
            result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        }
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void mintSpcDequeueNoteAll (MintSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
        mintSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void mintSpcInactiveTrack(MintSpcSeqStat *seq, int track)
{
    int tr;

    seq->track[track].active = false;
    for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            return;
    }
    seq->active = false;
}

/** increment loop count. */
static void mintSpcAddTrackLoopCount(MintSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (mintSpcLoopMax > 0) ? mintSpcLoopMax : 0xffff;
    for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= mintSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void mintSpcSeqAdvTick(MintSpcSeqStat *seq)
{
    int minTickStep = 0;
    int tr;

    for (tr = MINTSPC_TRACK_MAX - 1; tr >= 0; tr--) {
        if (seq->track[tr].active) {
            if (minTickStep == 0)
                minTickStep = seq->track[tr].tick - seq->tick;
            else
                minTickStep = min(minTickStep, seq->track[tr].tick - seq->tick);
        }
    }
    seq->tick += minTickStep;
    seq->time += (double) 60 / mintSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void mintSpcEventUnknownInline (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
}

/** vcmds: unidentified event. */
static void mintSpcEventUnidentified (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    mintSpcEventUnknownInline(seq, ev);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void mintSpcEventUnknown0 (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    mintSpcEventUnknownInline(seq, ev);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void mintSpcEventUnknown1 (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    mintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void mintSpcEventUnknown2 (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    mintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void mintSpcEventUnknown3 (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    mintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void mintSpcEventUnknown4 (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    MintSpcTrackStat *tr = &seq->track[ev->track];
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

    mintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void mintSpcEventUnknown5 (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    MintSpcTrackStat *tr = &seq->track[ev->track];
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

    mintSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void mintSpcEventNOP (MintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00-7f: set note param. */
static void mintSpcEventNoteParam (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int arg1, arg2;

    // read arguments (variable length)
    if (seq->aRAM[*p] <= 0x7f)
    {
        ev->size++;
        arg1 = seq->aRAM[*p];
        (*p)++;

        if (seq->aRAM[*p] <= 0x7f)
        {
            ev->size++;
            arg2 = seq->aRAM[*p];
            (*p)++;
        }
    }

    if (ev->size > 1)
    {
        tr->noteDur = arg1;

        if (ev->size > 2)
        {
            tr->noteVel = arg2;
        }
    }

    sprintf(ev->note, "Note Param, len = %d", noteByte);
    if (ev->size > 1)
    {
        sprintf(argDumpStr, ", dur = %d", arg1);
        strcat(ev->note, argDumpStr);
    }
    if (ev->size > 2)
    {
        sprintf(argDumpStr, ", arg2 = %d", arg2);
        strcat(ev->note, argDumpStr);
    }
    strcat(ev->classStr, " ev-note");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // note: do not add tr->tick here, because
    // wait occurs just after the next vcmd (must be 80-ff)
    if (noteByte != 0)
    {
        tr->noteLen = noteByte;
        tr->noteWaitLen = tr->noteLen;
    }
}

/** vcmd 80-b0, b1-bf: note. */
static void mintSpcEventNote (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    bool hasParam = (noteByte >= 0xa0 && noteByte <= 0xbf);
    int keyOffset, arg1;
    int len, key, dur;
    bool tieNote;
    bool tie;

    keyOffset = noteByte & 0x1f;

    len = tr->noteLen;
    key = tr->noteKey + keyOffset;
    tieNote = (tr->noteDur == 0);
    dur = tieNote ? len : tr->noteDur;
    if (dur > len)
    {
        dur = len; // really? I thought so when I listen Nanako's Theme of Gokinjo Bouken Tai
    }

    tie = (tr->lastNote.active && tr->lastNote.tied &&
        tr->lastNote.key == key);

    if (hasParam)
    {
        ev->size++;
        arg1 = seq->aRAM[*p];
        (*p)++;

        if (arg1 <= 0x7f)
        {
            tr->noteDur = arg1;
            strcat(ev->note, argDumpStr);
        }
        else
        {
            tr->noteVel = (arg1 & 0x7f);
        }
    }
    getNoteName(argDumpStr, key + seq->transpose + tr->note.transpose
        + seq->ver.patchFix[tr->note.patch].key
        + SPC_NOTE_KEYSHIFT);
    sprintf(ev->note, "%s, len = %d, dur = %d%s, vel = %d", argDumpStr, len, dur, tieNote ? " (tie)" : "", tr->noteVel);
    strcat(ev->classStr, " ev-note");

    if (tieNote)
    {
        if (!mintSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }

    // output old note first
    tie = false; // TODO: NYI because the current mechanism cannot support chords
    if (!tie)
    {
        mintSpcDequeueNote(seq, ev->track);
    }

    // set new note
    if (tie) {
        tr->lastNote.dur += dur;
    }
    else {
        tr->lastNote.tick = ev->tick;
        tr->lastNote.dur = dur;
        tr->lastNote.key = key;
        tr->lastNote.vel = tr->noteVel;
        tr->lastNote.transpose = seq->transpose + tr->note.transpose;
        tr->lastNote.patch = tr->note.patch;
    }
    tr->lastNote.tied = tieNote;
    tr->lastNote.active = true;

    // prevent double processing
    if (!tr->viaNoteParam)
    {
        tr->noteWaitLen = len;
    }
}

/** vcmd d1: set note key. */
static void mintSpcEventSetNoteKey (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->noteKey = arg1;

    getNoteName(argDumpStr, tr->noteKey + seq->transpose + tr->note.transpose
        + seq->ver.patchFix[tr->note.patch].key
        + SPC_NOTE_KEYSHIFT);
    sprintf(ev->note, "Set Note Key, key = %s", argDumpStr);
    strcat(ev->classStr, " ev-notekey");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d2: octave up. */
static void mintSpcEventOctaveUp (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->noteKey += 12;

    getNoteName(argDumpStr, tr->noteKey + seq->transpose + tr->note.transpose
        + seq->ver.patchFix[tr->note.patch].key
        + SPC_NOTE_KEYSHIFT);
    sprintf(ev->note, "Octave Up, key = %s", argDumpStr);
    strcat(ev->classStr, " ev-octaveup");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d3: octave down. */
static void mintSpcEventOctaveDown (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->noteKey -= 12;

    getNoteName(argDumpStr, tr->noteKey + seq->transpose + tr->note.transpose
        + seq->ver.patchFix[tr->note.patch].key
        + SPC_NOTE_KEYSHIFT);
    sprintf(ev->note, "Octave Down, key = %s", argDumpStr);
    strcat(ev->classStr, " ev-octavedown");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d4: rest. */
static void mintSpcEventRest (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    tr->noteWaitLen = tr->noteLen;

    sprintf(ev->note, "Rest, len = %d", tr->noteWaitLen);
    strcat(ev->classStr, " ev-rest");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd c0: set instrument. */
static void mintSpcEventInstrument (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int instrItemAddr;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    instrItemAddr = (*p + arg1) & 0xffff;

    sprintf(ev->note, "Set Instrument/Envelope, procedure = $%04X", instrItemAddr);
    strcat(ev->classStr, " ev-patch");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd c1: set panpot. */
static void mintSpcEventPanpot (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    if (arg1 >= 0)
    {
        sprintf(ev->note, "Panpot, balance = %d/32", arg1);

        if (arg1 <= 32)
        {
            int panValue = arg1 * 4;
            if (panValue > 127)
            {
                panValue = 127;
            }
            smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, panValue);
        }
        else
        {
            if (!mintSpcLessTextInSMF)
                smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
        }
    }
    else
    {
        sprintf(ev->note, "Panpot, balance = (random)");

        if (!mintSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }
    strcat(ev->classStr, " ev-pan");
}

/** vcmd c3: set tempo. */
static void mintSpcEventSetTempo (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;

    sprintf(ev->note, "Set Tempo, tempo = %d (bpm %.1f)", arg1, mintSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    smfInsertTempoBPM(seq->smf, ev->tick, 0, mintSpcTempo(seq));
}

/** vcmd c5: set volume. */
static void mintSpcEventVolume (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->volume = arg1;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, mintSpcMidiVolOf(tr->volume));
}

/** vcmd c6: set priority. */
static void mintSpcEventPriority (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Priority, priority = %d", arg1);
    strcat(ev->classStr, " ev-priority");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd c7: fine tuning (absolute). */
static void mintSpcEventFineTuningAbs (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Fine Tuning (Absolute), tuning = %d/256 semitones", arg1);
    strcat(ev->classStr, " ev-tuning");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d6: pitch bend range. */
static void mintSpcEventPitchBendRange (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Pitch Bend Range, range = %d", arg1);
    strcat(ev->classStr, " ev-pitchbendrange");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNM, 0);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNL, 0);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYM, arg1);
}

/** vcmd d7: coarse tuning (absolute). */
static void mintSpcEventCoarseTuningAbs (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Coarse Tuning (Absolute), tuning = %d semitones", arg1);
    strcat(ev->classStr, " ev-tuning");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d8: coarse tuning (relative). */
static void mintSpcEventCoarseTuningRel (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Coarse Tuning (Relative), tuning += %d semitones", arg1);
    strcat(ev->classStr, " ev-tuning");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d9: fine tuning (relative). */
static void mintSpcEventFineTuningRel (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Fine Tuning (Relative), tuning += %d/256 semitones", arg1);
    strcat(ev->classStr, " ev-tuning");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd da: key on. */
static void mintSpcEventKeyOn (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Key On");
    strcat(ev->classStr, " ev-keyon");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd db: key off. */
static void mintSpcEventKeyOff (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Key Off");
    strcat(ev->classStr, " ev-keyoff");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd dc: add volume. */
static void mintSpcEventAddVolume (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int newVolume;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    newVolume = tr->volume + arg1;
    if (newVolume < 0) {
        newVolume = 0;
    }
    else if (newVolume > 0xff) {
    	newVolume = 0xff;
    }
    tr->volume = newVolume;

    sprintf(ev->note, "Volume (Add), vol += %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, mintSpcMidiVolOf(tr->volume));
}

/** vcmd dd: pitch bend. */
static void mintSpcEventPitchBend (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Bend, pitch = %d", arg1);
    strcat(ev->classStr, " ev-pitchbend");

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertPitchBend(seq->smf, ev->tick, ev->track, ev->track, arg1 * 16);
}

/** vcmd c8: echo on. */
static void mintSpcEventEchoOn (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd c9: echo off. */
static void mintSpcEventEchoOff (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];

    sprintf(ev->note, "Echo Off");
    strcat(ev->classStr, " ev-echooff");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 0);
}

/** vcmd ca: set echo delay, volume, feedback, FIR. */
static void mintSpcEventEchoParam (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 4;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, delay = %d, volume = %d, feedback = %d, FIR = %d", arg1, arg2, arg3, arg4);
    strcat(ev->classStr, " ev-echoparam");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cb: goto. */
static void mintSpcEventJump (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    dest = (*p + arg1) & 0xffff;

    sprintf(ev->note, "Jump, dest = $%04X", dest);
    strcat(ev->classStr, " ev-jump");

    // assumes backjump = loop
    if (dest < *p) {
        mintSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = dest;

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cc: call subroutine. */
static void mintSpcEventSubroutine (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    dest = (*p + arg1) & 0xffff;

    sprintf(ev->note, "Call Subroutine, dest = $%04X", dest);
    strcat(ev->classStr, " ev-call");

    if (tr->callStackPtr + 2 > tr->callStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        mintSpcInactiveTrack(seq, ev->track);
        return;
    }

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->callStack[tr->callStackPtr++] = (byte)(*p);
    tr->callStack[tr->callStackPtr++] = (byte)((*p) >> 8);
    *p = dest;
}

/** vcmd cd: end subroutine. */
static void mintSpcEventEndSubroutine (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    if (tr->callStackPtr == 0) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        mintSpcInactiveTrack(seq, ev->track);
        return;
    }
    else {
        sprintf(ev->note, "End Subroutine");
        strcat(ev->classStr, " ev-ret");

        if (tr->callStackPtr < 2) {
            fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
            mintSpcInactiveTrack(seq, ev->track);
            return;
        }

        tr->callStackPtr -= 2;
        *p = mget2l(&tr->callStack[tr->callStackPtr]);
    }

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd ce: loop start. */
static void mintSpcEventLoopStart (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Loop Start, count = %d", arg1);
    strcat(ev->classStr, " ev-loopstart");

    if (tr->callStackPtr + 3 > tr->callStackSize) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        mintSpcInactiveTrack(seq, ev->track);
        return;
    }

    tr->callStack[tr->callStackPtr++] = (byte)(*p);
    tr->callStack[tr->callStackPtr++] = (byte)((*p) >> 8);
    tr->callStack[tr->callStackPtr++] = arg1;

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd cf: loop end. */
static void mintSpcEventLoopEnd (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;
    byte loopCount;

    sprintf(ev->note, "Loop End/Continue");
    strcat(ev->classStr, " ev-loopend");

    if (tr->callStackPtr == 0) {
        fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
        mintSpcInactiveTrack(seq, ev->track);
        return;
    }

    loopCount = tr->callStack[tr->callStackPtr - 1];
    if (--loopCount == 0) {
        // repeat end, fall through
        sprintf(ev->note, "Loop End");
        if (tr->callStackPtr < 3) {
            fprintf(stderr, "Call Stack Access Violation, sp = %d\n", tr->callStackPtr);
            mintSpcInactiveTrack(seq, ev->track);
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

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd d0: end of track. */
static void mintSpcEventEndOfTrack (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    MintSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    mintSpcInactiveTrack(seq, ev->track);

    //if (!mintSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e6: set timebase quality. */
static void mintSpcEventTimebaseHiRes (MintSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    MintSpcTrackStat *tr = &seq->track[ev->track];
    bool timebaseHiResOld = seq->timebaseHiRes;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->timebaseHiRes = ((arg1 & 1) != 0);

    sprintf(ev->note, "Set Timebase, doubled = %s", seq->timebaseHiRes ? "true" : "false");
    strcat(ev->classStr, " ev-timebase");

    if (!mintSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    if ((!timebaseHiResOld &&  seq->timebaseHiRes) ||
        ( timebaseHiResOld && !seq->timebaseHiRes))
    {
        smfInsertTempoBPM(seq->smf, ev->tick, 0, mintSpcTempo(seq));
    }
}

/** set pointers of each event. */
static void mintSpcSetEventList (MintSpcSeqStat *seq)
{
    int code;
    MintSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (MintSpcEvent) mintSpcEventUnidentified;
    }

    for (code = 0x00; code <= 0x7f; code++) {
        event[code] = (MintSpcEvent) mintSpcEventNoteParam;
    }
    for (code = 0x80; code <= 0xbf; code++) {
        event[code] = (MintSpcEvent) mintSpcEventNote;
    }
    event[0xc0] = (MintSpcEvent) mintSpcEventInstrument;
    event[0xc1] = (MintSpcEvent) mintSpcEventPanpot;
    event[0xc2] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xc3] = (MintSpcEvent) mintSpcEventSetTempo;
    event[0xc4] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xc5] = (MintSpcEvent) mintSpcEventVolume;
    event[0xc6] = (MintSpcEvent) mintSpcEventPriority;
    event[0xc7] = (MintSpcEvent) mintSpcEventFineTuningAbs;
    event[0xc8] = (MintSpcEvent) mintSpcEventEchoOn;
    event[0xc9] = (MintSpcEvent) mintSpcEventEchoOff;
    event[0xca] = (MintSpcEvent) mintSpcEventEchoParam;
    event[0xcb] = (MintSpcEvent) mintSpcEventJump;
    event[0xcc] = (MintSpcEvent) mintSpcEventSubroutine;
    event[0xcd] = (MintSpcEvent) mintSpcEventEndSubroutine;
    event[0xce] = (MintSpcEvent) mintSpcEventLoopStart;
    event[0xcf] = (MintSpcEvent) mintSpcEventLoopEnd;
    event[0xd0] = (MintSpcEvent) mintSpcEventEndOfTrack;
    event[0xd1] = (MintSpcEvent) mintSpcEventSetNoteKey;
    event[0xd2] = (MintSpcEvent) mintSpcEventOctaveUp;
    event[0xd3] = (MintSpcEvent) mintSpcEventOctaveDown;
    event[0xd4] = (MintSpcEvent) mintSpcEventRest;
    event[0xd5] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xd6] = (MintSpcEvent) mintSpcEventPitchBendRange;
    event[0xd7] = (MintSpcEvent) mintSpcEventCoarseTuningAbs;
    event[0xd8] = (MintSpcEvent) mintSpcEventCoarseTuningRel;
    event[0xd9] = (MintSpcEvent) mintSpcEventFineTuningRel;
    event[0xda] = (MintSpcEvent) mintSpcEventKeyOn;
    event[0xdb] = (MintSpcEvent) mintSpcEventKeyOff;
    event[0xdc] = (MintSpcEvent) mintSpcEventAddVolume;
    event[0xdd] = (MintSpcEvent) mintSpcEventPitchBend;
    event[0xdf] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe0] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe1] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe2] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe3] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe4] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe5] = (MintSpcEvent) mintSpcEventUnknown1;
    event[0xe6] = (MintSpcEvent) mintSpcEventTimebaseHiRes;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* mintSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    MintSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newMintSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = mintSpcCreateSmf(seq);

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

        for (ev.track = 0; ev.track < MINTSPC_TRACK_MAX; ev.track++) {

            MintSpcTrackStat *evtr = &seq->track[ev.track];

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
                // add tick (mintspc-specific)
                if (seq->ver.event[ev.code] != mintSpcEventNoteParam)
                {
                    evtr->tick += evtr->noteWaitLen;
                    evtr->noteWaitLen = 0;
                    evtr->viaNoteParam = false;
                }
                else
                {
                    evtr->viaNoteParam = true;
                }

                // dump event report
                if (mintSpcTextLoopMax == 0 || seq->looped < mintSpcTextLoopMax)
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
            for (tr = 0; tr < MINTSPC_TRACK_MAX; tr++) {
                seq->track[tr].tick = seq->tick;
                if (seq->track[tr].used)
                    smfSetEndTimingOfTrack(seq->smf, tr, seq->tick);
            }
        }
        else {
            mintSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= mintSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, mintSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    mintSpcTruncateNoteAll(seq);
    mintSpcDequeueNoteAll(seq);

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
        delMintSpcSeq(&seq);
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
Smf* mintSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = mintSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* mintSpcToMidiFromFile (const char *filename)
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

    smf = mintSpcToMidi(data, size);

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
    mintSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    mintSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    mintSpcForceSongListAddr = songListAddr;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (mintSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    mintSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    mintSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    mintSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    mintSpcMidiResetType = SMF_RESET_GM2;
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
            mintSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = mintSpcToMidiFromFile(gArgv[0]);
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
