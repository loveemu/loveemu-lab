/**
 * Nintendo spc2midi.
 * http://loveemu.yh.land.to/
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "nintspc.h"

#define VERSION "[2008-XX-XX]"

// MIDI limitations
#define SMF_PITCHBENDSENS_DEFAULT   2
#define SMF_PITCHBENDSENS_MAX       24

static int nintSpcLoopMax = 2;              // maximum loop count of parser
static int nintSpcTextLoopMax = 1;          // maximum loop count of text output
static double nintSpcTimeLimit = 1200;      // time limit of conversion (for safety)
static int nintSpcSongIndex = 0;            // song index of now playing (0 for autosearch)
static bool nintSpcLessTextInSMF = false;   // decreases amount of texts in SMF output

static int nintSpcPitchBendSens = 0;        // amount of pitch bend sensitivity (0=auto; <=SMF_PITCHBENDSENS_MAX)
static bool nintSpcNoPatchChange = false;   // XXX: hack, should be false for serious conversion
static bool nintSpcVolIsLinear = false;     // assumes volume curve between SPC and MIDI is linear

//----

#define SPC_VER_UNKNOWN     0
#define SPC_VER_SMW         1
#define SPC_VER_PTWS        2
#define SPC_VER_FZERO       3
#define SPC_VER_SC          4
#define SPC_VER_TEST        5
#define SPC_VER_LOZ         6
#define SPC_VER_MPNT        7
#define SPC_VER_SMK         8
#define SPC_VER_SF          9
#define SPC_VER_SMAS        10
#define SPC_VER_FE3         11
#define SPC_VER_SMET        12
#define SPC_VER_WT          13
#define SPC_VER_MO2         14
#define SPC_VER_KDC         15
#define SPC_VER_SNC         16
#define SPC_VER_YI          17
#define SPC_VER_KSS         18
#define SPC_VER_PDP         19
#define SPC_VER_TA          20
#define SPC_VER_FE4         21
#define SPC_VER_MVLS        22
#define SPC_VER_DOH         23
#define SPC_VER_BFN1        24
#define SPC_VER_TDM         25
#define SPC_VER_ZTM         26
#define SPC_VER_PLR         27
#define SPC_VER_PSB         28
#define SPC_VER_KKK         29
#define SPC_VER_STH         30

// any changes are not needed normally
#define SPC_TIMEBASE        48
#define SPC_SONG_MAX        0x80
#define SPC_TRACK_MAX       8
#define SPC_NOTE_KEYSHIFT   24

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
static const char *mycssfile = "nintspc.css";

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

typedef struct TagNintSpcTrackStat NintSpcTrackStat;
typedef struct TagNintSpcSeqStat NintSpcSeqStat;
typedef void (*NintSpcEvent) (NintSpcSeqStat *, SeqEventReport *, Smf *);

typedef struct TagNintSpcVerInfo {
    int gameId;         // game title number (SPC_VER_*)
    NintSpcEvent event[256]; // vcmds
} NintSpcVerInfo;

struct TagNintSpcTrackStat {
    int pos;            // current address on ARAM
    int tick;           // timing (must be synchronized with seq)
    int prevTick;       // previous timing (for pitch slide)
    int duration;       // length of note (tick)
    int durRate;        // duration rate (0-255)
    int velocity;       // velocity of note (0-255)
    int lastNoteTick;   // timing of the last note (tick)
    int lastNoteDur;    // length of the last note
    int lastNote;       // note number of the last note
    int lastLen;        // last length of the note cmd
    int lastDurRate;    // duration rate of the last note
    int lastVel;        // velocity of the last note
    bool ignoreRest;    // if 'rest' keeps current note (usually no, of course)
    bool ignoreNotePrm; // if note param vcmd works as a note cmd
    bool lastIsPerc;    // if the last note is percussion one
    bool tied;          // if the last note tied
    int transpose;      // transpose (note number, signed)
    int volume;         // volume (0-255)
    int patch;          // patch number (for pitch fix)
    bool portamento;    // portamento
    int loopStart;      // loop start address for loop command
    int loopCount;      // repeat count for loop command
    int retnAddr;       // return address for loop command
};

struct TagNintSpcSeqStat {
    byte* ARAM;                 // ARAM (65536 bytes)
    int songIndex;              // song index in song table
    int tick;                   // timing (tick)
    double time;                // timing (s)
    int tempo;                  // current tempo (bpm)
    int masterVolume;           // master volume
    int transpose;              // key shift (note number, signed)
    int addrOfHeader;           // sequence header address
    int blockPtr;               // current pos in block list
    int blockPtrAlt;            // alternate blockPtr (for logging)
    int blockLoopCnt;           // repeat count for block
    bool endBlock;              // if reached to end of block
    bool newNotePrm;            // if note param is Fire Emblem style
    int looped;                 // how many times the song looped (internal)
    int blockLooped;            // how many times looped block (internal)
    NintSpcVerInfo ver;         // game version info
    NintSpcTrackStat track[SPC_TRACK_MAX]; // status of each tracks
};

void getNoteName(char *name, int note);
bool isSpcSoundFile(const byte *data, size_t size);
static void nintSpcInitTrack(NintSpcSeqStat *seq, int track);
bool nintSpcDetectSeq(NintSpcSeqStat *seq);

static const char *nintSpcVerToStr(int version);
static int nintSpcCheckVer(NintSpcSeqStat *seq);
static void printHtmlHeader(void);
static void printHtmlFooter(void);
static void printEventTableHeader(NintSpcSeqStat *seq, Smf* smf);
static void printEventTableFooter(NintSpcSeqStat *seq, Smf* smf);

static int nintSpcNoteInfoByteMin(int version);
static int nintSpcNoteInfoByteMax(int version);
static int nintSpcNoteByteMin(int version);
static int nintSpcNoteByteMax(int version);
static int nintSpcTieByte(int version);
static int nintSpcRestByte(int version);
static int nintSpcPercByteMin(int version);
static int nintSpcPercByteMax(int version);
static int nintSpcIsPitchSlideByte(int vbyte, int version);
static int nintSpcDurRateOf(NintSpcSeqStat *seq, int index);
static int nintSpcVelRateOf(NintSpcSeqStat *seq, int index);
static int nintSpcDefaultTempo(int version);
static double nintSpcBpmOfTempo(int tempo);
static int nintSpcMidiVelOf(int value);
static int nintSpcMidiVolOf(int value);
static void nintSpcTruncateNote(NintSpcSeqStat *seq, int track);
static bool nintSpcDequeueNote(NintSpcSeqStat *seq, int track, Smf* smf);
static bool smfInsertNintSpcNote(Smf* seq, int time, int channel, int track, int note, bool perc, int velocity, int duration);

static void nintSpcEventUnknownInline(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventUnidentified(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventUnknown0(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventUnknown1(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventUnknown2(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventUnknown3(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventUnknown36(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventNOP(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventReserved(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventEndOfBlock(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventNoteInfo(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventNote(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTie(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventRest(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPercNote(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventSetPatch(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventSetPatchWithADSR(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPanpot(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPanFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventVibratoOn(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventVibratoOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventMasterVolume(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventMasterVolFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTempo(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTempoFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventKeyShift(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTranspose(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTremoloOn(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTremoloOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventVolume(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventVolumeFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventSubroutine(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventVibratoFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPitchEnvR(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPitchEnvA(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPitchEnvOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTuning(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventEchoVol(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventEchoOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventEchoParam(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventEchoVolFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventPitchSlide(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventSetPercBase(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventSkip2(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventShortJump(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventFE3FA(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTAFC(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcEventTAFD(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf);
static void nintSpcInitEventList(NintSpcSeqStat *seq);

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
FILE *nintSpcSetLogStreamHandle(FILE *stream)
{
    FILE *oldStream;

    oldStream = mystdout;
    mystdout = stream;
    return oldStream;
}

/** sets loop count of MIDI output. */
int nintSpcSetLoopCount(int count)
{
    int oldLoopCount;

    oldLoopCount = nintSpcLoopMax;
    nintSpcLoopMax = count;
    return oldLoopCount;
}

/** sets song index to convert. */
int nintSpcSetSongIndex(int index)
{
    int oldSongIndex;

    oldSongIndex = nintSpcSongIndex;
    if (index >= 0)
        nintSpcSongIndex = index;
    return oldSongIndex;
}

/** returns version string of music engine. */
static const char *nintSpcVerToStr(int version)
{
    switch (version) {
    case SPC_VER_SMW:
        return "Super Mario World";
    case SPC_VER_PTWS:
        return "Pilotwings";
    case SPC_VER_FZERO:
        return "F-Zero";
    case SPC_VER_SC:
        return "Sim City";
    case SPC_VER_TEST:
        return "SNES Test Program";
    case SPC_VER_LOZ:
        return "Legend of Zelda";
    case SPC_VER_MPNT:
        return "Mario Paint";
    case SPC_VER_SMK:
        return "Super Mario Kart";
    case SPC_VER_SF:
        return "Star Fox";
    case SPC_VER_SMAS:
        return "Super Maril All-Stars";
    case SPC_VER_FE3:
        return "Fire Emblem: Monshou no Nazo";
    case SPC_VER_SMET:
        return "Super Metroid";
    case SPC_VER_WT:
        return "Stunt Race FX";
    case SPC_VER_MO2:
        return "Earthbound";
    case SPC_VER_KDC:
        return "Kirby's Dream Course";
    case SPC_VER_SNC:
        return "Snoopy Concert";
    case SPC_VER_YI:
        return "Yoshi's Island";
    case SPC_VER_PDP:
        return "Panel de Pon";
    case SPC_VER_TA:
        return "Tetris Attack";
    case SPC_VER_KSS:
        return "Kirby Super Star / Kirby's Dream Land 3";
    case SPC_VER_FE4:
        return "Fire Emblem: Seisen no Keifu / Super Famicom Wars / Fire Emblem: Tracia 776";
    case SPC_VER_MVLS:
        return "Marvelous: Mouhitotsu no Takarajima";
    case SPC_VER_BFN1:
        return "Itoi Shigesato no Bass Tsuri No.1";
    case SPC_VER_TDM:
        return "Tetris &amp; Dr. Mario";
    case SPC_VER_ZTM:
        return "Zoot Mahjong";
    case SPC_VER_KKK:
        return "Kirby no Kira Kira Kids";
    case SPC_VER_STH:
        return "Sutte Hakkun";
    default:
        return "unknown version";
    }
}

/** returns what version the sequence is. */
static int nintSpcCheckVer(NintSpcSeqStat *seq)
{
    int version = SPC_VER_UNKNOWN;

    if (memcmp(&seq->ARAM[0x0549], "\xec\xfd\x00\xf0\xfb\x6d", 6) == 0)
        version = SPC_VER_SMW;
    if (memcmp(&seq->ARAM[0x0548], "\xec\xfd\x00\xf0\xfb\x6d", 6) == 0)
        version = SPC_VER_PTWS;
    else if (memcmp(&seq->ARAM[0x082d], "\x3f\x28\x17\xec\xfd\x00\xf0\xf8", 8) == 0)
        version = SPC_VER_FZERO;
    else if (memcmp(&seq->ARAM[0x0844], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0) {
        if (memcmp(&seq->ARAM[0x0854], "\xf6\xd4\x1a", 3) == 0) {
            version = SPC_VER_SC;
        }
        else {
            version = SPC_VER_LOZ;
        }
    }
    else if (memcmp(&seq->ARAM[0x083a], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_TEST;
    else if (memcmp(&seq->ARAM[0x0451], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_SF;
    else if (memcmp(&seq->ARAM[0x0544], "\xe4\x1b\xd0\xf9\x8d\x0a", 6) == 0)
        version = SPC_VER_MPNT;
    else if (memcmp(&seq->ARAM[0x087b], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_SMK;
    else if (memcmp(&seq->ARAM[0x0547], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_SMAS;
    else if (memcmp(&seq->ARAM[0x044a], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_FE3;
    else if (memcmp(&seq->ARAM[0x158e], "\xd0\x6e\x8d\x0a\xad\x05", 6) == 0)
        version = SPC_VER_SMET;
    else if (memcmp(&seq->ARAM[0x0868], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_WT;
    else if (memcmp(&seq->ARAM[0x0546], "\xe4\x1b\xd0\xf9\x8d\x0a", 6) == 0) {
        if (seq->ARAM[0x0544] == 0xd8)
            version = SPC_VER_MO2;
        else
            version = SPC_VER_SNC;
    }
    else if (memcmp(&seq->ARAM[0x0839], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_KDC;
    else if (memcmp(&seq->ARAM[0x0444], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_YI;
    else if (memcmp(&seq->ARAM[0x084f], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_PDP;
    else if (memcmp(&seq->ARAM[0x0754], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_TA;
    else if (memcmp(&seq->ARAM[0x0732], "\xfa\x02\xf6\xfa\xf6\x02", 6) == 0) {
        if (mget2l(&seq->ARAM[0x08e9]) == 0x028f)
            version = SPC_VER_KKK;
        else
            version = SPC_VER_KSS;
    }
    else if (memcmp(&seq->ARAM[0x06a4], "\x8d\x09\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_FE4;
    else if (memcmp(&seq->ARAM[0x04a3], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_MVLS;
    else if (memcmp(&seq->ARAM[0x082f], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_BFN1;
    else if (memcmp(&seq->ARAM[0x086c], "\x8d\x0a\xad\x05\xf0\x07", 6) == 0)
        version = SPC_VER_TDM;
    else if (memcmp(&seq->ARAM[0x0874], "\xe4\x00\x1c\x5d\x1f", 5) == 0)
        version = SPC_VER_ZTM;
    else if (memcmp(&seq->ARAM[0x075a], "\xe8\x7f\x8d\x0c\x3f", 5) == 0)
        version = SPC_VER_STH;

    seq->ver.gameId = version;
    return version;
}

/** initialize track params. */
static void nintSpcInitTrack(NintSpcSeqStat *seq, int track)
{
    // FIXME: most of these are still not changed from hbdqspc
    seq->track[track].tick = 0;
    seq->track[track].prevTick = seq->track[track].tick;
    seq->track[track].duration = 1;
    //seq->track[track].durRate = 0; // just in case
    //seq->track[track].velocity = 0; // just in case
    //seq->track[track].lastNoteTick = 0; // XXX
    //seq->track[track].lastNoteDur = 0; // XXX
    seq->track[track].lastNote = -1;
    //seq->track[track].lastLen = 0; // just in case
    //seq->track[track].lastDurRate = 0; // just in case
    //seq->track[track].lastVel = seq->track[track].velocity;
    seq->track[track].ignoreRest = false;
    seq->track[track].ignoreNotePrm = false;
    seq->track[track].volume = 0xff;
    seq->track[track].patch = 0;
    seq->track[track].transpose = 0;
    seq->track[track].portamento = false; // XXX
    seq->track[track].loopCount = 0; // just in case
    seq->track[track].retnAddr = 0; // just in case
}

/** read next block from block ptr. */
bool nintSpcReadNewBlock(NintSpcSeqStat *seq, Smf *smf)
{
    byte *ARAM = seq->ARAM;
    int blockAddr;
    bool nullSong = true;
    int tr;

    if (seq->blockLoopCnt == 0) {
        // read next block
        seq->blockPtr += 2; // skip old block addr
        seq->blockPtrAlt = seq->blockPtr;
        seq->blockLooped = 0;

        blockAddr = mget2l(&ARAM[seq->blockPtr]);
        if (blockAddr == 0)
            return false;
        else if ((blockAddr & 0xff00) == 0) {
            seq->blockLoopCnt = utos1(blockAddr & 0xff);
            seq->blockPtr += 2;
            blockAddr = mget2l(&ARAM[seq->blockPtr]);
            if (blockAddr == 0)
                return false;

            if (seq->blockLoopCnt < 0) {
                seq->looped++;
                if (nintSpcLoopMax > 0 && seq->looped >= nintSpcLoopMax)
                    return false;

                seq->blockLoopCnt = 1;
                seq->blockPtr = blockAddr;
                return nintSpcReadNewBlock(seq, smf);
            }
        }
    }
    else {
        // repeat block
        if (seq->blockLoopCnt > 0)
            seq->blockLoopCnt--;
        seq->blockLooped++;
        blockAddr = mget2l(&ARAM[seq->blockPtr]);
    }

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        // cancel repeats
        seq->track[tr].loopCount = 0;

        // if not used (hi-byte is zero)
        if (!mget1(&ARAM[blockAddr + tr * 2 + 1])) {
            // if the track 'was' used
            if (seq->track[tr].pos) {
                nintSpcDequeueNote(seq, tr, smf);
            }
            seq->track[tr].pos = 0;
            continue;
        }
        seq->track[tr].pos = mget2l(&ARAM[blockAddr + tr * 2]);
    }
    seq->endBlock = false;

    // if seq is null anyway, return false
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        if (seq->track[tr].pos) {
            nullSong = false;
            break;
        }
    }

    return !nullSong;
}

/** detects now playing and find sequence header for it. */
bool nintSpcDetectSeq(NintSpcSeqStat *seq)
{
    byte *ARAM = seq->ARAM;
    int addrOfSeqHedPtr;
    int addrOfCurBlockPtr;
    int headerOfs;
    int songIndex;
    int tr;

    switch(seq->ver.gameId) {
        case SPC_VER_SMW:
            addrOfSeqHedPtr = 0x135e; //mget2l(&ARAM[0x0b62]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_PTWS:
            addrOfSeqHedPtr = 0x167e; //mget2l(&ARAM[0x0ef6]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_FZERO:
            addrOfSeqHedPtr = 0x1fd6; //mget2l(&ARAM[0x0af6]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_SC:
            addrOfSeqHedPtr = mget2l(&ARAM[0x13d8]); // 0x1b0e;
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_TEST:
            addrOfSeqHedPtr = mget2l(&ARAM[0x09d6]); // 0x1134;
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_LOZ:
            addrOfSeqHedPtr = 0xcffe; // mget2l(&ARAM[0x0aaf]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_MPNT:
            addrOfSeqHedPtr = 0x26c1; // mget2l(&ARAM[0x0775]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_SMK:
            addrOfSeqHedPtr = 0x1564; // mget2l(&ARAM[0x0a23]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_SF:
            addrOfSeqHedPtr = 0xfdbe; // mget2l(&ARAM[0x077d]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_SMAS:
            addrOfSeqHedPtr = 0xbffe; // mget2l(&ARAM[0x0863]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_FE3:
            addrOfSeqHedPtr = 0x195e; // mget2l(&ARAM[0x0662]);
            addrOfCurBlockPtr = 0x1f;
            break;
        case SPC_VER_SMET:
            addrOfSeqHedPtr = mget2l(&ARAM[0x1749]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_WT:
            addrOfSeqHedPtr = 0x07ea; // mget2l(&ARAM[0x0a51]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_MO2:
            addrOfSeqHedPtr = mget2l(&ARAM[0x0780]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_KDC:
            addrOfSeqHedPtr = mget2l(&ARAM[0x09d5]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_SNC:
            addrOfSeqHedPtr = 0x283b; // mget2l(&ARAM[0x077d]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_YI:
            addrOfSeqHedPtr = 0xff8e; // mget2l(&ARAM[0x06fc]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_PDP:
            addrOfSeqHedPtr = 0x1d3e; // mget2l(&ARAM[0x0a36]);
            addrOfCurBlockPtr = 0x28;
            break;
        case SPC_VER_TA:
            addrOfSeqHedPtr = 0xff4b; // mget2l(&ARAM[0x0939]);
            addrOfCurBlockPtr = 0x25;
            break;
        case SPC_VER_KSS:
            addrOfSeqHedPtr = mget2l(&ARAM[0x08e9]);
            addrOfCurBlockPtr = 0x30;
            break;
        case SPC_VER_FE4:
            addrOfSeqHedPtr = 0xff7e; // mget2l(&ARAM[0x0849]);
            addrOfCurBlockPtr = 0x24;
            break;
        case SPC_VER_MVLS:
            addrOfSeqHedPtr = 0x2866; //mget2l(&ARAM[0x06d5]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_BFN1:
            addrOfSeqHedPtr = 0x43fe; //mget2l(&ARAM[0x0a11]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_TDM:
            addrOfSeqHedPtr = 0x1140; //mget2l(&ARAM[0x0a1a]);
            addrOfCurBlockPtr = 0x40;
            break;
        case SPC_VER_ZTM:
            addrOfSeqHedPtr = 0x1816; //mget2l(&ARAM[0x175b]);
            addrOfCurBlockPtr = 0x30;
            break;
        case SPC_VER_KKK:
            addrOfSeqHedPtr = mget2l(&ARAM[0x08e5]);
            addrOfCurBlockPtr = 0x30;
            break;
        case SPC_VER_STH:
            addrOfSeqHedPtr = 0x307e; //mget2l(&ARAM[0x0a6a]);
            addrOfCurBlockPtr = 0x40;
            break;
        default:
            return false;
    }

    songIndex = nintSpcSongIndex;
    if (nintSpcSongIndex == 0) {
        int songId;
        int dist, minDist = 0x10000;
        int curBlock = mget2l(&ARAM[addrOfCurBlockPtr]);
        int songAddr;

        // adjust current block
        if (curBlock) {
            curBlock -= 2;
        }

        // auto search
        songIndex = 1;
        for (songId = 1; songId < SPC_SONG_MAX; songId++) {
            songAddr = mget2l(&ARAM[addrOfSeqHedPtr + songId * 2]);
            dist = abs(curBlock - songAddr);
            if (songAddr > curBlock) {
                dist++; // penalty
            }
            if (dist < minDist) {
                songIndex = songId;
                minDist = dist;
            }
        }
    }

    headerOfs = mget2l(&ARAM[addrOfSeqHedPtr + songIndex * 2]);
    seq->songIndex = songIndex;
    seq->addrOfHeader = headerOfs;
    seq->blockPtr = headerOfs;
    seq->blockPtrAlt = seq->blockPtr;
    seq->blockLoopCnt = 0;
    seq->endBlock = false;

    // initialize
    seq->tick = 0;
    seq->time = 0;
    seq->tempo = nintSpcDefaultTempo(seq->ver.gameId);
    seq->masterVolume = 0xc0;
    seq->transpose = 0;
    seq->looped = 0;

    switch (seq->ver.gameId) {
      case SPC_VER_FE3:
        seq->newNotePrm = (mget1(&ARAM[0xca]) & 0x80);
        break;
      case SPC_VER_FE4:
        seq->newNotePrm = true;
        break;
      default:
        seq->newNotePrm = false;
    }

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        seq->track[tr].pos = 0;
        nintSpcInitTrack(seq, tr);
    }

    // Smf* is not needed, because no notes queued
    seq->blockPtr -= 2;
    return nintSpcReadNewBlock(seq, (Smf*) NULL);
}

/** outputs html header. */
static void printHtmlHeader()
{
    myprintf("<?xml version=\"1.0\" ?>\n");
    myprintf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
    myprintf("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n");
    myprintf("  <head>\n");
    myprintf("    <link rel=\"stylesheet\" type=\"text/css\" media=\"screen,tv,projection\" href=\"%s\" />\n", mycssfile);
    myprintf("    <title>Data View - Nintendo SPC2MIDI %s</title>\n", VERSION);
    myprintf("  </head>\n");
    myprintf("  <body>\n");
}

/** outputs html footer. */
static void printHtmlFooter()
{
    myprintf("  </body>\n");
    myprintf("</html>\n");
}

/** outputs event table header. */
static void printEventTableHeader(NintSpcSeqStat *seq, Smf* smf)
{
    int blockPtr = seq->blockPtrAlt;
    int tr;

    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        static char trackName[256];

        //if (!seq->track[tr].pos)
        //    continue;

        sprintf(trackName, "$%04X / $%04X", seq->blockPtrAlt, seq->track[tr].pos);
        if (!nintSpcLessTextInSMF)
            smfInsertMetaText(smf, seq->track[tr].tick, tr, SMF_META_TEXT, trackName);
    }

    myprintf("        <h3>Block $%04X</h3>\n", blockPtr);
    myprintf("        <div class=\"section\" id=\"block-%04x\">\n", blockPtr);
    myprintf("          <table class=\"dump\">\n");
    myprintf("            <tr><th class=\"track\">#</th><th class=\"tick\">Tick</th><th class=\"address\">Address</th><th class=\"hex\">Hex Dump</th><th class=\"note\">Note</th></tr>\n");
}

/** outputs event table footer. */
static void printEventTableFooter(NintSpcSeqStat *seq, Smf* smf)
{
    myprintf("          </table>\n");
    myprintf("        </div>\n");
}

//----

static char argDumpStr[256];

/** returns note name from vbyte. */
void getNoteNameFromVbyte(char *buf, int vbyte)
{
    getNoteName(buf, (vbyte & 0x7f) + SPC_NOTE_KEYSHIFT);
}

/** returns lowest note info vcmd. */
static int nintSpcNoteInfoByteMin(int version)
{
    return 0x01;
}

/** returns highest note info vcmd. */
static int nintSpcNoteInfoByteMax(int version)
{
    return 0x7f;
}

/** returns lowest note byte. */
static int nintSpcNoteByteMin(int version)
{
    return 0x80;
}

/** returns highest note byte. */
static int nintSpcNoteByteMax(int version)
{
    switch(version) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        return 0xc5;
    default:
        return 0xc7;
    }
}

/** returns tie vcmd. */
static int nintSpcTieByte(int version)
{
    switch(version) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        return 0xc6;
    default:
        return 0xc8;
    }
}

/** returns rest vcmd. */
static int nintSpcRestByte(int version)
{
    switch(version) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        return 0xc7;
    default:
        return 0xc9;
    }
}

/** returns lowest percussion note. */
static int nintSpcPercByteMin(int version)
{
    switch(version) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        return 0xd0;
    default:
        return 0xca;
    }
}

/** returns highest percussion note. */
static int nintSpcPercByteMax(int version)
{
    switch(version) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        return 0xd9;
    case SPC_VER_FE3:
        return 0xd5;
    case SPC_VER_PDP:
    case SPC_VER_TA:
    case SPC_VER_FE4:
        return 0xd9;
    default:
        return 0xdf;
    }
}

/** returns if the vbyte is pitch slide vcmd. */
static int nintSpcIsPitchSlideByte(int vbyte, int version)
{
    switch(version) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        return (vbyte == 0xdd);
    case SPC_VER_FE3:
        return (vbyte == 0xef);
    case SPC_VER_PDP:
    case SPC_VER_TA:
        return (vbyte == 0xf3);
    default:
        return (vbyte == 0xf9);
    }
}

/** read duration rate from table. */
static int nintSpcDurRateOf(NintSpcSeqStat *seq, int index)
{
    int durTableAddr;

    switch (seq->ver.gameId)
    {
      case SPC_VER_SMW:
      case SPC_VER_PTWS:
      {
        const int durTable[8] = {
            0x33, 0x66, 0x80, 0x99, 0xb3, 0xcc, 0xe6, 0xff,
        };
        return durTable[index];
        //durTableAddr = 0x1268; // Super Mario World
        //durTableAddr = 0x15f1; // Pilotwings
        //break;
      }
      case SPC_VER_FZERO:
        durTableAddr = 0x0500;
        break;
      case SPC_VER_SC:
        durTableAddr = 0x3496;
        break;
      case SPC_VER_TEST:
        durTableAddr = 0x3f00;
        break;
      case SPC_VER_LOZ:
        durTableAddr = 0x3d96;
        break;
      case SPC_VER_MPNT:
        durTableAddr = 0x6f00;
        break;
      case SPC_VER_SMK:
        durTableAddr = 0x3c70;
        break;
      case SPC_VER_SF:
        durTableAddr = 0x3ee8;
        break;
      case SPC_VER_SMAS:
        durTableAddr = 0x3ee8;
        break;
      case SPC_VER_FE3:
        durTableAddr = (seq->newNotePrm) ? 0xff00 : 0x0ea3;
        break;
      case SPC_VER_SMET:
        durTableAddr = 0x5800;
        break;
      case SPC_VER_WT:
        durTableAddr = 0x0660;
        break;
      case SPC_VER_MO2:
        durTableAddr = 0x6f80;
        break;
      case SPC_VER_KDC:
        durTableAddr = 0x03e0;
        break;
      case SPC_VER_SNC:
        durTableAddr = 0x5f80;
        break;
      case SPC_VER_YI:
        durTableAddr = 0x3fe8;
        break;
      case SPC_VER_PDP:
        durTableAddr = 0x12fd;
        break;
      case SPC_VER_TA:
        durTableAddr = 0xff20;
        break;
      case SPC_VER_KSS:
        durTableAddr = 0x10cf;
        break;
      case SPC_VER_FE4:
        durTableAddr = 0x1038;
        break;
      case SPC_VER_MVLS:
        durTableAddr = 0x5d80;
        break;
      case SPC_VER_BFN1:
        durTableAddr = 0x07e0;
        break;
      case SPC_VER_TDM:
        durTableAddr = 0x3f00;
        break;
      case SPC_VER_ZTM:
        durTableAddr = 0x1800;
        break;
      case SPC_VER_KKK:
        durTableAddr = 0x10cb;
        break;
      case SPC_VER_STH:
        durTableAddr = 0x7480;
        break;
      default:
        assert(false);
    }
    return mget1(&seq->ARAM[durTableAddr + index]);
}

/** read velocity from table. */
static int nintSpcVelRateOf(NintSpcSeqStat *seq, int index)
{
    int velTableAddr;

    switch (seq->ver.gameId)
    {
      case SPC_VER_SMW:
      case SPC_VER_PTWS:
      {
        const int velTable[16] = {
            0x08, 0x12, 0x1b, 0x24, 0x2c, 0x35, 0x3e, 0x47,
            0x51, 0x5a, 0x62, 0x6b, 0x7d, 0x8f, 0xa1, 0xb3,
        };
        return velTable[index];
        //velTableAddr = 0x1270; // Super Mario World
        //velTableAddr = 0x15f9; // Pilotwings
        //break;
      }
      case SPC_VER_FZERO:
        velTableAddr = 0x0508;
        break;
      case SPC_VER_SC:
        velTableAddr = 0x349c;
        break;
      case SPC_VER_TEST:
        velTableAddr = 0x3f08;
        break;
      case SPC_VER_LOZ:
        velTableAddr = 0x3d9e;
        break;
      case SPC_VER_MPNT:
        velTableAddr = 0x6f08;
        break;
      case SPC_VER_SMK:
        velTableAddr = 0x3c78;
        break;
      case SPC_VER_SF:
        velTableAddr = 0x3ef0;
        break;
      case SPC_VER_SMAS:
        velTableAddr = 0x3ef0;
        break;
      case SPC_VER_FE3:
        velTableAddr = (seq->newNotePrm) ? 0xff00 : 0x0eab;
        break;
      case SPC_VER_SMET:
        velTableAddr = 0x5808;
        break;
      case SPC_VER_WT:
        velTableAddr = 0x0668;
        break;
      case SPC_VER_MO2:
        velTableAddr = 0x6f88;
        break;
      case SPC_VER_KDC:
        velTableAddr = 0x03e8;
        break;
      case SPC_VER_SNC:
        velTableAddr = 0x5f88;
        break;
      case SPC_VER_YI:
        velTableAddr = 0x3ff0;
        break;
      case SPC_VER_PDP:
        velTableAddr = 0x1305;
        break;
      case SPC_VER_TA:
        velTableAddr = 0xff28;
        break;
      case SPC_VER_KSS:
        velTableAddr = 0x10d7;
        break;
      case SPC_VER_FE4:
        velTableAddr = 0x1038;
        break;
      case SPC_VER_MVLS:
        velTableAddr = 0x5d88;
        break;
      case SPC_VER_BFN1:
        velTableAddr = 0x07e8;
        break;
      case SPC_VER_TDM:
        velTableAddr = 0x3f08;
        break;
      case SPC_VER_ZTM:
        velTableAddr = 0x1800;
        break;
      case SPC_VER_KKK:
        velTableAddr = 0x10d3;
        break;
      case SPC_VER_STH:
        velTableAddr = 0x7488;
        break;
      default:
        assert(false);
    }
    return mget1(&seq->ARAM[velTableAddr + index]);
}

/** returns default tempo. */
static int nintSpcDefaultTempo(int version)
{
    switch(version) {
    case SPC_VER_SMW:
        return 0x36;
    default:
        return 0x20;
    }
}

/** convert SPC tempo into bpm. */
static double nintSpcBpmOfTempo(int tempo)
{
    return (double) tempo * 60000000 / 24576000; // 24576000 = (timer0) 2ms * 48 * 256?
}

/** convert SPC velocity into MIDI one. */
static int nintSpcMidiVelOf(int value)
{
    if (nintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** convert SPC channel volume into MIDI one. */
static int nintSpcMidiVolOf(int value)
{
    if (nintSpcVolIsLinear)
        return value/2; // linear
    else
        return (int) floor(sqrt((double) value/255) * 127 + 0.5); // more similar with MIDI?
}

/** truncate note. */
static void nintSpcTruncateNote(NintSpcSeqStat *seq, int track)
{
    if (seq->track[track].lastNote >= 0 && seq->track[track].lastNoteDur > 0) {
        if (seq->tick < seq->track[track].lastNoteTick + seq->track[track].lastNoteDur) {
            seq->track[track].lastNoteDur = seq->tick - seq->track[track].lastNoteTick;
            if (seq->track[track].lastNoteDur == 0)
                seq->track[track].lastNote = -1;
        }
    }
}

/** finalize note. */
static bool nintSpcDequeueNote(NintSpcSeqStat *seq, int track, Smf* smf)
{

    int tick = seq->track[track].lastNoteTick;
    int note = seq->track[track].lastNote;
    int length = seq->track[track].lastNoteDur;
    int duration;
    int durRate = seq->track[track].lastDurRate;
    int velocity = seq->track[track].lastVel;
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

        result = smfInsertNintSpcNote(smf, tick, track, track, note, perc, velocity, duration);
    }
    seq->track[track].lastNote = -1;
    return result;
}

/** insert note. */
static bool smfInsertNintSpcNote(Smf* seq, int time, int channel, int track, int note, bool perc, int velocity, int duration)
{
    int realNote;

    if (perc)
        realNote = note;
    else
        realNote = note + SPC_NOTE_KEYSHIFT;

    return smfInsertNote(seq, time, channel, track, realNote, nintSpcMidiVelOf(velocity/2), duration);
}

/** vcmds: unknown event (without status change). */
static void nintSpcEventUnknownInline(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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
static void nintSpcEventUnidentified(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    ev->unidentified = true;
    nintSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (no args). */
static void nintSpcEventUnknown0(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    nintSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: unknown event (1 byte arg). */
static void nintSpcEventUnknown1(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmds: unknown event (2 byte args). */
static void nintSpcEventUnknown2(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg1/2 = %d", arg1, arg2, arg2 * 256 + arg1);
    strcat(ev->note, argDumpStr);
}

/** vcmd: unknown event (3 byte args). */
static void nintSpcEventUnknown3(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d, arg2 = %d, arg3 = %d", arg1, arg2, arg3);
    strcat(ev->note, argDumpStr);
}

/** vcmd: unknown event (36 byte args). */
static void nintSpcEventUnknown36(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int *p = &seq->track[ev->track].pos;

    ev->size += 36;
    (*p) += 36;

    nintSpcEventUnknownInline(seq, ev, smf);
}

/** vcmds: no operation. */
static void nintSpcEventNOP(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "NOP");
}

/** vcmds: reserved. */
static void nintSpcEventReserved(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Reserved (Event %02X)", ev->code);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd 00: end of block. */
static void nintSpcEventEndOfBlock(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    if (seq->track[ev->track].loopCount == 0) {
        sprintf(ev->note, "End Of Block");
        seq->endBlock = true;
    }
    else {
        seq->track[ev->track].loopCount--;
        if (seq->track[ev->track].loopCount == 0) {
            seq->track[ev->track].pos = seq->track[ev->track].retnAddr;
            sprintf(ev->note, "Return, addr = $%04X", seq->track[ev->track].retnAddr);
        }
        else {
            sprintf(ev->note, "Loop, addr = $%04X", seq->track[ev->track].loopStart);
            seq->track[ev->track].pos = seq->track[ev->track].loopStart;
        }
    }
}

/** vcmds: note info params. */
static void nintSpcEventNoteInfo(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int *p = &seq->track[ev->track].pos;
    int arg1 = ev->code, arg2, arg3;
    bool hasNextArg;

    sprintf(ev->note, "Note Param, length = %d", arg1);
    strcat(ev->classStr, " ev-noteparam");

    if (seq->track[ev->track].ignoreNotePrm) {
        // force dispatch them as note cmd
        nintSpcEventNote(seq, ev, smf);
        seq->track[ev->track].ignoreNotePrm = false;
        return;
    }

    arg2 = seq->ARAM[*p];
    hasNextArg = (arg2 < nintSpcNoteByteMin(seq->ver.gameId));

    seq->track[ev->track].duration = arg1;
    if (hasNextArg) {
        int durRateIndex, durVal;
        int velIndex, velVal;

        if (seq->newNotePrm) {
            // intelligent style
            do {
                arg2 = seq->ARAM[*p];

                (*p)++;
                ev->size++;

                if (arg2 < 0x40) {
                    durRateIndex = arg2 & 0x3f;
                    durVal = nintSpcDurRateOf(seq, durRateIndex);
                    seq->track[ev->track].durRate = durVal;

                    sprintf(argDumpStr, ", dur = %d:$%02X", durRateIndex, durVal);
                    strcat(ev->note, argDumpStr);
                }
                else {
                    velIndex = arg2 & 0x3f;
                    velVal = nintSpcVelRateOf(seq, velIndex);
                    seq->track[ev->track].velocity = velVal;

                    sprintf(argDumpStr, ", vel = %d:$%02X", velIndex, velVal);
                    strcat(ev->note, argDumpStr);
                }

                arg3 = seq->ARAM[*p];
            } while (arg2 < 0x40 && arg3 < 0x80);
        }
        else {
            (*p)++;
            ev->size++;

            durRateIndex = (arg2 >> 4) & 7;
            velIndex = arg2 & 15;

            seq->track[ev->track].durRate = nintSpcDurRateOf(seq, durRateIndex);
            seq->track[ev->track].velocity = nintSpcVelRateOf(seq, velIndex);

            sprintf(argDumpStr, ", dur/vel = $%02X", arg2);
            strcat(ev->note, argDumpStr);
        }
    }

    if (seq->ver.gameId != SPC_VER_FE3)
        seq->track[ev->track].ignoreNotePrm = true;
}

/** vcmds: note. */
static void nintSpcEventNote(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int note = ev->code;

    if (seq->track[ev->track].ignoreRest)
        seq->track[ev->track].ignoreRest = false;

    // outputput old note first
    nintSpcTruncateNote(seq, ev->track);
    nintSpcDequeueNote(seq, ev->track, smf);

    // set new note
    seq->track[ev->track].lastNoteTick = ev->tick;
    seq->track[ev->track].lastNoteDur = seq->track[ev->track].duration;
    seq->track[ev->track].lastNote = (note & 0x7f) + seq->transpose + seq->track[ev->track].transpose;
    if (seq->track[ev->track].lastNote < 0)
        seq->track[ev->track].lastNote = 0;
    seq->track[ev->track].lastIsPerc = false;
    seq->track[ev->track].lastLen = seq->track[ev->track].duration;
    seq->track[ev->track].lastDurRate = seq->track[ev->track].durRate;
    seq->track[ev->track].lastVel = seq->track[ev->track].velocity;
    seq->track[ev->track].tied = false;

    // step
    seq->track[ev->track].tick += seq->track[ev->track].duration;

    sprintf(ev->note, "Note ");
    getNoteNameFromVbyte(argDumpStr, note);
    strcat(ev->note, argDumpStr);
    strcat(ev->classStr, " ev-note");
}

/** vcmd: tie. */
static void nintSpcEventTie(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    if (seq->track[ev->track].ignoreRest)
        seq->track[ev->track].ignoreRest = false;

    if (seq->track[ev->track].lastNote >= 0) {
        seq->track[ev->track].lastNoteDur += seq->track[ev->track].duration;
    }
    seq->track[ev->track].tied = true;

    seq->track[ev->track].lastLen = seq->track[ev->track].duration;
    //seq->track[ev->track].lastDurRate = seq->track[ev->track].durRate;
    seq->track[ev->track].tick += seq->track[ev->track].duration;

    sprintf(ev->note, "Tie");
    strcat(ev->classStr, " ev-tie");
}

/** vcmd: rest. */
static void nintSpcEventRest(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    // outputput old note first
    nintSpcTruncateNote(seq, ev->track);

    if (seq->track[ev->track].ignoreRest) {
        // works as tie
        if (seq->track[ev->track].lastNote >= 0) { // just in case
            seq->track[ev->track].lastNoteDur = seq->tick - seq->track[ev->track].lastNoteTick + seq->track[ev->track].duration;
            seq->track[ev->track].lastDurRate = 0x100; // ignores quantize
            seq->track[ev->track].tied = true;
        }
        seq->track[ev->track].ignoreRest = false;
    }
    else
        nintSpcDequeueNote(seq, ev->track, smf);

    seq->track[ev->track].lastLen = seq->track[ev->track].duration;
    //seq->track[ev->track].lastDurRate = seq->track[ev->track].durRate;
    seq->track[ev->track].tick += seq->track[ev->track].duration;

    sprintf(ev->note, "Rest");
    strcat(ev->classStr, " ev-rest");
}

/** vcmds: percussion note. */
static void nintSpcEventPercNote(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int note = ev->code | 0x80;

    if (seq->track[ev->track].ignoreRest)
        seq->track[ev->track].ignoreRest = false;

    // outputput old note first
    nintSpcTruncateNote(seq, ev->track);
    nintSpcDequeueNote(seq, ev->track, smf);

    // set new note
    seq->track[ev->track].lastNoteTick = ev->tick;
    seq->track[ev->track].lastNoteDur = seq->track[ev->track].duration;
    seq->track[ev->track].lastNote = note - nintSpcPercByteMin(seq->ver.gameId);
    if (seq->track[ev->track].lastNote < 0)
        seq->track[ev->track].lastNote = 0;
    seq->track[ev->track].lastIsPerc = true;
    seq->track[ev->track].lastLen = seq->track[ev->track].duration;
    seq->track[ev->track].lastDurRate = seq->track[ev->track].durRate;
    seq->track[ev->track].lastVel = seq->track[ev->track].velocity;
    seq->track[ev->track].tied = false;

    // step
    seq->track[ev->track].tick += seq->track[ev->track].duration;

    sprintf(ev->note, "Perc %d", note - nintSpcPercByteMin(seq->ver.gameId) + 1);
    strcat(ev->classStr, " ev-perc ev-note");
}

/** vcmd: set patch. */
static void nintSpcEventSetPatch(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    if (!nintSpcNoPatchChange) {
        smfInsertControl(smf, seq->track[ev->track].tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, arg1 >> 7);
        smfInsertProgram(smf, seq->track[ev->track].tick, ev->track, ev->track, arg1 & 0x7f);
    }
    seq->track[ev->track].patch = arg1;
    sprintf(ev->note, "Set Patch, patch = %d", arg1);
    strcat(ev->classStr, " ev-patch");
}

/** vcmd: set patch with ADSR. */
static void nintSpcEventSetPatchWithADSR(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = mget2b(&seq->ARAM[*p]);
    (*p) += 2;

    if (!nintSpcNoPatchChange) {
        smfInsertControl(smf, seq->track[ev->track].tick, ev->track, ev->track, SMF_CONTROL_BANKSELL, arg1 >> 7);
        smfInsertProgram(smf, seq->track[ev->track].tick, ev->track, ev->track, arg1 & 0x7f);
    }
    seq->track[ev->track].patch = arg1;
    sprintf(ev->note, "Set Patch with ADSR, patch = %d, ADSR = $%04X", arg1, arg2);
    strcat(ev->classStr, " ev-patch-adsr");

    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
}

/** vcmd: panpot. */
static void nintSpcEventPanpot(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Panpot, val = $%02X", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pan");
}

/** vcmd: panpot fade. */
static void nintSpcEventPanFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Pan Fade, length = $%02X, to = $%02X", arg1, arg2);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-panfade");
}

/** vcmd: vibrato on. */
static void nintSpcEventVibratoOn(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    sprintf(ev->note, "Vibrato On, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratoon");
}

/** vcmd: vibrato off. */
static void nintSpcEventVibratoOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Vibrato Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratooff");
}

/** vcmd: master volume. */
static void nintSpcEventMasterVolume(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume, val = %d", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-mastervol");
}

/** vcmd: master volume fade. */
static void nintSpcEventMasterVolFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Master Volume Fade, length = %d, to = %d", arg1, arg2);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-mastervolfade");
}

/** vcmd: tempo. */
static void nintSpcEventTempo(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    double bpm;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    seq->tempo = arg1;
    bpm = nintSpcBpmOfTempo(arg1);
    sprintf(ev->note, "Tempo, val = %d (%f bpm)", arg1, bpm);

    smfInsertTempoBPM(smf, seq->track[ev->track].tick, 0, bpm);
    //smfInsertTempoBPM(smf, seq->track[ev->track].tick, ev->track, bpm);
    strcat(ev->classStr, " ev-tempo");
}

/** vcmd: tempo fade. */
static void nintSpcEventTempoFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;
    double bpm;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    bpm = nintSpcBpmOfTempo(arg2);

    sprintf(ev->note, "Tempo Fade, length = %d, to = %d (%f bpm)", arg1, arg2, bpm);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tempofade");
}

/** vcmd: transpose (global). */
static void nintSpcEventKeyShift(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;

    seq->transpose = arg1;
    sprintf(ev->note, "Global Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose");
}

/** vcmd: transpose (local). */
static void nintSpcEventTranspose(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;

    seq->track[ev->track].transpose = arg1;
    sprintf(ev->note, "Channel Transpose, key = %d", arg1);
    strcat(ev->classStr, " ev-transpose-ch");
}

/** vcmd: tremolo on. */
static void nintSpcEventTremoloOn(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    sprintf(ev->note, "Tremolo On, delay = %d, rate = %d, depth = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tremoloon");
}

/** vcmd: tremolo off. */
static void nintSpcEventTremoloOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Tremolo Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tremolooff");
}

/** vcmd: volume. */
static void nintSpcEventVolume(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume, val = %d", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-volume");
}

/** vcmd: volume fade. */
static void nintSpcEventVolumeFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2;
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Volume Fade, length = %d, to = %d", arg1, arg2);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-volumefade");
}

/** vcmd: repeat n times. */
static void nintSpcEventSubroutine(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int dest, count;
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    dest = mget2l(&seq->ARAM[*p]);
    (*p) += 2;
    count = seq->ARAM[*p];
    (*p)++;

    seq->track[ev->track].retnAddr = *p;
    *p = dest;
    seq->track[ev->track].loopStart = dest;
    seq->track[ev->track].loopCount = count;

    sprintf(ev->note, "Call/Repeat, addr = $%04X, count = %d", dest, count);
    strcat(ev->classStr, " ev-call");
}

/** vcmd: vibrato fade(-in). */
static void nintSpcEventVibratoFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Vibrato Fade, length = %d", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-vibratofade");
}

/** vcmd: pitch envelope (release). */
static void nintSpcEventPitchEnvR(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;
    arg3 = utos1(seq->ARAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Envelope (Release), delay = %d, length = %d, key = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvrelease");
}

/** vcmd: pitch envelope (attack). */
static void nintSpcEventPitchEnvA(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;

    ev->size += 3;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;
    arg3 = utos1(seq->ARAM[*p]);
    (*p)++;

    sprintf(ev->note, "Pitch Envelope (Attack), delay = %d, length = %d, key = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvattack");
}

/** vcmd: pitch envelope off. */
static void nintSpcEventPitchEnvOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Pitch Envelope Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchenvoff");
}

/** vcmd: tuning. */
static void nintSpcEventTuning(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    sprintf(ev->note, "Tuning, amount = %d/256", arg1);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-tuning");
}

/** vcmd: set echo vbits, volume. */
static void nintSpcEventEchoVol(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    sprintf(ev->note, "Echo Volume, EON = $%02X, EVOL(L) = %d, EVOL(R) = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echovol");
}

/** vcmd: disable echo. */
static void nintSpcEventEchoOff(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    sprintf(ev->note, "Echo Off");
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echooff");
}

/** vcmd: set echo delay, feedback, filter. */
static void nintSpcEventEchoParam(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    sprintf(ev->note, "Echo Param, EDL = %d, EFB = %d, FIR = index %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echoparam");
}

/** vcmd: echo volume fade. */
static void nintSpcEventEchoVolFade(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
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

    sprintf(ev->note, "Echo Volume Fade, length = %d, to L = %d, to R = %d", arg1, arg2, arg3);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-echovolfade");
}

/* vcmd: pitch slide. */
static void nintSpcEventPitchSlide(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1, arg2, arg3;
    int *p = &seq->track[ev->track].pos;

    // rewrite event timing
    ev->tick = seq->track[ev->track].prevTick;

    ev->size += 3;
    arg1 = seq->ARAM[*p];
    (*p)++;
    arg2 = seq->ARAM[*p];
    (*p)++;
    arg3 = seq->ARAM[*p];
    (*p)++;

    getNoteNameFromVbyte(argDumpStr, arg3);
    sprintf(ev->note, "Pitch Slide, delay = %d, length = %d, key = %s", arg1, arg2, argDumpStr);
    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-pitchslide");
}

/** vcmd: set perc base. */
static void nintSpcEventSetPercBase(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;

    switch (seq->ver.gameId) {
      case SPC_VER_KSS:
      case SPC_VER_FE4:
      case SPC_VER_KKK:
        sprintf(ev->note, "Perc Base (NYI), arg1 = %d", arg1);
        break;
      default:
        sprintf(ev->note, "Perc Base, arg1 = %d", arg1);
    }

    if (!nintSpcLessTextInSMF)
        smfInsertMetaText(smf, ev->tick, ev->track, SMF_META_TEXT, ev->note);
    strcat(ev->classStr, " ev-percbase");
}

/** vcmd: skip 2 bytes. */
static void nintSpcEventSkip2(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int *p = &seq->track[ev->track].pos;

    ev->size += 2;
    (*p) += 2;

    sprintf(ev->note, "Skip 2 Bytes");
    strcat(ev->classStr, " ev-skip2");
}

/** vcmd: short jump (forward only). */
static void nintSpcEventShortJump(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = seq->ARAM[*p]; // unsigned
    (*p)++;

    (*p) += arg1;
    ev->size += arg1;

    sprintf(ev->note, "Short Jump, arg1 = %d", arg1);
    strcat(ev->classStr, " ev-shortjump");
}

/** vcmd: Fire Emblem vcmd fa. */
static void nintSpcEventFE3FA(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;

    ev->size++;
    arg1 = utos1(seq->ARAM[*p]);
    (*p)++;

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
    if (arg1 < 0) {
        if (seq->ver.gameId != SPC_VER_FE4) {
            ev->size += 6;
            (*p) += 6;
        }
    }
    else {
        ev->size += (arg1 * 4);
        (*p) += (arg1 * 4);
    }
}

/** vcmd: Tetris Attack vcmd fc. */
static void nintSpcEventTAFC(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int n;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    n = ((arg1 & 15) + 1);
    ev->size += n * 3;
    (*p) += n * 3;

    nintSpcEventUnknownInline(seq, ev, smf);
    //sprintf(argDumpStr, ", arg1 = %d", arg1);
    //strcat(ev->note, argDumpStr);
}

/** vcmd: Tetris Attack vcmd fd. */
static void nintSpcEventTAFD(NintSpcSeqStat *seq, SeqEventReport *ev, Smf* smf)
{
    int arg1;
    int *p = &seq->track[ev->track].pos;
    int n;

    ev->size++;
    arg1 = seq->ARAM[*p];
    (*p)++;

    switch (arg1) {
      case 0x00:
        if (seq->ver.gameId == SPC_VER_FE3) {
            ev->size += 3;
            (*p) += 3;
        }
        break;
      case 0x01:
        ev->size++;
        (*p)++;
        break;
      case 0x02:
        ev->size++;
        (*p)++;
        break;
      case 0x03:
        break;
      case 0x04:
        break;
      case 0x05:
        if (seq->ver.gameId == SPC_VER_FE3) {
            ev->size++;
            (*p)++;
        }
        break;
    }

    nintSpcEventUnknownInline(seq, ev, smf);
    sprintf(argDumpStr, ", arg1 = %d", arg1);
    strcat(ev->note, argDumpStr);
}

/** set pointers of each event. */
static void nintSpcInitEventList(NintSpcSeqStat *seq)
{
    int code;
    NintSpcEvent *event = seq->ver.event;
    int vcmdStart;

    // disable them all first
    for(code = 0x00; code <= 0xff; code++) {
        event[code] = (NintSpcEvent) nintSpcEventUnidentified;
    }

    event[0x00] = (NintSpcEvent) nintSpcEventEndOfBlock;
    for(code = nintSpcNoteInfoByteMin(seq->ver.gameId); code <= nintSpcNoteInfoByteMax(seq->ver.gameId); code++) {
        event[code] = (NintSpcEvent) nintSpcEventNoteInfo;
    }
    for(code = nintSpcNoteByteMin(seq->ver.gameId); code <= nintSpcNoteByteMax(seq->ver.gameId); code++) {
        event[code] = (NintSpcEvent) nintSpcEventNote;
    }
    event[nintSpcTieByte(seq->ver.gameId)]  = (NintSpcEvent) nintSpcEventTie;
    event[nintSpcRestByte(seq->ver.gameId)] = (NintSpcEvent) nintSpcEventRest;
    for(code = nintSpcPercByteMin(seq->ver.gameId); code <= nintSpcPercByteMax(seq->ver.gameId); code++) {
        event[code] = (NintSpcEvent) nintSpcEventPercNote;
    }

    switch (seq->ver.gameId) {
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        vcmdStart = 0xda;
        break;
    case SPC_VER_FE3:
        vcmdStart = 0xd6;
        break;
    case SPC_VER_PDP:
    case SPC_VER_TA:
    case SPC_VER_FE4:
        vcmdStart = 0xda;
        break;
    default:
        vcmdStart = 0xe0;
    }

    // set basic vcmds
    switch (seq->ver.gameId) {
    // old version
    case SPC_VER_SMW:
    case SPC_VER_PTWS:
        //for(code = 0xc8; code <= 0xcf; code++) {
        //    event[code] = (NintSpcEvent) nintSpcEventRest;
        //}
        event[vcmdStart+0x00] = (NintSpcEvent) nintSpcEventSetPatch;
        event[vcmdStart+0x01] = (NintSpcEvent) nintSpcEventPanpot;
        event[vcmdStart+0x02] = (NintSpcEvent) nintSpcEventPanFade;
        event[vcmdStart+0x03] = (NintSpcEvent) nintSpcEventPitchSlide;
        event[vcmdStart+0x04] = (NintSpcEvent) nintSpcEventVibratoOn;
        event[vcmdStart+0x05] = (NintSpcEvent) nintSpcEventVibratoOff;
        event[vcmdStart+0x06] = (NintSpcEvent) nintSpcEventMasterVolume;
        event[vcmdStart+0x07] = (NintSpcEvent) nintSpcEventMasterVolFade;
        event[vcmdStart+0x08] = (NintSpcEvent) nintSpcEventTempo;
        event[vcmdStart+0x09] = (NintSpcEvent) nintSpcEventTempoFade;
        event[vcmdStart+0x0a] = (NintSpcEvent) nintSpcEventKeyShift;
        event[vcmdStart+0x0b] = (NintSpcEvent) nintSpcEventUnknown3;
        event[vcmdStart+0x0c] = (NintSpcEvent) nintSpcEventUnknown0;
        event[vcmdStart+0x0d] = (NintSpcEvent) nintSpcEventUnknown1;
        event[vcmdStart+0x0e] = (NintSpcEvent) nintSpcEventUnknown2;
        event[vcmdStart+0x0f] = (NintSpcEvent) nintSpcEventSubroutine;
        event[vcmdStart+0x10] = (NintSpcEvent) nintSpcEventVibratoFade;
        event[vcmdStart+0x11] = (NintSpcEvent) nintSpcEventPitchEnvR;
        event[vcmdStart+0x12] = (NintSpcEvent) nintSpcEventPitchEnvA;
        //event[vcmdStart+0x13] = (NintSpcEvent) nintSpcEventReserved;
        event[vcmdStart+0x14] = (NintSpcEvent) nintSpcEventTuning;
        event[vcmdStart+0x15] = (NintSpcEvent) nintSpcEventEchoVol;
        event[vcmdStart+0x16] = (NintSpcEvent) nintSpcEventEchoOff;
        event[vcmdStart+0x17] = (NintSpcEvent) nintSpcEventEchoParam;
        event[vcmdStart+0x18] = (NintSpcEvent) nintSpcEventEchoVolFade;
        //for(code = vcmdStart+0x19; code <= 0xff; code++) {
        //    event[code] = (NintSpcEvent) nintSpcEventReserved;
        //}
        break;
    default:
        event[vcmdStart+0x00] = (NintSpcEvent) nintSpcEventSetPatch;
        event[vcmdStart+0x01] = (NintSpcEvent) nintSpcEventPanpot;
        event[vcmdStart+0x02] = (NintSpcEvent) nintSpcEventPanFade;
        event[vcmdStart+0x03] = (NintSpcEvent) nintSpcEventVibratoOn;
        event[vcmdStart+0x04] = (NintSpcEvent) nintSpcEventVibratoOff;
        event[vcmdStart+0x05] = (NintSpcEvent) nintSpcEventMasterVolume;
        event[vcmdStart+0x06] = (NintSpcEvent) nintSpcEventMasterVolFade;
        event[vcmdStart+0x07] = (NintSpcEvent) nintSpcEventTempo;
        event[vcmdStart+0x08] = (NintSpcEvent) nintSpcEventTempoFade;
        event[vcmdStart+0x09] = (NintSpcEvent) nintSpcEventKeyShift;
        event[vcmdStart+0x0a] = (NintSpcEvent) nintSpcEventTranspose;
        event[vcmdStart+0x0b] = (NintSpcEvent) nintSpcEventTremoloOn;
        event[vcmdStart+0x0c] = (NintSpcEvent) nintSpcEventTremoloOff;
        event[vcmdStart+0x0d] = (NintSpcEvent) nintSpcEventVolume;
        event[vcmdStart+0x0e] = (NintSpcEvent) nintSpcEventVolumeFade;
        event[vcmdStart+0x0f] = (NintSpcEvent) nintSpcEventSubroutine;
        event[vcmdStart+0x10] = (NintSpcEvent) nintSpcEventVibratoFade;
        event[vcmdStart+0x11] = (NintSpcEvent) nintSpcEventPitchEnvR;
        event[vcmdStart+0x12] = (NintSpcEvent) nintSpcEventPitchEnvA;
        event[vcmdStart+0x13] = (NintSpcEvent) nintSpcEventPitchEnvOff;
        event[vcmdStart+0x14] = (NintSpcEvent) nintSpcEventTuning;
        event[vcmdStart+0x15] = (NintSpcEvent) nintSpcEventEchoVol;
        event[vcmdStart+0x16] = (NintSpcEvent) nintSpcEventEchoOff;
        event[vcmdStart+0x17] = (NintSpcEvent) nintSpcEventEchoParam;
        event[vcmdStart+0x18] = (NintSpcEvent) nintSpcEventEchoVolFade;
        event[vcmdStart+0x19] = (NintSpcEvent) nintSpcEventPitchSlide;
        event[vcmdStart+0x1a] = (NintSpcEvent) nintSpcEventSetPercBase;
        //for(code = vcmdStart+0x1b; code <= 0xff; code++) {
        //    event[code] = (NintSpcEvent) nintSpcEventReserved;
        //}
    }

    // extra vcmds
    switch (seq->ver.gameId) {
    case SPC_VER_TEST:
    case SPC_VER_MPNT:
    case SPC_VER_SMET:
    case SPC_VER_MO2:
    case SPC_VER_SNC:
        event[0xfb] = (NintSpcEvent) nintSpcEventSkip2;
        event[0xfc] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xfd] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xfe] = (NintSpcEvent) nintSpcEventUnknown0;
        break;
    case SPC_VER_FE3:
        event[0xf1] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf2] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf3] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf4] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf5] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xf6] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xf7] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xf8] = (NintSpcEvent) nintSpcEventShortJump;
        event[0xf9] = (NintSpcEvent) nintSpcEventUnknown36;
        event[0xfa] = (NintSpcEvent) nintSpcEventFE3FA;
        event[0xfb] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xfc] = (NintSpcEvent) nintSpcEventUnknown2;
        event[0xfd] = (NintSpcEvent) nintSpcEventUnknown2;
        break;
    case SPC_VER_PDP:
    case SPC_VER_TA:
        event[0xf5] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf6] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf7] = (NintSpcEvent) nintSpcEventUnknown2;
        event[0xf8] = (NintSpcEvent) nintSpcEventUnknown2;
        event[0xf9] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xfa] = (NintSpcEvent) nintSpcEventFE3FA;
        event[0xfb] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xfc] = (NintSpcEvent) nintSpcEventTAFC;
        event[0xfd] = (NintSpcEvent) nintSpcEventTAFD;
        break;
    case SPC_VER_FE4:
        event[0xf5] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf6] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xf7] = (NintSpcEvent) nintSpcEventUnknown2;
        event[0xf8] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xf9] = (NintSpcEvent) nintSpcEventUnknown0;
        event[0xfa] = (NintSpcEvent) nintSpcEventFE3FA;
        event[0xfb] = (NintSpcEvent) nintSpcEventUnknown1;
        event[0xfc] = (NintSpcEvent) nintSpcEventTAFC;
        event[0xfd] = (NintSpcEvent) nintSpcEventTAFD;
        break;
    case SPC_VER_MVLS:
        event[0xfb] = (NintSpcEvent) nintSpcEventSetPatchWithADSR;
        event[0xfd] = (NintSpcEvent) nintSpcEventSetPatch;
        break;
    case SPC_VER_STH:
        event[0xfb] = (NintSpcEvent) nintSpcEventSetPatchWithADSR;
        break;
    }
}

//----

/** convert spc to midi data from ARAM (65536 bytes). */
Smf* nintSpcARAMToMidi(const byte *ARAM)
{
    Smf* smf = NULL;
    NintSpcSeqStat seq;
    bool trackHasEnd[8];
    bool seqHasEnd = false;
    bool oldIgnoreNotePrm;
    bool inSub;
    int oldTick;
    int minTickStep;
    int tr;
    int i;

    printHtmlHeader();
    myprintf("    <h1>Nintendo SPC2MIDI %s</h1>\n", VERSION);
    myprintf("    <div class=\"section\">\n");
    myprintf("      <p>This document is generated automatically by nintspc. For details, visit <a href=\"http://loveemu.yh.land.to/\">loveemu labo</a>.</p>\n\n", mycssfile);

    seq.ARAM = (byte *) ARAM;
    nintSpcCheckVer(&seq);
    if (seq.ver.gameId == SPC_VER_UNKNOWN) {
        fprintf(stderr, "Error: Unsupported version.\n");
        goto abort;
    }
    if (!nintSpcDetectSeq(&seq)) {
        fprintf(stderr, "Error: Unable to find sequence data.\n");
        goto abort;
    }

    smf = smfCreate(SPC_TIMEBASE);
    smfInsertMetaText(smf, 0, 0, SMF_META_SEQUENCENAME, "Nintendo SPC2MIDI");
    smfInsertTempoBPM(smf, 0, 0, nintSpcBpmOfTempo(seq.tempo));
    smfInsertGM1SystemOn(smf, 0, 0, 0);
    //smfInsertNintSpcMasterVolume(smf, 0, 0, seq.masterVolume);
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        trackHasEnd[tr] = !seq.track[tr].pos;
        if (!trackHasEnd[tr]) {
            //smfInsertNintSpcVolume(smf, 0, tr, tr, seq.track[tr].volume);
            //smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_REVERB, 20);

            if (nintSpcNoPatchChange) {
                smfInsertControl(smf, 0, tr, tr, SMF_CONTROL_BANKSELL, 0);
                smfInsertProgram(smf, 0, tr, tr, 0);
            }
        }
    }

    myprintf("      <h2>Informations</h2>\n");
    myprintf("      <div class=\"section\" id=\"informations\">\n");
    myprintf("        <ul class=\"info-tree\">\n");
    myprintf("          <li>Version: %s</li>\n", nintSpcVerToStr(seq.ver.gameId)); // FIXME: encoding string
    myprintf("          <li>Sequence: $%04X (Song $%02X)<ul>\n", seq.addrOfHeader, seq.songIndex);
    {
        // dump block list
        int blockPtr = seq.addrOfHeader;
        int blockAddr;
        int blockCnt;
        int dumpBlockPtr;

        do {
            bool hasBlock;

            dumpBlockPtr = blockPtr;
            blockAddr = mget2l(&ARAM[blockPtr]);
            if (blockAddr != 0 && (blockAddr & 0xff00) == 0) {
                blockCnt = utos1(blockAddr & 0xff);
                if (blockCnt > 0)
                    blockCnt++;
                blockPtr += 2;
                blockAddr = mget2l(&ARAM[blockPtr]);
            }
            else
                blockCnt = 0;

            hasBlock = (blockAddr & 0xff00) && (blockCnt >= 0);
            myprintf("            <li>");
            if (hasBlock)
                myprintf("<a href=\"#block-%04x\">", dumpBlockPtr);
            myprintf("Block $%04X", dumpBlockPtr);
            if (hasBlock)
                myprintf("</a>");
            if (blockCnt < 0)
                myprintf(" -> [$%02X] $%04X", blockCnt & 0xff, blockAddr);
            else {
                myprintf(": $%04X", blockAddr);
                if (blockCnt && blockAddr != 0)
                    myprintf(" * %d", blockCnt);
            }

            if (hasBlock) {
                myprintf("<ul>\n");
                myprintf("              <li>");

                for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                    if (tr)
                        myprintf(" ");

                    myprintf("%d:$%04X", tr + 1, mget2l(&ARAM[blockAddr + tr * 2]));
                }

                myprintf("</li>\n");
                myprintf("            </ul>");
            }

            myprintf("</li>\n");
            blockPtr += 2;
        } while (blockAddr != 0 && blockCnt >= 0);
    }
    myprintf("          </ul></li>\n");
    myprintf("        </ul>\n");
    myprintf("      </div>\n\n");

    myprintf("      <h2>Data Dump</h2>\n");
    myprintf("      <div class=\"section\" id=\"data-dump\">\n");
    myprintf("        <p>You can filter output by using stylesheet. Write %s as you like!</p>\n", mycssfile);

    printEventTableHeader(&seq, smf);

    nintSpcInitEventList(&seq);

    while(!seqHasEnd) {

        SeqEventReport ev;

        for (ev.track = 0; ev.track < SPC_TRACK_MAX; ev.track++) {

            while (!seqHasEnd && !seq.endBlock && seq.track[ev.track].pos && seq.track[ev.track].tick <= seq.tick) {

                ev.tick = seq.tick;
                ev.addr = seq.track[ev.track].pos;
                ev.size = 0;
                ev.unidentified = false;
                strcpy(ev.note, "");

                inSub = (seq.track[ev.track].loopCount > 0);

                // read first byte
                ev.size++;
                ev.code = ARAM[ev.addr];
                sprintf(ev.classStr, "ev%02X", ev.code);
                seq.track[ev.track].pos++;
                // in subroutine?
                strcat(ev.classStr, inSub ? " sub" : "");

                oldIgnoreNotePrm = seq.track[ev.track].ignoreNotePrm;
                if (!nintSpcIsPitchSlideByte(ev.code, seq.ver.gameId))
                    seq.track[ev.track].prevTick = seq.track[ev.track].tick;
                // dispatch event
                seq.ver.event[ev.code](&seq, &ev, smf);
                // 
                if (seq.track[ev.track].ignoreNotePrm == oldIgnoreNotePrm)
                    seq.track[ev.track].ignoreNotePrm = false;

                if (nintSpcTextLoopMax == 0 || max(seq.looped, seq.blockLooped) < nintSpcTextLoopMax) {
                    myprintf("            <tr class=\"track%d %s\">", ev.track + 1, ev.classStr);
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

                // check if each track has end
                for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                    trackHasEnd[tr] = !seq.track[tr].pos;
                }

                // check if all tracks has end
                seqHasEnd = true;
                for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                    if (!trackHasEnd[tr]) {
                        seqHasEnd = false;
                        break;
                    }
                }
            }
        }

        // enter to new block, or quit, if needed
        if (seq.endBlock) {
            // rewind tracks to end point
            for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                seq.track[tr].tick = seq.tick;
                seq.track[tr].prevTick = seq.tick;
                smfSetEndTimingOfTrack(smf, tr, seq.tick);
            }
            if (!nintSpcReadNewBlock(&seq, smf))
                seqHasEnd = true;
            else {
                // put new table
                if (nintSpcTextLoopMax == 0 || max(seq.looped, seq.blockLooped) < nintSpcTextLoopMax) {
                    printEventTableFooter(&seq, smf);
                    printEventTableHeader(&seq, smf);
                }

                // exception for rest byte
                for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                    if (seq.track[tr].lastNote >= 0 && seq.track[tr].lastNoteTick + seq.track[tr].lastNoteDur > seq.tick) {
                        seq.track[tr].ignoreRest = true;
                    }
                }
            }
        }
        else {
            // step
            minTickStep = 0;
            for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
                if (seq.track[tr].pos) {
                    if (minTickStep == 0)
                        minTickStep = seq.track[tr].tick - seq.tick;
                    else
                        minTickStep = min(minTickStep, seq.track[tr].tick - seq.tick);
                }
            }
            //minTickStep = 1;

            seq.tick += minTickStep;
            seq.time += (double) 60 / nintSpcBpmOfTempo(seq.tempo) * minTickStep / SPC_TIMEBASE;

            // check time limit
            if (seq.time >= nintSpcTimeLimit) {
                seqHasEnd = true;
            }
        }
    }

quitConversion:

    // finalize for all notes
    for (tr = 0; tr < SPC_TRACK_MAX; tr++) {
        nintSpcTruncateNote(&seq, tr);
        nintSpcDequeueNote(&seq, tr, smf);
    }

    printEventTableFooter(&seq, smf);
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

/** convert spc to midi data from SPC file located in memory. */
Smf* nintSpcToMidi(const byte *data, size_t size)
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
Smf* nintSpcToMidiFromFile(const char *filename)
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
        else if (strcmp((*argv)[0], "--loop") == 0) {
            if (*argc <= 2) {
                fprintf(stderr, "Error: invalid use of \"--loop [count=0-]\"\n");
            }
            else {
                nintSpcSetLoopCount(atoi((*argv)[1]));
                dispatched = true;
                (*argv)++; (*argc)--;
            }
        }
        else if (strcmp((*argv)[0], "--song") == 0) {
            if (*argc <= 2) {
                fprintf(stderr, "Error: invalid use of \"--song [count=1-, 0:auto]\"\n");
            }
            else {
                nintSpcSetSongIndex(atoi((*argv)[1]));
                dispatched = true;
                (*argv)++; (*argc)--;
            }
        }
        else if (strcmp((*argv)[0], "--nopatch") == 0) {
            nintSpcNoPatchChange = true;
            dispatched = true;
        }
        else if (strcmp((*argv)[0], "--linear") == 0) {
            nintSpcVolIsLinear = true;
            dispatched = true;
        }
        else if (strcmp((*argv)[0], "--bendrange") == 0) {
            if (*argc <= 2) {
                fprintf(stderr, "Error: invalid use of \"--bendrange [range=0-24]\"\n");
            }
            else {
                nintSpcPitchBendSens = atoi((*argv)[1]);
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
    const char *cmdname = "nintspc";
    //const char *cmdname = (cmd != NULL) ? cmd : "nintspc";

    fprintf(stderr, "nintspc - Nintendo SPC2MIDI %s\n", VERSION);
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
    fprintf(stderr, "http://loveemu.yh.land.to/\n");

    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "--help         show usage\n");
    fprintf(stderr, "--song N       specify target song (0:auto)\n");
    fprintf(stderr, "--loop N       loop count of midi output\n");
    fprintf(stderr, "--nopatch      disable program change\n");
    fprintf(stderr, "--linear       assume midi volume is linear\n");
    fprintf(stderr, "--bendrange N  pitch bend sensitivity (0:auto)\n");
}

/** display about nintspc application. */
void about(const char *cmd)
{
    const char *cmdname = "nintspc";
    //const char *cmdname = (cmd != NULL) ? cmd : "nintspc";

    fprintf(stderr, "nintspc - Nintendo SPC2MIDI %s\n", VERSION);
    fprintf(stderr, "Programmed by loveemu - http://loveemu.yh.land.to/\n");
    fprintf(stderr, "Syntax: %s (options) [spcfile] [midfile] (htmlfile)\n", cmdname);
}

/** nintspc application main. */
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
            nintSpcSetLogStreamHandle(htmlFile);
    }
    else {
        //nintSpcSetLogStreamHandle(stdout);
        nintSpcSetLogStreamHandle(NULL);
    }

    fprintf(stderr, "%s: Conversion started!\n", argv[0]);
    smf = nintSpcToMidiFromFile(argv[0]);
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
