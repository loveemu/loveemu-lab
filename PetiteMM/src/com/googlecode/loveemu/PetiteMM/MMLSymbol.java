package com.googlecode.loveemu.PetiteMM;

public class MMLSymbol {

	/**
	 * MML note name table.
	 */
	public final static String[] NOTES = { "c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b" };

	/**
	 * MML text for rest.
	 */
	public final static String REST = "r";

	/**
	 * MML text for tie.
	 */
	public final static String TIE = "^";

	/**
	 * MML text for setting octave.
	 */
	public final static String OCTAVE = "o";

	/**
	 * MML text for tempo.
	 */
	public final static String TEMPO = "t";

	/**
	 * MML text for instrument.
	 */
	public final static String INSTRUMENT = "@";

	/**
	 * MML text for increasing octave.
	 */
	public final static String OCTAVE_UP = "<";

	/**
	 * MML text for decreasing octave.
	 */
	public final static String OCTAVE_DOWN = ">";

	/**
	 * MML text for end of track.
	 */
	public final static String TRACK_END = ";";

	/**
	 * MML text for triplet start.
	 */
	public final static String TRIPLET_START = "{";

	/**
	 * MML text for triplet end.
	 */
	public final static String TRIPLET_END = "}";
}
