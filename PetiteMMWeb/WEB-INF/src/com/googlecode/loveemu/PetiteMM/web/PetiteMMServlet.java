package com.googlecode.loveemu.PetiteMM.web;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiSystem;

import org.apache.commons.fileupload.FileItem;
import org.apache.commons.fileupload.FileUploadException;
import org.apache.commons.fileupload.disk.DiskFileItemFactory;
import org.apache.commons.fileupload.servlet.ServletFileUpload;

import com.googlecode.loveemu.PetiteMM.Midi2MML;

public class PetiteMMServlet extends HttpServlet {

	private static final long serialVersionUID = -3889106608881079433L;

	private static final int UPLOAD_FILE_SIZE_MAX = 1024 * 1024;

	@Override
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException
	{
		if (ServletFileUpload.isMultipartContent(request))
		{
			DiskFileItemFactory factory = new DiskFileItemFactory();
			//factory.setSizeThreshold(16 * 1024);
			//factory.setRepository(new File(System.getProperty("java.io.tmpdir")));

			ServletFileUpload upload = new ServletFileUpload(factory);
			//upload.setSizeMax(UPLOAD_FILE_SIZE_MAX); // request message size
			upload.setFileSizeMax(UPLOAD_FILE_SIZE_MAX);

			// get request parameters
			List<FileItem> itemList;
			try {
				itemList = upload.parseRequest(request);
			} catch (FileUploadException e) {
				throw new ServletException(e);
			}

			// convert request list to map
			Map<String, FileItem> items = new HashMap<String, FileItem>();
			for (FileItem item : itemList) {
				items.put(item.getFieldName(), item);
			}

			if (!items.containsKey("midi") || items.get("midi").isFormField())
			{
				throw new IllegalArgumentException("Input file is invalid");
			}
			InputStream midiStream = items.get("midi").getInputStream();

			Midi2MML converter = new Midi2MML();
			try {
				converter.writeMML(MidiSystem.getSequence(midiStream), response.getWriter());
			} catch (InvalidMidiDataException e) {
				throw new ServletException("Input file is invalid", e);
			}
		}
	}
}
