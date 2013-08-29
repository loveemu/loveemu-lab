package com.googlecode.loveemu.PetiteMM;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class MMLNoteConverter {

	/**
	 * Constant number for rest.
	 */
	public final static int KEY_REST = MMLNoteInfo.KEY_REST;

	/**
	 * Tick to MML note conversion table.
	 */
	private MMLNoteInfo[] notes;

	/**
	 * Tick to premitive note lengths table.
	 */
	private List<List<Integer>> noteLengths;

	/**
	 * Ticks per quarter note of MML.
	 */
	private int tpqn;

	/**
	 * Construct new MML note converter.
	 * @param tpqn Tick per quarter note of MML.
	 */
	public MMLNoteConverter(int tpqn) {
		setTPQN(tpqn);
		InitNoteTable(tpqn, -1);
	}

	/**
	 * Construct new MML note converter.
	 * @param tpqn Tick per quarter note of MML.
	 * @param maxDotCount Maximum count of dots of dotted-note allowed.
	 */
	public MMLNoteConverter(int tpqn, int maxDotCount) {
		setTPQN(tpqn);
		InitNoteTable(tpqn, maxDotCount);
	}

	/**
	 * Get the MML text of a note.
	 * @param length Note length in tick(s).
	 * @return MML text for the note. (octave will not be included)
	 */
	public String getNote(int length)
	{
		if (length < 0)
		{
			throw new IllegalArgumentException("Note length is negative.");
		}

		StringBuffer sb = new StringBuffer();
		while (length > (tpqn * 8))
		{
			sb.append(notes[tpqn * 8].getText());
			sb.append(MMLSymbol.TIE);
			length -= tpqn * 8;
		}
		sb.append(notes[length].getText());
		return sb.toString();
	}

	/**
	 * Get the MML text of a note.
	 * @param length Note length in tick(s).
	 * @param key Key of note, specify KEY_REST for a rest.
	 * @return MML text for the note. (octave will not be included)
	 */
	public String getNote(int length, int key)
	{
		if (length < 0)
		{
			throw new IllegalArgumentException("Note length must be a positive number.");
		}

		StringBuffer sb = new StringBuffer();
		while (length > (tpqn * 8))
		{
			sb.append(notes[tpqn * 8].getText(key));
			if (key != MMLNoteConverter.KEY_REST)
			{
				sb.append(MMLSymbol.TIE);
			}
			length -= tpqn * 8;
		}
		sb.append(notes[length].getText(key));
		return sb.toString();
	}

	/**
	 * Get list of notes which are necessary to express a certain length note.
	 * @param length Length of note to be expressed.
	 * @return List of note lengths.
	 */
	public List<Integer> getPrimitiveNoteLengths(int length)
	{
		List<Integer> lengths = new ArrayList<Integer>();

		if (length < 0)
			throw new IllegalArgumentException("Note length must be a positive number.");
		else if (length == 0)
		{
			lengths.add(0);
			return lengths;
		}

		// construct the final length list
		while (length > (tpqn * 8))
		{
			lengths.addAll(noteLengths.get(tpqn * 8));
			length -= tpqn * 8;
		}
		lengths.addAll(noteLengths.get(length));
		return lengths;
	}

	/**
	 * Get if the given note is a simple note.
	 * @param length Note length in tick(s).
	 * @return true if the note is simple enough, false otherwise.
	 */
	public boolean isSimpleNote(int length)
	{
		if (length < 0)
			throw new IllegalArgumentException("Note length must be a positive number.");
		else if (length == 0)
			return true;

		List<Integer> lengths = noteLengths.get(length % (tpqn * 4));
		return (lengths.size() <= 1);
	}

	/**
	 * Get timebase of MML.
	 * @return Ticks per quarter note.
	 */
	public int getTPQN() {
		return tpqn;
	}

	/**
	 * Set timebase of MML.
	 * @param tpqn Ticks per quarter note.
	 */
	public void setTPQN(int tpqn) {
		if (tpqn < 0)
		{
			throw new IllegalArgumentException("TPQN is negative.");
		}
		this.tpqn = tpqn;
	}

	/**
	 * Initialize the table for note conversion.
	 * @param tpqn Tick per quarter note of MML.
	 * @param maxDotCount Maximum count of dots of dotted-note allowed.
	 */
	private void InitNoteTable(int tpqn, int maxDotCount)
	{
		int tick;

		if (tpqn < 0)
		{
			throw new IllegalArgumentException("TPQN is negative.");
		}

		// construct the note table
		notes = new MMLNoteInfo[tpqn * 8 + 1];
		notes[0] = new MMLNoteInfo("");

		// initialize length table
		List<List<Integer>> singleNoteLengths = new ArrayList<List<Integer>>(tpqn * 8 + 1);
		noteLengths = new ArrayList<List<Integer>>(tpqn * 8 + 1);
		for (int mmlNoteLen = 0; mmlNoteLen <= (tpqn * 8); mmlNoteLen++)
		{
			singleNoteLengths.add(null);
			noteLengths.add(null);
		}

		// set single notes
		MMLNoteInfo[] singleNotes = new MMLNoteInfo[tpqn * 8 + 1];
		for (int mmlNoteLen = 1; mmlNoteLen <= (tpqn * 4); mmlNoteLen++)
		{
			if ((tpqn * 4) % mmlNoteLen != 0)
			{
				continue;
			}

			// simple note
			tick = (tpqn * 4) / mmlNoteLen;

			// create length table
			List<Integer> simpleNoteLength = new ArrayList<Integer>();
			simpleNoteLength.add(tick);

			// add new note
			notes[tick] = new MMLNoteInfo("$N" + mmlNoteLen);
			noteLengths.set(tick, simpleNoteLength);
			singleNotes[tick] = notes[tick];
			singleNoteLengths.set(tick, simpleNoteLength);

			// dotted notes
			int dot = 1;
			int baseNoteTick = tick;
			String mml = notes[tick].getText();
			while (baseNoteTick % (1 << dot) == 0)
			{
				// limit the maximum dot count
				if (maxDotCount >= 0 && dot > maxDotCount)
				{
					break;
				}

				// quit if the note length exceeds c1^c1
				tick += (baseNoteTick >> dot);
				if (tick > (tpqn * 8))
				{
					break;
				}

				// skip existing definitions
				if (notes[tick] != null)
				{
					dot++;
					continue;
				}

				// create length table
				List<Integer> dottedNoteLength = new ArrayList<Integer>();
				dottedNoteLength.add(tick);

				// add new note
				mml = mml + ".";
				notes[tick] = new MMLNoteInfo(mml);
				noteLengths.set(tick, dottedNoteLength);
				singleNotes[tick] = notes[tick];
				singleNoteLengths.set(tick, dottedNoteLength);

				dot++;
			}
		}

		// search for combinations such as c4^c16
		// having less notes (shorter text) is preferred
		boolean tableIsFilled = false;
		while (!tableIsFilled)
		{
			// make a shallow copy the note table to prevent recursive update
			MMLNoteInfo[] prevNotes = Arrays.copyOf(notes, notes.length);

			// process for all items
			for (tick = 1; tick < notes.length; tick++)
			{
				// skip existing definitions
				if (notes[tick] != null)
				{
					continue;
				}

				// search the combination
				String mml = null;
				List<Integer> multipleNoteLengths = null;
				for (int tickSub = tick - 1; tickSub > 0; tickSub--)
				{
					if (prevNotes[tickSub] != null && singleNotes[tick - tickSub] != null)
					{
						String newMML = prevNotes[tickSub].getText() + MMLSymbol.TIE + singleNotes[tick - tickSub].getText();
						if (mml == null || mml.length() > newMML.length())
						{
							mml = newMML;
							multipleNoteLengths = new ArrayList<Integer>(noteLengths.get(tickSub));
							multipleNoteLengths.addAll(noteLengths.get(tick - tickSub));
						}
					}
				}
				// add new note if available
				if (mml != null)
				{
					notes[tick] = new MMLNoteInfo(mml);
					noteLengths.set(tick, multipleNoteLengths);
				}
			}

			// quit if all items are set
			tableIsFilled = true;
			for (tick = 1; tick < notes.length; tick++)
			{
				if (notes[tick] == null)
				{
					tableIsFilled = false;
					break;
				}
			}
		}

		for (tick = 1; tick < tpqn * 8; tick++)
		{
			System.out.println("" + tick + "\t" + notes[tick]);
		}
	}
}
