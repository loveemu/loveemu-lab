package com.googlecode.loveemu.PetiteMM;

import java.util.Arrays;

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
	 * true if note uses tie, false otherwise.
	 */
	private boolean[] noteUsesTie;

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
			throw new IllegalArgumentException("Note length is negative.");
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
	 * Get if the given note is a simple note.
	 * @param length Note length in tick(s).
	 * @return true if the note is simple enough, false otherwise.
	 */
	public boolean isSimpleNote(int length)
	{
		return !noteUsesTie[length % (tpqn * 4)];
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
		noteUsesTie = new boolean[tpqn * 8 + 1];
		noteUsesTie[0] = false;

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
			notes[tick] = new MMLNoteInfo("$N" + mmlNoteLen);
			notes[tick].usesMultipleNotes = false;
			singleNotes[tick] = notes[tick];

			// dotted notes
			int dot = 1;
			int baseNoteTick = tick;
			String mml = notes[tick].getText();
			while (tick % (1 << dot) == 0)
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
					continue;
				}

				// add new note
				mml = mml + ".";
				notes[tick] = new MMLNoteInfo(mml);
				notes[tick].usesMultipleNotes = false;
				singleNotes[tick] = notes[tick];

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
				for (int tickSub = tick - 1; tickSub > 0; tickSub--)
				{
					if (prevNotes[tickSub] != null && singleNotes[tick - tickSub] != null)
					{
						String newMML = prevNotes[tickSub].getText() + MMLSymbol.TIE + singleNotes[tick - tickSub].getText();
						if (mml == null || mml.length() > newMML.length())
						{
							mml = newMML;
						}
					}
				}
				// add new note if available
				if (mml != null)
				{
					notes[tick] = new MMLNoteInfo(mml);
					notes[tick].usesMultipleNotes = true;
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

		// construct the single note table at last
		for (int mmlNoteLen = 1; mmlNoteLen <= (tpqn * 8); mmlNoteLen++)
		{
			noteUsesTie[mmlNoteLen] = (singleNotes[mmlNoteLen] == null);
		}
	}
}
