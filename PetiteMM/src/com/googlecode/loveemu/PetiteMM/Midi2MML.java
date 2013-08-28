package com.googlecode.loveemu.PetiteMM;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.Sequence;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.Track;

public class Midi2MML {

	/**
	 * Maximum dot counts allowed for dotted-note.
	 */
	private int mmlMaxDotCount = -1;

	/**
	 * true if reverse the octave up/down effect.
	 */
	private boolean octaveReversed = false;

	/**
	 * true if replace triple single notes to triplet.
	 */
	private boolean useTriplet = false;

	/**
	 * Construct a new MIDI to MML converter.
	 */
	public Midi2MML()
	{
	}

	/**
	 * Construct a new MIDI to MML converter.
	 * @param obj
	 */
	public Midi2MML(Midi2MML obj)
	{
		mmlMaxDotCount = obj.mmlMaxDotCount;
		octaveReversed = obj.octaveReversed;
		useTriplet = obj.useTriplet;
	}

	/**
	 * Get the maximum dot counts allowed for dotted-note.
	 * @return Maximum dot counts allowed for dotted-note.
	 */
	public int getMmlMaxDotCount() {
		return mmlMaxDotCount;
	}

	/**
	 * Set the maximum dot counts allowed for dotted-note.
	 * @param mmlMaxDotCount Maximum dot counts allowed for dotted-note.
	 */
	public void setMmlMaxDotCount(int mmlMaxDotCount) {
		this.mmlMaxDotCount = mmlMaxDotCount;
	}

	/**
	 * Get if the octave up/down effect is reversed.
	 * @return True if reverse the octave up/down effect.
	 */
	public boolean isOctaveReversed() {
		return octaveReversed;
	}

	/**
	 * Set if the octave up/down effect is reversed.
	 * @param octaveReversed True if reverse the octave up/down effect.
	 */
	public void setOctaveReversed(boolean octaveReversed) {
		this.octaveReversed = octaveReversed;
	}

	/**
	 * Write MML of given sequence.
	 * @param seq Sequence to be converted.
	 * @param writer Destination to write MML text.
	 * @throws IOException throws if I/O error is happened.
	 * @throws UnsupportedOperationException throws if the situation is not supported.
	 * @throws InvalidMidiDataException throws if unexpected MIDI event is appeared.
	 */
	public void writeMML(Sequence seq, Writer writer) throws IOException, UnsupportedOperationException, InvalidMidiDataException
	{
		// sequence must be tick-based
		if (seq.getDivisionType() != Sequence.PPQ)
		{
			throw new UnsupportedOperationException("SMPTE is not supported.");
		}

		// preprocess
		// the converter assumes that all events in a track are for a single channel,
		// when the input file is SMF format 0 or something like that, it requires preprocessing.
		seq = MidiUtil.SeparateMixedChannel(seq);

		// scan MIDI notes
		int trackCount = seq.getTracks().length;
		List<List<MidiNote>> midiTrackNotes = new ArrayList<List<MidiNote>>(trackCount);
		long[] midiTracksEndTick = new long[trackCount];
		for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
		{
			Track track = seq.getTracks()[trackIndex];

			List<MidiNote> midiNotes = new ArrayList<MidiNote>();
			for (int midiEventIndex = 0; midiEventIndex < track.size(); midiEventIndex++)
			{
				MidiEvent event = track.get(midiEventIndex);
				midiTracksEndTick[trackIndex] = event.getTick();
				if (event.getMessage() instanceof ShortMessage)
				{
					ShortMessage message = (ShortMessage)event.getMessage();

					if (message.getCommand() == ShortMessage.NOTE_OFF ||
							(message.getCommand() == ShortMessage.NOTE_ON && message.getData2() == 0))
					{
						// search from head, for overlapping notes
						ListIterator<MidiNote> iter = midiNotes.listIterator();
						while (iter.hasNext())
						{
							MidiNote note = iter.next();
							int noteNumber = message.getData1();
							if (note.getLength() == -1 && note.getNoteNumber() == noteNumber)
							{
								note.setLength(event.getTick() - note.getTime());
								break;
							}
						}
					}
					else if (message.getCommand() == ShortMessage.NOTE_ON)
					{
						midiNotes.add(new MidiNote(message.getChannel(), event.getTick(), -1, message.getData1(), message.getData2()));
					}
				}
			}
			for (MidiNote note : midiNotes)
			{
				if (note.getLength() <= 0) {
					throw new InvalidMidiDataException("Sequence contains an unfinished or zero-length note.");
				}
				// dump for debug
				//System.out.format("[ch%d/%d] Note (%d) len=%d vel=%d\n", note.getChannel(), note.getTime(), note.getNoteNumber(), note.getLength(), note.getVelocity());
			}
			midiTrackNotes.add(midiNotes);
		}

		// reset track parameters
		Midi2MMLTrack[] mmlTracks = new Midi2MMLTrack[trackCount];
		for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
		{
			mmlTracks[trackIndex] = new Midi2MMLTrack();
			mmlTracks[trackIndex].setUseTriplet(useTriplet);
		}
		// reset subsystems
		MMLNoteConverter noteConv = new MMLNoteConverter(seq.getResolution(), mmlMaxDotCount);

		// convert tracks at the same time
		// reading tracks one by one would be simpler than the tick-based loop,
		// but it would limit handling a global event such as time signature.
		long tick = 0;
		int measure = 0;
		long measureLength = seq.getResolution() * 4;
		long nextMeasureTick = measureLength;
		int noteIndex = 0;
		int currNoteIndex = 0;
		boolean mmlFinished = false;
		while (!mmlFinished)
		{
			for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
			{
				Midi2MMLTrack mmlTrack = mmlTracks[trackIndex];
				Track track = seq.getTracks()[trackIndex];
				List<MidiNote> midiNotes = midiTrackNotes.get(trackIndex);

				while (!mmlTrack.isFinished())
				{
					// stop conversion when all events are dispatched
					if (mmlTrack.getMidiEventIndex() >= track.size())
					{
						mmlTrack.setFinished(true);
						break;
					}

					// get next MIDI message
					MidiEvent event = track.get(mmlTrack.getMidiEventIndex());
					if (event.getTick() != tick)
					{
						break;
					}
					mmlTrack.setMidiEventIndex(mmlTrack.getMidiEventIndex() + 1);

					// branch by event type for more detailed access
					List<MMLEvent> mmlEvents = new ArrayList<MMLEvent>();
					long mmlLastTick = mmlTrack.getTick();
					int mmlLastNoteNumber = mmlTrack.getNoteNumber();
					boolean mmlKeepCurrentNote = (mmlLastNoteNumber != MMLNoteConverter.KEY_REST);
					if (event.getMessage() instanceof ShortMessage)
					{
						ShortMessage message = (ShortMessage)event.getMessage();

						if (message.getCommand() == ShortMessage.NOTE_OFF ||
								(message.getCommand() == ShortMessage.NOTE_ON && message.getData2() == 0))
						{
							if (message.getData1() == mmlTrack.getNoteNumber())
							{
								MidiNote midiNextNote = (currNoteIndex + 1 < midiNotes.size()) ? midiNotes.get(currNoteIndex + 1) : null;
								long minLength = tick - mmlLastTick;
								long maxLength = ((midiNextNote != null) ? midiNextNote.getTime() : midiTracksEndTick[trackIndex]) - mmlLastTick;

								mmlTrack.setTick(mmlLastTick + minLength);
								mmlTrack.setNoteNumber(MMLNoteConverter.KEY_REST);
								mmlKeepCurrentNote = false;
							}
						}
						else if (message.getCommand() == ShortMessage.NOTE_ON)
						{
							int noteNumber = message.getData1();
							int noteOctave = noteNumber / 12;

							// write some initialization for the first note
							if (mmlTrack.isFirstNote())
							{
								mmlTrack.setOctave(noteOctave);
								mmlTrack.setFirstNote(false);
								mmlEvents.add(new MMLEvent(MMLSymbol.OCTAVE, new String[] { String.format("%d", noteOctave) }));
							}

							mmlEvents.addAll(convertMidiEventToMML(event, mmlTrack));

							// remember new note
							mmlTrack.setTick(tick);
							mmlTrack.setOctave(noteOctave);
							mmlTrack.setNoteNumber(noteNumber);
							mmlKeepCurrentNote = false;

							currNoteIndex = noteIndex;
							noteIndex++;
						}
						else
						{
							List<MMLEvent> newMML = convertMidiEventToMML(event, mmlTrack);
							if (newMML.size() != 0)
							{
								mmlEvents.addAll(newMML);
								if (tick >= mmlLastTick)
									mmlTrack.setTick(tick);
							}
						}
					}
					else if (event.getMessage() instanceof MetaMessage)
					{
						MetaMessage message = (MetaMessage)event.getMessage();
						byte[] data = message.getData();

						switch (message.getType())
						{
						case MidiUtil.META_TIME_SIGNATURE:
							if (data.length != 4)
							{
								throw new InvalidMidiDataException("Illegal time signature event.");
							}

							if (nextMeasureTick - measureLength != tick)
							{
								throw new InvalidMidiDataException("Time signature event is not located at the measure boundary.");
							}

							int newMeasureLength = ((seq.getResolution() * 4) * (data[0] & 0xff)) >> (data[1] & 0xff);
							nextMeasureTick = (nextMeasureTick - measureLength) + newMeasureLength;
							measureLength = newMeasureLength;
							break;

						default:
							List<MMLEvent> newMML = convertMidiEventToMML(event, mmlTrack);
							if (newMML.size() != 0)
							{
								mmlEvents.addAll(newMML);
								if (tick >= mmlLastTick)
									mmlTrack.setTick(tick);
							}
							break;
						}
					}
					else
					{
						List<MMLEvent> newMML = convertMidiEventToMML(event, mmlTrack);
						if (newMML.size() != 0)
						{
							mmlEvents.addAll(newMML);
							if (tick >= mmlLastTick)
								mmlTrack.setTick(tick);
						}
					}

					// final event,
					// seek to the last whether the last event has been dispatched.
					//if (mmlTrack.getMidiEventIndex() == track.size())
					//{
					//	if (!mmlTrack.isEmpty())
					//	{
					//		if (mmlTrack.getTick() < tick)
					//		{
					//			mmlTrack.setTick(tick);
					//		}
					//	}
					//}

					// timing changed,
					// write the last note/rest and finish the seek
					if (mmlTrack.getTick() != mmlLastTick)
					{
						mmlTrack.add(new MMLEvent(noteConv.getNote((int)(mmlTrack.getTick() - mmlLastTick), mmlLastNoteNumber)));
						if (mmlKeepCurrentNote)
						{
							mmlTrack.add(new MMLEvent(MMLSymbol.TIE));
						}

						if (mmlTrack.getMeasure() != measure)
						{
							mmlTrack.add(new MMLEvent(System.getProperty("line.separator")));
							mmlTrack.setMeasure(measure);
						}
					}

					// event is dispatched,
					// write the new MML command
					if (mmlEvents.size() != 0)
					{
						mmlTrack.addAll(mmlEvents);
					}
				}
			}

			mmlFinished = true;
			for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
			{
				if (!mmlTracks[trackIndex].isFinished())
				{
					mmlFinished = false;
					break;
				}
			}

			tick++;
			if (tick >= nextMeasureTick)
			{
				measure++;
				nextMeasureTick += measureLength;
			}
		}

		boolean firstTrackWrite = true;
		for (Midi2MMLTrack mmlTrack : mmlTracks)
		{
			if (!mmlTrack.isEmpty())
			{
				if (firstTrackWrite)
					firstTrackWrite = false;
				else
				{
					writer.write(MMLSymbol.TRACK_END);
					writer.write(System.getProperty("line.separator"));
				}
				mmlTrack.writeMML(writer);
			}
		}
		writer.flush();
	}

	/**
	 * Convert specified MIDI event to MML.
	 * @param event MIDI event to be converted.
	 * @param mmlTrack MML track status.
	 * @return Converted text, null if event is ignored.
	 * @throws InvalidMidiDataException throws if unexpected MIDI event is appeared.
	 */
	private List<MMLEvent> convertMidiEventToMML(MidiEvent event, Midi2MMLTrack mmlTrack) throws InvalidMidiDataException
	{
		List<MMLEvent> mmlEvents = new ArrayList<MMLEvent>();
		if (event.getMessage() instanceof ShortMessage)
		{
			ShortMessage message = (ShortMessage)event.getMessage();

			if (message.getCommand() == ShortMessage.NOTE_ON)
			{
				int noteNumber = message.getData1();
				int noteOctave = noteNumber / 12;
				int mmlOctave = mmlTrack.getOctave();

				// adjust octave
				while (mmlOctave < noteOctave)
				{
					mmlEvents.add(new MMLEvent(!octaveReversed ? MMLSymbol.OCTAVE_UP : MMLSymbol.OCTAVE_DOWN));
					mmlOctave++;
				}
				while (mmlOctave > noteOctave)
				{
					mmlEvents.add(new MMLEvent(!octaveReversed ? MMLSymbol.OCTAVE_DOWN : MMLSymbol.OCTAVE_UP));
					mmlOctave--;
				}

				// for some reasons, this function does not return the actual note command.
			}
			else if (message.getCommand() == ShortMessage.PROGRAM_CHANGE)
			{
				//mmlEvents.add(new MMLEvent(MMLSymbol.INSTRUMENT, new String[] { String.format("%d", message.getData1()) }));
			}
		}
		else if (event.getMessage() instanceof MetaMessage)
		{
			MetaMessage message = (MetaMessage)event.getMessage();
			byte[] data = message.getData();

			switch (message.getType())
			{
			case MidiUtil.META_TEMPO:
				if (data.length != 3)
				{
					throw new InvalidMidiDataException("Illegal tempo event.");
				}

				int usLenOfQN = ((data[0] & 0xff) << 16) | ((data[1] & 0xff) << 8) | (data[2] & 0xff);
				double bpm = 60000000.0 / usLenOfQN;
				mmlEvents.add(new MMLEvent(MMLSymbol.TEMPO, new String[] { String.format("%.0f", bpm) }));
				break;
			}
		}
		return mmlEvents;
	}

	/**
	 * Get triplet preference.
	 * @return true if replace triple single notes to triplet.
	 */
	public boolean getTripletPreference() {
		return useTriplet;
	}

	/**
	 * Set triplet preference.
	 * @return true if replace triple single notes to triplet.
	 */
	public void setTripletPreference(boolean useTriplet) {
		this.useTriplet = useTriplet;
	}
}
