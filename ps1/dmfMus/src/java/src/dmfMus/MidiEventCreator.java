package dmfMus;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.SysexMessage;

public class MidiEventCreator {

	final static public int CONTROL_BANKSELECTMSB = 0;
	final static public int CONTROL_MODULATION = 1;
	final static public int CONTROL_PORTAMENTOTIME = 5;
	final static public int CONTROL_DATAENTRYMSB = 6;
	final static public int CONTROL_VOLUME = 7;
	final static public int CONTROL_PANPOT = 10;
	final static public int CONTROL_EXPRESSION = 11;
	final static public int CONTROL_BANKSELECTLSB = 32;
	final static public int CONTROL_DATAENTRYLSB = 38;
	final static public int CONTROL_HOLDPEDAL = 64;
	final static public int CONTROL_PORTAMENTO = 65;
	final static public int CONTROL_SOSTENUTO = 66;
	final static public int CONTROL_SOFTPEDAL = 67;
	final static public int CONTROL_RESONANCE = 71;
	final static public int CONTROL_RELEASETIME = 72;
	final static public int CONTROL_ATTACKTIME = 73;
	final static public int CONTROL_BRIGHTENESS = 74;
	final static public int CONTROL_DECAYTIME = 75;
	final static public int CONTROL_VIBRATORATE = 76;
	final static public int CONTROL_VIBRATODEPTH = 77;
	final static public int CONTROL_VIBRATODELAY = 78;
	final static public int CONTROL_PORTAMENTOCONTROl = 84;
	final static public int CONTROL_REVERB = 91;
	final static public int CONTROL_TREMOLODEPTH = 92;
	final static public int CONTROL_CHORUS = 93;
	final static public int CONTROL_DELAY = 94;
	final static public int CONTROL_PHASERDEPTH = 95;
	final static public int CONTROL_DATAINCREMENT = 96;
	final static public int CONTROL_DATADECREMENT = 97;
	final static public int CONTROL_NRPNLSB = 98;
	final static public int CONTROL_NRPNMSB = 99;
	final static public int CONTROL_RPNLSB = 100;
	final static public int CONTROL_RPNMSB = 101;
	final static public int CONTROL_ALLSOUNDOFF = 120;
	final static public int CONTROL_RESETALLCONTROLLER = 121;
	final static public int CONTROL_ALLNOTEOFF = 123;
	final static public int CONTROL_OMNIMODEOFF = 124;
	final static public int CONTROL_OMNIMODEON = 125;
	final static public int CONTROL_MONO = 126;
	final static public int CONTROL_POLY = 127;

	final static public int RPN_PITCHBENDRANGE = 1;
	final static public int RPN_CHANNELFINETUNING = 1;
	final static public int RPN_CHANNELCOARSETUNING = 2;
	final static public int RPN_TUNINGPROGRAMCHANGE = 3;
	final static public int RPN_TUNINGBANKSELECT = 4;
	final static public int RPN_MODULATIONDEPTHRANGE = 5;
	final static public int RPN_NULL = 127;

	final static public int META_EVENT_TEXT = 0x01;
	final static public int META_EVENT_COPYRIGHT = 0x02;
	final static public int META_EVENT_TRACKNAME = 0x03;
	final static public int META_EVENT_SEQUENCENAME = 0x03;
	final static public int META_EVENT_INSTRUMENTNAME = 0x04;
	final static public int META_EVENT_LYRIC = 0x05;
	final static public int META_EVENT_MARKER = 0x06;
	final static public int META_EVENT_CUEPOINT = 0x07;
	final static public int META_EVENT_CHANNELPREFIX = 0x20;
	final static public int META_EVENT_PORTPREFIX = 0x21;
	final static public int META_EVENT_ENDOFTRACK = 0x2f;
	final static public int META_EVENT_TEMPO = 0x51;
	final static public int META_EVENT_SMPTEOFFSET = 0x54;
	final static public int META_EVENT_TIMESIGNATURE = 0x58;
	final static public int META_EVENT_KEYSIGNATURE = 0x59;
	final static public int META_EVENT_PROPRIETARY = 0x7f;

	static MidiEvent createNoteOffEvent(long tick, int channel, int key) throws InvalidMidiDataException
	{
		// use note on for better compression ratio
		return createNoteOnEvent(tick, channel, key, 0);
	}

	static MidiEvent createNoteOffEvent(long tick, int channel, int key, int velocity) throws InvalidMidiDataException
	{
		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.NOTE_OFF, channel, key, velocity);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createNoteOnEvent(long tick, int channel, int key, int velocity) throws InvalidMidiDataException
	{
		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.NOTE_ON, channel, key, velocity);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createPolyPressureEvent(long tick, int channel, int key, int aftertouch) throws InvalidMidiDataException
	{
		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.POLY_PRESSURE, channel, key, aftertouch);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createControlChangeEvent(long tick, int channel, int control, int value) throws InvalidMidiDataException
	{
		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.CONTROL_CHANGE, channel, control, value);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createProgramChangeEvent(long tick, int channel, int patch) throws InvalidMidiDataException
	{
		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.PROGRAM_CHANGE, channel, patch, 0);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createChannelPressureEvent(long tick, int channel, int aftertouch) throws InvalidMidiDataException
	{
		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.CHANNEL_PRESSURE, channel, aftertouch, 0);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createPitchBendEvent(long tick, int channel, int pitch) throws InvalidMidiDataException
	{
		if (pitch < -8192 || pitch > 8191)
		{
			throw new InvalidMidiDataException("Pitchbend " + pitch + " is out of range.");
		}
		pitch += 8192;

		ShortMessage message = new ShortMessage();
		message.setMessage(ShortMessage.PITCH_BEND, channel, pitch & 0x7f, (pitch >> 7) & 0x7f);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createSysexEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		SysexMessage message = new SysexMessage();
		message.setMessage(data, data.length);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createSysexEvent(long tick, int status, byte[] data) throws InvalidMidiDataException
	{
		SysexMessage message = new SysexMessage();
		message.setMessage(status, data, data.length);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createGMResetEvent(long tick) throws InvalidMidiDataException
	{
		return createSysexEvent(tick, 0xf0, new byte[] {0x7e, 0x7f, 0x09, 0x01, (byte)0xf7});
	}

	static MidiEvent createGM2ResetEvent(long tick) throws InvalidMidiDataException
	{
		return createSysexEvent(tick, 0xf0, new byte[] {0x7e, 0x7f, 0x09, 0x03, (byte)0xf7});
	}

	static MidiEvent createGSResetEvent(long tick) throws InvalidMidiDataException
	{
		return createSysexEvent(tick, 0xf0, new byte[] {0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, (byte)0xf7});
	}

	static MidiEvent createXGResetEvent(long tick) throws InvalidMidiDataException
	{
		return createSysexEvent(tick, 0xf0, new byte[] {0x43, 0x10, 0x4c, 0x00, 0x00, 0x7e, 0x00, (byte)0xf7});
	}

	static MidiEvent createMasterVolumeEvent(long tick, int volume) throws InvalidMidiDataException
	{
		return createSysexEvent(tick, 0xf0, new byte[] {0x7f, 0x7f, 0x04, 0x01, 0x00, (byte)(volume & 0x7f), (byte)0xf7});
	}

	static MidiEvent createMetaEvent(long tick, int type, byte[] data) throws InvalidMidiDataException
	{
		MetaMessage message = new MetaMessage();
		message.setMessage(type, data, data.length);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createTextMetaEvent(long tick, int type, String text) throws InvalidMidiDataException
	{
		MetaMessage message = new MetaMessage();
		byte[] data = text.getBytes();
		message.setMessage(type, data, data.length);
		return new MidiEvent(message, tick);
	}

	static MidiEvent createTextEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_TEXT, text);
	}

	static MidiEvent createTextEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_TEXT, data);
	}

	static MidiEvent createCopyrightEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_COPYRIGHT, text);
	}

	static MidiEvent createCopyrightEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_COPYRIGHT, data);
	}

	static MidiEvent createSequenceNameEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_SEQUENCENAME, text);
	}

	static MidiEvent createSequenceNameEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_SEQUENCENAME, data);
	}

	static MidiEvent createTrackNameEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_TRACKNAME, text);
	}

	static MidiEvent createTrackNameEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_TRACKNAME, data);
	}

	static MidiEvent createInstrumentNameEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_INSTRUMENTNAME, text);
	}

	static MidiEvent createInstrumentNameEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_INSTRUMENTNAME, data);
	}

	static MidiEvent createLyricEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_LYRIC, text);
	}

	static MidiEvent createLyricEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_LYRIC, data);
	}

	static MidiEvent createMarkerEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_MARKER, text);
	}

	static MidiEvent createMarkerEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_MARKER, data);
	}

	static MidiEvent createCuePointEvent(long tick, String text) throws InvalidMidiDataException
	{
		return createTextMetaEvent(tick, META_EVENT_CUEPOINT, text);
	}

	static MidiEvent createCuePointEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_CUEPOINT, data);
	}

	static MidiEvent createChannelPrefixEvent(long tick, int prefix) throws InvalidMidiDataException
	{
		byte[] data = { (byte)prefix };
		return createMetaEvent(tick, META_EVENT_CHANNELPREFIX, data);
	}

	static MidiEvent createPortPrefixEvent(long tick, int prefix) throws InvalidMidiDataException
	{
		byte[] data = { (byte)prefix };
		return createMetaEvent(tick, META_EVENT_PORTPREFIX, data);
	}

	static MidiEvent createEndOfTrackEvent(long tick) throws InvalidMidiDataException
	{
		byte[] data = {};
		return createMetaEvent(tick, META_EVENT_ENDOFTRACK, data);
	}

	static MidiEvent createTempoEvent(long tick, double bpm) throws InvalidMidiDataException
	{
		long quarterNoteLen = (long)(60000000.0 / bpm);
		byte[] data = { (byte)(quarterNoteLen >> 16), (byte)(quarterNoteLen >> 8), (byte)quarterNoteLen };
		return createMetaEvent(tick, META_EVENT_TEMPO, data);
	}

	static MidiEvent createSMPTEOffsetEvent(long tick, int hr, int mn, int se, int fr, int ff) throws InvalidMidiDataException
	{
		byte[] data = { (byte)hr, (byte)mn, (byte)se, (byte)fr, (byte)ff };
		return createMetaEvent(tick, META_EVENT_SMPTEOFFSET, data);
	}

	static MidiEvent createTimeSignatureEvent(long tick, int numer, int denomMul) throws InvalidMidiDataException
	{
		return createTimeSignatureEvent(tick, numer, denomMul, 24, 8);
	}

	static MidiEvent createTimeSignatureEvent(long tick, int numer, int denomMul, int tickPerClick, int n32PerN4) throws InvalidMidiDataException
	{
		byte[] data = { (byte)numer, (byte)denomMul, (byte)tickPerClick, (byte)n32PerN4 };
		return createMetaEvent(tick, META_EVENT_TIMESIGNATURE, data);
	}

	static MidiEvent createKeySignatureEvent(long tick, int numSharpFlat, int majorOrMinor) throws InvalidMidiDataException
	{
		byte[] data = { (byte)numSharpFlat, (byte)majorOrMinor };
		return createMetaEvent(tick, META_EVENT_KEYSIGNATURE, data);
	}

	static MidiEvent createProprietaryMetaEvent(long tick, byte[] data) throws InvalidMidiDataException
	{
		return createMetaEvent(tick, META_EVENT_PROPRIETARY, data);
	}

}
