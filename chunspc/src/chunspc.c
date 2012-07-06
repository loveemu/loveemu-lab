/**
 * chunsoft summer/winter spc2midi.
 * http://loveemu.yh.land.to/
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "chunspc.h"

#define VERSION "[2008-11-29]"

// MIDI limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

static int chunSpcLoopMax = 2;            // maximum loop count of parser
static int chunSpcTextLoopMax = 1;        // maximum loop count of text output
static double chunSpcTimeLimit = 1200;    // time limit of conversion (for safety)
static bool chunSpcLessTextInSMF = false; // decreases amount of texts in SMF output

static int chunSpcPitchBendSens = 0;      // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static int chunSpcChannelVolOffset = 0;   // XXX: hack, should be 0 for serious conversion
static bool chunSpcNoPatchChange = false; // XXX: hack, should be false for serious conversion
static bool chunSpcVolIsLinear = false;   // assumes volume curve between SPC and MIDI is linear

//----

#define SPC_VER_UNKNOWN     0
#define SPC_VER_OTOGIRISOU  1       // summer OTOGIRISOU version $Revision: 2.0.1.3
#define SPC_VER_DQ5         2       // winter DQ5 version $Revision: 1.44
#define SPC_VER_TORNECO     3       // winter F version $Revision: 2.3
#define SPC_VER_KAMAITACHI  4       // winter SN2 version $Revision: 3.32

// any changes are not needed normally
#define SPC_TIMEBASE        48
#define SPC_SONG_MAX        8
#define SPC_NOTE_KEYSHIFT   23
#define CMD_CALL_MAX        3

static const double spcARTable[0x10] = {
    4.1, 2.6, 1.5, 1.0, 0.640, 0.380, 0.260, 0.160,
    0.096, 0.064, 0.040, 0.024, 0.016, 0.010, 0.006, 0
};
static const double spcDRTable[0x08] = {
    1.2, 0.740, 0.440, 0.290, 0.180, 0.110, 0.074, 0.037
};
static const double spcSRTable[0x20] = {
    0, 38, 28, 24, 19, 14, 12, 9.4, 7.1, 5.9, 4.7, 3.5, 2.9, 2.4, 1.8, 1.5,
    1.2, 0.880, 0.740, 0.590, 0.440, 0.370, 0.290, 0.220, 0.180, 0.150, 0.110,
    0.0092, 0.0074, 0.0055, 0.0037, 0.0018
};

static FILE *mystdout = NULL;
static int myprintf(const char *format, ...)
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
static const char *mycssfile = "chunspc.css";

typedef struct TagChunSpcTrackStat {
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    int duration;       // length of note (tick)
    int durationRate;   // duration rate (0-255)
    int lastNote;       // note number of the last note (used for tie)
    int tieBeginTick;   // beginning of tied note (tick)
    int tieDuration;    // duration for tie
    int tieTranspose;   // key shift for tie
    bool refDuration;   // refer duration info from other channel
    int transpose;      // key shift (note number, signed)
    int callSP;         // stack pointer for call/ret (<=CMD_CALL_MAX)
    int retnAddr[CMD_CALL_MAX]; // return address of call
    int loopCount;      // repeat count for loop command
    int loopCountAlt;   // repeat count for alternative loop command
    int patch;          // patch number (for pitch fix)
    int masterVolume;   // master volume (volume)
    int volume;         // volume (expression)
    int pan;            // panpot
    int pitchSlideStartTick; // start timing of pitch slide (tick)
    int pitchSlideEndTick; // end timing of pitch slide (tick)
    int pitchSlideTarget; // target pitch of slide (-8192-8192)
    int lastPitch;      // last output value of pitch bend
    int pitchSlideMax;  // limit of pitch slide for MIDI output
    int looped;         // how many times 'the song' looped (internal)
    bool hasEnd;        // has reached to EOT? (internal)
} ChunSpcTrackStat;

typedef struct TagChunSpcSeqStat {
    byte* ARAM;                 // ARAM (65536 bytes)
    int version;                // SPC_VER_*
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // current tempo (bpm)
    int masterTempo;            // tempo base (bpm), used for SN2
    int addrOfHeader;           // sequence header address
    int numOfTracks;            // number of tracks (<=8)
    int cpuFlag;                // flag which is updated via CPU (I assume)
    ChunSpcTrackStat track[8];  // status of each tracks
} ChunSpcSeqStat;

typedef struct TagSeqEventReport {
    int track;          // track number
    int tick;           // timing (tick)
    int addr;           // address of the event
    int size;           // size of the event
    int code;           // event type (first byte)
    bool unidentified;  // unidentified event or not
    char note[256];     // note of the event
    char classStr[256]; // html classes
} SeqEventReport;

void getNoteName(char *name, int note);
bool isSpcSoundFile(const byte *data, size_t size);
bool chunSpcDetectSeq(ChunSpcSeqStat *seq);

static const char *chunSpcVerToStr(int version);
static int chunSpcCheckVer(ChunSpcSeqStat *seq);
static void printHtmlHeader(void);
static void printHtmlFooter(void);

static int chunSpcMulRate(int y, int a, int spcVer);
static int chunSpcActualDurationOf(int duration, int durationRate, int spcVer);
static bool chunSpcDequeueTiedNote(ChunSpcSeqStat *seq, int track, Smf* smf);
static bool smfInsertChunSpcNote(Smf* seq, int time, int channel, int track, int key, int duration);
static int chunSpcMidiVolOf(int value);
static int chunSpcMidiVolExprOf(int value);
static bool smfInsertChunSpcVolume(Smf* seq, int time, int channel, int track, int value);
static bool smfInsertChunSpcExpression(Smf* seq, int time, int channel, int track, int value);
static int chunSpcMidiPanOf(int value);
static bool smfInsertChunSpcPanpot(Smf* seq, int time, int channel, int track, int value);
static bool chunSpcEventCondJump(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf, int flag);
static bool chunSpcEventCondJumpCnt(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf, int count, int *counter);
typedef void (*ChunSpcEvent) (ChunSpcSeqStat *, SeqEventReport *, Smf *);
static void chunSpcEventUnknownInline(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventUnidentified(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventUnknown0(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventUnknown1(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventUnknown2(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventUnknown3(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventNOP(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventReserved(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventNote(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventDurFromTable(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventDB(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventDC(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventDDInline(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventDD(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventDE(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventE0(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventE2(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventE6(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventE8(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventEA(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventEB(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventEC(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventED(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventEE(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventEFInline(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventEF(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF0(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF1(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF2(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF4(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF5(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF3(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF6(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF7(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF8(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventF9(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventFA(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventFB(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcEventFF(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void chunSpcInitEventList(ChunSpcSeqStat *seq, ChunSpcEvent *event);

static bool handleCmdLineOpts(int *argc, char **argv[], const char *cmd);

void man(const char *cmd);
void about(const char *cmd);
int main(int argc, char *argv[]);

//----

void getNoteName(char *name, int note)
{
    //char *nameTable[] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
    char *nameTable[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int n = note % 12;
    int oct = note / 12;

    oct--;
    sprintf(name, "%s%d", nameTable[n], oct);
}

/** check if input is SPC file. */
bool isSpcSoundFile(const byte *data, size_t size)
{
    if (size < 0x10100) {
        return false;
    }

    if (memcmp(data, "SNES-SPC700 Sound File Data", 27) != 0) {
        return false;
    }

    return true;
}

//----

/** sets html stream to new target. */
FILE *chunSpcSetLogStreamHandle(FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int chunSpcSetLoopCount(int count)
{
    int oldLoopCount;

    oldLoopCount = chunSpcLoopMax;
    chunSpcLoopMax = count;
    return oldLoopCount;
}

/** returns version string of music engine. */
static const char *chunSpcVerToStr(int version)
{
    if (version == SPC_VER_OTOGIRISOU)
        return "summer OTOGIRISOU version $Revision: 2.0.1.3";
    else if (version == SPC_VER_DQ5)
        return "winter DQ5 version $Revision: 1.44";
    else if (version == SPC_VER_TORNECO)
        return "winter F version $Revision: 2.3";
    else if (version == SPC_VER_KAMAITACHI)
        return "winter SN2 version $Revision: 3.32";
    else
        return "unknown version";
}

/** returns what version the sequence is. */
static int chunSpcCheckVer(ChunSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;

    if (memcmp(&seq->ARAM[0x0760], "\x00ummer OTOGIRISOU version $Revision: 2.0.1.3 $l", 0x2f) == 0)
        version = SPC_VER_OTOGIRISOU;
    else if (memcmp(&seq->ARAM[0x0750], "winter DQ5 version $Revision: 1.44 $l", 0x25) == 0)
        version = SPC_VER_DQ5;
    else if (memcmp(&seq->ARAM[0x0780], "winter F version $Revision: 2.3 $l", 0x22) == 0)
        version = SPC_VER_TORNECO;
    else if (memcmp(&seq->ARAM[0x0880], "winter SN2 version $Revision: 3.32 $l", 0x25) == 0)
        version = SPC_VER_KAMAITACHI;

    seq->version = version;
    return version;
}

/** detects now playing and find sequence header for it. */
bool chunSpcDetectSeq(ChunSpcSeqStat *seq)
{
    const int sumConstTable[][3] = {
        { 0x0525, 0x051D, 0x218F }, // OTOGIRISOU
    };
    const int winConstTable[][4] = {
        { 0x03F9, 0x03AF, 0x04C7, 0x05D5 }, // DQ5
        { 0x03F9, 0x03AF, 0x04C7, 0x0635 }, // F
        { 0x040A, 0x03B7, 0x04D8, 0x06E6 }, // SN2
    };
    byte *ARAM = seq->ARAM;
    int headerOfs;
    int A, X, Y;
    int tr;
    int cpuFlagAddr;
    int songIndex;

    if (seq->version < SPC_VER_DQ5) {
        // summer
        const int *sumConst = sumConstTable[seq->version - SPC_VER_OTOGIRISOU];

        for (X = SPC_SONG_MAX - 1; X >= 0; X--) {
            if (ARAM[sumConst[0] + X]) { // tempo
                break;
            }
        }
        if (X < 0) {
            fprintf(stderr, "Warning: Unable to guess now playing\n");
            //assert(false);
            X = 0;
        }
        songIndex = X;
        A = ARAM[sumConst[1] + X];
        Y = mget2l(&ARAM[sumConst[2]]);
        headerOfs = mget2l(&ARAM[Y + A * 2]);
    }
    else {
        // winner
        const int *winConst = winConstTable[seq->version - SPC_VER_DQ5];

        // FIXME: NYI
        songIndex = 0;

        X = ARAM[winConst[0]];
        Y = ARAM[winConst[1] + X];
        A = winConst[3] + (ARAM[winConst[2] + Y] * 6);
        headerOfs = mget2l(&ARAM[A + 1]);
    }
    seq->addrOfHeader = headerOfs;

    if (headerOfs == 0) {
        return false;
    }

    // initialize
    seq->tick = 0;
    seq->time = 0;
    seq->tempo = seq->masterTempo = ARAM[headerOfs];
    seq->numOfTracks = ARAM[headerOfs + 1];
    seq->cpuFlag = 0;
    for (tr = 0; tr < seq->numOfTracks; tr++) {
        seq->track[tr].pos = mget2l(&ARAM[headerOfs + 2 + tr * 2]);
        if (seq->version >= SPC_VER_DQ5)
            seq->track[tr].pos += headerOfs;

        seq->track[tr].tick = 0;
        seq->track[tr].duration = 1;
        seq->track[tr].durationRate = 0xcc;
        seq->track[tr].lastNote = 0;
        seq->track[tr].tieBeginTick = 0;
        seq->track[tr].tieDuration = 0;
        seq->track[tr].tieTranspose = 0;
        seq->track[tr].refDuration = false;
        seq->track[tr].transpose = 0;
        seq->track[tr].callSP = 0;
        seq->track[tr].loopCount = 0;
        seq->track[tr].loopCountAlt = 0;
        seq->track[tr].patch = 0;
        seq->track[tr].masterVolume = 0x60;
        seq->track[tr].volume = 0x80;
        seq->track[tr].pan = 0;
        seq->track[tr].pitchSlideStartTick = -1;
        seq->track[tr].pitchSlideEndTick = -1;
        seq->track[tr].pitchSlideTarget = 0;
        seq->track[tr].lastPitch = 0;
        seq->track[tr].pitchSlideMax = (chunSpcPitchBendSens == 0) ? SMF_PITCHBENDSENS_DEFAULT : chunSpcPitchBendSens;

        seq->track[tr].looped = 0;
        seq->track[tr].hasEnd = false;
    }

    // XXX: try to read a flag from ARAM
    switch(seq->version) {
    case SPC_VER_OTOGIRISOU:
        cpuFlagAddr = 0x0555;
        break;
    case SPC_VER_DQ5:
    case SPC_VER_TORNECO:
        cpuFlagAddr = 0x03E7;
        break;
    case SPC_VER_KAMAITACHI:
        cpuFlagAddr = 0x03F7;
        break;
    default:
        assert(false);
    }
    seq->cpuFlag = ARAM[cpuFlagAddr + songIndex];

    return true;
}

/** outputs html header. */
static void printHtmlHeader()
{
    myprintf("<?xml version=\"1.0\" ?>\n");
    myprintf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
    myprintf("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n");
    myprintf("  <head>\n");
    myprintf("    <link rel=\"stylesheet\" type=\"text/css\" media=\"screen,tv,projection\" href=\"%s\" />\n", mycssfile);
    myprintf("    <title>Data View - Chunsoft summer/winter SPC2MIDI %s</title>\n", VERSION);
    myprintf("  </head>\n");
    myprintf("  <body>\n");
}

/** outputs html footer. */
static void printHtmlFooter()
{
    myprintf("  </body>\n");
    myprintf("</html>\n");
}

//----

static char argDumpStr[256];

/** near Y*A/256, $1D0F of Kamaitachi no Yoru. */
static int chunSpcMulRate(int y, int a, int spcVer)
{
    int ret = y;
    if (a != 0xff) {
        ret *= (a >= 0x80) ? (a + 1) : a;
        if (spcVer < SPC_VER_DQ5) {
            ret += 0xff; // roundup
        }
        ret >>= 8;
    }
    return ret;
}

/** calc actual duration for note. */
static int chunSpcActualDurationOf(int duration, int durationRate, int spcVer)
{
    if (durationRate == 0xfe) {
        durationRate--;
    }
    return (durationRate == 0) ? duration : chunSpcMulRate(duration, durationRate, spcVer);
}

/** finalize tied note. */
static bool chunSpcDequeueTiedNote(ChunSpcSeqStat *seq, int track, Smf* smf)
{
    bool tied = (seq->track[track].tieDuration > 0);
    if (tied) {
        smfInsertChunSpcNote(smf, seq->track[track].tieBeginTick,
            track, track, seq->track[track].lastNote + seq->track[track].tieTranspose,
            seq->track[track].tieDuration);
        seq->track[track].tieDuration = 0;
    }
    return tied;
}

/** insert note. */
static bool smfInsertChunSpcNote(Smf* seq, int time, int channel, int track, int key, int duration)
{
    return smfInsertNote(seq, time, channel, track, key + SPC_NOTE_KEYSHIFT, 127, duration);
}

/** convert SPC channel master volume into MIDI one. */
static int chunSpcMidiVolOf(int value)
{
    int vol;

    if (chunSpcVolIsLinear)
        vol = value/2; // linear
    else
        vol = (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
    vol += chunSpcChannelVolOffset;
    vol = clip(0, vol, 127);
    return vol;
}

/** convert SPC volume into MIDI one. */
static int chunSpcMidiVolExprOf(int value)
{
    if (chunSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** insert channel master volume. */
static bool smfInsertChunSpcVolume(Smf* seq, int time, int channel, int track, int value)
{
    return smfInsertControl(seq, time, channel, track, SMF_CONTROL_VOLUME, chunSpcMidiVolOf(value));
}

/** insert volume expression. */
static bool smfInsertChunSpcExpression(Smf* seq, int time, int channel, int track, int value)
{
    return smfInsertControl(seq, time, channel, track, SMF_CONTROL_EXPRESSION, chunSpcMidiVolExprOf(value));
}

/** convert SPC panpot into MIDI once. */
static int chunSpcMidiPanOf(int value)
{
    return (value/2) + 64; // linear
}

/** insert a pan command. */
static bool smfInsertChunSpcPanpot(Smf* seq, int time, int channel, int track, int value)
{
    return smfInsertControl(seq, time, channel, track, SMF_CONTROL_PANPOT, chunSpcMidiPanOf(value));
}

/** conditional jump with a flag. */
static bool chunSpcEventCondJump(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf, int flag)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    bool jump;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->ARAM[*p]));
    (*p) += 2;

    jump = (flag != 0);
    dest = *p + arg1;
    if (jump) {
        *p = dest;
    }

    sprintf(argDumpStr, ", addr = $%04X%s", dest, jump ? "" : "*");
    strcat(ev->note, argDumpStr);
    return jump;
}

/** conditional jump with a counter. */
static bool chunSpcEventCondJumpCnt(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf, int count, int *counter)
{
    bool result;
    int orgCounter;

    if (*counter == 0) {
        *counter = count;
    }
    orgCounter = *counter;
    (*counter)--;

    result = chunSpcEventCondJump(seq, ev, smf, *counter);
    sprintf(argDumpStr, ", counter = %d", orgCounter);
    strcat(ev->note, argDumpStr);
    return result;
}

/** vcmds: unknown event (without status change). */
static void chunSpcEventUnknownInline(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Unknown Event %02X", ev->code);
    strcat(ev->classStr, " unknown");
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    fprintf(stderr, "Warning: Skipped unknown event %02X\n", ev->code);
}

/** vcmds: unidentified event. */
static void chunSpcEventUnidentified(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    ev->unidentified = true;
    chunSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (no args). */
static void chunSpcEventUnknown0(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    chunSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (1 byte arg). */
static void chunSpcEventUnknown1(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmds: unknown event (2 byte args). */
static void chunSpcEventUnknown2(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmd: unknown event (3 byte args). */
static void chunSpcEventUnknown3(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;
    arg3 = seq->ARAM[*p];
    (*p)++;

    chunSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
}

/** vcmd F7: no operation. */
static void chunSpcEventNOP(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** vcmds: reserved. */
static void chunSpcEventReserved(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Reserved (Event %02X)", ev->code);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmds 00-9F: note, rest, tie. */
static void chunSpcEventNote(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int note = ev->code;
    bool hasDuration = (note >= 0x50);
    int actualDuration;

    if (hasDuration) {
        note -= 0x50;

        ev->size++;
        arg1 = seq->ARAM[*p];
        (*p)++;

        seq->track[ev->track].duration = arg1;
    }
    // before new note (not tied)
    if (note != 0x4f) {
        // put tied note before handling the new note
        chunSpcDequeueTiedNote(seq, ev->track, smf);
        // finish pitch slide
        if (seq->track[ev->track].pitchSlideEndTick >= 0 && ev->tick >= seq->track[ev->track].pitchSlideEndTick) {
            smfInsertPitchBend(smf, seq->track[ev->track].tick, ev->track, ev->track, 0);
            seq->track[ev->track].pitchSlideEndTick = -1;
            seq->track[ev->track].lastPitch = 0;
        }
        // remember the key of new note
        if (note != 0x00) {
            seq->track[ev->track].lastNote = note;
        }
    }
    actualDuration = chunSpcActualDurationOf(seq->track[ev->track].duration, seq->track[ev->track].durationRate, seq->version);
    // note (not a rest)
    if (note != 0x00) {
        // tied note
        if (seq->track[ev->track].durationRate == 0 || note == 0x4f) {
            // beginning of tie
            if (seq->track[ev->track].tieDuration == 0) {
                seq->track[ev->track].tieBeginTick = ev->tick;
                seq->track[ev->track].tieTranspose = seq->track[ev->track].transpose;
            }
            seq->track[ev->track].tieDuration += actualDuration;
        }
        else {
            smfInsertChunSpcNote(smf, seq->track[ev->track].tick,
                ev->track, ev->track, note + seq->track[ev->track].transpose, actualDuration);
        }
    }
    // step
    seq->track[ev->track].tick += seq->track[ev->track].duration;
    smfSetEndTimingOfTrack(smf, ev->track, seq->track[ev->track].tick);

    if (note == 0x00)
        sprintf(ev->note, "Rest");
    else {
        if (note == 0x4f)
            sprintf(ev->note, "Tie ");
        else
            sprintf(ev->note, "Note ");
        getNoteName(argDumpStr, seq->track[ev->track].lastNote + SPC_NOTE_KEYSHIFT + seq->track[ev->track].transpose);
        strcat(ev->note, argDumpStr);
    }
    sprintf(argDumpStr, ", duration = %d%s", seq->track[ev->track].duration, seq->track[ev->track].refDuration ? " *" : "");
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " note");
}

/** vcmds A0-B5: set duration rate from table. */
static void chunSpcEventDurFromTable(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    const int durRateTable[] = {
        0x0D, 0x1A, 0x26, 0x33, 0x40, 0x4D, 0x5A, 0x66,
        0x73, 0x80, 0x8C, 0x99, 0xA6, 0xB3, 0xBF, 0xCC,
        0xD9, 0xE6, 0xF2, 0xFE, 0xFF, 0x00
    };
    int rate = durRateTable[ev->code - 0xa0];

    seq->track[ev->track].durationRate = rate;
    sprintf(ev->note, "Duration Rate, rate = %d/255", rate);
}

/** vcmd DB: flag repeat (alternative). */
static void chunSpcEventDB(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Flag Repeat (alternative)");
    chunSpcEventCondJump(seq, ev, smf, seq->track[ev->track].loopCountAlt);
}

/** vcmd DC: repeat once more (alternative). */
static void chunSpcEventDC(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Repeat Once More (alternative)");
    chunSpcEventCondJumpCnt(seq, ev, smf, 2, &seq->track[ev->track].loopCountAlt);
}

/** vcmd DD: set release rate (instance). */
static void chunSpcEventDDInline(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p] & 0x1f;
    (*p)++;

    sprintf(argDumpStr, ", release rate = %d (", arg1);
    strcat(ev->note, argDumpStr);
    sprintf(argDumpStr, "%.1f%s", (spcSRTable[arg1] >= 1) ? spcSRTable[arg1] : spcSRTable[arg1] * 1000,
        (spcSRTable[arg1] >= 1) ? "s" : "ms");
    strcat(ev->note, (arg1 == 0) ? "INF" : argDumpStr);
    strcat(ev->note, ")");
}

/** vcmd DD: set release rate. */
static void chunSpcEventDD(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    strcpy(ev->note, "Release Rate");
    chunSpcEventDDInline(seq, ev, smf);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd DE: set ADSR. */
static void chunSpcEventDE(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int *p = &seq->track[ev->track].pos;

    strcpy(ev->note, "ADSR");
    chunSpcEventEFInline(seq, ev, smf);
    chunSpcEventDDInline(seq, ev, smf);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd E0: CPU related conditional jump. */
static void chunSpcEventE0(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    int dest;
    bool jump;

    ev->size += 3;
    arg1 = utos2(mget2l(&seq->ARAM[*p]));
    (*p) += 2;
    arg2 = seq->ARAM[*p];
    (*p)++;

    dest = *p + arg1;
    jump = ((seq->cpuFlag & 0x7f) == arg2);
    if (jump) {
        *p = dest;
    }
    seq->cpuFlag |= 0x80;

    fprintf(stderr, "Warning: Conditional jump E0 is unstable at present. Be careful.\n");
    sprintf(ev->note, "Conditional Jump (CPU Related?), addr = $%04X%s", dest, jump ? "" : "*");
}

/** vcmd E2: set vibrato/tune parameters. */
static void chunSpcEventE2(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato/Tune, index = %d%s", arg1, (arg1 == 0xff) ? "OFF" : "");
}

/** vcmd E6: volume fade. */
static void chunSpcEventE6(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    int tick;
    int vol;
    int initVol;
    int lastVol;
    int targetVol;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    // FIXME: do realtime fade if needed
    tick = 0;
    targetVol = arg1;
    if (arg2 > 0) {
        initVol = seq->track[ev->track].volume;
        lastVol = chunSpcMidiVolExprOf(initVol);
        for(tick = 0; tick < arg2; tick++) {
            vol = (int) floor((targetVol - initVol) * ((double) tick / arg2) + initVol + 0.5);
            if (chunSpcMidiVolExprOf(vol) != lastVol) {
                smfInsertChunSpcExpression(smf, seq->track[ev->track].tick + tick, ev->track, ev->track, vol);
                lastVol = chunSpcMidiVolExprOf(vol);
            }
        }
    }
    vol = targetVol;
    smfInsertChunSpcExpression(smf, seq->track[ev->track].tick + tick, ev->track, ev->track, vol);
    seq->track[ev->track].volume = vol;

    sprintf(ev->note, "Volume Fade, target = %d, step = %d", arg1, arg2);
}

/** vcmd E8: pan fade. */
static void chunSpcEventE8(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    int tick;
    int pan;
    int initPan;
    int lastPan;
    int targetPan;

    ev->size += 2;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    // FIXME: do realtime fade if needed
    tick = 0;
    targetPan = arg1;
    if (arg2 > 0) {
        initPan = seq->track[ev->track].pan;
        lastPan = chunSpcMidiPanOf(initPan);
        for(tick = 0; tick < arg2; tick++) {
            pan = (int) floor((targetPan - initPan) * ((double) tick / arg2) + initPan + 0.5);
            if (chunSpcMidiPanOf(pan) != lastPan) {
                smfInsertChunSpcPanpot(smf, seq->track[ev->track].tick + tick, ev->track, ev->track, pan);
                lastPan = chunSpcMidiPanOf(pan);
            }
        }
    }
    pan = targetPan;
    smfInsertChunSpcPanpot(smf, seq->track[ev->track].tick + tick, ev->track, ev->track, pan);
    seq->track[ev->track].pan = pan;

    sprintf(ev->note, "Pan Fade, target = %d, step = %d", arg1, arg2);
}

/** vcmd EA: jump. */
static void chunSpcEventEA(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->ARAM[*p]));
    (*p) += 2;

    // assumes backjump = loop
    // FIXME: not always true!
    // see Boss Battle of Dragon Quest 5
    if (arg1 <= 0) {
        seq->track[ev->track].looped++;
    }
    dest = *p + arg1;
    *p = dest;

    sprintf(ev->note, "Jump, addr = $%04X", dest);
}

/** vcmd EB: tempo. */
static void chunSpcEventEB(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Tempo");
    if (seq->version < SPC_VER_KAMAITACHI) {
        seq->tempo = arg1;
        sprintf(argDumpStr, ", bpm = %d", arg1);
    }
    else {
        seq->tempo = seq->masterTempo * arg1 / 64;
        sprintf(argDumpStr, ", rate = %d/64 (%f%%)", arg1,
            (double) seq->tempo / seq->masterTempo);
    }
    strcat(ev->note, argDumpStr);

    smfInsertTempoBPM(smf, seq->track[ev->track].tick, ev->track, seq->tempo);
}

/** vcmd EC: set duration rate. */
static void chunSpcEventEC(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    seq->track[ev->track].durationRate = arg1;
    sprintf(ev->note, "Duration Rate, rate = %d/255", arg1);
}

/** vcmd ED: channel master volume. */
static void chunSpcEventED(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    seq->track[ev->track].masterVolume = arg1;
    smfInsertChunSpcVolume(smf, seq->track[ev->track].tick, ev->track, ev->track, arg1);
    sprintf(ev->note, "Channel Volume, val = %d", arg1);
}

/** vcmd EE: panpot. */
static void chunSpcEventEE(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;

    seq->track[ev->track].pan = arg1;
    smfInsertChunSpcPanpot(smf, seq->track[ev->track].tick, ev->track, ev->track, arg1);
    sprintf(ev->note, "Pan, val = %d", arg1);
}

/** vcmd EF: set ADSR (instance). */
static void chunSpcEventEFInline(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    bool adsr;
    int ar, dr, sl, sr;

    ev->size += 2;
    arg1 = mget2b(&seq->ARAM[*p]);
    (*p) += 2;

    adsr = (arg1 & 0x8000);
    ar = (arg1 & 0x0800) >> 8;
    dr = (arg1 & 0x7000) >> 12;
    sl = (arg1 & 0x00e0) >> 5;
    sr = (arg1 & 0x001f);

    sprintf(argDumpStr, ", mode = %s", adsr ? "ADSR" : "GAIN");
    strcat(ev->note, argDumpStr);
    if (adsr) {
        sprintf(argDumpStr, ", attack = %d (%.1f%s), decay = ", ar, (spcARTable[ar] >= 1) ? spcARTable[ar] : 
            spcARTable[ar] * 1000, (spcARTable[ar] >= 1) ? "s" : "ms");
        strcat(ev->note, argDumpStr);
        sprintf(argDumpStr, "%d (%.1f%s), sustain level = ", dr, (spcDRTable[dr] >= 1) ? spcDRTable[dr] : 
            spcDRTable[dr] * 1000, (spcDRTable[dr] >= 1) ? "s" : "ms");
        strcat(ev->note, argDumpStr);
        sprintf(argDumpStr, "%d/8, sustain rate = ", sl);
        strcat(ev->note, argDumpStr);
        sprintf(argDumpStr, "%d (%.1f%s)", sr, (spcSRTable[sr] >= 1) ? spcSRTable[sr] : 
            spcSRTable[sr] * 1000, (spcSRTable[sr] >= 1) ? "s" : "ms");
        strcat(ev->note, (sr == 0) ? "INF" : argDumpStr);
    }
}

/** vcmd EF: set ADSR. */
static void chunSpcEventEF(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    strcpy(ev->note, "Set ADSR");
    chunSpcEventEFInline(seq, ev, smf);
    if (!chunSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd F0: set patch. */
static void chunSpcEventF0(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    if (!chunSpcNoPatchChange) {
        smfInsertProgram(smf, seq->track[ev->track].tick, ev->track, ev->track, arg1);
    }
    seq->track[ev->track].patch = arg1;
    sprintf(ev->note, "Set Patch, patch = %d", arg1);
}

/** vcmd F1: duration copy on, or... */
static void chunSpcEventF1(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    if (seq->version >= SPC_VER_DQ5) {
        chunSpcEventF2(seq, ev, smf);
        return;
    }
    chunSpcEventUnknown0(seq, ev, smf);
}

/** vcmd F2: duration copy on. */
static void chunSpcEventF2(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    seq->track[ev->track].refDuration = true;

    // refresh duration info promptly
    if (ev->track > 0) {
        seq->track[ev->track].duration = seq->track[ev->track-1].duration;
        seq->track[ev->track].durationRate = seq->track[ev->track-1].durationRate;
    }

    sprintf(ev->note, "Duration Copy On");
}

/** vcmd F3: duration copy off. */
static void chunSpcEventF3(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    seq->track[ev->track].refDuration = false;
    sprintf(ev->note, "Duration Copy Off");
}

/** vcmd F4: repeat once more. */
static void chunSpcEventF4(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Repeat Once More");
    chunSpcEventCondJumpCnt(seq, ev, smf, 2, &seq->track[ev->track].loopCount);
}

/** vcmd F5: repeat n times. */
static void chunSpcEventF5(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Repeat N Times");
    sprintf(argDumpStr, ", count = %d", arg1);
    strcat(ev->note, argDumpStr);
    chunSpcEventCondJumpCnt(seq, ev, smf, arg1, &seq->track[ev->track].loopCount);
}

/** vcmd F6: volume. */
static void chunSpcEventF6(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    seq->track[ev->track].volume = arg1;
    smfInsertChunSpcExpression(smf, seq->track[ev->track].tick, ev->track, ev->track, arg1);
    sprintf(ev->note, "Volume, val = %d", arg1);
}

/** vcmd F7: nop, or... */
static void chunSpcEventF7(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    if (seq->version >= SPC_VER_DQ5) {
        chunSpcEventNOP(seq, ev, smf);
        return;
    }
    chunSpcEventUnknown1(seq, ev, smf);
}

/** vcmd F8: call subroutine. */
static void chunSpcEventF8(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int dest;

    ev->size += 2;
    arg1 = utos2(mget2l(&seq->ARAM[*p]));
    (*p) += 2;
    dest = *p + arg1;

    if (seq->track[ev->track].callSP < CMD_CALL_MAX) {
        seq->track[ev->track].retnAddr[seq->track[ev->track].callSP] = *p;
        seq->track[ev->track].callSP++;
        *p = dest;
    }

    sprintf(ev->note, "Call, addr = $%04X", dest);
}

/** vcmd F9: return from subroutine. */
static void chunSpcEventF9(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int *p = &seq->track[ev->track].pos;
    bool jump = false;

    if (seq->track[ev->track].callSP > 0) {
        seq->track[ev->track].callSP--;
        *p = seq->track[ev->track].retnAddr[seq->track[ev->track].callSP];
        jump = true;
    }

    sprintf(ev->note, "Return");
    if (jump) {
        sprintf(argDumpStr, ", addr = $%04X", *p);
        strcat(ev->note, argDumpStr);
    }
}

/** vcmd FA: key shift (transpose). */
static void chunSpcEventFA(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;

    seq->track[ev->track].transpose = arg1;
    sprintf(ev->note, "Transpose, val = %d", arg1);
}

/** vcmd FB: pitch slide. */
static void chunSpcEventFB(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    int pitch;
    int prevSlideMax;

    ev->size += 2;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    if (chunSpcPitchBendSens == 0) {
        prevSlideMax = seq->track[ev->track].pitchSlideMax;
        seq->track[ev->track].pitchSlideMax = min(abs(arg1), SMF_PITCHBENDSENS_MAX);
        if (seq->track[ev->track].pitchSlideMax != prevSlideMax) {
            smfInsertPitchBendSensitivity(smf, ev->tick, ev->track, ev->track, seq->track[ev->track].pitchSlideMax);
        }
    }
    if (seq->track[ev->track].pitchSlideMax > SMF_PITCHBENDSENS_MAX) {
        fprintf(stderr, "Warning: Pitch range over (%d), in tick %d at track %d\n", arg1, ev->tick, ev->track);
    }

    seq->track[ev->track].pitchSlideStartTick = ev->tick;
    seq->track[ev->track].pitchSlideEndTick = ev->tick + arg2;
    seq->track[ev->track].pitchSlideTarget = arg1 * 8192 / seq->track[ev->track].pitchSlideMax;
    sprintf(ev->note, "Pitch slide, key = %d, step = %d", arg1, arg2);

    pitch = (arg2 == 0) ? seq->track[ev->track].pitchSlideTarget : 0;
    if (pitch == 8192)
        pitch--;
    smfInsertPitchBend(smf, seq->track[ev->track].tick, ev->track, ev->track, pitch);
    seq->track[ev->track].lastPitch = pitch;
}

/** vcmd FF: end of track. */
static void chunSpcEventFF(ChunSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    seq->track[ev->track].hasEnd = true;
    sprintf(ev->note, "End of Track");
}

/** set pointers of each event. */
static void chunSpcInitEventList(ChunSpcSeqStat *seq, ChunSpcEvent *event)
{
    int code;

    // fill all with unidentified event
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (ChunSpcEvent) chunSpcEventUnidentified;
    }

    // note
    for(code = 0x00; code <= 0x9f; code++) {
        event[code] = (ChunSpcEvent) chunSpcEventNote;
    }
    // reserved
    for(code = 0xa0; code <= 0xdd; code++) {
        event[code] = (ChunSpcEvent) chunSpcEventReserved;
    }
    // duration rate
    if (seq->version >= SPC_VER_DQ5) {
        for(code = 0xa0; code <= 0xb5; code++) {
            event[code] = (ChunSpcEvent) chunSpcEventDurFromTable;
        }
    }
    // vcmd list
    if (seq->version >= SPC_VER_DQ5) {
        event[0xDB] = (ChunSpcEvent) chunSpcEventDB;
        event[0xDC] = (ChunSpcEvent) chunSpcEventDC;
    }
    event[0xDD] = (ChunSpcEvent) chunSpcEventDD;
    event[0xDE] = (ChunSpcEvent) chunSpcEventDE;
    event[0xDF] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xE0] = (ChunSpcEvent) chunSpcEventE0;
    event[0xE1] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xE2] = (ChunSpcEvent) chunSpcEventE2;
    event[0xE3] = (ChunSpcEvent) chunSpcEventUnknown1; // ?
    event[0xE4] = (ChunSpcEvent) chunSpcEventUnknown1; // ?
    event[0xE5] = (ChunSpcEvent) chunSpcEventUnknown2;
    event[0xE6] = (ChunSpcEvent) chunSpcEventE6;
    event[0xE7] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xE8] = (ChunSpcEvent) chunSpcEventE8;
    event[0xE9] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xEA] = (ChunSpcEvent) chunSpcEventEA;
    event[0xEB] = (ChunSpcEvent) chunSpcEventEB;
    event[0xEC] = (ChunSpcEvent) chunSpcEventEC;
    event[0xED] = (ChunSpcEvent) chunSpcEventED;
    event[0xEE] = (ChunSpcEvent) chunSpcEventEE;
    event[0xEF] = (ChunSpcEvent) chunSpcEventEF;
    event[0xF0] = (ChunSpcEvent) chunSpcEventF0;
    event[0xF1] = (ChunSpcEvent) chunSpcEventF1;
    event[0xF2] = (ChunSpcEvent) chunSpcEventF2;
    event[0xF3] = (ChunSpcEvent) chunSpcEventF3;
    event[0xF4] = (ChunSpcEvent) chunSpcEventF4;
    event[0xF5] = (ChunSpcEvent) chunSpcEventF5;
    event[0xF6] = (ChunSpcEvent) chunSpcEventF6;
    event[0xF7] = (ChunSpcEvent) chunSpcEventF7;
    event[0xF8] = (ChunSpcEvent) chunSpcEventF8;
    event[0xF9] = (ChunSpcEvent) chunSpcEventF9;
    event[0xFA] = (ChunSpcEvent) chunSpcEventFA;
    event[0xFB] = (ChunSpcEvent) chunSpcEventFB;
    event[0xFC] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xFD] = (ChunSpcEvent) chunSpcEventUnknown0;
    event[0xFE] = (ChunSpcEvent) chunSpcEventUnknown1;
    event[0xFF] = (ChunSpcEvent) chunSpcEventFF;
}

//----

/** convert summer/winter spc to midi data from ARAM (65536 bytes). */
Smf* chunSpcARAMToMidi(const byte *ARAM)
{
    Smf* smf = NULL;
    ChunSpcSeqStat seq;
    ChunSpcEvent event[256];
    bool trackHasEnd[8];
    bool seqHasEnd = false;
    bool inSub;
    int seqLooped = 0;
    int minTickStep;
    int tick;
    int tr;
    int i;

    printHtmlHeader();
    myprintf("    <h1>Chunsoft summer/winter SPC2MIDI %s</h1>\n", VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by chunspc. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", mycssfile);

    seq.ARAM = (byte *) ARAM;
    chunSpcCheckVer(&seq);
    if (seq.version == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Unsupported version.\n");
        goto abort;
    }
    if (!chunSpcDetectSeq(&seq)) {
        fprintf(stderr, "Error: Unable to find sequence data.\n");
        goto abort;
    }

    smf = smfCreate(SPC_TIMEBASE);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, "Chunsoft summer/winter SPC2MIDI");
    smfInsertTempoBPM(smf, 0, 0, seq.tempo);
    smfInsertGM1SystemOn(smf, 0, 0, 0);
    for (tr = 0; tr < seq.numOfTracks; tr++) {
        static char trackName[256];

        trackHasEnd[tr] = false;

        if (chunSpcNoPatchChange) {
            smfInsertProgram(smf, 0, tr, tr, 48);
        }
        smfInsertChunSpcVolume(smf, 0, tr, tr, seq.track[tr].masterVolume);
        smfInsertChunSpcExpression(smf, 0, tr, tr, seq.track[tr].volume);

        smfInsertPitchBend(smf, 0, tr, tr, 0);
        if (chunSpcPitchBendSens != 0) {
            smfInsertPitchBendSensitivity(smf, 0, tr, tr, seq.track[tr].pitchSlideMax);
        }

        sprintf(trackName, "Track %d : $%04X", tr + 1, seq.track[tr].pos);
        smfInsertMetaText(smf, 0, tr, SMF_META_TRACKNAME, trackName);
    }

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\">\n");
    myprintf("        <ul>\n");
    myprintf("          <li>Version: %s</li>\n", chunSpcVerToStr(seq.version));
    myprintf("          <li>Sequence header: $%04X</li>\n", seq.addrOfHeader);
    myprintf("          <li>Initial tempo: %d</li>\n", seq.tempo);
    myprintf("          <li>Number of tracks: %d</li>\n", seq.numOfTracks);
    myprintf("        </ul>\n");
    myprintf("      </div>\n\n");

    myprintf("      <h2>Data Dump</h2>\n");
    myprintf("      <div class=\"section\">\n");
    myprintf("        <p>You can filter output by using stylesheet. Write %s as you like!</p>\n", mycssfile);

    myprintf("        <table class=\"dump\">\n");
    myprintf("          <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");

    chunSpcInitEventList(&seq, event);

    while(!seqHasEnd) {

        SeqEventReport ev;

        for (ev.track = 0; ev.track < seq.numOfTracks; ev.track++) {

            if (seq.track[ev.track].refDuration && ev.track > 0) {
                seq.track[ev.track].duration = seq.track[ev.track-1].duration;
                seq.track[ev.track].durationRate = seq.track[ev.track-1].durationRate;
            }

            while (!seqHasEnd && !seq.track[ev.track].hasEnd && seq.track[ev.track].tick <= seq.tick) {

                ev.tick = seq.tick;
                ev.addr = seq.track[ev.track].pos;
                ev.size = 0;
                ev.unidentified = false;
                strcpy(ev.note, "");

                inSub = (seq.track[ev.track].callSP > 0) || (seq.track[ev.track].loopCount > 0);

                // read first byte
                ev.size++;
                ev.code = ARAM[ev.addr];
                sprintf(ev.classStr, "ev%02X", ev.code);
                seq.track[ev.track].pos++;
                // in subroutine?
                strcat(ev.classStr, inSub ? " sub" : "");

                // dispatch event
                event[ev.code](&seq, &ev, smf);

                if (chunSpcTextLoopMax == 0 || seqLooped < chunSpcTextLoopMax) {
                    myprintf("          <tr class=\"track%d %s\">", ev.track + 1, ev.classStr);
                    myprintf("<td class=\"track\">%d</td>", ev.track + 1);
                    myprintf("<td class=\"tick\">%d</td>", ev.tick);
                    myprintf("<td class=\"address\">$%04X</td>", ev.addr);
                    myprintf("<td class=\"hex\">");
                    for (i = 0; i < ev.size; i++) {
                        if (i > 0)
                            myprintf(" ");
                        myprintf("%02X", ARAM[ev.addr + i]);
                    }
                    myprintf("</td>");
                    myprintf("<td class=\"note\">%s</td>", ev.note);
                    myprintf("</tr>\n");
                }

                if (ev.unidentified)
                    goto quitConversion;

                // check loop count for all
                seqLooped = seq.track[0].looped;
                for (tr = 1; tr < seq.numOfTracks; tr++) {
                    seqLooped = min(seqLooped, seq.track[tr].looped);
                }

                // check if each track has end
                for (tr = 0; tr < seq.numOfTracks; tr++) {
                    trackHasEnd[tr] = seq.track[tr].hasEnd || (chunSpcLoopMax > 0 && seq.track[tr].looped >= chunSpcLoopMax);
                }

                // check if all tracks has end
                seqHasEnd = true;
                for (tr = 0; tr < seq.numOfTracks; tr++) {
                    if (!trackHasEnd[tr]) {
                        seqHasEnd = false;
                        break;
                    }
                }
            }
        }

        // step, faster than seq.tick++;
        minTickStep = 0;
        for (tr = 0; tr < seq.numOfTracks; tr++) {
            if (!seq.track[tr].hasEnd) {
                if (minTickStep == 0)
                    minTickStep = seq.track[tr].tick - seq.tick;
                else
                    minTickStep = min(minTickStep, seq.track[tr].tick - seq.tick);
            }
        }

        // faders
        for (tr = 0; tr < seq.numOfTracks; tr++) {
            for (tick = seq.tick; tick < seq.tick + minTickStep; tick++) {
                // pitch slide
                if (seq.track[tr].pitchSlideEndTick >= 0) {
                    int pitch;
                    int targetPitch = seq.track[tr].pitchSlideTarget;
                    int step = seq.track[tr].pitchSlideEndTick - seq.track[tr].pitchSlideStartTick;

                    if (tick > seq.track[tr].pitchSlideEndTick)
                        break;

                    pitch = (int) floor(targetPitch * ((double) (tick - seq.track[tr].pitchSlideStartTick) / step) + 0.5);
                    if (pitch != seq.track[tr].lastPitch) {
                        if (pitch == 8192)
                            pitch--;
                        smfInsertPitchBend(smf, tick, tr, tr, pitch);
                        seq.track[tr].lastPitch = pitch;
                    }
                }
            }
        }

        seq.tick += minTickStep;
        seq.time += (double) 60 / seq.tempo * minTickStep / SPC_TIMEBASE;
        // check time limit
        if (seq.time >= chunSpcTimeLimit) {
            seqHasEnd = true;
        }
    }

quitConversion:

    // finalize for all tied notes
    for (tr = 0; tr < seq.numOfTracks; tr++) {
        chunSpcDequeueTiedNote(&seq, tr, smf);
    }

    myprintf("        </table>\n");
    if (seqHasEnd) {
        myprintf("        <p>Congratulations! MIDI conversion went successfully!</p>\n");
    }
    else {
        myprintf("        <p>Conversion aborted! Apparently something went wrong...</p>\n");
    }
    myprintf("      </div>\n");

finalize:
    myprintf("    </div>\n");
    printHtmlFooter();
    return smf;

abort:
    if (smf != NULL) {
        smfDelete(smf);
        smf = NULL;
    }
    goto finalize;
}

/** convert summer/winter spc to midi data from SPC file located in memory. */
Smf* chunSpcToMidi(const byte *data, size_t size)
{
    Smf* smf = NULL;

    if (!isSpcSoundFile(data, size)) {
        goto finalize;
    }

    smf = chunSpcARAMToMidi(&data[0x0100]);

finalize:

    return smf;
}

/** convert summer/winter spc to midi data from SPC file. */
Smf* chunSpcToMidiFromFile(const char *filename)
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

    smf = chunSpcToMidi(data, size);

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

/** handle command-line options. */
static bool handleCmdLineOpts(int *argc, char **argv[], const char *cmd)
{
    bool dispatched = false;

    (*argv)++; (*argc)--;
    while(*argc && (*argv)[0][0] == '-') {
        if (strcmp((*argv)[0], "--help") == 0) {
            man(cmd);
            dispatched = true;
        }
        else if (strcmp((*argv)[0], "--loopcount") == 0) {
            if (*argc <= 2) {
                fprintf(stderr, "Error: invalid use of \"--loopcount [count=0-]\"\n");
            }
            else {
                chunSpcSetLoopCount(atoi((*argv)[1]));
                dispatched = true;
                (*argv)++; (*argc)--;
            }
        }
        else if (strcmp((*argv)[0], "--nopatch") == 0) {
            chunSpcNoPatchChange = true;
            dispatched = true;
        }
        else if (strcmp((*argv)[0], "--voloffset") == 0) {
            if (*argc <= 2) {
                fprintf(stderr, "Error: invalid use of \"--voloffset [offset=0-127]\"\n");
            }
            else {
                chunSpcChannelVolOffset = atoi((*argv)[1]);
                dispatched = true;
                (*argv)++; (*argc)--;
            }
        }
        else if (strcmp((*argv)[0], "--linear") == 0) {
            chunSpcVolIsLinear = true;
            dispatched = true;
        }
        else if (strcmp((*argv)[0], "--bendrange") == 0) {
            if (*argc <= 2) {
                fprintf(stderr, "Error: invalid use of \"--bendrange [range=0-24]\"\n");
            }
            else {
                chunSpcPitchBendSens = atoi((*argv)[1]);
                dispatched = true;
                (*argv)++; (*argc)--;
            }
        }
        else {
            fprintf(stderr, "Error: unknown option \"%s\"\n", (*argv)[0]);
        }
        (*argv)++; (*argc)--;
    }
    return dispatched;
}

/** display how to use. */
void man(const char *cmd)
{
    const char *cmdname = "chunspc";
    //const char *cmdname = (cmd != NULL) ? cmd : "chunspc";

    fprintf(stderr, "chunspc - Chunsoft summer/winter SPC2MIDI %s\n", VERSION);
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
    fprintf(stderr, "http://loveemu.yh.land.to/\n");

    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "--help         show usage\n");
    fprintf(stderr, "--loopcount    loop count of midi output\n");
    fprintf(stderr, "--nopatch      disable program change\n");
    fprintf(stderr, "--voloffset N  offset of midi volume\n");
    fprintf(stderr, "--linear       assume midi volume is linear\n");
    fprintf(stderr, "--bendrange N  pitch bend sensitivity (0:auto)\n");
}

/** display about chunspc application. */
void about(const char *cmd)
{
    const char *cmdname = "chunspc";
    //const char *cmdname = (cmd != NULL) ? cmd : "chunspc";

    fprintf(stderr, "chunspc - Chunsoft summer/winter SPC2MIDI %s\n", VERSION);
    fprintf(stderr, "Programmed by loveemu - http://loveemu.yh.land.to/\n");
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
}

/** chunspc application main. */
int main(int argc, char *argv[])
{
    Smf* smf;
    FILE *htmlFile = NULL;
    const char *cmd = (const char *) argv[0];
    bool cmdDispatched;

    cmdDispatched = handleCmdLineOpts(&argc, &argv, cmd);

    if (argc < 2 || argc > 3) {
        if (!cmdDispatched) {
            about(cmd);
            fprintf(stderr, "Run with --help, for more details.\n");
        }
        else {
            fprintf(stderr, "Error: too few/many arguments.\n");
        }
        return EXIT_SUCCESS;
    }

    if (argc >= 3) {
        htmlFile = fopen(argv[2], "w");
        if (htmlFile != NULL)
            chunSpcSetLogStreamHandle(htmlFile);
    }
    else {
        //chunSpcSetLogStreamHandle(stdout);
        chunSpcSetLogStreamHandle(NULL);
    }

    fprintf(stderr, "%s: Conversion started!\n", argv[0]);
    smf = chunSpcToMidiFromFile(argv[0]);
    if (smf != NULL) {
        smfWriteFile(smf, argv[1]);
    }
    else {
        fprintf(stderr, "Error: Conversion failed.\n");
    }

    if (htmlFile != NULL) {
        fclose(htmlFile);
    }

    return EXIT_SUCCESS;
}
