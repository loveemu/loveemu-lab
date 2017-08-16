/**
 * Heart Beat DQ6/DQ3 spc2midi.
 * http://loveemu.yh.land.to/
 * 
 * This is actually a sort of N-SPC variant. I noticed that after a
 * half year ago from when I made this for the first time X(
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "spcseq.h"
#include "hbdqspc.h"

#define APPNAME         "Heart Beat DQ6/DQ3 SPC2MIDI"
#define APPSHORTNAME    "hbdqspc"
#define VERSION         "[2014-02-15]"
#define AUTHOR          "loveemu"
#define WEBSITE         "http://loveemu.yh.land.to/"

static int hbSpcLoopMax = 2;            // maximum loop count of parser
static int hbSpcTextLoopMax = 1;        // maximum loop count of text output
static double hbSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool hbSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int hbSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool hbSpcNoPatchChange = false; // XXX: hack, should be false for serious conversion
static bool hbSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int hbSpcForceSongIndex = -1;
static int hbSpcForceSongListAddr = -1;
static int hbSpcForceSongBaseAddrPtr = -1;
static int hbSpcForceDurTableAddr = -1;
static int hbSpcForceVelTableAddr = -1;

static bool hbSpcPatchFixOverride = false;
static PatchFixInfo hbSpcPatchFix[256];

static int hbSpcContConvCnt = 0;
static int hbSpcContConvNum = 1;

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int hbSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_STD,            // Dragon Quest VI / III
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TIMEBASE        24
#define SPC_SONG_MAX        16
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

typedef struct TagHbSpcTrackStat HbSpcTrackStat;
typedef struct TagHbSpcSeqStat HbSpcSeqStat;
typedef void (*HbSpcEvent) (HbSpcSeqStat *, SeqEventReport *);

typedef struct TagHbSpcVerInfo {
    int id;
    int songIndex;
    int seqListAddr;
    int seqHeaderAddr;
    int instTableAddr;
    int songBaseAddrPtr;
    int vbytePtrAddrLo;
    int vbytePtrAddrHi;
    int durTableAddr;
    int velTableAddr;
    HbSpcEvent event[256];  // vcmds
    HbSpcEvent subCmd[256]; // for vcmd f9
    PatchFixInfo patchFix[256];
} HbSpcVerInfo;

typedef struct TagHbSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int lastDur;        // length for note vcmd (tick)
    int dur;            // duration (tick)
    int vel;            // velocity
    int durRate;        // duration rate (n/256)
    bool tied;          // if the note tied
    bool portamento;    // portamento flag
    int key;            // key
    int patch;          // instrument
    int transpose;      // transpose
} HbSpcNoteParam;

struct TagHbSpcTrackStat {
    bool active;        // if the channel is still active
    bool used;          // if the channel used once or not
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    HbSpcNoteParam note;     // current note param
    HbSpcNoteParam lastNote; // note params for last note
    int loopCount;      // repeat count for loop command
    int retnAddr;       // return address for loop command
    int looped;         // how many times looped (internal)
    int volume;         // volume (0-255)
};

struct TagHbSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int masterVolume;           // master volume
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    HbSpcVerInfo ver;           // game version info
    HbSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void hbSpcSetEventList (HbSpcSeqStat *seq);

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
FILE *hbSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int hbSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = hbSpcLoopMax;
    hbSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool hbSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        hbSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        hbSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            hbSpcPatchFix[patch].bankSelM = patch >> 7;
            hbSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            hbSpcPatchFix[patch].bankSelM = 0;
            hbSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        hbSpcPatchFix[patch].patchNo = patch & 0x7f;
        hbSpcPatchFix[patch].key = 0;
        hbSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        hbSpcPatchFix[src].bankSelM = bankM & 0x7f;
        hbSpcPatchFix[src].bankSelL = bankL & 0x7f;
        hbSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        hbSpcPatchFix[src].key = key;
        hbSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    hbSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *hbSpcVerToStrHtml (HbSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_STD:
        return "Standard (Dragon Quest VI &amp; III)";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void hbSpcResetTrackParam (HbSpcSeqStat *seq, int track)
{
    HbSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->note.lastDur = 0x10;
    tr->note.durRate = 0xaf;
    tr->note.vel = 0xff; // just in case
    tr->note.patch = 0; // just in case
    tr->note.transpose = 0;
    tr->note.portamento = false;
    tr->lastNote.active = false;
    tr->loopCount = 0;
    tr->retnAddr = 0;
    tr->volume = 0xff;
}

/** reset before play/convert song. */
static void hbSpcResetParam (HbSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x10;
    seq->masterVolume = 0x50;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        HbSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        hbSpcResetTrackParam(seq, track);
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
    if (hbSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &hbSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int hbSpcCheckVer (HbSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;
    int pos1;

    seq->ver.seqListAddr = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.instTableAddr = -1;
    seq->ver.songBaseAddrPtr = -1;
    seq->ver.vbytePtrAddrLo = -1;
    seq->ver.vbytePtrAddrHi = -1;
    seq->ver.durTableAddr = -1;
    seq->ver.velTableAddr = -1;

    if (hbSpcForceSongListAddr >= 0) {
        seq->ver.seqListAddr = hbSpcForceSongListAddr;
        version = SPC_VER_STD;
    }
    else {
        pos1 = indexOfHexPat(aRAM, (const byte *) "\xee\\\xf6..\xc4.\\\xf6..\xc4.\xf8.\xdd\xd5..\x8d\x00", SPC_ARAM_SIZE, NULL);
        if (pos1 >= 0 && (aRAM[pos1 + 7] - aRAM[pos1 + 2] == 12) && (aRAM[pos1 + 10] - aRAM[pos1 + 5] == 1)) {
            seq->ver.seqListAddr = mget2l(&aRAM[pos1 + 2]);
            version = SPC_VER_STD;
        }
    }

    if (hbSpcForceSongBaseAddrPtr >= 0)
        seq->ver.songBaseAddrPtr = hbSpcForceSongBaseAddrPtr;
    else {
        pos1 = indexOfHexPat(aRAM, (const byte *) "\\\xf5..\x28\x0f\\\xfd\\\xf6..\xc4.\\\xf6..\xc4.", SPC_ARAM_SIZE, NULL);
        if (pos1 >= 0 && (aRAM[pos1 + 15] - aRAM[pos1 + 10] == 1)) {
            seq->ver.songBaseAddrPtr = aRAM[pos1 + 10];
        }
    }

    pos1 = indexOfHexPat(aRAM, (const byte *) "\\\xf7.\xc4.\\\xfc\xd5..\\\xf7.\x04\x01\\\xf0.\\\xf7.\xd5..", SPC_ARAM_SIZE, NULL);
    if (pos1 >= 0) {
        seq->ver.vbytePtrAddrLo = mget2l(&aRAM[pos1 + 6]);
        seq->ver.vbytePtrAddrHi = mget2l(&aRAM[pos1 + 17]);
    }

    pos1 = indexOfHexPat(aRAM, (const byte *) "\x2d\x9f\x28\x0f\\\xfd\\\xf6..\xd5..\xae\x28\x0f\\\xfd\\\xf6..\xd5..", SPC_ARAM_SIZE, NULL);
    if (pos1 >= 0) {
        seq->ver.durTableAddr = mget2l(&aRAM[pos1 + 6]);
        seq->ver.velTableAddr = mget2l(&aRAM[pos1 + 16]);
    }
    if (hbSpcForceDurTableAddr >= 0)
        seq->ver.durTableAddr = hbSpcForceDurTableAddr;
    if (hbSpcForceVelTableAddr >= 0)
        seq->ver.velTableAddr = hbSpcForceVelTableAddr;

    if (hbSpcForceSongIndex >= 0)
        seq->ver.songIndex = hbSpcForceSongIndex;
    else if (seq->ver.vbytePtrAddrLo >= 0 && seq->ver.vbytePtrAddrHi >= 0) {
        int songIndex = 0;
        int songId;
        int dist, minDist = SPC_ARAM_SIZE * SPC_TRACK_MAX;
        int curPos[SPC_TRACK_MAX];
        int songAddr, vbyteStartAddr, vbyteStartOfs;
        int track;

        // read current position first
        for (track = 0; track < SPC_TRACK_MAX; track++) {
            curPos[track] = 0;
        }
        for (track = 0; track < SPC_TRACK_MAX; track++) {
            int trackAddr = (aRAM[seq->ver.vbytePtrAddrHi + track] << 8) | aRAM[seq->ver.vbytePtrAddrLo + track];

            if (!trackAddr)
                break;
            if (seq->ver.songBaseAddrPtr >= 0)
                trackAddr += mget2l(&aRAM[seq->ver.songBaseAddrPtr]);
            curPos[track] = trackAddr;
        }

        // autosearch 1
        for (songId = 0; songId < SPC_SONG_MAX; songId++) {
            songAddr = (aRAM[seq->ver.seqListAddr + 12 + songId] << 8)
                | aRAM[seq->ver.seqListAddr + songId];
            if (!songAddr)
                break;

            dist = 0;
            for (track = 0; track < SPC_TRACK_MAX; track++) {
                vbyteStartAddr = mget2l(&aRAM[songAddr + 2 + track * 2]) & 0xffff;
                vbyteStartOfs = vbyteStartAddr;
                if (vbyteStartAddr && seq->ver.songBaseAddrPtr >= 0) {
                    vbyteStartAddr += songAddr;
                }
                if (!vbyteStartAddr) {
/*
                    for (; track < SPC_TRACK_MAX; track++) {
                        if (curPos[track])
                            dist += SPC_ARAM_SIZE;
                    }
*/
                    break;
                }
                if (!curPos[track])
                    break;

                if (seq->ver.songBaseAddrPtr >= 0 && (vbyteStartOfs & 0x8000))
                    dist += SPC_ARAM_SIZE * SPC_TRACK_MAX;
                else
                    dist += abs(curPos[track] - vbyteStartAddr);
            }
            if (dist < minDist) {
                songIndex = songId;
                minDist = dist;
            }
        }
        seq->ver.songIndex = songIndex;
    }
    else if (seq->ver.songBaseAddrPtr >= 0) {
        int songIndex = 0;
        int songId, songAddr;
        int curSongAddr = mget2l(&aRAM[seq->ver.songBaseAddrPtr]);
        int dist, minDist = SPC_ARAM_SIZE;

        // autosearch 2
        if (curSongAddr) {
            for (songId = 0; songId < SPC_SONG_MAX; songId++) {
                songAddr = (aRAM[seq->ver.seqListAddr + 12 + songId] << 8)
                    | aRAM[seq->ver.seqListAddr + songId];

                dist = abs(songAddr - curSongAddr);
                if (dist < minDist) {
                    songIndex = songId;
                    minDist = dist;
                }
            }
        }
        seq->ver.songIndex = songIndex;
    }
    else
        seq->ver.songIndex = 0;

    if (seq->ver.seqListAddr >= 0 && seq->ver.songIndex >= 0) {
        seq->ver.songIndex += hbSpcContConvCnt;
        seq->ver.seqHeaderAddr = (aRAM[seq->ver.seqListAddr + 12 + seq->ver.songIndex] << 8)
            | aRAM[seq->ver.seqListAddr + seq->ver.songIndex];
    }

    if (seq->ver.seqHeaderAddr >= 0)
        seq->ver.instTableAddr = seq->ver.seqHeaderAddr + mget2l(&seq->aRAM[seq->ver.seqHeaderAddr]);

    seq->ver.id = version;
    hbSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool hbSpcDetectSeq (HbSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int seqHeaderAddr;
    bool result;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    // disable all tracks first
    result = false;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].active = false;
    }

    seqHeaderAddr = seq->ver.seqHeaderAddr;
    // track list (relative offset)
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].pos = mget2l(&aRAM[seqHeaderAddr + 2 + tr * 2]);
        if (!seq->track[tr].pos)
            break;
        seq->track[tr].pos += seqHeaderAddr;
        seq->track[tr].pos &= 0xffff;

        seq->track[tr].active = true;
        result = true;
    }
    hbSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static HbSpcSeqStat *newHbSpcSeq (const byte *aRAM)
{
    HbSpcSeqStat *newSeq = (HbSpcSeqStat *) calloc(1, sizeof(HbSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        hbSpcCheckVer(newSeq);
        if (!hbSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delHbSpcSeq (HbSpcSeqStat **seq)
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
static void printHtmlInfoList (HbSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", hbSpcVerToStrHtml(seq));
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Current Song Pointer: $%04X", seq->ver.songBaseAddrPtr);
    if (seq->ver.songBaseAddrPtr >= 0) {
        myprintf("<ul>\n");
        myprintf("            <li>Current Song: $%04X</li>\n", mget2l(&seq->aRAM[seq->ver.songBaseAddrPtr]));
        myprintf("          </ul>");
    }
    myprintf("</li>\n");
    myprintf("          <li>Voice Stream Pointer (lo/hi): $%04X/$%04X</li>\n", seq->ver.vbytePtrAddrLo, seq->ver.vbytePtrAddrHi);
    myprintf("          <li>Duration Table: $%04X</li>\n", seq->ver.durTableAddr);
    myprintf("          <li>Velocity Table: $%04X</li>\n", seq->ver.velTableAddr);
    myprintf("          <li>Sequence Header: $%04X (Song $%02X)</li>\n", seq->ver.seqHeaderAddr, seq->ver.songIndex);
    myprintf("          <li>Instrument Table: $%04X ($%04X)</li>\n", seq->ver.instTableAddr, (seq->ver.instTableAddr - seq->ver.seqHeaderAddr) & 0xffff);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (HbSpcSeqStat *seq)
{
    int track;

    if (seq == NULL)
        return;

    myprintf("          <li>Voices<ul>\n");
    myprintf("            <li>Address: ");
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        if (!seq->track[track].active)
            break;
        if (track)
            myprintf(" ");
        myprintf("%d:$%04X", track + 1, seq->track[track].pos);
    }
    myprintf("</li>\n");
    myprintf("            <li>Offset : ");
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        if (!seq->track[track].active)
            break;
        if (track)
            myprintf(" ");
        myprintf("%d:$%04X", track + 1, seq->track[track].pos - seq->ver.seqHeaderAddr);
    }
    myprintf("</li>\n");
    myprintf("          </ul></li>\n");
}

/** output other seq detail for valid seq. */
static void printHtmlInfoOthers (HbSpcSeqStat *seq)
{
    if (seq->ver.instTableAddr >= 0) {
        int inst, instNum, ofs;

        myprintf("        <h3>Instruments</h3>\n");
        myprintf("        <div class=\"section\">\n");
        myprintf("          <table>\n");
        myprintf("            <tr><th>Address</th><th>Item #1</th><th>ADSR(1)</th><th>ADSR(2)</th><th>Item #4</th><th>Item #5</th><th>Item #6</th></tr>\n");
        instNum = abs((seq->track[0].pos - seq->ver.instTableAddr) / 6);
        instNum = min(instNum, 256);
        for (inst = 0; inst < instNum; inst++) {
            int instEntryAddr = seq->ver.instTableAddr + inst * 6;

            myprintf("            <tr><th>$%04X</th>", instEntryAddr);
            for (ofs = 0; ofs < 6; ofs++) {
                myprintf("<td>$%02X</td>", seq->aRAM[instEntryAddr + ofs]);
            }
            myprintf("  </tr>\n");
        }
        myprintf("          </table>\n");
        myprintf("        </div>\n");
    }
}

/** output event dump. */
static void printHtmlEventDump (HbSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (HbSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("        <h3>Sequence</h3>\n");
    myprintf("        <div class=\"section\">\n");
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

/** output event table footer. */
static void printEventTableFooter (HbSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

//----

/** convert SPC tempo into bpm. */
static double hbSpcTempo (HbSpcSeqStat *seq)
{
    return (double) seq->tempo * 60000000 / 12240000; // 12240000 = (timer0) 2ms * 24 * 256
}

/** convert SPC velocity into MIDI one. */
static int hbSpcMidiVelOf(int value)
{
    // TODO: I don't know which method is better, more investigation is needed.
/*
    if (hbSpcVolIsLinear)
        return (int) floor(pow((double) value/255, 2) * 127 + 0.5);
    else
        return value/2;
*/
    if (hbSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int hbSpcMidiVolOf (int value)
{
    // TODO: I don't know which method is better, more investigation is needed.
/*
    if (hbSpcVolIsLinear)
        return (int) floor(pow((double) value/255, 2) * 127 + 0.5);
    else
        return value/2;
*/
    if (hbSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC master volume into MIDI one. */
static int hbSpcMidiMasterVolOf(int value)
{
    // TODO: Is this correct?
    //if (hbSpcVolIsLinear)
        return value/2; // linear
    //else
    //    return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC panpot into MIDI once. */
static int hbSpcMidiPanOf(int value)
{
    return (value/2) + 64; // linear
}

/** create new smf object and link to spc seq. */
static Smf *hbSpcCreateSmf (HbSpcSeqStat *seq)
{
    static char songTitle[512];
    Smf* smf;
    int tr;

    smf = smfCreate(SPC_TIMEBASE);
    if (!smf)
        return NULL;
    seq->smf = smf;

    sprintf(songTitle, "%s %s", APPNAME, VERSION);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, songTitle);

    switch (hbSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, hbSpcTempo(seq));
    smfInsertMasterVolume(smf, 0, 0, 0, hbSpcMidiMasterVolOf(seq->masterVolume));

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (!seq->track[tr].active)
            continue;

        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, hbSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 14);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);

        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].pos);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void hbSpcTruncateNote (HbSpcSeqStat *seq, int track)
{
    HbSpcTrackStat *tr = &seq->track[track];

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
static void hbSpcTruncateNoteAll (HbSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        hbSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool hbSpcDequeueNote (HbSpcSeqStat *seq, int track)
{
    HbSpcTrackStat *tr = &seq->track[track];
    HbSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        if (lastNote->tied)
            dur = (lastNote->dur * lastNote->durRate) >> 8;
        else
            dur = (lastNote->dur - lastNote->lastDur)
                + ((lastNote->lastDur * lastNote->durRate) >> 8);
        if (dur == 0)
            dur++;

        key = lastNote->key + lastNote->transpose
            + seq->ver.patchFix[tr->lastNote.patch].key
            + SPC_NOTE_KEYSHIFT;
        vel = hbSpcMidiVelOf(lastNote->vel);
        if (vel == 0)
            vel++;

        result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void hbSpcDequeueNoteAll (HbSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        hbSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void hbSpcInactiveTrack(HbSpcSeqStat *seq, int track)
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
static void hbSpcAddTrackLoopCount(HbSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (hbSpcLoopMax > 0) ? hbSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= hbSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void hbSpcSeqAdvTick(HbSpcSeqStat *seq)
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
    seq->time += (double) 60 / hbSpcTempo(seq) * minTickStep / SPC_TIMEBASE;
}

/** vcmds: unknown event (without status change). */
static void hbSpcEventUnknownInline (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X [Track %d]\n", ev->code, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X [Track %d]\n", ev->code, ev->track + 1);
}

/** vcmds: unidentified event. */
static void hbSpcEventUnidentified (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    hbSpcEventUnknownInline(seq, ev);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void hbSpcEventUnknown0 (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    hbSpcEventUnknownInline(seq, ev);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void hbSpcEventUnknown1 (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    hbSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void hbSpcEventUnknown2 (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    hbSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void hbSpcEventUnknown3 (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    hbSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f1: set duration/velocity (instance). */
static void hbSpcEventDurVelInline (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int lo, hi;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    const byte durTable_[0x10] = {
        0x23, 0x46, 0x69, 0x8c, 0xaf, 0xd2, 0xf5, 0xff,
        0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82, // out of table! (just in case)
    };
    const byte velTable_[0x10] = {
        0x19, 0x28, 0x37, 0x46, 0x55, 0x64, 0x73, 0x82,
        0x91, 0xa0, 0xb0, 0xbe, 0xcd, 0xdc, 0xeb, 0xff,
    };
    const byte *durTable;
    const byte *velTable;

    durTable = (seq->ver.durTableAddr >= 0) ? &seq->aRAM[seq->ver.durTableAddr] : durTable_;
    velTable = (seq->ver.velTableAddr >= 0) ? &seq->aRAM[seq->ver.velTableAddr] : velTable_;

    ev->size++;
    hi = (seq->aRAM[*p] & 0xf0) >> 4;
    lo = seq->aRAM[*p] & 0x0f;
    (*p)++;

    tr->note.durRate = durTable[hi];
    tr->note.vel = velTable[lo];

    sprintf(argDumpStr, ", dur = %d/255, vel = %d/255",
        tr->note.durRate, tr->note.vel);
    strcat(ev->note, argDumpStr);
}

/** vcmd 00: end of track. */
static void hbSpcEventEndOfTrack (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    hbSpcInactiveTrack(seq, ev->track);
}

/** vcmd 01-7f: set note param. */
static void hbSpcEventNoteParam (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    tr->note.dur = ev->code;
    sprintf(ev->note, "Note Param, length = %d", ev->code);
    if (seq->aRAM[*p] < 0x80) {
        hbSpcEventDurVelInline(seq, ev);
    }
    strcat(ev->classStr, " ev-noteparam");
}

/** vcmd 80-cf: note. */
static void hbSpcEventNote (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int note = (ev->code - 0x80);
    HbSpcTrackStat *tr = &seq->track[ev->track];

    // outputput old note first
    hbSpcDequeueNote(seq, ev->track);

    // set new note
    tr->lastNote.tick = ev->tick;
    tr->lastNote.dur = tr->note.dur;
    tr->lastNote.lastDur = tr->note.dur;
    tr->lastNote.key = note;
    tr->lastNote.durRate = tr->note.durRate;
    tr->lastNote.vel = tr->note.vel;
    tr->lastNote.transpose = seq->transpose + tr->note.transpose;
    tr->lastNote.patch = tr->note.patch;
    tr->lastNote.tied = false;
    tr->lastNote.active = true;

    // step
    tr->tick += tr->note.dur;

    getNoteName(argDumpStr, note + SPC_NOTE_KEYSHIFT + seq->transpose + tr->note.transpose);
    sprintf(ev->note, "Note %s", argDumpStr);
    strcat(ev->classStr, " ev-note");
}

/** vcmd d0: tie. */
static void hbSpcEventTie (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];

    if (tr->lastNote.active) {
        tr->lastNote.dur += tr->note.dur;
    }
    tr->lastNote.tied = true;

    tr->lastNote.lastDur = tr->note.dur;
    tr->lastNote.durRate = tr->note.durRate;
    tr->tick += tr->note.dur;

    sprintf(ev->note, "Tie");
    strcat(ev->classStr, " ev-tie");
}

/** vcmd d1: rest. */
static void hbSpcEventRest (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];

    // outputput old note
    hbSpcDequeueNote(seq, ev->track);

    tr->lastNote.lastDur = tr->note.dur;
    tr->lastNote.durRate = tr->note.durRate;
    tr->tick += tr->note.dur;

    sprintf(ev->note, "Rest");
    strcat(ev->classStr, " ev-rest");
}

/** vcmd d2: portamento on. */
static void hbSpcEventPortamentoOn (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];

    tr->note.portamento = true;
    sprintf(ev->note, "Portamento On");
    strcat(ev->classStr, " ev-portamento-on");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 127);
}

/** vcmd d3: portamento off. */
static void hbSpcEventPortamentoOff (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];

    tr->note.portamento = false;
    sprintf(ev->note, "Portamento Off");
    strcat(ev->classStr, " ev-portamento-off");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 0);
}

/** vcmd d4: set patch. */
static void hbSpcEventInstrument(HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->note.patch = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[arg1].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[arg1].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[arg1].patchNo);

    sprintf(ev->note, "Set Patch, patch = %d", arg1);
    strcat(ev->classStr, " ev-patch");
}

/** vcmd d6: panpot. */
static void hbSpcEventPanpot (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot, val = %d", arg1); // no phase reverse surround
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pan");
}

/** vcmd d7: panpot fade. */
static void hbSpcEventPanFade (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Pan Fade, step = %d, to = %d", arg1, arg2);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-panfade");
}

/** vcmd d8: vibrato on. */
static void hbSpcEventVibratoOn (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato On, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratoon");
}

/** vcmd d9: vibrato fade(-in). */
static void hbSpcEventVibratoFade (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato Fade, length = %d", arg1);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratofade");
}

/** vcmd da: vibrato off. */
static void hbSpcEventVibratoOff (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Vibrato Off");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratooff");
}

/** vcmd db: master volume. */
static void hbSpcEventMasterVolume (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->masterVolume = arg1;
    smfInsertMasterVolume(seq->smf, ev->tick, 0, ev->track, hbSpcMidiMasterVolOf(seq->masterVolume));

    sprintf(ev->note, "Master Volume, value = %d", arg1);
    strcat(ev->classStr, " ev-mastervol");
}


/** vcmd dc: master volume fade. */
static void hbSpcEventMasterVolFade (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume Fade, length = %d, to = %d", arg1, arg2);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-mastervolfade");
}

/** vcmd dd: tempo. */
static void hbSpcEventTempo (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    seq->tempo = arg1;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, hbSpcTempo(seq));

    sprintf(ev->note, "Tempo, bpm = %f", hbSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");
}

/** vcmd df: transpose (global). */
static void hbSpcEventKeyShift (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    seq->transpose = arg1;
    sprintf(ev->note, "Global Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");
}

/** vcmd e0: transpose (per-voice). */
static void hbSpcEventTranspose (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;
    sprintf(ev->note, "Channel Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose-ch");
}

/** vcmd e1: tremolo on. */
static void hbSpcEventTremoloOn (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2, arg3;
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Tremolo On, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tremoloon");
}

/** vcmd e2: tremolo off. */
static void hbSpcEventTremoloOff (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Tremolo Off");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tremolooff");
}

/** vcmd e3: volume. */
static void hbSpcEventVolume (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->volume = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, hbSpcMidiVolOf(arg1));

    sprintf(ev->note, "Volume, val = %d", arg1);
    strcat(ev->classStr, " ev-vol");
}

/** vcmd e4: volume fade. */
static void hbSpcEventVolumeFade (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1, arg2;
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Fade, length = %d, to = %d", arg1, arg2);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-volumefade");
}

/** vcmd e6: pitch envelope (to). */
static void hbSpcEventPitchEnvTo (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Envelope (To), delay = %d, step = %d, key = %d", arg1, arg2, arg3);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvfrom");
}

/** vcmd e7: pitch envelope (from). */
static void hbSpcEventPitchEnvFrom (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Envelope (From), delay = %d, step = %d, key = %d", arg1, arg2, arg3);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvto");
}

/** vcmd e8: pitch envelope off. */
static void hbSpcEventPitchEnvOff (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Pitch Envelope Off");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvoff");
}

/** vcmd: tuning e9. */
static void hbSpcEventTuning (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNM, 0);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNL, 1);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYM, 64 + arg1 / 4);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYL, (arg1 % 4) * 32);

    sprintf(ev->note, "Tuning, amount = %d/256", arg1);
    strcat(ev->classStr, " ev-tuning");
}

/** vcmd ea: set echo level. */
static void hbSpcEventEchoVol (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Level, L = %d, R = %d", arg1, arg2);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echolevel");
}

/** vcmd eb: set echo param. */
static void hbSpcEventEchoParam (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, delay = %d (%dms), feedback = %d, FIR preset = %d", arg1, arg1 * 16, arg2, arg3);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echoparam");
}

/** vcmd ed: echo off. */
static void hbSpcEventEchoOff (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo Off");
    strcat(ev->classStr, " ev-echooff");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 0);
}

/** vcmd ee: echo on. */
static void hbSpcEventEchoOn (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Echo On");
    strcat(ev->classStr, " ev-echoon");
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, 100);
}

/** vcmd ef: set echo FIR. */
static void hbSpcEventEchoFIR (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int i;

    ev->size += 8;

    sprintf(ev->note, "Echo FIR, filter =");
    for (i = 0; i < 8; i++) {
        sprintf(argDumpStr, " %02X", seq->aRAM[*p]);
        strcat(ev->note, argDumpStr);
        (*p)++;
    }
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echofir");
}

/** vcmd f0: set ADSR. */
static void hbSpcEventADSR (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
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
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-adsr");
}

/** vcmd f1: set duration/velocity. */
static void hbSpcEventDurVel (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Duration/Velocity");
    hbSpcEventDurVelInline(seq, ev);
    strcat(ev->classStr, " ev-durvel");
}

/** vcmd f2: jump. */
static void hbSpcEventJump (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;

    // assumes backjump = loop
    dest = (seq->ver.seqHeaderAddr + arg1) & 0xffff;
    if (dest <= *p) {
        hbSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = dest;

    sprintf(ev->note, "Jump, addr = $%04X", dest);
    strcat(ev->classStr, " ev-jump");
}

/** vcmd f3: call subroutine. */
static void hbSpcEventSubroutine (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;
    tr->retnAddr = *p - seq->ver.seqHeaderAddr;

    dest = (seq->ver.seqHeaderAddr + arg1) & 0xffff;
    *p = dest;

    sprintf(ev->note, "Call, addr = $%04X", dest);
    strcat(ev->classStr, " ev-subroutine");
}

/** vcmd f4: return from subroutine. */
static void hbSpcEventReturn (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int dest;

    dest = (seq->ver.seqHeaderAddr + tr->retnAddr) & 0xffff;
    *p = dest;

    sprintf(ev->note, "Return From Subroutine, addr = $%04X", dest);
    strcat(ev->classStr, " ev-ret");
}

/** vcmd f5: noise on. */
static void hbSpcEventNoiseOn (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Noise On");
    strcat(ev->classStr, " ev-noiseon");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f6: noise off. */
static void hbSpcEventNoiseOff (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Noise Off");
    strcat(ev->classStr, " ev-noiseoff");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f7: set noise clock. */
static void hbSpcEventNoiseClk (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &tr->pos;
    int nck;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    nck = arg1 & 0x1f;

    sprintf(ev->note, "Noise Clock, freq = %d Hz", spcNCKTable[nck]);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-noiseclock");
}

/** f9 subcmds: unknown subcmd. */
static void hbSpcEventSubUnidentified (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(argDumpStr, "Unknown Subcmd %02X", ev->code);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " unknown");
    ev->unidentified = true;
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    fprintf(stderr, "Error: Encountered unidentified subcmd %02X [Track %d]\n", ev->code, ev->track + 1);
}

/** f9 subcmd 00: set repeat count. */
static void hbSpcEventSubLoopCnt (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->loopCount = arg1;
    sprintf(ev->note, "Loop Count, count = %d", arg1);
}

/** f9 subcmd 01: conditional loop. */
static void hbSpcEventSubLoop (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int dest;
    bool jump;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->aRAM[*p]));
    (*p) += 2;

    if (tr->loopCount)
        tr->loopCount--;
    jump = (tr->loopCount);
    dest = (seq->ver.seqHeaderAddr + arg1) & 0xffff;

    if (jump)
        *p = dest;

    sprintf(ev->note, "Conditional Loop, addr = $%02X%s", dest, jump ? "" : "*");
}

/** f9 subcmd 02: ?. */
static void hbSpcEventSub02 (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Subcmd 02");
    strcat(ev->classStr, " unknown");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    fprintf(stderr, "Warning: Skipped unknown subcmd %02X [Track %d]\n", ev->code, ev->track + 1);
}

/** f9 subcmd 03: set attack rate. */
static void hbSpcEventSubAR (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int ar;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    ar = arg1 & 0x0f;

    sprintf(ev->note, "Attack Rate, rate = %d (%.1f%s)", arg1,
        (spcARTable[ar] >= 1) ? spcARTable[ar] : 
        spcARTable[ar] * 1000, (spcARTable[ar] >= 1) ? "s" : "ms");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** f9 subcmd 04: set decay rate. */
static void hbSpcEventSubDR (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int dr;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    dr = arg1 & 0x07;

    sprintf(ev->note, "Decay Rate, rate = %d (%.1f%s)", arg1,
        (spcDRTable[dr] >= 1) ? spcDRTable[dr] : 
        spcDRTable[dr] * 1000, (spcDRTable[dr] >= 1) ? "s" : "ms");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** f9 subcmd 05: set sustain level. */
static void hbSpcEventSubSL (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int sl;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sl = arg1 & 0x07;

    sprintf(ev->note, "Sustain Level, rate = %d (%d/8)", sl, sl + 1);
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** f9 subcmd 06: set release rate. */
static void hbSpcEventSubRR (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int rr;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    rr = arg1 & 0x1f;

    sprintf(ev->note, "Release Rate, rate = %d (%.1f%s)", arg1,
        (spcSRTable[rr] >= 1) ? spcSRTable[rr] : 
        spcSRTable[rr] * 1000, (spcSRTable[rr] >= 1) ? "s" : "ms");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** f9 subcmd 07: set sustain rate. */
static void hbSpcEventSubSR (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    int sr;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sr = arg1 & 0x1f;

    sprintf(ev->note, "Sustain Rate, rate = %d (%.1f%s)", arg1,
        (spcSRTable[sr] >= 1) ? spcSRTable[sr] : 
        spcSRTable[sr] * 1000, (spcSRTable[sr] >= 1) ? "s" : "ms");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** f9 subcmd 09: surround effect (phase reverse). */
static void hbSpcEventSubSurround (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Phase Reverse Surround, L = %s, R = %s", arg1 ? "on" : "off", arg2 ? "on" : "off");
    strcat(ev->classStr, " ev-surround");
    if (!hbSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd f9: do subcmd. */
static void hbSpcEventSubCmd (HbSpcSeqStat *seq, SeqEventReport *ev)
{
    HbSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte evRealCode = ev->code;

    ev->size++;
    ev->code = seq->aRAM[*p];
    (*p)++;

    sprintf(argDumpStr, " ev-sub%02x", ev->code);
    strcat(ev->classStr, argDumpStr);

    sprintf(ev->note, "%02X: ", evRealCode);

    // dispatch subcmd
    seq->ver.subCmd[ev->code](seq, ev);
}

/** set pointers of each event. */
static void hbSpcSetEventList (HbSpcSeqStat *seq)
{
    int code;
    HbSpcEvent *event = seq->ver.event;
    HbSpcEvent *subCmd = seq->ver.subCmd;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (HbSpcEvent) hbSpcEventUnidentified;
    }
    for(code = 0x00; code <= 0xff; code++) {
        subCmd[code] = (HbSpcEvent) hbSpcEventSubUnidentified;
    }

    event[0x00] = (HbSpcEvent) hbSpcEventEndOfTrack;
    for(code = 0x01; code <= 0x7f; code++) {
        event[code] = (HbSpcEvent) hbSpcEventNoteParam;
    }
    for(code = 0x80; code <= 0xcf; code++) {
        event[code] = (HbSpcEvent) hbSpcEventNote;
    }
    event[0xd0] = (HbSpcEvent) hbSpcEventTie;
    event[0xd1] = (HbSpcEvent) hbSpcEventRest;
    event[0xd2] = (HbSpcEvent) hbSpcEventPortamentoOn;
    event[0xd3] = (HbSpcEvent) hbSpcEventPortamentoOff;
    event[0xd4] = (HbSpcEvent) hbSpcEventInstrument;
    event[0xd5] = (HbSpcEvent) hbSpcEventUnknown0; // 7?
    event[0xd6] = (HbSpcEvent) hbSpcEventPanpot;
    event[0xd7] = (HbSpcEvent) hbSpcEventPanFade;
    event[0xd8] = (HbSpcEvent) hbSpcEventVibratoOn;
    event[0xd9] = (HbSpcEvent) hbSpcEventVibratoFade;
    event[0xda] = (HbSpcEvent) hbSpcEventVibratoOff;
    event[0xdb] = (HbSpcEvent) hbSpcEventMasterVolume;
    event[0xdc] = (HbSpcEvent) hbSpcEventMasterVolFade;
    event[0xdd] = (HbSpcEvent) hbSpcEventTempo;
    event[0xde] = (HbSpcEvent) hbSpcEventUnknown1;
    event[0xdf] = (HbSpcEvent) hbSpcEventKeyShift;
    event[0xe0] = (HbSpcEvent) hbSpcEventTranspose;
    event[0xe1] = (HbSpcEvent) hbSpcEventTremoloOn;
    event[0xe2] = (HbSpcEvent) hbSpcEventTremoloOff;
    event[0xe3] = (HbSpcEvent) hbSpcEventVolume;
    event[0xe4] = (HbSpcEvent) hbSpcEventVolumeFade;
    event[0xe5] = (HbSpcEvent) hbSpcEventUnknown3;
    event[0xe6] = (HbSpcEvent) hbSpcEventPitchEnvTo;
    event[0xe7] = (HbSpcEvent) hbSpcEventPitchEnvFrom;
    event[0xe8] = (HbSpcEvent) hbSpcEventPitchEnvOff;
    event[0xe9] = (HbSpcEvent) hbSpcEventTuning;
    event[0xea] = (HbSpcEvent) hbSpcEventEchoVol;
    event[0xeb] = (HbSpcEvent) hbSpcEventEchoParam;
    event[0xec] = (HbSpcEvent) hbSpcEventUnknown3;
    event[0xed] = (HbSpcEvent) hbSpcEventEchoOff;
    event[0xee] = (HbSpcEvent) hbSpcEventEchoOn;
    event[0xef] = (HbSpcEvent) hbSpcEventEchoFIR;
    event[0xf0] = (HbSpcEvent) hbSpcEventADSR;
    event[0xf1] = (HbSpcEvent) hbSpcEventDurVel;
    event[0xf2] = (HbSpcEvent) hbSpcEventJump;
    event[0xf3] = (HbSpcEvent) hbSpcEventSubroutine;
    event[0xf4] = (HbSpcEvent) hbSpcEventReturn;
    event[0xf5] = (HbSpcEvent) hbSpcEventNoiseOn;
    event[0xf6] = (HbSpcEvent) hbSpcEventNoiseOff;
    event[0xf7] = (HbSpcEvent) hbSpcEventNoiseClk;
    event[0xf8] = (HbSpcEvent) hbSpcEventUnknown0;
    event[0xf9] = (HbSpcEvent) hbSpcEventSubCmd;

    subCmd[0x00] = (HbSpcEvent) hbSpcEventSubLoopCnt;
    subCmd[0x01] = (HbSpcEvent) hbSpcEventSubLoop;
    subCmd[0x02] = (HbSpcEvent) hbSpcEventSub02; // NOP?
    subCmd[0x03] = (HbSpcEvent) hbSpcEventSubAR;
    subCmd[0x04] = (HbSpcEvent) hbSpcEventSubDR;
    subCmd[0x05] = (HbSpcEvent) hbSpcEventSubSL;
    subCmd[0x06] = (HbSpcEvent) hbSpcEventSubRR;
    subCmd[0x07] = (HbSpcEvent) hbSpcEventSubSR;
    subCmd[0x09] = (HbSpcEvent) hbSpcEventSubSurround;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* hbSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    HbSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newHbSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = hbSpcCreateSmf(seq);

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

            HbSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (hbSpcTextLoopMax == 0 || seq->looped < hbSpcTextLoopMax)
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
            hbSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= hbSpcTimeLimit) {
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    hbSpcTruncateNoteAll(seq);
    hbSpcDequeueNoteAll(seq);

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
        delHbSpcSeq(&seq);
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
Smf* hbSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = hbSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* hbSpcToMidiFromFile (const char *filename)
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

    smf = hbSpcToMidi(data, size);

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
static bool cmdOptCount (void);
static bool cmdOptSong (void);
static bool cmdOptSongList (void);
static bool cmdOptSongBase (void);
static bool cmdOptDurTbl (void);
static bool cmdOptVelTbl (void);
static bool cmdOptLoop (void);
static bool cmdOptPatchFix (void);
static bool cmdOptGS (void);
static bool cmdOptXG (void);
static bool cmdOptGM1 (void);
static bool cmdOptGM2 (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "count", '\0', 1, cmdOptCount, "<n>", NULL/*"convert n songs continuously"*/ },
    { "song", '\0', 1, cmdOptSong, "<index>", "force set song index" },
    { "songlist", '\0', 1, cmdOptSongList, "<addr>", "force set song (list) address" },
    { "songbase", '\0', 1, cmdOptSongBase, "<addr>", "specify current song pointer address (advanced)" },
    { "durtbl", '\0', 1, cmdOptDurTbl, "<addr>", "specify duration table address (advanced)" },
    { "veltbl", '\0', 1, cmdOptVelTbl, "<addr>", "specify velocity table address (advanced)" },
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

/** set number of songs to convert. */
static bool cmdOptCount (void)
{
    int count = strtol(gArgv[0], NULL, 0);
    hbSpcContConvNum = count;
    return true;
}

/** set loop song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    hbSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    hbSpcForceSongListAddr = songListAddr;
    return true;
}

/** set current song pointer address. */
static bool cmdOptSongBase (void)
{
    int songBaseAddrPtr = strtol(gArgv[0], NULL, 16);
    hbSpcForceSongBaseAddrPtr = songBaseAddrPtr;
    return true;
}

/** set duration table address. */
static bool cmdOptDurTbl (void)
{
    int tableAddr = strtol(gArgv[0], NULL, 16);
    hbSpcForceDurTableAddr = tableAddr;
    return true;
}

/** set velocity table address. */
static bool cmdOptVelTbl (void)
{
    int tableAddr = strtol(gArgv[0], NULL, 16);
    hbSpcForceVelTableAddr = tableAddr;
    return true;
}

/** set loop count. */
static bool cmdOptLoop (void)
{
    int loopCount = strtol(gArgv[0], NULL, 0);
    hbSpcSetLoopCount(loopCount);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (hbSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    hbSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    hbSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    hbSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    hbSpcMidiResetType = SMF_RESET_GM2;
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
    char tmpPath[PATH_MAX];
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
    for (hbSpcContConvCnt = 0; hbSpcContConvCnt < hbSpcContConvNum; hbSpcContConvCnt++) {
        strcpy(spcPath, spcBasePath);
        strcpy(midPath, midBasePath);
        strcpy(htmlPath, htmlBasePath);
        if (hbSpcContConvCnt) {
            sprintf(tmpPath, "%s-%03d.mid", removeExt(midPath), hbSpcContConvCnt + 1);
            strcpy(midPath, tmpPath);
            if (htmlPath[0] != '\0') {
                sprintf(tmpPath, "%s-%03d.html", removeExt(htmlPath), hbSpcContConvCnt + 1);
                strcpy(htmlPath, tmpPath);
            }
        }

        // set html handle if needed
        htmlFile = (htmlPath[0] != '\0') ? fopen(htmlPath, "w") : NULL;
        hbSpcSetLogStreamHandle(htmlFile);

        fprintf(stderr, "%s", spcPath);
        if (hbSpcContConvCnt)
            fprintf(stderr, "(%d)", hbSpcContConvCnt + 1);
        fprintf(stderr, ":\n");

        smf = hbSpcToMidiFromFile(spcPath);
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
