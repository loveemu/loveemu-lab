/**
 * Wagan Paradise spc2midi.
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
#include "wgpspc.h"

#define APPNAME         "Wagan Paradise SPC2MIDI"
#define APPSHORTNAME    "wgpspc"
#define VERSION         "[2014-02-15]"
#define AUTHOR          "loveemu"
#define WEBSITE         "http://loveemu.yh.land.to/"

// from VS2008 math.h
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616

static int wgpSpcLoopMax = 2;            // maximum loop count of parser
static int wgpSpcTextLoopMax = 1;        // maximum loop count of text output
static double wgpSpcTimeLimit = 2400;    // time limit of conversion (for safety)
static bool wgpSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int wgpSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool wgpSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int wgpSpcTimeBase = 32;
static int wgpSpcForceSongIndex = -1;
static int wgpSpcForceSongSlotIndex = -1;
static int wgpSpcForceSongListAddr = -1;

static bool wgpSpcPatchFixOverride = false;
static PatchFixInfo wgpSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int wgpSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_WGP,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_SONG_MAX        0x60
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

typedef struct TagWgpSpcTrackStat WgpSpcTrackStat;
typedef struct TagWgpSpcSeqStat WgpSpcSeqStat;
typedef void (*WgpSpcEvent) (WgpSpcSeqStat *, SeqEventReport *);

typedef struct TagWgpSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int seqHeaderAddr;
    WgpSpcEvent event[256];  // vcmds
    PatchFixInfo patchFix[256];
} WgpSpcVerInfo;

typedef struct TagWgpSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // duration (tick)
    int vel;            // velocity
    bool tied;          // if the note tied
    int key;            // key
    int patch;          // instrument
    int transpose;      // transpose
} WgpSpcNoteParam;

struct TagWgpSpcTrackStat {
    WgpSpcNoteParam note;     // current note param
    WgpSpcNoteParam lastNote; // note params for last note
    byte volume;              // voice volume (8 bits)
    byte volBalance;          // volume balance (L/R 4 bits for each channel)
};

struct TagWgpSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    int pos;                    // current address on ARAM (pointer for voice stream)
    byte tempo;                 // song tempo
    byte deltaTime;             // delta time
    byte deltaScale;            // delta time scale
    int subRetnAddr;            // return address (subroutine)
    int rptCount;               // repeat count
    int rptCountAlt;            // repeat count (alternate)
    WgpSpcVerInfo ver;          // game version info
    WgpSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void wgpSpcSetEventList (WgpSpcSeqStat *seq);

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
FILE *wgpSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int wgpSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = wgpSpcLoopMax;
    wgpSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool wgpSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        wgpSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        wgpSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            wgpSpcPatchFix[patch].bankSelM = patch >> 7;
            wgpSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            wgpSpcPatchFix[patch].bankSelM = 0;
            wgpSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        wgpSpcPatchFix[patch].patchNo = patch & 0x7f;
        wgpSpcPatchFix[patch].key = 0;
        wgpSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        wgpSpcPatchFix[src].bankSelM = bankM & 0x7f;
        wgpSpcPatchFix[src].bankSelL = bankL & 0x7f;
        wgpSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        wgpSpcPatchFix[src].key = key;
        wgpSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    wgpSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *wgpSpcVerToStrHtml (WgpSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_WGP:
        return "Wagan Paradise";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void wgpSpcResetTrackParam (WgpSpcSeqStat *seq, int track)
{
    WgpSpcTrackStat *tr = &seq->track[track];

    //tr->active = false;
    tr->note.vel = 127;
    tr->note.patch = 0; // just in case
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->volume = 0x88;
    tr->volBalance = 0x88;
}

/** reset before play/convert song. */
static void wgpSpcResetParam (WgpSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->transpose = 0;
    seq->looped = 0;
    seq->deltaTime = 1;
    seq->deltaScale = 1;
    seq->subRetnAddr = 0;
    seq->rptCount = 0;
    seq->rptCountAlt = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        WgpSpcTrackStat *tr = &seq->track[track];
        wgpSpcResetTrackParam(seq, track);
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
    if (wgpSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &wgpSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int wgpSpcCheckVer (WgpSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;

    int pos1;

    seq->timebase = wgpSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;

    // (Wagan Paradise)
    // 05cc: 68 60     cmp   a,#$60
    // 05ce: b0 0a     bcs   $05da
    // 05d0: cd 18     mov   x,#$18
    // 05d2: d8 3c     mov   $3c,x
    // 05d4: cd dc     mov   x,#$dc
    // 05d6: d8 3d     mov   $3d,x             ; $dc18 - song list
    // 05d8: 2f 0a     bra   $05e4
    // 05da: cd 00     mov   x,#$00
    // 05dc: d8 3c     mov   $3c,x
    // 05de: cd f4     mov   x,#$f4
    // 05e0: d8 3d     mov   $3d,x             ; $f400 - song list (sfx?)
    // 05e2: 28 1f     and   a,#$1f
    // 05e4: 8d 03     mov   y,#$03
    // 05e6: cf        mul   ya
    // 05e7: fd        mov   y,a
    // 05e8: f7 3c     mov   a,($3c)+y         ; read slot index (priority?)
    // 05ea: 1c        asl   a
    if ((pos1 = indexOfHexPat(aRAM, (const byte *) "\x68.\xb0\x0a\xcd.\xd8.\xcd.\xd8.\x2f\x0a\xcd.\xd8.\xcd.\xd8.\x28\x1f\x8d\x03\xcf\xfd\xf7.\x1c", SPC_ARAM_SIZE, NULL)) != -1 &&
            aRAM[pos1 +  7] + 1 == aRAM[pos1 + 11] &&
            aRAM[pos1 +  7] == aRAM[pos1 + 17] &&
            aRAM[pos1 + 11] == aRAM[pos1 + 21])
    {
        seq->ver.seqListAddr = aRAM[pos1 + 5] | (aRAM[pos1 + 9] << 8);
        version = SPC_VER_WGP;
    }

    // overwrite song list
    if (wgpSpcForceSongListAddr >= 0)
    {
        seq->ver.seqListAddr = wgpSpcForceSongListAddr;
        version = SPC_VER_WGP;
    }

    // song search
    if (seq->ver.seqListAddr != -1)
    {
        int songSlotOffset = (wgpSpcForceSongSlotIndex != -1) ? (wgpSpcForceSongSlotIndex * 2) : aRAM[0xde];
        int songIndex = aRAM[0x49 + songSlotOffset] & 0x7f;
        int songHeaderAddr = seq->ver.seqListAddr + (songIndex * 3);
        if (songIndex < SPC_SONG_MAX &&
                songHeaderAddr + 3 <= SPC_ARAM_SIZE &&
                aRAM[songHeaderAddr] >= 0 && aRAM[songHeaderAddr] <= 3 &&
                aRAM[songHeaderAddr + 2] != 0x00)
        {
            seq->ver.songIndex = songIndex;
        }
        else
            seq->ver.songIndex = 1;

        //int songIndex;
        //int songAddrDist = SPC_ARAM_SIZE;
        //int songSlot = wgpSpcForceSongSlotIndex;
        //int spcVcmdPtr = mget2l(&aRAM[0x00 + songSlot * 2]);
        //
        //seq->ver.songIndex = -1;
        //do
        //{
        //    int songHeaderAddr;
        //    for (songIndex = 0; songIndex < SPC_SONG_MAX; songIndex++)
        //    {
        //        int seqStartAddr;
        //    
        //        songHeaderAddr = seq->ver.seqListAddr + (songIndex * 3);
        //        if (songHeaderAddr + 3 > SPC_ARAM_SIZE)
        //            break;
        //    
        //        // check the song slot
        //        if (aRAM[songHeaderAddr] != songSlot)
        //            continue;
        //    
        //        seqStartAddr = mget2l(&aRAM[songHeaderAddr + 1]);
        //        //if (seqStartAddr <= spcVcmdPtr)
        //        {
        //            if (songAddrDist > abs(spcVcmdPtr - seqStartAddr))
        //            {
        //                songAddrDist = abs(spcVcmdPtr - seqStartAddr);
        //                seq->ver.songIndex = songIndex;
        //            }
        //        }
        //    }
        //    if (songHeaderAddr + 3 > SPC_ARAM_SIZE)
        //        break;
        //} while (false);
        //
        //if (seq->ver.songIndex == -1)
        //    seq->ver.songIndex = 1;
    }

    // overwrite song index
    if (wgpSpcForceSongIndex >= 0)
    {
        seq->ver.songIndex = wgpSpcForceSongIndex;
    }

    if (seq->ver.seqListAddr != -1 && seq->ver.songIndex != -1)
    {
        seq->ver.seqHeaderAddr = seq->ver.seqListAddr + (seq->ver.songIndex * 3);
        if (seq->ver.seqHeaderAddr >= SPC_ARAM_SIZE)
            seq->ver.seqHeaderAddr = -1;
    }

    seq->ver.id = version;
    wgpSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool wgpSpcDetectSeq (WgpSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    bool result;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    if (seq->ver.seqHeaderAddr == -1)
        return false;

    seq->pos = mget2l(&aRAM[seq->ver.seqHeaderAddr + 1]);
    result = true;

    wgpSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static WgpSpcSeqStat *newWgpSpcSeq (const byte *aRAM)
{
    WgpSpcSeqStat *newSeq = (WgpSpcSeqStat *) calloc(1, sizeof(WgpSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        wgpSpcCheckVer(newSeq);
        if (!wgpSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delWgpSpcSeq (WgpSpcSeqStat **seq)
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
static void printHtmlInfoList (WgpSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", wgpSpcVerToStrHtml(seq));
    myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X (song %d)</li>", seq->ver.seqHeaderAddr, seq->ver.songIndex);
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (WgpSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output other seq detail for valid seq. */
static void printHtmlInfoOthers (WgpSpcSeqStat *seq)
{
}

/** output event dump. */
static void printHtmlEventDump (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int i;

    if (seq == NULL || ev == NULL)
        return;

    myprintf("            <tr class=\"%s\">", ev->classStr); // track%d is removed, because it is useless in this driver
    //myprintf("<td class=\"track\">%d</td>", ev->track + 1);
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
static void printEventTableHeader (WgpSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("        <h3>Sequence</h3>\n");
    myprintf("        <div class=\"section\">\n");
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n"); // "<th class=\"track\">#</th>"
}

/** output event table footer. */
static void printEventTableFooter (WgpSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

//----

/** convert SPC tempo into bpm. */
static double wgpSpcTempo (WgpSpcSeqStat *seq)
{
    return (double) 60000000 / (16750 * wgpSpcTimeBase);
}

/** convert SPC channel volume into MIDI one. */
static int wgpSpcMidiVolOf (int value)
{
    if (wgpSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC volume balance into MIDI one. */
static int wgpSpcMidiVolBalance (int value, int *pVolRate)
{
    byte volL = (value & 0xf0) >> 4;
    byte volR = (value & 0x0f);
    byte midiVol, midiPan;
    double vol, pan;

    if (volL == 0 && volR == 0)
    {
        if (pVolRate != NULL)
            *pVolRate = 0;
        return -1; // NaN
    }

    // linear volume/panpot
    vol = (double) (volL + volR) / (16 + 16);
    pan = (double) volR / (volL + volR);

    // make it GM2 compatible
    if (!wgpSpcVolIsLinear)
    {
        // GM2 vol => dB: pow(2) curve
        // GM2 pan => dB: sin/cos curve
        // SPC vol => linear

        double linearVol = vol;
        double linearPan = pan;
        double panPI2 = atan2(linearPan, 1.0 - linearPan);

        vol = sqrt(linearVol / (cos(panPI2) + sin(panPI2)));
        pan = panPI2 / M_PI_2;
    }

    // calculate the final value
    midiVol = (byte) (vol * 127 + 0.5);
    if (vol != 0)
    {
        midiPan = (byte) (pan * 126 + 0.5);
        if (midiPan != 0)
            midiPan++;
    }

    if (pVolRate != NULL)
        *pVolRate = midiVol;
    return midiPan;
}

/** create new smf object and link to spc seq. */
static Smf *wgpSpcCreateSmf (WgpSpcSeqStat *seq)
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

    switch (wgpSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, wgpSpcTempo(seq));

    // put initial info for each track
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        int midiVol, midiPan;

        //if (!seq->track[tr].active)
        //    continue;

        smfInsertControl(seq->smf, 0, tr, tr, SMF_CONTROL_VOLUME, wgpSpcMidiVolOf(seq->track[tr].volume));
        midiPan = wgpSpcMidiVolBalance(seq->track[tr].volBalance, &midiVol);
        if (midiVol != 0)
        {
            smfInsertControl(seq->smf, 0, tr, tr, SMF_CONTROL_PANPOT, midiPan);
        }
        smfInsertControl(seq->smf, 0, tr, tr, SMF_CONTROL_EXPRESSION, midiVol);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
        //smfInsertPitchBend(seq->smf, 0, tr, tr, 0);
        //if (wgpSpcPitchBendSens != 0) {
        //    smfInsertPitchBendSensitivity(smf, 0, tr, tr, seq->track[tr].pitchBendSensMax);
        //}

        sprintf(songTitle, "Track %d", tr + 1);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void wgpSpcTruncateNote (WgpSpcSeqStat *seq, int track)
{
    WgpSpcTrackStat *tr = &seq->track[track];

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
static void wgpSpcTruncateNoteAll (WgpSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        wgpSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool wgpSpcDequeueNote (WgpSpcSeqStat *seq, int track)
{
    WgpSpcTrackStat *tr = &seq->track[track];
    WgpSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        int dur;
        int key;
        int vel;

        dur = lastNote->dur;
        if (dur == 0)
            dur++;

        key = lastNote->key/* + lastNote->transpose
            + seq->ver.patchFix[tr->lastNote.patch].key
            + SPC_NOTE_KEYSHIFT*/;
        vel = lastNote->vel;
        if (vel == 0)
            vel++;

        result = smfInsertNote(seq->smf, lastNote->tick, track, track, key, vel, dur);
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void wgpSpcDequeueNoteAll (WgpSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        wgpSpcDequeueNote(seq, tr);
    }
}

/** increment loop count. */
static void wgpSpcAddLoopCount(WgpSpcSeqStat *seq, int count)
{
    seq->looped += count;
    if (seq->looped >= wgpSpcLoopMax) {
        seq->active = false;
    }
}

/** vcmds: unknown event (without status change). */
static void wgpSpcEventUnknownInline (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X\n", ev->code, ev->addr);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X\n", ev->code, ev->addr);
}

/** vcmds: unidentified event. */
static void wgpSpcEventUnidentified (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    wgpSpcEventUnknownInline(seq, ev);
    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void wgpSpcEventUnknown0 (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    wgpSpcEventUnknownInline(seq, ev);
    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void wgpSpcEventUnknown1 (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    wgpSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void wgpSpcEventUnknown2 (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    wgpSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void wgpSpcEventUnknown3 (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    int *p = &seq->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    wgpSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void wgpSpcEventUnknown4 (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    int *p = &seq->pos;

    ev->size += 4;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;
    arg4 = seq->aRAM[*p];
    (*p)++;

    wgpSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 00: set delta time. */
static void wgpSpcEventSetDeltaTime (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Delta Time, tick = %d", arg1);
    strcat(ev->classStr, " ev-deltatime");

    seq->deltaTime = arg1;
}

/** vcmd 01: set active voices. */
static void wgpSpcEventSetActiveVoices (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Active Voices, bits = $%02X", arg1);
    strcat(ev->classStr, " ev-activevoices");
}

/** vcmd 02: call subroutine. */
static void wgpSpcEventSubroutine (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    seq->subRetnAddr = *p;
    *p = arg1;

    sprintf(ev->note, "Call, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-call");
}

/** vcmd 03: end of subroutine. */
static void wgpSpcEventEndSubroutine (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->pos;

    if ((seq->subRetnAddr & 0xff00) != 0)
    {
        // return from subroutine
        sprintf(ev->note, "End Subroutine");
        strcat(ev->classStr, " ev-ret");
        *p = seq->subRetnAddr;
        seq->subRetnAddr = 0;
    }
    else
    {
        // end of track
        sprintf(ev->note, "End of Track");
        strcat(ev->classStr, " ev-end");
        seq->active = false;
    }
}

/** vcmd 04: set delta time multiplier. */
static void wgpSpcEventSetDeltaScale (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Delta Multiplier, scale = %d", arg1);
    strcat(ev->classStr, " ev-deltascale");

    seq->deltaScale = arg1;
}

/** vcmd 05: set master volume. */
static void wgpSpcEventMasterVolume (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-mastervol");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 06: repeat until. */
static void wgpSpcEventRepeatUntil (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int dest;
    int *p = &seq->pos;
    bool doJump;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;
    dest = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    seq->rptCount = (seq->rptCount + 1) & 0xff;
    doJump = (seq->rptCount != arg1);
    if (doJump)
    {
        *p = dest;
    }
    else
        seq->rptCount = 0;

    sprintf(ev->note, "Repeat Until, count = %d, dest = $%04X, jump = %s", arg1, dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-repeatuntil");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 07: repeat break. */
static void wgpSpcEventRepeatBreak (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int dest;
    int *p = &seq->pos;
    bool doJump;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;
    dest = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    seq->rptCount = (seq->rptCount + 1) & 0xff;
    doJump = (seq->rptCount == arg1);
    if (doJump)
    {
        *p = dest;
        seq->rptCount = 0;
    }

    sprintf(ev->note, "Repeat Break, count = %d, dest = $%04X, jump = %s", arg1, dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-repeatbreak");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 0f: repeat until (alternate). */
static void wgpSpcEventRepeatUntilAlt (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int dest;
    int *p = &seq->pos;
    bool doJump;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;
    dest = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    seq->rptCountAlt = (seq->rptCountAlt + 1) & 0xff;
    doJump = (seq->rptCountAlt != arg1);
    if (doJump)
    {
        *p = dest;
    }
    else
        seq->rptCountAlt = 0;

    sprintf(ev->note, "Repeat Until (Alternate), count = %d, dest = $%04X, jump = %s", arg1, dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-repeatuntilalt");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 10: repeat break (alternate). */
static void wgpSpcEventRepeatBreakAlt (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int dest;
    int *p = &seq->pos;
    bool doJump;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;
    dest = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    seq->rptCountAlt = (seq->rptCountAlt + 1) & 0xff;
    doJump = (seq->rptCountAlt == arg1);
    if (doJump)
    {
        *p = dest;
        seq->rptCountAlt = 0;
    }

    sprintf(ev->note, "Repeat Break (Alternate), count = %d, dest = $%04X, jump = %s", arg1, dest, doJump ? "true" : "false");
    strcat(ev->classStr, " ev-repeatbreakalt");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 08: jump. */
static void wgpSpcEventJump (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    // assumes backjump = loop
    // check repeat count for Csikos Post
    if (arg1 < *p && seq->rptCount == 0) {
        wgpSpcAddLoopCount(seq, 1);
    }
    *p = arg1;

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");
}

#define WGP_NOTE_REST   0x54
#define WGP_NOTE_PERC   0x80

/** vcmd 09: note (with wait). */
static void wgpSpcEventNote (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->pos;
    int vbits;
    int channel;
    int waitAmount;

    ev->size++;
    vbits = seq->aRAM[*p];
    (*p)++;

    strcpy(argDumpStr, "");
    for (channel = 0; channel < SPC_TRACK_MAX; channel++)
    {
        WgpSpcTrackStat *tr = &seq->track[channel];
        if ((vbits & (0x80 >> channel)) != 0)
        {
            char noteName[64];
            char s[64];
            int key;
            int midiKey;

            ev->size++;
            key = seq->aRAM[*p];
            (*p)++;

            if (argDumpStr[0] != '\0')
                strcat(argDumpStr, ", ");

            if (key == WGP_NOTE_REST) {
                midiKey = 0;
                strcpy(noteName, "Rest");
            } else if (key >= WGP_NOTE_PERC) {
                midiKey = key - WGP_NOTE_PERC;
                sprintf(noteName, "P%d", key - WGP_NOTE_PERC);
            } else if (key > WGP_NOTE_REST) {
                midiKey = 96 + (key & 0x1f);
                sprintf(noteName, "N%d", key & 0x1f);
            } else {
                midiKey = key + seq->transpose + tr->note.transpose
                    + seq->ver.patchFix[tr->note.patch].key
                    + SPC_NOTE_KEYSHIFT;
                getNoteName(noteName, midiKey);
            }

            sprintf(s, "[%d] = %s", channel, noteName);
            strcat(argDumpStr, s);

            // output old note first
            wgpSpcDequeueNote(seq, channel);

            // set new note
            if (key != WGP_NOTE_REST) {
                tr->lastNote.tick = ev->tick;
                tr->lastNote.dur = 0; // set later
                tr->lastNote.key = midiKey;
                tr->lastNote.vel = 127;
                tr->lastNote.transpose = seq->transpose + tr->note.transpose;
                tr->lastNote.patch = tr->note.patch;
                tr->lastNote.tied = false;
                tr->lastNote.active = true;
            }
        }
    }

    waitAmount = (seq->deltaTime * seq->deltaScale);

    sprintf(ev->note, "Note, tick = %d, %s", waitAmount, argDumpStr);
    strcat(ev->classStr, " ev-note");

    for (channel = 0; channel < SPC_TRACK_MAX; channel++)
    {
        WgpSpcTrackStat *tr = &seq->track[channel];
        if (tr->lastNote.active)
        {
            tr->lastNote.dur += waitAmount;
        }
    }
    seq->tick += waitAmount;
    seq->time += (double) 60 / wgpSpcTempo(seq) * waitAmount / seq->timebase;
}

/** vcmd 0a: set echo delay. */
static void wgpSpcEventEchoDelay (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Delay, delay = %d", arg1);
    strcat(ev->classStr, " ev-echodelay");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 0b: unknown. */
static void wgpSpcEvent0B (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->pos;
    int vbits;
    int channel;

    ev->size++;
    vbits = seq->aRAM[*p];
    (*p)++;

    strcpy(argDumpStr, "");
    for (channel = 0; channel < SPC_TRACK_MAX; channel++)
    {
        WgpSpcTrackStat *tr = &seq->track[channel];
        if ((vbits & (0x80 >> channel)) != 0)
        {
            char midiText[256];
            char s[64];
            int val;

            ev->size++;
            val = seq->aRAM[*p];
            (*p)++;

            if (argDumpStr[0] != '\0')
                strcat(argDumpStr, ", ");
            sprintf(s, "[%d] = %d", channel, val);
            strcat(argDumpStr, s);

            sprintf(midiText, "Unknown Event %02X, value = %d (0x%02x)", ev->code, val, val);
            if (!wgpSpcLessTextInSMF)
                smfInsertMetaText(seq->smf, ev->tick, channel, SMF_META_TEXT, midiText);
        }
    }

    strcpy(ev->note, "Unknown");
    strcat(ev->note, ", ");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " unknown");
}

/** vcmd 0d: set echo write on/off. */
static void wgpSpcEventEchoWrite (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Write, sw = %s", arg1 ? "on" : "off");
    strcat(ev->classStr, " ev-echowrite");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 0e: wait. */
static void wgpSpcEventWait (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int waitAmount = (seq->deltaTime * seq->deltaScale);
    int channel;

    sprintf(ev->note, "Wait, tick = %d", waitAmount);
    strcat(ev->classStr, " ev-wait");

    for (channel = 0; channel < SPC_TRACK_MAX; channel++)
    {
        WgpSpcTrackStat *tr = &seq->track[channel];
        if (tr->lastNote.active)
        {
            tr->lastNote.dur += waitAmount;
        }
    }
    seq->tick += waitAmount;
    seq->time += (double) 60 / wgpSpcTempo(seq) * waitAmount / seq->timebase;
}

/** vcmd 11: set echo feedback. */
static void wgpSpcEventEchoFeedback (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Feedback, feedback = %d", arg1);
    strcat(ev->classStr, " ev-echofeedback");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 12: set echo filter. */
static void wgpSpcEventEchoFIR (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo FIR, index = %d", arg1);
    strcat(ev->classStr, " ev-echoFIR");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 13: set echo volume. */
static void wgpSpcEventEchoVolume (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->pos;

    ev->size += 2;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;
    arg2 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Echo Volume, L = %d, R = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echovol");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 14: set echo address. */
static void wgpSpcEventEchoAddress (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Address, ESA = $%04X", arg1 * 0x0100);
    strcat(ev->classStr, " ev-echoaddr");

    if (!wgpSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

#define EV_VOICE_PATCH      0x00
#define EV_VOICE_VOLUME     0x01
#define EV_VOICE_VOLBALANCE 0x02
#define EV_VOICE_ADSR       0x0a

/** vcmd 20-2a: set voice param. */
static void wgpSpcEventSetVoiceParam (WgpSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->pos;
    int code = ev->code & 0x0f;
    int vbits;
    int channel;
    const char* eventName[] = {
        "Instrument (SRCN)",
        "Volume",
        "Volume Balance (L/R 4 bits)",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "ADSR Pattern",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
    };
    const char* eventClassName[] = {
        "ev-patch",
        "ev-vol",
        "ev-volbalance",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "ev-adsr",
        "",
        "",
        "",
        "",
        "",
    };

    ev->size++;
    vbits = seq->aRAM[*p];
    (*p)++;

    strcpy(argDumpStr, "");
    for (channel = 0; channel < SPC_TRACK_MAX; channel++)
    {
        WgpSpcTrackStat *tr = &seq->track[channel];
        if ((vbits & (0x80 >> channel)) != 0)
        {
            char s[64];
            int val;

            ev->size++;
            val = seq->aRAM[*p];
            (*p)++;

            if (argDumpStr[0] != '\0')
                strcat(argDumpStr, ", ");
            sprintf(s, "[%d] = %d", channel, val);
            strcat(argDumpStr, s);

            switch(code)
            {
            case EV_VOICE_PATCH:
                tr->note.patch = val;
                smfInsertControl(seq->smf, ev->tick, channel, channel, SMF_CONTROL_BANKSELM, seq->ver.patchFix[val].bankSelM);
                smfInsertControl(seq->smf, ev->tick, channel, channel, SMF_CONTROL_BANKSELL, seq->ver.patchFix[val].bankSelL);
                smfInsertProgram(seq->smf, ev->tick, channel, channel, seq->ver.patchFix[val].patchNo);
                break;

            case EV_VOICE_VOLUME:
                tr->volume = val;
                smfInsertControl(seq->smf, ev->tick, channel, channel, SMF_CONTROL_VOLUME, wgpSpcMidiVolOf(tr->volume));
                break;

            case EV_VOICE_VOLBALANCE:
                {
                    int midiVol, midiPan;
                    tr->volBalance = val;
                    midiPan = wgpSpcMidiVolBalance(tr->volBalance, &midiVol);
                    if (midiVol != 0)
                    {
                        smfInsertControl(seq->smf, ev->tick, channel, channel, SMF_CONTROL_PANPOT, midiPan);
                    }
                    smfInsertControl(seq->smf, ev->tick, channel, channel, SMF_CONTROL_EXPRESSION, midiVol);
                }
                break;

            default:
                {
                    char midiText[256];

                    if (strcmp(eventName[code], "Unknown") == 0 || strcmp(eventName[code], "Undefined") == 0)
                    {
                        sprintf(midiText, "%s Event %02X, value = %d (0x%02x)", eventName[code], ev->code, val, val);
                    }
                    else
                    {
                        sprintf(midiText, "%s, value = %d", eventName[code], val);
                    }

                    if (!wgpSpcLessTextInSMF)
                        smfInsertMetaText(seq->smf, ev->tick, channel, SMF_META_TEXT, midiText);
                }
                break;
            }
        }
    }

    strcpy(ev->note, eventName[code]);
    strcat(ev->note, ", ");
    strcat(ev->note, argDumpStr);
    if (eventClassName[code][0] != '\0')
        strcat(ev->classStr, eventClassName[code]);
    strcat(ev->classStr, " ev-voiceparam");

    if (strcmp(eventName[code], "Unknown") == 0)
    {
        strcat(ev->classStr, " unknown");
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X\n", ev->code, ev->addr);
    }
    else if (strcmp(eventName[code], "Undefined") == 0)
    {
        ev->unidentified = true;
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X\n", ev->code, ev->addr);
    }
}

/** set pointers of each event. */
static void wgpSpcSetEventList (WgpSpcSeqStat *seq)
{
    int code;
    WgpSpcEvent *event = seq->ver.event;

    // disable them all first
    for (code = 0x00; code <= 0xff; code++) {
        event[code] = (WgpSpcEvent) wgpSpcEventUnidentified;
    }

    // events
    event[0x00] = (WgpSpcEvent) wgpSpcEventSetDeltaTime;
    event[0x01] = (WgpSpcEvent) wgpSpcEventSetActiveVoices;
    event[0x02] = (WgpSpcEvent) wgpSpcEventSubroutine;
    event[0x03] = (WgpSpcEvent) wgpSpcEventEndSubroutine;
    event[0x04] = (WgpSpcEvent) wgpSpcEventSetDeltaScale;
    event[0x05] = (WgpSpcEvent) wgpSpcEventMasterVolume;
    event[0x06] = (WgpSpcEvent) wgpSpcEventRepeatUntil;
    event[0x07] = (WgpSpcEvent) wgpSpcEventRepeatBreak;
    event[0x08] = (WgpSpcEvent) wgpSpcEventJump;
    event[0x09] = (WgpSpcEvent) wgpSpcEventNote;
    event[0x0a] = (WgpSpcEvent) wgpSpcEventEchoDelay;
    event[0x0b] = (WgpSpcEvent) wgpSpcEvent0B;
    event[0x0c] = (WgpSpcEvent) wgpSpcEventUnknown1;
    event[0x0d] = (WgpSpcEvent) wgpSpcEventEchoWrite;
    event[0x0e] = (WgpSpcEvent) wgpSpcEventWait;
    event[0x0f] = (WgpSpcEvent) wgpSpcEventRepeatUntilAlt;
    event[0x10] = (WgpSpcEvent) wgpSpcEventRepeatBreakAlt;
    event[0x11] = (WgpSpcEvent) wgpSpcEventEchoFeedback;
    event[0x12] = (WgpSpcEvent) wgpSpcEventEchoFIR;
    event[0x13] = (WgpSpcEvent) wgpSpcEventEchoVolume;
    event[0x14] = (WgpSpcEvent) wgpSpcEventEchoAddress;
    for (code = 0x18; code <= 0xff; code++) {
        event[code] = (WgpSpcEvent) wgpSpcEventSetVoiceParam;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* wgpSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    WgpSpcSeqStat *seq = NULL;
    Smf* smf = NULL;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newWgpSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = wgpSpcCreateSmf(seq);

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
        bool inSub;
        int oldLooped = seq->looped;

        // init event report
        ev.tick = seq->tick;
        ev.addr = seq->pos;
        ev.size = 0;
        ev.track = 0; // used for error report
        ev.unidentified = false;
        strcpy(ev.note, "");

        // read first byte
        ev.size++;
        ev.code = aRAM[ev.addr];
        sprintf(ev.classStr, "ev%02X", ev.code);
        seq->pos++;
        // in subroutine?
        inSub = false; // NYI
        strcat(ev.classStr, inSub ? " sub" : "");

        // dispatch event
        seq->ver.event[ev.code](seq, &ev);

        // dump event report
        if (wgpSpcTextLoopMax == 0 || oldLooped < wgpSpcTextLoopMax)
            printHtmlEventDump(seq, &ev);

        if (ev.unidentified) {
            abortFlag = true;
            goto quitConversion;
        }

        // end of seq, quit
        if (!seq->active) {
            // rewind tracks to end point (removed)
        }
        else {
            // check time limit
            if (seq->time >= wgpSpcTimeLimit) {
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    wgpSpcTruncateNoteAll(seq);
    wgpSpcDequeueNoteAll(seq);

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
        delWgpSpcSeq(&seq);
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
Smf* wgpSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = wgpSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* wgpSpcToMidiFromFile (const char *filename)
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

    smf = wgpSpcToMidi(data, size);

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
static bool cmdOptSong (void);
static bool cmdOptSongSlot (void);
static bool cmdOptSongList (void);
static bool cmdOptLinear (void);
static bool cmdOptBendRange (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm1", '\0', 0, cmdOptGM1, "", "Insert GM1 System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
    { "timebase", '\0', 0, cmdOptTimeBase, "", "Set SMF timebase (tick count for quarter note)" },
    { "song", '\0', 1, cmdOptSong, "<index>", "force set song index" },
    { "songslot", '\0', 1, cmdOptSongSlot, "<index>", "force set song slot index for auto song search (0~3)" },
    { "songlist", '\0', 1, cmdOptSongList, "<addr>", "force set song (list) address" },
    { "linear", '\0', 0, cmdOptLinear, "", "Use linear volume/panpot (GM2 uses exp/sin curve)" },
    { "bendrange", '\0', 0, cmdOptBendRange, "<range>", "set pitch bend sensitivity (0:auto)" },
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
    wgpSpcSetLoopCount(loopCount);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (wgpSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    wgpSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    wgpSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    wgpSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    wgpSpcMidiResetType = SMF_RESET_GM2;
    return true;
}

/** set SMF division. */
static bool cmdOptTimeBase (void)
{
    int timebase = strtol(gArgv[0], NULL, 0);
    wgpSpcTimeBase = timebase;
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    wgpSpcForceSongIndex = songIndex;
    return true;
}

/** set song slot index. */
static bool cmdOptSongSlot (void)
{
    int songSlotIndex = strtol(gArgv[0], NULL, 0);
    wgpSpcForceSongSlotIndex = songSlotIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    wgpSpcForceSongListAddr = songListAddr;
    return true;
}

/** set volume/panpot conversion mode. */
static bool cmdOptLinear (void)
{
    wgpSpcVolIsLinear = true;
    return true;
}

/** set pitchbend range. */
static bool cmdOptBendRange (void)
{
    int range = strtol(gArgv[0], NULL, 0);
    wgpSpcPitchBendSens = range;
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
        wgpSpcSetLogStreamHandle(htmlFile);

        fprintf(stderr, "%s:\n", spcPath);

        smf = wgpSpcToMidiFromFile(spcPath);
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
