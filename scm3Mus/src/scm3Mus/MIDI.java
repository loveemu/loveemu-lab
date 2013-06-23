package scm3Mus;

import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;

/**
 * Standard MIDI File main class
 *
 * This is part of the GBA SongRiper (c) 2012 by Bregalad
 * This is free and open source software.
 *
 * Notes : I tried to separate the GBA-related stuff and MIDI related stuff as much as possible in different classes
 * that way anyone can re-use this program for building MIDIs out of different data.
 *
 * SMF is a sequenced music file format based on MIDI interface. This class (and related classes) serves to create
 * data to build a format 0 MIDI file. I restricted it to format 0 MIDIS because it was simpler to only have one
 * single track to deal with. It wouldn't be too hard to upgrade this class to support format 1 MIDIs, though.
 *
 * All MIDI classes contains a field named midi that links to the MIDI main class to know which
 * MIDI file they relates to. (this would make it possible to build multiple SF2 files at a time).
 * Adding MIDI events is made extremely simple by a set of functions in this class. However if this is not sufficient
 * it is always possible to use the general purpose addMidiEvent() function, or adding data bytes directly to output.
 *
 * The MIDI standard uses the concept of delta_times before events. Since this is very much unpractical, this
 * class supposes the user to simply increment the time_ctr variable when needed, and not worry about delta
 * time values anmore.
 *
 * The building of a MIDI file is done in two passes :
 * - Creating the sequence data which is cached in memory
 * - Writing sequence data from cache to output file
 */

public class MIDI {

	final int delta_time_per_beat;

	//Static variables
	//Last RPN type used for each channel
	int[] last_rpn_type = new int[]{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,-1};
	//Last NRPN type used for each channel
	int[] last_nrpn_type = new int[]{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	//Changing this tables allows to reorder MIDI channels differently
	int[] chanel_reorder = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	//Last event type and channel, used for compression
	int last_chanel = -1;
	MIDIEventType last_event_type;

	//Time counter
	int time_ctr = 0;

	//Data in the track chunk
	//(this would become an array list of array list if this class were to be updated
	// to support format 1 MIDIs)
	ArrayList<Byte> data = new ArrayList<Byte>();

	MIDI(int delta_time) {
		delta_time_per_beat = delta_time;
	}

	//Write cached data to midi file
	void write(DataOutputStream out) throws IOException {
		//Add end-of-track meta event
		data.add((byte)0x00);
		data.add((byte)0xff);
		data.add((byte)0x2f);
		data.add((byte)0x00);

		//Write MIDI header
		out.writeBytes("MThd");
		out.writeInt(6);
		out.writeShort(0);
		out.writeShort(1);
		out.writeShort(delta_time_per_beat);

		//Write MIDI track data
		//we use SMF-0 standard therefore there is only a single track :)
		out.writeBytes("MTrk");
		out.writeInt(data.size());

		//Use a buffer to increase speed
		byte[] outbuf = new byte[data.size()];
		for(int i=0; i<data.size(); i++)
			outbuf[i] = data.get(i);

		out.write(outbuf);

		out.close();
	}

	//Add delta time in MIDI variable lenght coding
	void addVariableLengthCode(int code) {
		int word1 = code & 0x7f;
		int word2 = (code >> 7) & 0x7f;
		int word3 = (code >> 14) & 0x7f;
		int word4 = (code >> 21) & 0x7f;

		if(word4 != 0) {
			data.add((byte)(word4 | 0x80));
			data.add((byte)(word3 | 0x80));
			data.add((byte)(word2 | 0x80));
		}
		else if (word3 != 0) {
			data.add((byte)(word3 | 0x80));
			data.add((byte)(word2 | 0x80));
		}
		else if (word2 != 0) {
			data.add((byte)(word2 | 0x80));
		}
		data.add((byte)(word1));
	}

	//Add delta time event
	void addDeltaTime() {
		addVariableLengthCode(time_ctr);
		//Reset time counter to zero
		time_ctr = 0;
	}

	enum MIDIEventType{
		noteOff(8),
		noteOn(9),
		noteAft(10),
		controller(11),
		pchange(12),
		chnAft(13),
		pitchBend(14);

		final int value;

		MIDIEventType(int i) {
			value = i;
		}
	}

	void addMidiEvent(MIDIEventType type, int chn, int param1, int param2) {
		addDeltaTime();
		if(chn != last_chanel || type != last_event_type) {
			last_chanel = chn;
			last_event_type = type;
			data.add((byte)(type.value << 4 | chanel_reorder[chn]));
		}
		data.add((byte)param1);
		data.add((byte)param2);
	}

	void addMidiEvent(MIDIEventType type, int chn, int param) {
		addDeltaTime();
		if(chn != last_chanel || type != last_event_type) {
			last_chanel = chn;
			last_event_type = type;
			data.add((byte)(type.value << 4 | chanel_reorder[chn]));
		}
		data.add((byte)param);
	}

	//Add key on event
	void addNoteOn(int chn, int key, int vel) {
		addMidiEvent(MIDIEventType.noteOn, chn, key, vel);
	}

	//Add key off event
	void addNoteOff(int chn, int key, int vel) {
		addMidiEvent(MIDIEventType.noteOff, chn, key, vel);
	}

	//Add controller event
	void addController(int chn, int ctrl, int value) {
		addMidiEvent(MIDIEventType.controller, chn, ctrl, value);
	}

	//Add channel aftertouch
	void addChanAft(int chn, int value) {
		addMidiEvent(MIDIEventType.chnAft, chn, value);
	}

	//Add conventional program change event
	void addPChange(int chn, int number) {
		addMidiEvent(MIDIEventType.pchange, chn, number);
	}

	//Add pitch bend event
	void addPitchBend(int chn, int value) {
		int lo = value & 0x7f;
		int hi = value>>7 & 0x7f;
		addMidiEvent(MIDIEventType.pitchBend, chn, lo, hi);
	}

	//Add pitch bend event with only the MSB used
	void addPitchBend(int chn, byte value) {
		addMidiEvent(MIDIEventType.pitchBend, chn, 0, value);
	}

	//Add RPN event
	void addRPN(int chn, int type, int value) {
		if(last_rpn_type[chn] != type) {
			last_rpn_type[chn] = type;
			addMidiEvent(MIDIEventType.controller, chn, 100, type&0x7f);
			addMidiEvent(MIDIEventType.controller, chn, 101, type>>7);
			addMidiEvent(MIDIEventType.controller, chn, 6, value >> 7);
		} else {
			addMidiEvent(MIDIEventType.controller, chn, 6, value >> 7);
		}
		if((value & 0x7f) != 0)
			addMidiEvent(MIDIEventType.controller, chn, 38, value & 0x7f);
	}

	//Add RPN event with only the MSB of value used
	void addRPN(int chn, int type, byte value) {
		addRPN(chn, type, value<<7);
	}

	//Add NRPN event
	void addNRPN(int chn, int type, int value) {
		if(last_nrpn_type[chn] != type) {
			last_nrpn_type[chn] = type;
			addMidiEvent( MIDIEventType.controller, chn, 98, type&0x7f);
			addMidiEvent(MIDIEventType.controller, chn, 99, type>>7);
		}
		addMidiEvent(MIDIEventType.controller, chn, 6, value >> 7);
		if((value & 0x7f) != 0)
			addMidiEvent(MIDIEventType.controller, chn, 38, value & 0x7f);
	}

	//Add NRPN event with only the MSB of value used
	void addNRPN(int chn, int type, byte value) {
		addNRPN(chn, type, value<<7);
	}

	void addMarker(String text) {
		addDeltaTime();
		data.add((byte)-1);
		//Add text meta event if marker is false, marker meta even if true
		data.add((byte)6);
		addVariableLengthCode(text.length());
		//Add text itself
		for(int i=0; i<text.length(); i++)
			data.add((byte)text.charAt(i));
	}

	void addSysex(byte[] sysex_data) {
		addDeltaTime();
		data.add((byte)0xf0);
		//Actually variable length code
		addVariableLengthCode(sysex_data.length + 1);
		for(int i=0; i<sysex_data.length; i++)
			data.add(sysex_data[i]);
		data.add((byte)0xf7);
	}

	void addTempoChange(double tempo) {
		int t = (int)(60000000.0 / tempo);
		byte t1 = (byte)(t);
		byte t2 = (byte)(t>>8);
		byte t3 = (byte)(t>>16);

		addDeltaTime();
		data.add((byte)0xff);
		data.add((byte)0x51);
		data.add((byte)0x03);
		data.add(t3);
		data.add(t2);
		data.add(t1);
	}
}
