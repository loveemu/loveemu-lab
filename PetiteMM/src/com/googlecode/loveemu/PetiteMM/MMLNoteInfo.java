package com.googlecode.loveemu.PetiteMM;

public class MMLNoteInfo {

	/**
	 * Constant number for rest.
	 */
	public final static int KEY_REST = -1000;

	/**
	 * Raw MML text for the note.
	 */
	private String text;

	/**
	 * Set true if the note is made by two or more notes with tie.
	 */
	public boolean usesMultipleNotes = false;

	/**
	 * Construct a new note info.
	 */
	public MMLNoteInfo() {
	}

	/**
	 * Construct a new note info.
	 * @param text MML for the note, $N will be replaced to a requested key at getText().
	 */
	public MMLNoteInfo(String text) {
		setText(text);
	}

	/**
	 * Construct a new note info.
	 * @param note Source note info.
	 */
	public MMLNoteInfo(MMLNoteInfo note) {
		setText(note.getText());
		this.usesMultipleNotes = note.usesMultipleNotes;
	}

	/**
	 * Get the MML text of a note.
	 */
	public String getText() {
		return text;
	}

	/**
	 * Get the MML text of a note.
	 * @param key Key of note, specify KEY_REST for a rest.
	 * @return MML text for the note. (octave will not be included)
	 */
	public String getText(int key) {
		if (text == null)
		{
			return null;
		}
		else if (key == KEY_REST)
		{
			return text.replaceAll("\\$N", MMLSymbol.REST).replaceAll("\\" + MMLSymbol.TIE, "");
		}
		else
		{
			int keyIndex;
			if (key >= 0)
			{
				keyIndex = key % MMLSymbol.NOTES.length;
			}
			else
			{
				keyIndex = -(-key % MMLSymbol.NOTES.length);
				if (keyIndex < 0)
				{
					keyIndex += MMLSymbol.NOTES.length;
				}
			}
			return text.replaceAll("\\$N", MMLSymbol.NOTES[keyIndex]);
		}
	}

	/**
	 * Set the MML text of a note.
	 * @param text MML for the note, $N will be replaced to a requested key at getText().
	 */
	public void setText(String text) {
		this.text = text;
	}
}
