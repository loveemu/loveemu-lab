package com.googlecode.loveemu.PetiteMM.web;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.StringWriter;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.Sequence;

import com.googlecode.loveemu.PetiteMM.Midi2MML;

public class PetiteMMBean {

	private Integer timebase;

	private Integer dots;

	private Boolean octaveReverse;

	private Boolean useTriplet;

	private String mml;

	private String errorMessage;

	public Integer getTimebase() {
		return timebase;
	}

	public void setTimebase(Integer timebase) {
		this.timebase = timebase;
	}

	public Integer getDots() {
		return dots;
	}

	public void setDots(Integer dots) {
		this.dots = dots;
	}

	public Boolean getOctaveReverse() {
		return octaveReverse;
	}

	public void setOctaveReverse(Boolean octaveReverse) {
		this.octaveReverse = octaveReverse;
	}

	public Boolean getUseTriplet() {
		return useTriplet;
	}

	public void setUseTriplet(Boolean useTriplet) {
		this.useTriplet = useTriplet;
	}

	public String getMml() {
		return mml;
	}

	public void setMml(String mml) {
		this.mml = mml;
	}

	public String getErrorMessage() {
		return errorMessage;
	}

	public void setErrorMessage(String errorMessage) {
		this.errorMessage = errorMessage;
	}

	public void convertToMML(Sequence seq) throws IOException, InvalidMidiDataException {
		BufferedWriter writer = null;
		try
		{
			Midi2MML converter = new Midi2MML();
			if (timebase != null)
				converter.setTargetResolution(timebase);
			if (dots != null)
				converter.setMaxDots(dots);
			if (octaveReverse != null)
				converter.setOctaveReversed(octaveReverse);
			if (useTriplet != null)
				converter.setTripletPreference(useTriplet);

			StringWriter strWriter = new StringWriter();
			writer = new BufferedWriter(strWriter);
			converter.writeMML(seq, writer);
			writer.flush();
			this.setMml(strWriter.toString());
			this.setErrorMessage(null);
		}
		catch (IOException e)
		{
			this.setMml(null);
			this.setErrorMessage(e.getMessage());
			throw e;
		}
		catch (InvalidMidiDataException e)
		{
			this.setMml(null);
			this.setErrorMessage(e.getMessage());
			throw e;
		}
		finally
		{
			try {
				writer.close();
			} catch (IOException e) {}
		}
	}
}
