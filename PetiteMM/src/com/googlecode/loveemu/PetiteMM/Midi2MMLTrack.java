package com.googlecode.loveemu.PetiteMM;

import java.io.IOException;
import java.io.Writer;

class Midi2MMLTrack {

	/**
	 * Output MML text.
	 */
	private StringBuffer mml = new StringBuffer();

	/**
	 * Current position of conversion in tick.
	 */
	private long tick = 0L;

	/**
	 * Measure number of current position.
	 */
	private int measure = 0;

	/**
	 * Current octave.
	 */
	private int octave = 4;

	/**
	 * true if the first note is not processed yet.
	 */
	private boolean firstNote = true;

	/**
	 * Current note number. (for MIDI2MML conversion)
	 */
	private int noteNumber = MMLNoteConverter.KEY_REST;

	/**
	 * Current MIDI event index.
	 */
	private int midiEventIndex = 0;

	/**
	 * True if conversion is already finished.
	 */
	private boolean finished = false;

	/**
	 * Construct new MML track conversion object.
	 */
	Midi2MMLTrack()
	{
	}

	/**
	 * Get current position of conversion in tick.
	 * @return Current time in tick.
	 */
	public long getTick() {
		return tick;
	}

	/**
	 * Set current position of conversion in tick.
	 * @param tick Current time in tick.
	 */
	public void setTick(long tick) {
		this.tick = tick;
	}

	/**
	 * Get measure number of current position.
	 * @return Measure number of current position.
	 */
	public int getMeasure() {
		return measure;
	}

	/**
	 * Set measure number of current position.
	 * @param measure Measure number of current position.
	 */
	public void setMeasure(int measure) {
		this.measure = measure;
	}

	/**
	 * Get current octave.
	 * @return Current octave.
	 */
	public int getOctave() {
		return octave;
	}

	/**
	 * Set current octave.
	 * @param octave Current octave.
	 */
	public void setOctave(int octave) {
		this.octave = octave;
	}

	/**
	 * Increase current octave.
	 */
	public void increaseOctave() {
		this.octave++;
	}

	/**
	 * Decrease current octave.
	 */
	public void decreaseOctave() {
		this.octave--;
	}

	/**
	 * Get current note number.
	 * @return Current note number, MMLNoteConverter.KEY_REST if rest.
	 */
	public int getNoteNumber() {
		return noteNumber;
	}

	/**
	 * Set current note number.
	 * @param noteNumber Current note number, MMLNoteConverter.KEY_REST if rest.
	 */
	public void setNoteNumber(int noteNumber) {
		this.noteNumber = noteNumber;
	}

	/**
	 * Get current MIDI event index.
	 * @return Current MIDI event index.
	 */
	public int getMidiEventIndex() {
		return midiEventIndex;
	}

	/**
	 * Set current MIDI event index.
	 * @param midiEventIndex Current MIDI event index.
	 */
	public void setMidiEventIndex(int midiEventIndex) {
		this.midiEventIndex = midiEventIndex;
	}

	/**
	 * Clear the current MML text.
	 */
	void clear()
	{
		mml.setLength(0);
	}

	/**
	 * Get if the first note is processed or not.
	 * @return true if the first note is not processed yet.
	 */
	public boolean isFirstNote() {
		return firstNote;
	}

	/**
	 * Set if the first note is processed or not.
	 * @param firstNote true if the first note is not processed yet.
	 */
	public void setFirstNote(boolean firstNote) {
		this.firstNote = firstNote;
	}

	/**
	 * Get if conversion is already finished.
	 * @return True if conversion is already finished.
	 */
	public boolean isFinished() {
		return finished;
	}

	/**
	 * Set if conversion is already finished.
	 * @return True if conversion is already finished.
	 */
	public void setFinished(boolean finished) {
		this.finished = finished;
	}

	/**
	 * Appends the specified MML text.
	 * @param str MML text.
	 */
	public void appendMML(String str)
	{
		mml.append(str);
	}

	/**
	 * Returns true if, and only if, length() is 0.
	 * @return true if length() is 0, otherwise false
	 */
	public boolean isEmpty()
	{
		return mml.length() == 0;
	}

	/**
	 * Use triplet rather than a simple note.
	 */
	public void convertToTriplet()
	{
		String mmlString = mml.toString();
		for (int i = 0; i < 8; i++)
		{
			int noteLenTo = 1 << i;
			int noteLenFrom = noteLenTo * 3;
			mmlString = mmlString.replaceAll("([abcdefgr\\^][\\+\\-]*)" + noteLenFrom + "(\\s*[abcdefgr\\^][\\+\\-]*)" + noteLenFrom + "(\\s*[abcdefgr\\^][\\+\\-]*)" + noteLenFrom, "\\" + MMLSymbol.TRIPLET_START + "$1\\" + noteLenTo + "$2\\" + noteLenTo + "$3\\" + noteLenTo + "\\" + MMLSymbol.TRIPLET_END);
		}

		mml.setLength(0);
		mml.append(mmlString);
	}

	/**
	 * Write the final MML.
	 * @param writer Destination to write MML text.
	 * @throws IOException throws if I/O error is happened.
	 */
	void writeMML(Writer writer) throws IOException
	{
		if (mml.length() != 0)
		{
			String mmlString = mml.toString();
			writer.write(mmlString);

			if (!mmlString.endsWith(System.getProperty("line.separator")))
			{
				writer.write(System.getProperty("line.separator"));
			}
		}
	}
}
