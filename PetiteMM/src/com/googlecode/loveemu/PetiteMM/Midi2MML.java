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
	 * true if write debug informations to stdout.
	 */
	final static private boolean debugDump = false;

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

		// get track count (this must be after the preprocess)
		int trackCount = seq.getTracks().length;

		// scan end timing for each tracks
		long[] midiTracksEndTick = new long[trackCount];
		for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
		{
			Track track = seq.getTracks()[trackIndex];
			midiTracksEndTick[trackIndex] = track.get(track.size() - 1).getTick();
		}

		// scan MIDI notes
		List<List<MidiNote>> midiTrackNotes = getMidiNotes(seq);

		// scan time signatures
		List<MidiTimeSignature> timeSignatures = getMidiTimeSignatures(seq);
		if (debugDump)
		{
			for (MidiTimeSignature timeSignature : timeSignatures)
				System.out.println(timeSignature);
		}

		// reset track parameters
		Midi2MMLTrack[] mmlTracks = new Midi2MMLTrack[trackCount];
		int[] noteIndex = new int[trackCount];
		int[] currNoteIndex = new int[trackCount];
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

					// dump for debug
					if (debugDump)
						System.out.format("MidiEvent: track=%d,tick=%d,message=%s\n", trackIndex, event.getTick(), byteArrayToString(event.getMessage().getMessage()));

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
								MidiNote midiNextNote = (currNoteIndex[trackIndex] + 1 < midiNotes.size()) ? midiNotes.get(currNoteIndex[trackIndex] + 1) : null;
								long minLength = tick - mmlLastTick;
								long maxLength = ((midiNextNote != null) ? midiNextNote.getTime() : midiTracksEndTick[trackIndex]) - mmlLastTick;
								long length = maxLength;

								// separate rest longer or equal to whole note
								while (length - (seq.getResolution() * 4) >= minLength)
									length -= (seq.getResolution() * 4);

								// halve the length as far as possible
								while ((length / 2) >= minLength)
									length /= 2;

								// quantization
								int divider = 1;
								while (divider < 4 && seq.getResolution() % divider == 0)
									divider *= 2;
								while (length - (seq.getResolution() / divider) >= minLength)
									length -= (seq.getResolution() / divider);

								if (debugDump)
									System.out.format("Note Off: tick=%d,mmlLastTick=%d,length=%d,minLength=%d,maxLength=%d,next=%s\n", tick, mmlLastTick, length, minLength, maxLength, (midiNextNote != null) ? midiNextNote.toString() : "null");

								mmlTrack.setTick(mmlLastTick + length);
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

							// remember new note
							mmlTrack.setTick(tick);
							mmlTrack.setNoteNumber(noteNumber);
							mmlKeepCurrentNote = false;

							currNoteIndex[trackIndex] = noteIndex[trackIndex];
							noteIndex[trackIndex]++;
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
					if (mmlTrack.getMidiEventIndex() == track.size())
					{
						if (!mmlTrack.isEmpty())
						{
							if (mmlTrack.getTick() < tick)
							{
								mmlTrack.setTick(tick);
							}
						}
					}

					// timing changed,
					// write the last note/rest and finish the seek
					if (mmlTrack.getTick() != mmlLastTick)
					{
						if (debugDump)
							System.out.println("Timing: " + mmlLastTick + " -> " + mmlTrack.getTick());

						if (mmlLastNoteNumber == MMLNoteConverter.KEY_REST)
						{
							List<Integer> lengths = noteConv.getPrimitiveNoteLengths((int)(mmlTrack.getTick() - mmlLastTick));
							for (int length : lengths)
							{
								mmlTrack.add(new MMLEvent(noteConv.getNote(length, mmlLastNoteNumber)));

								int lastMeasure = MidiTimeSignature.getMeasureByTick(mmlLastTick, timeSignatures, seq.getResolution());
								int currentMeasure = MidiTimeSignature.getMeasureByTick(mmlLastTick + length, timeSignatures, seq.getResolution());
								if (currentMeasure != lastMeasure)
								{		
									mmlTrack.add(new MMLEvent(System.getProperty("line.separator")));
									mmlTrack.setMeasure(currentMeasure);
								}
							}
						}
						else
						{
							int mmlOctave = mmlTrack.getOctave();
							int noteOctave = mmlLastNoteNumber / 12;
							while (mmlOctave < noteOctave)
							{
								mmlTrack.add(new MMLEvent(!octaveReversed ? MMLSymbol.OCTAVE_UP : MMLSymbol.OCTAVE_DOWN));
								mmlOctave++;
							}
							while (mmlOctave > noteOctave)
							{
								mmlTrack.add(new MMLEvent(!octaveReversed ? MMLSymbol.OCTAVE_DOWN : MMLSymbol.OCTAVE_UP));
								mmlOctave--;
							}
							mmlTrack.setOctave(noteOctave);

							mmlTrack.add(new MMLEvent(noteConv.getNote((int)(mmlTrack.getTick() - mmlLastTick), mmlLastNoteNumber)));
							if (mmlKeepCurrentNote)
							{
								mmlTrack.add(new MMLEvent(MMLSymbol.TIE));
							}

							int lastMeasure = MidiTimeSignature.getMeasureByTick(mmlLastTick, timeSignatures, seq.getResolution());
							int currentMeasure = MidiTimeSignature.getMeasureByTick(mmlTrack.getTick(), timeSignatures, seq.getResolution());
							if (currentMeasure != lastMeasure)
							{	
								mmlTrack.add(new MMLEvent(System.getProperty("line.separator")));
								mmlTrack.setMeasure(currentMeasure);
							}
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
	 * Get MIDI notes from sequence.
	 * @param seq Input MIDI sequence.
	 * @return List of MIDI notes.
	 * @throws InvalidMidiDataException throws if unexpected MIDI event is appeared.
	 */
	private List<List<MidiNote>> getMidiNotes(Sequence seq) throws InvalidMidiDataException
	{
		final int trackCount = seq.getTracks().length;

		List<List<MidiNote>> midiTrackNotes = new ArrayList<List<MidiNote>>(trackCount);
		for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
		{
			Track track = seq.getTracks()[trackIndex];

			List<MidiNote> midiNotes = new ArrayList<MidiNote>();
			for (int midiEventIndex = 0; midiEventIndex < track.size(); midiEventIndex++)
			{
				MidiEvent event = track.get(midiEventIndex);
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
				if (debugDump)
					System.out.format("[ch%d/%d] Note (%d) len=%d vel=%d\n", note.getChannel(), note.getTime(), note.getNoteNumber(), note.getLength(), note.getVelocity());
			}
			midiTrackNotes.add(midiNotes);
		}
		return midiTrackNotes;
	}

	/**
	 * Get MIDI time signatures from sequence.
	 * @param seq Input MIDI sequence.
	 * @return List of MIDI time signatures.
	 * @throws InvalidMidiDataException throws if unexpected MIDI event is appeared.
	 */
	private List<MidiTimeSignature> getMidiTimeSignatures(Sequence seq) throws InvalidMidiDataException
	{
		List<MidiTimeSignature> timeSignatures = new ArrayList<MidiTimeSignature>();

		final int trackCount = seq.getTracks().length;
		final int defaultNumerator = 4;
		final int defaultDenominator = 2;

		int numerator = defaultNumerator;
		int denominator = defaultDenominator;
		long measureLength = ((seq.getResolution() * 4 * numerator) >> denominator);
		long nextMeasureTick = measureLength;

		long tick = 0;
		int measure = 0;
		int measureOfLastSignature = -1;
		boolean finished = false;
		int[] eventIndex = new int[trackCount];
		while (!finished)
		{
			if (tick == nextMeasureTick)
			{
				nextMeasureTick += measureLength;
				measure++;
			}

			for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
			{
				Track track = seq.getTracks()[trackIndex];
				while (eventIndex[trackIndex] < track.size())
				{
					MidiEvent event = track.get(eventIndex[trackIndex]);
					if (event.getTick() != tick)
						break;
					eventIndex[trackIndex]++;

					if (event.getMessage() instanceof MetaMessage)
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

							if (measure == measureOfLastSignature)
							{
								throw new InvalidMidiDataException("Two or more time signature event are located at the same time.");
							}

							if (timeSignatures.isEmpty() && measure != 0)
							{
								throw new InvalidMidiDataException("First time signature is not located at the first measure.");
							}

							MidiTimeSignature newTimeSignature = new MidiTimeSignature(data[0] & 0xff, data[1] & 0xff, measure);
							int newMeasureLength = newTimeSignature.getLength(seq.getResolution());
							nextMeasureTick = (nextMeasureTick - measureLength) + newMeasureLength;
							measureLength = newMeasureLength;
							measureOfLastSignature = measure;
							timeSignatures.add(newTimeSignature);
							break;
						}
					}
				}
			}

			finished = true;
			for (int trackIndex = 0; trackIndex < trackCount; trackIndex++)
			{
				Track track = seq.getTracks()[trackIndex];
				if (eventIndex[trackIndex] < track.size())
				{
					finished = false;
					break;
				}
			}

			tick++;
		}
		
		if (timeSignatures.isEmpty())
		{
			timeSignatures.add(new MidiTimeSignature(defaultNumerator, defaultDenominator));
		}

		return timeSignatures;
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
				// for some reasons, this function does not dispatch note on.
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

	private String byteArrayToString(byte[] bytes) {
		StringBuffer buf = new StringBuffer();
		for (byte b : bytes) {
			if (buf.length() != 0)
				buf.append(" ");
			buf.append(String.format("%02X", b));
		}
		return buf.toString();
	}
}
