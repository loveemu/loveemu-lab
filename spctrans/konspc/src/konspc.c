/**
 * Konami spc2midi.
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
#include "konspc.h"

#define APPNAME "Konami SPC2MIDI"
#define APPSHORTNAME "konspc"
#define VERSION "[2014-06-14]"

static int konamiSpcLoopMax = 2;            // maximum loop count of parser
static int konamiSpcTextLoopMax = 1;        // maximum loop count of text output
static double konamiSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool konamiSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static bool konamiSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

static int konamiSpcTimeBase = 48;
static int konamiSpcForceSongIndex = -1;
static int konamiSpcForceSongListAddr = -1;

static bool konamiSpcPatchFixOverride = false;
static PatchFixInfo konamiSpcPatchFix[256];

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int konamiSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_PNTB,
    SPC_VER_GG4,
};

const int GG4_EVENT_LENGTH_TABLE[] = {
    0x0001, 0x0002, 0x0001, 0x0001, 0x0003, 0x0003, 0x0000, 0x0003,
    0x0000, 0x0003, 0x0001, 0x0002, 0x0001, 0x0001, 0x0001, 0x0002,
    0x0001, 0x0003, 0x0001, 0x0003, 0x0003, 0x0003, 0x0000, 0x0000,
    0x0002, 0x0001, 0x0003, 0x0001, 0x0001, 0x0002, 0x0002, 0x0000,
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24
#define SPC_ARAM_SIZE       0x10000

typedef struct TagKonamiSpcTrackStat KonamiSpcTrackStat;
typedef struct TagKonamiSpcSeqStat KonamiSpcSeqStat;
typedef void (*KonamiSpcEvent) (KonamiSpcSeqStat *, SeqEventReport *);

typedef struct TagKonamiSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int seqHeaderAddr;
    int vcmdLenTableAddr;
    KonamiSpcEvent event[256];
    PatchFixInfo patchFix[256];
    bool seqDetected;
    bool needsRelocateEvent;
} KonamiSpcVerInfo;

typedef struct TagKonamiSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // total length (tick)
    int vel;            // note volume
    bool tied;          // if the note tied/slur
    int key;            // key
    int transpose;      // transpose
    int patch;          // instrument
} KonamiSpcNoteParam;

struct TagKonamiSpcTrackStat {
    bool active;            // if the channel is still active
    bool used;              // if the channel used once or not
    int pos;                // current address on ARAM
    int tick;               // timing (must be synchronized with seq)
    int prevTick;           // previous timing (for pitch slide)
    KonamiSpcNoteParam note;     // current note param
    KonamiSpcNoteParam lastNote; // note params for last note
    int lastNoteLen;        // last note length ($0230+x)
    int looped;             // how many times looped (internal)
    int patch;              // patch number (for pitch fix)
    int repCount1;          // loop: looped count #1 ($50+x)
    int repCount2;          // loop: looped count #2 ($51+x)
    int repRetnAddr1;       // loop: return address #1 ($0150+x)
    int repRetnAddr2;       // loop: return address #2 ($0160+x)
    int repRetnAddr3a;      // loop: return address #3-a ($0170+x)
    int repRetnAddr3b;      // loop: return address #3-b ($0180+x)
    bool repCalled3a;       // loop: visited flag #1 ($0020 bit 6)
    bool repCalled3b;       // loop: visited flag #2 ($0020 bit 7)
    bool subCalled;         // sub: during subroutine call
    int subRetnAddr;        // sub: return address
};

struct TagKonamiSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int timebase;               // SMF division
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    KonamiSpcVerInfo ver;       // game version info
    KonamiSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void konamiSpcSetEventList (KonamiSpcSeqStat *seq);

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
FILE *konamiSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int konamiSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = konamiSpcLoopMax;
    konamiSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool konamiSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        konamiSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        konamiSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            konamiSpcPatchFix[patch].bankSelM = patch >> 7;
            konamiSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            konamiSpcPatchFix[patch].bankSelM = 0;
            konamiSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        konamiSpcPatchFix[patch].patchNo = patch & 0x7f;
        konamiSpcPatchFix[patch].key = 0;
        konamiSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        konamiSpcPatchFix[src].bankSelM = bankM & 0x7f;
        konamiSpcPatchFix[src].bankSelL = bankL & 0x7f;
        konamiSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        konamiSpcPatchFix[src].key = key;
        konamiSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    konamiSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *konamiSpcVerToStrHtml (KonamiSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_PNTB:
        return "Pop 'N' Twinbee";
    case SPC_VER_GG4:
        return "goemon 4";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void konamiSpcResetTrackParam (KonamiSpcSeqStat *seq, int track)
{
    KonamiSpcTrackStat *tr = &seq->track[track];

    tr->used = false;
    tr->prevTick = tr->tick;
    tr->looped = 0;
    tr->note.transpose = 0;
    tr->lastNote.active = false;
    tr->lastNoteLen = 0;

    tr->repCount1 = 0;
    tr->repCount2 = 0;
    tr->repCalled3a = false;
    tr->repCalled3b = false;
    tr->subCalled = false;
}

/** reset before play/convert song. */
static void konamiSpcResetParam (KonamiSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x40;
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = 0; track < SPC_TRACK_MAX; track++) {
        KonamiSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        konamiSpcResetTrackParam(seq, track);
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
    if (konamiSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &konamiSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }

}

/** returns what version the sequence is, and sets individual info. */
static int konamiSpcCheckVer (KonamiSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;
    int songLdCodeAddr;
    int songLdCodeAddrOld;
    int vcmdCallCodeAddr;
    int evtIndex;

    seq->timebase = konamiSpcTimeBase;
    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.vcmdLenTableAddr = -1;
    seq->ver.seqDetected = false;

    // mov   $06,#$..
    // mov   $0a,#$..
    // mov   $0b,#$..
    // mov   x,#$00
    songLdCodeAddr = indexOfHexPat(seq->aRAM, "\x8f.\x06\x8f.\x0a\x8f.\x0b\xcd\\\x00", SPC_ARAM_SIZE, NULL);
    if (songLdCodeAddr == -1)
    {
        // 0abd: c4 0c     mov   $0c,a
        // 0abf: 8f 1b 04  mov   $04,#$1b
        // 0ac2: 8f 05 05  mov   $05,#$05
        // 0ac5: 8d 05     mov   y,#$05
        // 0ac7: cf        mul   ya
        // 0ac8: 7a 04     addw  ya,$04            ; $04/5 = 0x051b + (y * 5)
        // 0aca: da 04     movw  $04,ya
        songLdCodeAddrOld = indexOfHexPat(seq->aRAM, "\xc4\x0c\x8f.\x04\x8f.\x05\x8d\x05\xcf\x7a\x04\xda\x04", SPC_ARAM_SIZE, NULL);
    }
    if (songLdCodeAddr != -1 || songLdCodeAddrOld != -1)
    {
        // asl   a
        // mov   y,a
        // mov   a,$....+y
        // push  a
        // mov   a,$....+y
        // push  a
        // mov   a,$....+y
        // beq   $....
        vcmdCallCodeAddr = indexOfHexPat(seq->aRAM, "\x1c\\\xfd\\\xf6..\x2d\\\xf6..\x2d\\\xf6..\\\xf0.", SPC_ARAM_SIZE, NULL);
        if (vcmdCallCodeAddr != -1)
        {
            int vcmdLenTableAddr = mget2l(&seq->aRAM[vcmdCallCodeAddr + 11]);

            if (songLdCodeAddr != -1)
            {
                seq->ver.seqHeaderAddr = seq->aRAM[songLdCodeAddr + 4] | (seq->aRAM[songLdCodeAddr + 7] << 8); // $0a/b
                version = SPC_VER_GG4;
            }
            else
            {
                int songIndexReg;
                int addrSongEntry;

                seq->ver.seqListAddr = seq->aRAM[songLdCodeAddrOld + 3] | (seq->aRAM[songLdCodeAddrOld + 6] << 8); // $04/5
                songIndexReg = seq->aRAM[songLdCodeAddrOld + 1];

                if (konamiSpcForceSongListAddr != -1)
                {
                    seq->ver.seqListAddr = konamiSpcForceSongListAddr;
                }

                seq->ver.songIndex = seq->aRAM[songIndexReg];
                addrSongEntry = seq->ver.seqListAddr + (seq->ver.songIndex * 5);

                if (konamiSpcForceSongIndex != -1)
                {
                    seq->ver.songIndex = konamiSpcForceSongIndex;
                }

                if (addrSongEntry + 5 <= 0x10000)
                {
                    seq->ver.seqHeaderAddr = seq->aRAM[addrSongEntry + 3] | (seq->aRAM[addrSongEntry + 4] << 8); // $0a/b
                    version = SPC_VER_PNTB;
                }
            }
            seq->ver.vcmdLenTableAddr = vcmdLenTableAddr;

            // verify length table
            for (evtIndex = 0; evtIndex < countof(GG4_EVENT_LENGTH_TABLE); evtIndex++)
            {
                if (seq->aRAM[evtIndex * 2 + vcmdLenTableAddr] != GG4_EVENT_LENGTH_TABLE[evtIndex])
                {
                    seq->ver.needsRelocateEvent = true;
                }
            }
        }
    }

    seq->ver.id = version;
    konamiSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool konamiSpcDetectSeq (KonamiSpcSeqStat *seq)
{
    bool result = true;
    int trackMax = SPC_TRACK_MAX;
    int tr;

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return false;

    konamiSpcResetParam(seq);
    // track list
    for (tr = 0; tr < trackMax; tr++) {
        int trackAddr = mget2l(&seq->aRAM[tr * 2 + seq->ver.seqHeaderAddr]);
        seq->track[tr].pos = trackAddr;

        // TODO: HACK to know proper track count
        if (trackAddr < (SPC_TRACK_MAX * 2 + seq->ver.seqHeaderAddr) &&
            trackAddr >= seq->ver.seqHeaderAddr)
        {
            trackMax = (trackAddr - seq->ver.seqHeaderAddr) / 2;
        }

        seq->track[tr].active = true;
        result = true;
    }

    return result;
}

/** create new spc2mid object. */
static KonamiSpcSeqStat *newKonamiSpcSeq (const byte *aRAM)
{
    KonamiSpcSeqStat *newSeq = (KonamiSpcSeqStat *) calloc(1, sizeof(KonamiSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        konamiSpcCheckVer(newSeq);
        newSeq->ver.seqDetected = konamiSpcDetectSeq(newSeq);
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delKonamiSpcSeq (KonamiSpcSeqStat **seq)
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
static void printHtmlInfoList (KonamiSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s%s</li>\n", konamiSpcVerToStrHtml(seq), seq->ver.needsRelocateEvent ? " (modified)" : "");

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;

    if (seq->ver.seqListAddr != -1)
    {
        myprintf("          <li>Song List: $%04X</li>\n", seq->ver.seqListAddr);
    }
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    myprintf("          <li>Voice Cmd Length Table: $%04X", seq->ver.vcmdLenTableAddr);
    if (seq->ver.songIndex != -1) 
    {
        myprintf(" (Song $%02x)", seq->ver.songIndex);
    }
    myprintf("</li>\n");
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (KonamiSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (KonamiSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (KonamiSpcSeqStat *seq, Smf* smf)
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
static void printEventTableFooter (KonamiSpcSeqStat *seq, Smf* smf)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

/** convert SPC tempo into bpm. */
static double konamiSpcTempo (KonamiSpcSeqStat *seq)
{
    return (double) seq->tempo * 60000000 / 98304000; // 49152000 = (timer0) 4ms * 48 TPQN * 256
}

/** convert SPC velocity into MIDI one. */
static int konamiSpcMidiVelOf (int value)
{
    if (konamiSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int konamiSpcMidiVolOf (int value)
{
    if (konamiSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** create new smf object and link to spc seq. */
static Smf *konamiSpcCreateSmf (KonamiSpcSeqStat *seq)
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

    switch (konamiSpcMidiResetType) {
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
    smfInsertTempoBPM(smf, 0, 0, konamiSpcTempo(seq));

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

        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_VOLUME, konamiSpcMidiVolOf(seq->track[tr].volume));
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RELEASETIME, 64 + 6);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void konamiSpcTruncateNote (KonamiSpcSeqStat *seq, int track)
{
    KonamiSpcTrackStat *tr = &seq->track[track];

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
static void konamiSpcTruncateNoteAll (KonamiSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        konamiSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool konamiSpcDequeueNote (KonamiSpcSeqStat *seq, int track)
{
    KonamiSpcTrackStat *tr = &seq->track[track];
    KonamiSpcNoteParam *lastNote = &tr->lastNote;
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
static void konamiSpcDequeueNoteAll (KonamiSpcSeqStat *seq)
{
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        konamiSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void konamiSpcInactiveTrack(KonamiSpcSeqStat *seq, int track)
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
static void konamiSpcAddTrackLoopCount(KonamiSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (konamiSpcLoopMax > 0) ? konamiSpcLoopMax : 0xffff;
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= konamiSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void konamiSpcSeqAdvTick(KonamiSpcSeqStat *seq)
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
    seq->time += (double) 60 / konamiSpcTempo(seq) * minTickStep / seq->timebase;
}

/** vcmds: unknown event (without status change). */
static void konamiSpcEventUnknownInline (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X at $%04X [Track %d]\n", ev->code, *p, ev->track + 1);
}

/** vcmds: unidentified event. */
static void konamiSpcEventUnidentified (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    konamiSpcEventUnknownInline(seq, ev);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void konamiSpcEventUnknown0 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    konamiSpcEventUnknownInline(seq, ev);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void konamiSpcEventUnknown1 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void konamiSpcEventUnknown2 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void konamiSpcEventUnknown3 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (4 byte args). */
static void konamiSpcEventUnknown4 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
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

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d", arg1, arg2, arg3, arg4);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (5 byte args). */
static void konamiSpcEventUnknown5 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3, arg4, arg5;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
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

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d, arg4 = %d, arg5 = %d", arg1, arg2, arg3, arg4, arg5);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: no operation. */
static void konamiSpcEventNOP (KonamiSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** vcmd 00-5f, 80-df: note. */
static void konamiSpcEventNote (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;
    byte noteByte = ev->code;
    int note = noteByte & 0x7f;
    int len, arg2, arg3, vel;

    if ((noteByte & 0x80) == 0)
    {
        ev->size++;
        len = seq->aRAM[*p];
        tr->lastNoteLen = len;
        (*p)++;
    }
    else
    {
        len = tr->lastNoteLen;
    }

    ev->size++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    if ((arg2 & 0x80) == 0)
    {
       ev->size++;
       arg3 = seq->aRAM[*p];
       vel = arg3 & 0x7f;
       (*p)++;
    }
    else
    {
        vel = arg2 & 0x7f;
    }

    getNoteName(ev->note, note + seq->transpose + tr->note.transpose
        + seq->ver.patchFix[tr->note.patch].key
        + SPC_NOTE_KEYSHIFT);
    sprintf(argDumpStr, ", len = %d", len);
    strcat(ev->note, argDumpStr);
    if ((arg2 & 0x80) == 0)
    {
        sprintf(argDumpStr, ", arg2 = %d", arg2);
        strcat(ev->note, argDumpStr);
    }
    sprintf(argDumpStr, ", vel = %d", vel);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-note");

    //if (!konamiSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    // output old note first
    konamiSpcDequeueNote(seq, ev->track);

    // set new note
    tr->lastNote.tick = ev->tick;
    tr->lastNote.dur = len;
    tr->lastNote.key = note;
    tr->lastNote.vel = vel;
    tr->lastNote.transpose = seq->transpose + tr->note.transpose;
    tr->lastNote.patch = tr->note.patch;
    tr->lastNote.tied = false;
    tr->lastNote.active = true;

    tr->tick += len;
}

/** vcmd e0: unknown event (1 byte arg). */
static void konamiSpcEventE0 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->lastNoteLen = arg1;
    tr->tick += arg1;
}

/** vcmd e1: unknown event (2 byte args). */
static void konamiSpcEventE1 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    konamiSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    tr->lastNoteLen = arg1;
    tr->tick += arg1;
}

/** vcmd e2: set instrument. */
static void konamiSpcEventInstrument (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
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

/** vcmd ee: set volume. */
static void konamiSpcEventVolume (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, vol = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!konamiSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, konamiSpcMidiVolOf(arg1));
}

/** vcmd fc: set volume and instrument. */
static void konamiSpcEventVolumeAndInstrument (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    tr->note.patch = arg2;

    sprintf(ev->note, "Volume And Instrument, vol = %d, patch = %d", arg1, arg2);
    strcat(ev->classStr, " ev-vol ev-patch");

    //if (!konamiSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, konamiSpcMidiVolOf(arg1));

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[arg1].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[arg1].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[arg1].patchNo);
}

/** vcmd f5: set echo delay, feedback, FIR. */
static void konamiSpcEventEchoParam (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, delay = %d, feedback = %d, arg3 = %d (ignored)", arg1, arg2, arg3);
    strcat(ev->classStr, " ev-echoparam");

    if (!konamiSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd e6: start loop. */
static void konamiSpcEventLoopStart (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    // save return address
    tr->repRetnAddr1 = *p;

    sprintf(ev->note, "Loop Start");
    strcat(ev->classStr, " ev-loopstart");
}

/** vcmd e7: end loop. */
static void konamiSpcEventLoopEnd (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    if (arg1 == 0 || ++tr->repCount1 != arg1)
    {
        // repeat again
        *p = tr->repRetnAddr1;
    }
    else
    {
        // repeat end
        tr->repCount1 = 0;
    }

    if (arg1 == 0)
    {
        konamiSpcAddTrackLoopCount(seq, ev->track, 1);
    }

    sprintf(ev->note, "Loop End, count = %d%s, arg2 = %d, arg3 = %d", arg1, (arg1 == 0) ? " (Infinite)" : "", arg2, arg3);
    strcat(ev->classStr, " ev-loopend");
}

/** vcmd e8: start loop #2. */
static void konamiSpcEventLoopStart2 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    // save return address
    tr->repRetnAddr2 = *p;

    sprintf(ev->note, "Loop Start #2");
    strcat(ev->classStr, " ev-loopstart");
}

/** vcmd e9: end loop #2. */
static void konamiSpcEventLoopEnd2 (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2, arg3;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;
    arg3 = seq->aRAM[*p];
    (*p)++;

    if (arg1 == 0 || ++tr->repCount2 != arg1)
    {
        // repeat again
        *p = tr->repRetnAddr2;
    }
    else
    {
        // repeat end
        tr->repCount2 = 0;
    }

    if (arg1 == 0)
    {
        konamiSpcAddTrackLoopCount(seq, ev->track, 1);
    }

    sprintf(ev->note, "Loop End #2, count = %d%s, arg2 = %d, arg3 = %d", arg1, (arg1 == 0) ? " (Infinite)" : "", arg2, arg3);
    strcat(ev->classStr, " ev-loopend");
}

/** vcmd f6: start complexed loop. */
static void konamiSpcEventLoopExStart (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    // save return address
    tr->repRetnAddr3a = *p;
    tr->repCalled3a = false;
    tr->repCalled3b = false;

    sprintf(ev->note, "Loop Start Ex");
    strcat(ev->classStr, " ev-loopstartex");
}

/** vcmd f7: end complexed loop. */
static void konamiSpcEventLoopExEnd (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    if (tr->repCalled3a)
    {
        // second time
        tr->repCalled3a = false;
        tr->repCalled3b = true;
        // save return address
        tr->repRetnAddr3b = *p;
        // jump
        *p = tr->repRetnAddr3a;

        sprintf(ev->note, "Loop End Ex (Stage 2), dest = $%04X", *p);
    }
    else if (tr->repCalled3b)
    {
        // third time
        tr->repCalled3a = true;
        tr->repCalled3b = false;
        // jump
        *p = tr->repRetnAddr3b;

        sprintf(ev->note, "Loop End Ex (Stage 3), dest = $%04X", *p);
    }
    else
    {
        // first time
        tr->repCalled3a = true;
        sprintf(ev->note, "Loop End Ex (Stage 1)");
    }
    strcat(ev->classStr, " ev-loopendex");
}

/** vcmd ea: set tempo. */
static void konamiSpcEventSetTempo (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Set Tempo, tempo = %d (bpm %.1f)", arg1, konamiSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");

    if (ev->track == 0)
    {
        seq->tempo = arg1;
        smfInsertTempoBPM(seq->smf, ev->tick, 0, konamiSpcTempo(seq));
    }
    else
    {
        if (!konamiSpcLessTextInSMF)
            smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    }
}

/** vcmd ec: transpose (absolute). */
static void konamiSpcEventTransposeAbs (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    tr->note.transpose = arg1;

    sprintf(ev->note, "Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");

    //if (!konamiSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd fd: jump. */
static void konamiSpcEventJump (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &tr->pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    // assumes backjump = loop
    if (arg1 < *p) {
        konamiSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    *p = arg1;

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");
}

/** vcmd fe: call subroutine. */
static void konamiSpcEventSubroutine (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2l(&seq->aRAM[*p]);
    (*p) += 2;

    sprintf(ev->note, "Call Subroutine, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-call");

    tr->subCalled = true;
    tr->subRetnAddr = *p;
    *p = arg1;
}

/** vcmd ff: end subroutine / end of track. */
static void konamiSpcEventEndSubroutine (KonamiSpcSeqStat *seq, SeqEventReport *ev)
{
    KonamiSpcTrackStat *tr = &seq->track[ev->track];
    int *p = &seq->track[ev->track].pos;

    if (!tr->subCalled) {
        sprintf(ev->note, "End of Track");
        strcat(ev->classStr, " ev-end");

        konamiSpcInactiveTrack(seq, ev->track);
    }
    else {
        sprintf(ev->note, "End Subroutine");
        strcat(ev->classStr, " ev-ret");

        *p = tr->subRetnAddr;
        tr->subCalled = false;
    }

    //if (!konamiSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** set pointers of each event. */
static void konamiSpcSetEventList (KonamiSpcSeqStat *seq)
{
    int code;
    KonamiSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (KonamiSpcEvent) konamiSpcEventUnidentified;
    }

    for (code = 0x00; code <= 0x5f; code++) {
        event[code] = (KonamiSpcEvent) konamiSpcEventNote;
        event[code | 0x80] = (KonamiSpcEvent) konamiSpcEventNote;
    }
    event[0x60] = (KonamiSpcEvent) konamiSpcEventUnknown0;
    event[0x61] = (KonamiSpcEvent) konamiSpcEventUnknown0;
    event[0x62] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    for (code = 0x63; code <= 0x7f; code++) {
        event[code] = (KonamiSpcEvent) konamiSpcEventUnknown0;
    }
    event[0xe0] = (KonamiSpcEvent) konamiSpcEventE0;
    event[0xe1] = (KonamiSpcEvent) konamiSpcEventE1;
    event[0xe2] = (KonamiSpcEvent) konamiSpcEventInstrument;
    event[0xe3] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    event[0xe4] = (KonamiSpcEvent) konamiSpcEventUnknown3;
    event[0xe5] = (KonamiSpcEvent) konamiSpcEventUnknown3;
    event[0xe6] = (KonamiSpcEvent) konamiSpcEventLoopStart;
    event[0xe7] = (KonamiSpcEvent) konamiSpcEventLoopEnd;
    event[0xe8] = (KonamiSpcEvent) konamiSpcEventLoopStart2;
    event[0xe9] = (KonamiSpcEvent) konamiSpcEventLoopEnd2;
    event[0xea] = (KonamiSpcEvent) konamiSpcEventSetTempo;
    event[0xeb] = (KonamiSpcEvent) konamiSpcEventUnknown2;
    event[0xec] = (KonamiSpcEvent) konamiSpcEventTransposeAbs;
    event[0xed] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    event[0xee] = (KonamiSpcEvent) konamiSpcEventVolume;
    event[0xef] = (KonamiSpcEvent) konamiSpcEventUnknown2;
    event[0xf0] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    event[0xf1] = (KonamiSpcEvent) konamiSpcEventUnknown5;
    event[0xf2] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    event[0xf3] = (KonamiSpcEvent) konamiSpcEventUnknown5;
    event[0xf4] = (KonamiSpcEvent) konamiSpcEventUnknown3;
    event[0xf5] = (KonamiSpcEvent) konamiSpcEventEchoParam;
    event[0xf6] = (KonamiSpcEvent) konamiSpcEventLoopExStart;
    event[0xf7] = (KonamiSpcEvent) konamiSpcEventLoopExEnd;
    event[0xf8] = (KonamiSpcEvent) konamiSpcEventUnknown2;
    event[0xf9] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    event[0xfa] = (KonamiSpcEvent) konamiSpcEventUnknown3;
    event[0xfb] = (KonamiSpcEvent) konamiSpcEventUnknown1;
    event[0xfc] = (KonamiSpcEvent) konamiSpcEventVolumeAndInstrument;
    event[0xfd] = (KonamiSpcEvent) konamiSpcEventJump;
    event[0xfe] = (KonamiSpcEvent) konamiSpcEventSubroutine;
    event[0xff] = (KonamiSpcEvent) konamiSpcEventEndSubroutine;

    if (seq->ver.needsRelocateEvent)
    {
        int vcmdLenTableAddr = seq->ver.vcmdLenTableAddr;
        int evtIndex;

        for (evtIndex = 0; evtIndex < countof(GG4_EVENT_LENGTH_TABLE); evtIndex++)
        {
            int evtActualLength = seq->aRAM[evtIndex * 2 + vcmdLenTableAddr];
            int evtCode = 0xe0 + evtIndex;
            if (evtActualLength != GG4_EVENT_LENGTH_TABLE[evtIndex])
            {
                fprintf(stderr, "Warning: Event Length Mismatch! Replaced Event %02X (%d -> %d bytes)\n", evtCode, GG4_EVENT_LENGTH_TABLE[evtIndex], evtActualLength);
                switch(evtActualLength)
                {
                case 0:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnknown0;
                    break;
                case 1:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnknown1;
                    break;
                case 2:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnknown2;
                    break;
                case 3:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnknown3;
                    break;
                case 4:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnknown4;
                    break;
                case 5:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnknown5;
                    break;
                default:
                    event[evtCode] = (KonamiSpcEvent) konamiSpcEventUnidentified;
                    break;
                }
            }
        }
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* konamiSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    KonamiSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newKonamiSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN || !seq->ver.seqDetected) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = konamiSpcCreateSmf(seq);

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

            KonamiSpcTrackStat *evtr = &seq->track[ev.track];

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
                inSub = evtr->subCalled;
                strcat(ev.classStr, inSub ? " sub" : "");

                //if (ev.code != seq->ver.pitchSlideByte)
                //    evtr->prevTick = evtr->tick;
                evtr->used = true;
                // dispatch event
                seq->ver.event[ev.code](seq, &ev);

                // dump event report
                if (konamiSpcTextLoopMax == 0 || seq->looped < konamiSpcTextLoopMax)
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
            konamiSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= konamiSpcTimeLimit) {
            	fprintf(stderr, "TIMEOUT %f %f\n", seq->time, konamiSpcTimeLimit);
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    konamiSpcTruncateNoteAll(seq);
    konamiSpcDequeueNoteAll(seq);

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
        delKonamiSpcSeq(&seq);
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
Smf* konamiSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = konamiSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* konamiSpcToMidiFromFile (const char *filename)
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

    smf = konamiSpcToMidi(data, size);

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
    konamiSpcSetLoopCount(loopCount);
    return true;
}

/** set song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    konamiSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    konamiSpcForceSongListAddr = songListAddr;
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (konamiSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    konamiSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    konamiSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    konamiSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    konamiSpcMidiResetType = SMF_RESET_GM2;
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
            konamiSpcSetLogStreamHandle(htmlFile);
    }

    // convert input file
    fprintf(stderr, "%s:\n", gArgv[0]);
    smf = konamiSpcToMidiFromFile(gArgv[0]);
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
