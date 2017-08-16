/**
 * Capcom spc2midi.
 * http://loveemu.yh.land.to/
 */

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "spcseq.h"
#include "capspc.h"

#define APPNAME         "Capcom SPC2MIDI"
#define APPSHORTNAME    "capspc"
#define VERSION         "[2014-07-17]"
#define AUTHOR          "loveemu"
#define WEBSITE         "http://loveemu.yh.land.to/"

static int capSpcLoopMax = 2;               // maximum loop count of parser
static int capSpcTextLoopMax = 1;           // maximum loop count of text output
static double capSpcTimeLimit = 2400;       // time limit of conversion (for safety)
static bool capSpcLessTextInSMF = false;    // decreases amount of texts in SMF output

static bool capSpcVolIsLinear = false;      // assumes volume curve between SPC and MIDI is linear
static bool capSpcPanIsLinear = false;      // assumes pan curve between SPC and MIDI is linear
static bool capSpcUsePitchBend = false;     // use pitchbend for portamento (both true and false are incomplete)
static bool capSpcNoRelRate = false;        // do not output release rate to smf

static int capSpcForceSongIndex = -1;
static int capSpcForceSongListAddr = -1;

static bool capSpcPatchFixOverride = false;
static PatchFixInfo capSpcPatchFix[256];

static int capSpcContConvCnt = 0;
static int capSpcContConvNum = 1;

enum {
    SMF_RESET_GM1 = 0,      // General MIDI Level 1
    SMF_RESET_GS,           // Roland GS
    SMF_RESET_XG,           // YAMAHA XG
    SMF_RESET_GM2,          // General MIDI Level 2
};
static int capSpcMidiResetType = SMF_RESET_GM2;
static bool preferBankMSB = true;

static const char *mycssfile = APPSHORTNAME ".css";

//----

enum {
    SPC_VER_UNKNOWN = 0,
    SPC_VER_1,              // Super Ghouls 'n' Ghosts
    SPC_VER_2,              // The Magical Quest Starring Mickey Mouse
    SPC_VER_3,              // Mega Man X
};

// MIDI/SMF limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

// any changes are not needed normally
#define SPC_TIMEBASE        48
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   0
#define SPC_ARAM_SIZE       0x10000
#define SPC_LOOPCOUNT_NUM   4
#define SPC_PITCHBENDSENS_NUM       SMF_PITCHBENDSENS_MAX

typedef struct TagCapSpcTrackStat CapSpcTrackStat;
typedef struct TagCapSpcSeqStat CapSpcSeqStat;
typedef void (*CapSpcEvent) (CapSpcSeqStat *, SeqEventReport *);

typedef struct TagCapSpcVerInfo {
    int id;
    int seqListAddr;
    int songIndex;
    int seqHeaderAddr;
    int seqPriority;
    bool useSongList;
    CapSpcEvent event[256]; // vcmds
    PatchFixInfo patchFix[256];
} CapSpcVerInfo;

typedef struct TagCapSpcNoteParam {
    bool active;        // if the following params are used or not
    int tick;           // timing (tick)
    int dur;            // duration (tick)
    int durRate;        // duration rate (n/256)
    int relRate;        // release rate (GAIN)
    bool slur;          // if the note slurred
    bool insPtmnt;      // insert portamento event
    int key;            // key
    int pitchbend;      // pitchbend (for slur)
    int transpose;      // transpose
    int patch;          // instrument
} CapSpcNoteParam;

struct TagCapSpcTrackStat {
    bool active;        // if the channel is still active
    bool used;          // if the channel used once or not
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    CapSpcNoteParam note;     // current note param
    CapSpcNoteParam lastNote; // note params for last note
    int noteCtl;        // flag for note event (MMX $10+x)
                        // $07 - key offset (transpose)
                        // $08 - flag for 2 octave up
                        // $10 - dotted note
                        // $20 - triplet
                        // $40 - tie/slur
                        // $80 - ???
    int loopCount[SPC_LOOPCOUNT_NUM]; // repeat count for loop command
    int looped;         // how many times looped (internal)
};

struct TagCapSpcSeqStat {
    const byte* aRAM;           // SPC ARAM (65536 bytes)
    Smf* smf;                   // link for smf output
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // tempo (bpm)
    int transpose;              // global transpose
    int looped;                 // how many times the song looped (internal)
    bool active;                // if the seq is still active
    CapSpcVerInfo ver;         // game version info
    CapSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

static void capSpcSetEventList (CapSpcSeqStat *seq);

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
FILE *capSpcSetLogStreamHandle (FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int capSpcSetLoopCount (int count)
{
    int oldLoopCount;

    oldLoopCount = capSpcLoopMax;
    capSpcLoopMax = count;
    return oldLoopCount;
}

/** read patch fix info file. */
bool capSpcImportPatchFixFile (const char *filename)
{
    FILE *fp;
    int src, patch, bankL, bankM, key, mmlKey;
    char lineBuf[512];

    if (!filename) {
        capSpcPatchFixOverride = false;
        return false;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        capSpcPatchFixOverride = false;
        return false;
    }

    // reset patch fix
    for (patch = 0; patch < 256; patch++) {
        if (preferBankMSB)
        {
            capSpcPatchFix[patch].bankSelM = patch >> 7;
            capSpcPatchFix[patch].bankSelL = 0;
        }
        else
        {
            capSpcPatchFix[patch].bankSelM = 0;
            capSpcPatchFix[patch].bankSelL = patch >> 7;
        }
        capSpcPatchFix[patch].patchNo = patch & 0x7f;
        capSpcPatchFix[patch].key = 0;
        capSpcPatchFix[patch].mmlKey = 0;
    }
    // import patch fix
    while (fgets(lineBuf, countof(lineBuf), fp)) {
      strtok(lineBuf, ";"); // for comment support

      key = 0;
      mmlKey = 0;
      if (sscanf(lineBuf, "%d %d %d %d %d %d", &src, &bankM, &bankL, &patch, &key, &mmlKey) >= 4) {
        capSpcPatchFix[src].bankSelM = bankM & 0x7f;
        capSpcPatchFix[src].bankSelL = bankL & 0x7f;
        capSpcPatchFix[src].patchNo = (patch - 1) & 0x7f;
        capSpcPatchFix[src].key = key;
        capSpcPatchFix[src].mmlKey = mmlKey;
      }
    }
    capSpcPatchFixOverride = true;

    fclose(fp);
    return true;
}

//----

/** returns version string of music engine. */
static const char *capSpcVerToStrHtml (CapSpcSeqStat *seq)
{
    switch (seq->ver.id) {
    case SPC_VER_1:
        return "Song List Only (Super Ghost 'n' Ghouls)";
    case SPC_VER_2:
        return "Song List &amp; Constant Entry Point (The Magical Quest)";
    case SPC_VER_3:
        return "Constant Entry Point (Mega Man X and Other Capcom Games)";
    default:
        return "Unknown Version / Unsupported";
    }
}

/** reset for each track. */
static void capSpcResetTrackParam (CapSpcSeqStat *seq, int track)
{
    CapSpcTrackStat *tr = &seq->track[track];
    int i;

    tr->used = false;
    tr->note.durRate = 255; // just in case
    tr->note.relRate = 0x18; // GAIN = $b8
    tr->note.pitchbend = 0;
    tr->note.transpose = 0;
    tr->note.slur = false;
    tr->note.insPtmnt = false;
    tr->lastNote.active = false;
    tr->noteCtl = 0x00;
    for (i = 0; i < SPC_LOOPCOUNT_NUM; i++)
        tr->loopCount[i] = 0;
}

/** reset before play/convert song. */
static void capSpcResetParam (CapSpcSeqStat *seq)
{
    int track;
    int patch;

    seq->tick = 0;
    seq->time = 0;
    seq->tempo = 0x0200; // just in case
    seq->transpose = 0;
    seq->looped = 0;
    seq->active = true;

    // reset each track as well
    for (track = SPC_TRACK_MAX - 1; track >= 0; track--) {
        CapSpcTrackStat *tr = &seq->track[track];

        tr->tick = 0;
        capSpcResetTrackParam(seq, track);
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
    if (capSpcPatchFixOverride) {
        for (patch = 0; patch < 256; patch++) {
            memcpy(&seq->ver.patchFix[patch], &capSpcPatchFix[patch], sizeof(PatchFixInfo));
        }
    }
}

/** returns what version the sequence is, and sets individual info. */
static int capSpcCheckVer (CapSpcSeqStat *seq)
{
    const byte *aRAM = seq->aRAM;
    int version = SPC_VER_UNKNOWN;
    int pos1, pos2;
    int addr1, addr2;
    int tr;

    seq->ver.seqListAddr = -1;
    seq->ver.songIndex = -1;
    seq->ver.seqHeaderAddr = -1;
    seq->ver.seqPriority = -1;

    if (capSpcForceSongListAddr >= 0) {
        seq->ver.seqListAddr = capSpcForceSongListAddr;
        seq->ver.seqHeaderAddr = seq->ver.seqListAddr;
        seq->ver.useSongList = true;
        version= SPC_VER_3;
    }

    pos1 = indexOfHexPat(aRAM, (const byte *) "\x6f\x3f..\x8f..\x8f..\x3f..\x8d\\\x00\xdd", SPC_ARAM_SIZE, NULL);
    pos2 = indexOfHexPat(aRAM, (const byte *) "\x1c\x5d\\\xf5..\xc4.\\\xf5..\xc4.\x04.\\\xf0.", SPC_ARAM_SIZE, NULL);

    if (pos1 >= 0) {
        // Fixed BGM address
        addr1 = ((aRAM[pos1 + 5] << 8) | aRAM[pos1 + 8]) + 1;
    }
    if (pos2 >= 0) {
        // BGM/SFX list
        addr2 = min(mget2l(&aRAM[pos2 + 3]), mget2l(&aRAM[pos2 + 8]));
    }

    if (pos2 >= 0) {
        int songIndex;
        int seqListAddr;
        bool useSongList;

        seqListAddr = addr2;
        if (pos1 >= 0) {
            bool validBGMEntry = true;

            version= SPC_VER_2;

            if (addr1 <= addr2 && addr1 + 16 > addr2) {
                // - The Magical Quest Starring Mickey Mouse
                validBGMEntry = false;
            }

            // - Captain Commando
            if (addr1 + 16 <= 0x10000) {
                for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                    int seqTrackAddr = mget2b(&aRAM[addr1 + (tr * 2)]);
                    if ((seqTrackAddr & 0xff00) == 0) {
                        validBGMEntry = false;
                        break;
                    }
                }
            }
            else {
                validBGMEntry = false;
            }

            if (!validBGMEntry) {
                useSongList = true;
            }
            else {
                seq->ver.seqHeaderAddr = addr1;
                useSongList = false;
            }
        }
        else {
            version = SPC_VER_1;
            useSongList = true;
        }

        if (capSpcForceSongListAddr < 0) {
            seq->ver.seqListAddr = seqListAddr;
        }
        else {
            useSongList = true;
        }

        if (useSongList) {
            if (capSpcForceSongIndex >= 0) {
                songIndex = capSpcForceSongIndex;
            }
            else {
                int candidateIndex;
                int candidateScore = INT_MAX;

                songIndex = 0;
                for (candidateIndex = 1; candidateIndex <= 0x7f; candidateIndex++) {
                    int seqHeaderAddr;
                    bool seqInvalid = false;
                    int seqScore = 0;
                    int seqValidTracks = 0;

                    if (seqListAddr + candidateIndex * 2 + 2 > 0x10000) {
                        break;
                    }

                    seqHeaderAddr = mget2b(&aRAM[seqListAddr + candidateIndex * 2]);
                    if (seqHeaderAddr == 0) {
                        continue;
                    }
                    if (seqHeaderAddr + 1 + 16 > 0x10000) {
                        break;
                    }

                    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                        int seqTrackAddr = mget2b(&aRAM[seqHeaderAddr + 1 + (tr * 2)]);
                        if ((seqTrackAddr & 0xff00) == 0) {
                            seqInvalid = true;
                            break;
                        }
                    }
                    if (seqInvalid) {
                        break;
                    }

                    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
                        int seqTrackAddr = mget2b(&aRAM[seqHeaderAddr + 1 + ((7 - tr) * 2)]);

                        int currTrackAddr;;
                        if (version == SPC_VER_1) {
                            currTrackAddr = aRAM[0x00 + tr * 2 + 1] | (aRAM[0x10 + tr * 2 + 1] << 8);
                        }
                        else {
                            currTrackAddr = aRAM[0x00 + tr] | (aRAM[0x08 + tr] << 8);
                        }

                        if (currTrackAddr == 0) {
                            continue;
                        }

                        if (currTrackAddr < seqTrackAddr) {
                            seqValidTracks = 0;
                            break;
                        }
    
                        seqScore += (currTrackAddr - seqTrackAddr);
                        seqValidTracks++;
                    }

                    if (seqValidTracks > 0) {
                        seqScore = seqScore * 8 / seqValidTracks;
    
                        if (candidateScore > seqScore) {
                            songIndex = candidateIndex;
                            candidateScore = seqScore;
                        }
                    }
                }

                if (songIndex == 0) {
                    songIndex = 1;
                }
            }
            songIndex += capSpcContConvCnt;

            seq->ver.seqHeaderAddr = mget2b(&aRAM[seq->ver.seqListAddr + songIndex * 2]);
            if (seq->ver.seqHeaderAddr != 0) {
                seq->ver.seqPriority = aRAM[seq->ver.seqHeaderAddr];
                seq->ver.seqHeaderAddr++;
            }
            else
            {
                version = SPC_VER_UNKNOWN;
            }
        }
        seq->ver.songIndex = songIndex;
        seq->ver.useSongList = useSongList;
    }
    else if (pos1 >= 0) {
        version= SPC_VER_3;
        if (capSpcForceSongListAddr < 0) {
            seq->ver.seqHeaderAddr = addr1;
        }
    }

    seq->ver.id = version;
    capSpcSetEventList(seq);
    return version;
}

/** detect now playing and prepare for analyze. */
static bool capSpcDetectSeq (CapSpcSeqStat *seq)
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
    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        seq->track[tr].pos = mget2b(&aRAM[seqHeaderAddr + (7 - tr) * 2]);
        if (seq->track[tr].pos) {
            seq->track[tr].active = true;
            result = true;
        }
    }
    capSpcResetParam(seq);
    return result;
}

/** create new spc2mid object. */
static CapSpcSeqStat *newCapSpcSeq (const byte *aRAM)
{
    CapSpcSeqStat *newSeq = (CapSpcSeqStat *) calloc(1, sizeof(CapSpcSeqStat));

    if (newSeq) {
        newSeq->aRAM = aRAM;
        capSpcCheckVer(newSeq);
        if (!capSpcDetectSeq(newSeq)) {
            newSeq->ver.id = SPC_VER_UNKNOWN;
        }
    }
    return newSeq;
}

/** delete spc2mid object. */
static void delCapSpcSeq (CapSpcSeqStat **seq)
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
static void printHtmlInfoList (CapSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          <li>Version: %s</li>\n", capSpcVerToStrHtml(seq));
    if (seq->ver.seqListAddr != -1)
        myprintf("          <li>Song / SFX List: $%04X</li>\n", seq->ver.seqListAddr);
    myprintf("          <li>Song Entry: $%04X", seq->ver.seqHeaderAddr);
    if (seq->ver.useSongList)
        myprintf(" (Song $%02x)", seq->ver.songIndex);
    if (seq->ver.seqPriority >= 0)
        myprintf("          <li>Song Priority: $%02X</li>\n", seq->ver.seqPriority);
    myprintf("</li>\n");
}

/** output seq info list detail for valid seq. */
static void printHtmlInfoListMore (CapSpcSeqStat *seq)
{
    if (seq == NULL)
        return;
}

/** output event dump. */
static void printHtmlEventDump (CapSpcSeqStat *seq, SeqEventReport *ev)
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
static void printEventTableHeader (CapSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("        <h3>Sequence</h3>\n");
    myprintf("        <div class=\"section\">\n");
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

/** output event table footer. */
static void printEventTableFooter (CapSpcSeqStat *seq)
{
    if (seq == NULL)
        return;

    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

//----

/** convert SPC tempo into bpm. */
static double capSpcTempo (CapSpcSeqStat *seq)
{
    return (double) seq->tempo * 60000000 / 196608000; // 196608000 = (timer0) 8ms * 48 * 512? (guess)
}

/** insert release rate event. */
static bool capSpcInsertRelRate (CapSpcSeqStat *seq, int tick, int track, int amount)
{
    amount &= 0x1f;
    amount = 0x1f - amount;
    amount = (amount * 2) + (amount ? 1 : 0);
    return smfInsertControl(seq->smf, tick, track, track, SMF_CONTROL_RELEASETIME, 64 + amount);
}

/** convert SPC channel volume into MIDI one. */
static int capSpcMidiVolOf (int version_id, byte value)
{
    const byte volume_lookups[] = {
        0x00, 0x0c, 0x19, 0x26, 0x33, 0x40, 0x4c, 0x59,
        0x66, 0x73, 0x80, 0x8c, 0x99, 0xb3, 0xcc, 0xe6,
        0xff,
    };
    byte midi_volume;

    if (version_id == SPC_VER_1)
    {
        // linear volume
        midi_volume = (byte) value;
    }
    else
    {
        // use volume table (with linear interpolation)
        byte volume_index = (value * 16) >> 8;
        byte volume_rate = (value * 16) & 0xff;
        midi_volume = volume_lookups[volume_index] + ((volume_lookups[volume_index + 1] - volume_lookups[volume_index]) * volume_rate >> 8);
    }

    if (capSpcVolIsLinear)
    {
        midi_volume >>= 1;
    }
    else
    {
        midi_volume = (byte) floor(sqrt(midi_volume / 255.0) * 127.0 + 0.5);
    }

    return midi_volume;
}

/** convert SPC channel panpot into MIDI one. */
static int capSpcMidiPanOf (int version_id, byte value, int * midi_volume_scale)
{
    const byte pan_lookups[] = {
        0x00, 0x01, 0x03, 0x07, 0x0d, 0x15, 0x1e, 0x29,
        0x34, 0x42, 0x51, 0x5e, 0x67, 0x6e, 0x73, 0x77,
        0x7a, 0x7c, 0x7d, 0x7e, 0x7f, 0x7f,
    };

    byte midi_pan;
    double volume_scale;

    // signed -> unsigned
    value += 0x80;

    if (version_id == SPC_VER_1)
    {
        midi_pan = value >> 1;
    }
    else
    {
        // use pan table (with linear interpolation)
        byte pan_index = (value * 20) >> 8;
        byte pan_rate = (value * 20) & 0xff;
        midi_pan = pan_lookups[pan_index] + ((pan_lookups[pan_index + 1] - pan_lookups[pan_index]) * pan_rate >> 8);
    }

    if (capSpcPanIsLinear)
    {
        volume_scale = 1.0;
    }
    else
    {
        double panPI2;
        double linear_pan;
        double curved_pan;

        linear_pan = midi_pan / 127.0;
        panPI2 = atan2(linear_pan, 1.0 - linear_pan);
        curved_pan = panPI2 / M_PI_2;
        volume_scale = 1.0 / (cos(panPI2) + sin(panPI2));

        if (midi_pan != 0)
        {
            midi_pan = (int) floor(curved_pan * 126.0 + 0.5) + 1;
        }
    }

    if (!capSpcVolIsLinear)
    {
        volume_scale = sqrt(volume_scale);
    }

    if (midi_volume_scale != NULL)
    {
        *midi_volume_scale = (int) floor(volume_scale * 127.0 + 0.5);
    }

    return midi_pan;
}

/** create new smf object and link to spc seq. */
static Smf *capSpcCreateSmf (CapSpcSeqStat *seq)
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

    switch (capSpcMidiResetType) {
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

    // put initial info for each track
    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 0);
        //if (!capSpcNoRelRate)
            capSpcInsertRelRate(seq, 0, tr, seq->track[tr].note.relRate);
        smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_MONO, 127);
        if (capSpcUsePitchBend) {
            smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RPNM, 0);
            smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_RPNL, 0);
            smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_DATAENTRYM, SPC_PITCHBENDSENS_NUM);
            smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_DATAENTRYL, 0);
        }
        sprintf(songTitle, "Track %d - $%04X", tr + 1, seq->track[tr].pos);
        smfInsertMetaText(seq->smf, 0, tr, SMF_META_TRACKNAME, songTitle);
    }
    return smf;
}

//----

static char argDumpStr[512];

/** truncate note. */
static void capSpcTruncateNote (CapSpcSeqStat *seq, int track)
{
    CapSpcTrackStat *tr = &seq->track[track];

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
static void capSpcTruncateNoteAll (CapSpcSeqStat *seq)
{
    int tr;

    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        capSpcTruncateNote(seq, tr);
    }
}

/** finalize note. */
static bool capSpcDequeueNote (CapSpcSeqStat *seq, int track)
{
    CapSpcTrackStat *tr = &seq->track[track];
    CapSpcNoteParam *lastNote = &tr->lastNote;
    bool result = false;

    if (lastNote->active) {
        result = smfInsertNote(seq->smf, lastNote->tick, track, track,
            lastNote->key + lastNote->transpose, 127, lastNote->dur);
        if (lastNote->insPtmnt) {
            smfInsertControl(seq->smf, lastNote->tick, track, track, SMF_CONTROL_PORTAMENTO, 127);
            lastNote->insPtmnt = false;
        }
        lastNote->active = false;
    }
    return result;
}

/** finalize note for each track. */
static void capSpcDequeueNoteAll (CapSpcSeqStat *seq)
{
    int tr;

    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        capSpcDequeueNote(seq, tr);
    }
}

/** inactivate track. */
static void capSpcInactiveTrack(CapSpcSeqStat *seq, int track)
{
    int tr;

    seq->track[track].active = false;
    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        if (seq->track[tr].active)
            return;
    }
    seq->active = false;
}

/** increment loop count. */
static void capSpcAddTrackLoopCount(CapSpcSeqStat *seq, int track, int count)
{
    int tr;

    seq->track[track].looped += count;
    seq->looped = (capSpcLoopMax > 0) ? capSpcLoopMax : 0xffff;
    for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
        if (seq->track[tr].active)
            seq->looped = min(seq->looped, seq->track[tr].looped);
    }

    if (seq->looped >= capSpcLoopMax) {
        seq->active = false;
    }
}

/** advance seq tick. */
static void capSpcSeqAdvTick(CapSpcSeqStat *seq)
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
    seq->time += (double) 60 / capSpcTempo(seq) * minTickStep / SPC_TIMEBASE;
}

/** vcmds: unknown event (without status change). */
static void capSpcEventUnknownInline (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");

    if (ev->unidentified)
        fprintf(stderr, "Error: Encountered unidentified event %02X [Track %d]\n", ev->code, ev->track + 1);
    else
        fprintf(stderr, "Warning: Skipped unknown event %02X [Track %d]\n", ev->code, ev->track + 1);
}

/** vcmds: unidentified event. */
static void capSpcEventUnidentified (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    ev->unidentified = true;
    capSpcEventUnknownInline(seq, ev);
    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (no args). */
static void capSpcEventUnknown0 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventUnknownInline(seq, ev);
    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (1 byte arg). */
static void capSpcEventUnknown1 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    capSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds: unknown event (2 byte args). */
static void capSpcEventUnknown2 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    capSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: unknown event (3 byte args). */
static void capSpcEventUnknown3 (CapSpcSeqStat *seq, SeqEventReport *ev)
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

    capSpcEventUnknownInline(seq, ev);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: nop (1 byte arg). */
static void capSpcEventNOP1 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    (*p)++;

    sprintf(ev->note, "Event %02X (NOP)", ev->code);
    strcat(ev->classStr, " ev-nop");
}

/** vcmd: toggle triplet. */
static void capSpcEventToggleTriplet (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];

    tr->noteCtl ^= 0x20;

    sprintf(ev->note, "Triplet %s, state = $%02X",
        (tr->noteCtl & 0x20) ? "On" : "Off", tr->noteCtl);
    strcat(ev->classStr, " ev-triplet");
}

/** vcmd: toggle tie/slur. */
static void capSpcEventToggleSlur (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];
    bool slur;

    tr->noteCtl ^= 0x40;
    slur = (tr->noteCtl & 0x40);

    sprintf(ev->note, "Slur %s, state = $%02X",
        slur ? "On" : "Off", tr->noteCtl);
    strcat(ev->classStr, " ev-slur");

    //smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, slur ? 127 : 0);
}

/** vcmd: dotted note switch on. */
static void capSpcEventToggleDot (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];

    tr->noteCtl |= 0x10;

    sprintf(ev->note, "Dotted Note, state = $%02X", tr->noteCtl);
    strcat(ev->classStr, " ev-dot");
}

/** vcmd: octave up toggle. */
static void capSpcEventToggleOctUp (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];

    tr->noteCtl ^= 0x08;

    sprintf(ev->note, "Octave %s, state = $%02X",
        (tr->noteCtl & 0x08) ? "Up" : "Down", tr->noteCtl);
    strcat(ev->classStr, " ev-oct");
}

/** vcmd: set triplet/dotted/oct-up directly. */
static void capSpcEventNoteParamInline (CapSpcSeqStat *seq, SeqEventReport *ev, int val)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];

    tr->noteCtl &= ~0x68; // 0x97
    tr->noteCtl |= val;
}

/** vcmd: set triplet/dotted/oct-up directly. */
static void capSpcEventNoteParam (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    capSpcEventNoteParamInline(seq, ev, arg1);

    sprintf(ev->note, "Direct Note Param, state = $%02x", tr->noteCtl);
    strcat(ev->classStr, " ev-noteparam");
}

/** vcmd: tempo. */
static void capSpcEventTempo (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2b(&seq->aRAM[*p]);
    (*p) += 2;

    seq->tempo = arg1;
    smfInsertTempoBPM(seq->smf, ev->tick, 0, capSpcTempo(seq));

    sprintf(ev->note, "Tempo, bpm = %f", capSpcTempo(seq));
    strcat(ev->classStr, " ev-tempo");
}

/** vcmd: set duration rate. */
static void capSpcEventDurRate (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Duration Rate, rate = %d/256", arg1);
    strcat(ev->classStr, " ev-durrate");

    tr->note.durRate = arg1;
}

/** vcmd: volume. */
static void capSpcEventVolume (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    //CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, val = %d", arg1);
    strcat(ev->classStr, " ev-vol");

    //if (!capSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VOLUME, capSpcMidiVolOf(seq->ver.id, arg1));
}

/** vcmd: set instrument. */
static void capSpcEventInstrument (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->note.patch = arg1;
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELM, seq->ver.patchFix[tr->note.patch].bankSelM);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, seq->ver.patchFix[tr->note.patch].bankSelL);
    smfInsertProgram(seq->smf, ev->tick, ev->track, ev->track, seq->ver.patchFix[tr->note.patch].patchNo);

    sprintf(ev->note, "Instrument, patch = %d", arg1);
    strcat(ev->classStr, " ev-patch");
}

/** vcmd: set key offset. */
static void capSpcEventKeyOffset (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->noteCtl &= ~0x07; // 0xf8
    tr->noteCtl |= arg1;

    sprintf(ev->note, "Key Offset, state = $%02X", tr->noteCtl);
    strcat(ev->classStr, " ev-keyoffset");
}

/** vcmd: global transpose. */
static void capSpcEventTranspose (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Global Tranpose, key = %d", arg1);
    strcat(ev->classStr, " ev-gtranspose");

    seq->transpose = arg1;
}

/** vcmd: per-voice transpose. */
static void capSpcEventVoiceTranspose (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Voice Tranpose, key = %d", arg1);
    strcat(ev->classStr, " ev-vtranspose");

    tr->note.transpose = arg1;
}

/** vcmd: tuning. */
static void capSpcEventTuning (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Tuning, key = %d / 128", arg1);
    strcat(ev->classStr, " ev-tuning");

    //if (!capSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNM, 0);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_RPNL, 1);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYM, 64 + (arg1 / 4));
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_DATAENTRYL, 0);
}

/** vcmd: portamento time. */
static void capSpcEventPortamentoTime (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Portamento Time, rate = %d", arg1);
    strcat(ev->classStr, " ev-ptmnttime");

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTOTIME, arg1 / 2);
}

/** vcmd: loop if counter is (not) left. */
static void capSpcEventLoopInline (CapSpcSeqStat *seq, SeqEventReport *ev, int index, bool br)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    CapSpcTrackStat *tr = &seq->track[ev->track];
    bool jump;

    ev->size += 3;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = mget2b(&seq->aRAM[*p]);
    (*p) += 2;

    if (!br && tr->loopCount[index] == 0) {
        if (arg1 == 0) {
            capSpcAddTrackLoopCount(seq, ev->track, 1);
        }

        tr->loopCount[index] = arg1;
        jump = true;
    }
    else {
        int count;

        count = tr->loopCount[index] - 1;
        jump = (count != 0);
        if (!br)
            tr->loopCount[index] = count;
        else {
            jump = !jump;
            if (jump) {
                tr->loopCount[index] = count;
                capSpcEventNoteParamInline(seq, ev, arg1);
            }
        }
    }

    if (jump)
        *p = arg2;

    if (!br) {
        sprintf(ev->note, "Loop #%d, count = %d, dest = $%04X", index + 1, arg1, arg2);
    }
    else {
        sprintf(ev->note, "Loop Break #%d, note param = $%02X, dest = $%04X", index + 1, arg1, arg2);
    }
    strcat(ev->classStr, " ev-loop");
}

/** vcmd: loop #1. */
static void capSpcEventLoop1 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 0, false);
}

/** vcmd: loop #2. */
static void capSpcEventLoop2 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 1, false);
}

/** vcmd: loop #3. */
static void capSpcEventLoop3 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 2, false);
}

/** vcmd: loop #4. */
static void capSpcEventLoop4 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 3, false);
}

/** vcmd: loop break #1. */
static void capSpcEventLoopBreak1 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 0, true);
}

/** vcmd: loop break #2. */
static void capSpcEventLoopBreak2 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 1, true);
}

/** vcmd: loop break #3. */
static void capSpcEventLoopBreak3 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 2, true);
}

/** vcmd: loop break #4. */
static void capSpcEventLoopBreak4 (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    capSpcEventLoopInline(seq, ev, 3, true);
}

/** vcmd: non-conditional jump. */
static void capSpcEventJump (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = mget2b(&seq->aRAM[*p]);
    (*p) += 2;

    sprintf(ev->note, "Jump, dest = $%04X", arg1);
    strcat(ev->classStr, " ev-jump");

    if (arg1 <= *p && ev->tick > 0) { // back jump
        capSpcAddTrackLoopCount(seq, ev->track, 1);
    }
    (*p) = arg1;
}

/** vcmd: end of track. */
static void capSpcEventEndOfTrack (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    sprintf(ev->note, "End of Track");
    strcat(ev->classStr, " ev-end");

    capSpcInactiveTrack(seq, ev->track);
}

/** vcmd: pan. */
static void capSpcEventPan (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int volume_scale;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->aRAM[*p]);
    (*p)++;

    sprintf(ev->note, "Panpot, pos = %d", arg1);
    strcat(ev->classStr, " ev-pan");

    //if (!capSpcLessTextInSMF)
    //    smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);

    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_PANPOT, capSpcMidiPanOf(seq->ver.id, arg1, &volume_scale));
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_EXPRESSION, volume_scale);
}

/** vcmd: master volume. */
static void capSpcEventMasterVolume (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    //CapSpcTrackStat *tr = &seq->track[ev->track];

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume, val = %d", arg1);
    strcat(ev->classStr, " ev-mastervol");

    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: set LFO param. */
static void capSpcEventLFOParam (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    bool unknownType = false;
    bool dumpText = false;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "LFO Param, type = ");

    switch(arg1) {
      case 0:
        // pitch modulation (vibrato)
        sprintf(argDumpStr, "Vibrato Depth, depth = %d", arg2);
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_MODULATION, min(arg2 * 4, 127));
        break;
      case 1:
        // volume modulation (tremolo)
        sprintf(argDumpStr, "Tremolo Depth, depth = %d", arg2);
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_TREMOLODEPTH, min(arg2, 127));
        break;
      case 2:
        // LFO speed
        sprintf(argDumpStr, "LFO Speed, speed = %d", arg2);
        smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_VIBRATORATE, min(arg2, 127));
        break;
      case 3:
        // Reset LFO Per Note
        sprintf(argDumpStr, "Reset LFO Per Note, sw = %s", (arg2 & 1) ? "on" : "off");
        dumpText = true;
        break;
      default:
        sprintf(argDumpStr, "%d, value = %d", arg1, arg2);
        unknownType = true;
    }
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-lfoparam");

    if ((dumpText || unknownType) && !capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    if (unknownType)
        fprintf(stderr, "Warning: Unknown %s\n", ev->note);
}

/** vcmd: set echo param. */
static void capSpcEventEchoParam (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->aRAM[*p];
    (*p)++;
    arg2 = seq->aRAM[*p];
    (*p)++;

    sprintf(ev->note, "Echo Param, arg1 = %d, arg2 = %d", arg1, arg2);
    strcat(ev->classStr, " ev-echoparam");

    if (!capSpcLessTextInSMF)
        smfInsertMetaText(seq->smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: echo on/off. */
static void capSpcEventEchoOnOff (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    bool echo;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    echo = (arg1 & 1);
    smfInsertControl(seq->smf, ev->tick, ev->track, ev->track, SMF_CONTROL_REVERB, echo ? 86 : 0);

    sprintf(ev->note, "Echo On/Off, sw = %s", echo ? "on" : "off");
    strcat(ev->classStr, " ev-echosw");
}

/** vcmd: release rate. */
static void capSpcEventReleaseRate (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->aRAM[*p];
    (*p)++;

    tr->note.relRate = (arg1 & 0x1f);
    if (!capSpcNoRelRate)
        capSpcInsertRelRate(seq, ev->tick, ev->track, tr->note.relRate);

    sprintf(ev->note, "Release Rate, rate = %d", tr->note.relRate);
    strcat(ev->classStr, " ev-adsr-rr");
}

/** vcmd: note. */
static void capSpcEventNote (CapSpcSeqStat *seq, SeqEventReport *ev)
{
    CapSpcTrackStat *tr = &seq->track[ev->track];
    byte durVal = (ev->code >> 5) - 1;
    byte keyVal = (ev->code & 0x1f) - 1;
    bool rest = !(ev->code & 0x1f);
    bool oldSlur = tr->lastNote.active ? tr->lastNote.slur : false;
    bool curSlur = (tr->noteCtl & 0x40);
    bool useBend = capSpcUsePitchBend;
    bool sameNote, tieNote, slurEnd;
    int len, dur, key, transpose;
    int keyDiff, pitchbend;

    // real engine refers duration from table.
    // anyway, here's the formula to calc duration.
    len = 192 >> (6 - durVal);
    // dotted note
    if (tr->noteCtl & 0x10) {
        int newLen = len + (len / 2);

        if (len % 2 != 0 || newLen > 0xff)
            len = 0; // they cannot be expressed in byte table
        else
            len = newLen;

        tr->noteCtl &= ~0x10;
    }
    // triplet
    else if (tr->noteCtl & 0x20) {
        len = len * 2 / 3;
    }
    dur = curSlur ? len : (len * tr->note.durRate / 256);
    if (dur == 0)
        dur = len; // hack for SGnG stage 2 boss... correct?

    // octave correction
    key = keyVal;
    key += (tr->noteCtl & 0x08) ? 24 : 0;
    key += (tr->noteCtl & 0x07) * 12;
    transpose = seq->transpose + tr->note.transpose;
    transpose += seq->ver.patchFix[tr->note.patch].key;

    keyDiff = (tr->lastNote.active && oldSlur) ? (key + transpose - tr->lastNote.key - tr->lastNote.transpose) : 0;
    pitchbend = (int) (((double) keyDiff / SPC_PITCHBENDSENS_NUM * 8192) + 0.5);
    if (pitchbend == 8192)
        pitchbend = 8191;

    sameNote = tr->lastNote.active ? (key == tr->lastNote.key) : false;
    tieNote = !rest && oldSlur && (useBend || (!useBend && sameNote));
    if (useBend && tr->note.pitchbend != pitchbend && !rest) {
        smfInsertPitchBend(seq->smf, ev->tick, ev->track, ev->track, pitchbend);
        tr->note.pitchbend = pitchbend;
    }

    slurEnd = oldSlur && !curSlur;
    if (!tieNote) {
        capSpcDequeueNote(seq, ev->track);
    }

    if (rest) {
        sprintf(ev->note, "Rest");
        strcat(ev->classStr, " ev-rest");
    }
    else {
        getNoteName(ev->note, key + transpose + SPC_NOTE_KEYSHIFT);
        sprintf(argDumpStr, ", dur/len = %d/%d", dur, len);
        strcat(ev->note, argDumpStr);
        strcat(ev->classStr, " ev-note");
    }

    if (!rest) {
        if (tieNote) {
            tr->lastNote.dur += dur;
        }
        else {
            tr->lastNote.tick = ev->tick;
            tr->lastNote.dur = dur;
            tr->lastNote.key = key;
            tr->lastNote.transpose = transpose;
            tr->lastNote.insPtmnt = (!useBend && !oldSlur && curSlur);
        }
        tr->lastNote.durRate = tr->note.durRate;
        tr->lastNote.slur = curSlur;
        tr->lastNote.pitchbend = pitchbend;
        tr->lastNote.patch = tr->note.patch;
        tr->lastNote.active = true;
    }
    tr->tick += len;
    if (slurEnd)
        smfInsertControl(seq->smf, tr->tick, ev->track, ev->track, SMF_CONTROL_PORTAMENTO, 0);
}

/** set pointers of each event. */
static void capSpcSetEventList (CapSpcSeqStat *seq)
{
    int code;
    CapSpcEvent *event = seq->ver.event;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (CapSpcEvent) capSpcEventUnidentified;
    }

    event[0x00] = (CapSpcEvent) capSpcEventToggleTriplet;
    event[0x01] = (CapSpcEvent) capSpcEventToggleSlur;
    event[0x02] = (CapSpcEvent) capSpcEventToggleDot;
    event[0x03] = (CapSpcEvent) capSpcEventToggleOctUp;
    event[0x04] = (CapSpcEvent) capSpcEventNoteParam;
    event[0x05] = (CapSpcEvent) capSpcEventTempo;
    event[0x06] = (CapSpcEvent) capSpcEventDurRate;
    event[0x07] = (CapSpcEvent) capSpcEventVolume;
    event[0x08] = (CapSpcEvent) capSpcEventInstrument;
    event[0x09] = (CapSpcEvent) capSpcEventKeyOffset;
    event[0x0a] = (CapSpcEvent) capSpcEventTranspose;
    event[0x0b] = (CapSpcEvent) capSpcEventVoiceTranspose;
    event[0x0c] = (CapSpcEvent) capSpcEventTuning;
    event[0x0d] = (CapSpcEvent) capSpcEventPortamentoTime;
    event[0x0e] = (CapSpcEvent) capSpcEventLoop1;
    event[0x0f] = (CapSpcEvent) capSpcEventLoop2;
    event[0x10] = (CapSpcEvent) capSpcEventLoop3;
    event[0x11] = (CapSpcEvent) capSpcEventLoop4;
    event[0x12] = (CapSpcEvent) capSpcEventLoopBreak1;
    event[0x13] = (CapSpcEvent) capSpcEventLoopBreak2;
    event[0x14] = (CapSpcEvent) capSpcEventLoopBreak3;
    event[0x15] = (CapSpcEvent) capSpcEventLoopBreak4;
    event[0x16] = (CapSpcEvent) capSpcEventJump;
    event[0x17] = (CapSpcEvent) capSpcEventEndOfTrack;
    event[0x18] = (CapSpcEvent) capSpcEventPan;
    event[0x19] = (CapSpcEvent) capSpcEventMasterVolume;
    event[0x1a] = (CapSpcEvent) capSpcEventLFOParam;
    event[0x1b] = (CapSpcEvent) capSpcEventEchoParam;
    event[0x1c] = (CapSpcEvent) capSpcEventEchoOnOff;
    event[0x1d] = (CapSpcEvent) capSpcEventReleaseRate;
    event[0x1e] = (CapSpcEvent) capSpcEventUnknown1;
    event[0x1f] = (CapSpcEvent) capSpcEventUnknown1;
    //event[0x1e] = (CapSpcEvent) capSpcEventNOP1;
    //event[0x1f] = (CapSpcEvent) capSpcEventNOP1;
    // note
    for(code = 0x20; code <= 0xff; code++) {
        event[code] = (CapSpcEvent) capSpcEventNote;
    }

    if (seq->ver.id == SPC_VER_UNKNOWN)
        return;
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* capSpcARAMToMidi (const byte *aRAM)
{
    bool abortFlag = false;
    CapSpcSeqStat *seq = NULL;
    Smf* smf = NULL;
    int tr;

    printHtmlHeader();
    myprintf("    <h1>%s %s</h1>\n", APPNAME, VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by %s. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", APPSHORTNAME);

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");

    seq = newCapSpcSeq(aRAM);
    printHtmlInfoList(seq);

    if (seq->ver.id == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Invalid or unsupported data.\n");
        myprintf("        </ul>\n");
        myprintf("      </div>\n");
        goto abort;
    }
    smf = capSpcCreateSmf(seq);

    printHtmlInfoListMore(seq);

    myprintf("          </ul></li>\n");
    myprintf("        </ul>\n");
    myprintf("      </div>\n\n");

    myprintf("      <h2>Data Dump</h2>\n");
    myprintf("      <div class=\"section\" id=\"data-dump\">\n");
    myprintf("        <p>You can filter output by using stylesheet. Write %s as you like!</p>\n", mycssfile);

    printEventTableHeader(seq);

    while (seq->active && !abortFlag) {

        SeqEventReport ev;

        for (ev.track = SPC_TRACK_MAX - 1; ev.track >= 0; ev.track--) {

            CapSpcTrackStat *evtr = &seq->track[ev.track];

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
                if (capSpcTextLoopMax == 0 || seq->looped < capSpcTextLoopMax)
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
            for (tr = SPC_TRACK_MAX - 1; tr >= 0; tr--) {
                seq->track[tr].tick = seq->tick;
                //if (seq->track[tr].used)
                    smfSetEndTimingOfTrack(seq->smf, tr, seq->tick);
            }
        }
        else {
            capSpcSeqAdvTick(seq);

            // check time limit
            if (seq->time >= capSpcTimeLimit) {
                seq->active = false;
            }
        }
    }

quitConversion:

    // finalize for all notes
    capSpcTruncateNoteAll(seq);
    capSpcDequeueNoteAll(seq);

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
        delCapSpcSeq(&seq);
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
Smf* capSpcToMidi (const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = capSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert spc to midi data from SPC file. */
Smf* capSpcToMidiFromFile (const char *filename)
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

    smf = capSpcToMidi(data, size);

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
static bool cmdOptLoop (void);
static bool cmdOptPatchFix (void);
static bool cmdOptGS (void);
static bool cmdOptXG (void);
static bool cmdOptGM1 (void);
static bool cmdOptGM2 (void);
static bool cmdOptNoRelRate (void);

static CmdOptDefs optDef[] = {
    { "help", '\0', 0, cmdOptHelp, "", "show usage" },
    { "count", '\0', 1, cmdOptCount, "<n>", "convert n songs continuously" },
    { "song", '\0', 1, cmdOptSong, "<index>", "force set song index" },
    { "songlist", '\0', 1, cmdOptSongList, "<addr>", "force set song (list) address" },
    { "loop", '\0', 1, cmdOptLoop, "<times>", "set loop count" },
    { "patchfix", '\0', 1, cmdOptPatchFix, "<file>", "modify patch/transpose" },
    { "gs", '\0', 0, cmdOptGS, "", "Insert GS Reset at beginning of seq" },
    { "xg", '\0', 0, cmdOptXG, "", "Insert XG System On at beginning of seq" },
    { "gm1", '\0', 0, cmdOptGM1, "", "Insert GM1 System On at beginning of seq" },
    { "gm2", '\0', 0, cmdOptGM2, "", "Insert GM2 System On at beginning of seq" },
    { "norel", '\0', 0, cmdOptNoRelRate, "", "Don't output release rate event" },
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
    capSpcContConvNum = count;
    return true;
}

/** set loop song index. */
static bool cmdOptSong (void)
{
    int songIndex = strtol(gArgv[0], NULL, 0);
    capSpcForceSongIndex = songIndex;
    return true;
}

/** set song (list) address. */
static bool cmdOptSongList (void)
{
    int songListAddr = strtol(gArgv[0], NULL, 16);
    capSpcForceSongListAddr = songListAddr;
    return true;
}

/** set loop count. */
static bool cmdOptLoop (void)
{
    int loopCount = strtol(gArgv[0], NULL, 0);
    capSpcSetLoopCount(loopCount);
    return true;
}

/** import patch fix file. */
static bool cmdOptPatchFix (void)
{
    if (capSpcImportPatchFixFile(gArgv[0]))
        return true;
    else {
        fprintf(stderr, "Error: unable to import patchfix.\n");
        return false;
    }
}

/** use GS reset. */
static bool cmdOptGS (void)
{
    capSpcMidiResetType = SMF_RESET_GS;
    return true;
}

/** use XG reset. */
static bool cmdOptXG (void)
{
    capSpcMidiResetType = SMF_RESET_XG;
    return true;
}

/** use GM1 reset. */
static bool cmdOptGM1 (void)
{
    capSpcMidiResetType = SMF_RESET_GM1;
    return true;
}

/** use GM2 reset. */
static bool cmdOptGM2 (void)
{
    capSpcMidiResetType = SMF_RESET_GM2;
    return true;
}

/** no release rate output. */
static bool cmdOptNoRelRate (void)
{
    capSpcNoRelRate = true;
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
    for (capSpcContConvCnt = 0; capSpcContConvCnt < capSpcContConvNum; capSpcContConvCnt++) {
        strcpy(spcPath, spcBasePath);
        strcpy(midPath, midBasePath);
        strcpy(htmlPath, htmlBasePath);
        if (capSpcContConvCnt) {
            sprintf(tmpPath, "%s-%03d.mid", removeExt(midPath), capSpcContConvCnt + 1);
            strcpy(midPath, tmpPath);
            if (htmlPath[0] != '\0') {
                sprintf(tmpPath, "%s-%03d.html", removeExt(htmlPath), capSpcContConvCnt + 1);
                strcpy(htmlPath, tmpPath);
            }
        }

        // set html handle if needed
        htmlFile = (htmlPath[0] != '\0') ? fopen(htmlPath, "w") : NULL;
        capSpcSetLogStreamHandle(htmlFile);

        fprintf(stderr, "%s", spcPath);
        if (capSpcContConvCnt)
            fprintf(stderr, "(%d)", capSpcContConvCnt + 1);
        fprintf(stderr, ":\n");

        smf = capSpcToMidiFromFile(spcPath);
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
