package com.googlecode.loveemu.PetiteMM.web;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.servlet.RequestDispatcher;
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
				throw new IllegalArgumentException("Input file is invalid");
			InputStream midiStream = items.get("midi").getInputStream();

			PetiteMMBean bean = new PetiteMMBean();
			bean.setTimebase(getQueryInteger(items, "timebase"));
			bean.setDots(getQueryInteger(items, "dots"));
			bean.setOctaveReverse(getQueryString(items, "octaveReverse") != null);
			bean.setUseTriplet(getQueryString(items, "useTriplet") != null);
			try {
				bean.convertToMML(MidiSystem.getSequence(midiStream));
			} catch (IOException e) {
				//throw new ServletException("Conversion error", e);
			} catch (InvalidMidiDataException e) {
				//throw new ServletException("Conversion error", e);
			}
			request.setAttribute("petiteMMBean", bean);
			RequestDispatcher rDispatcher = request.getRequestDispatcher("/index.jsp");
			rDispatcher.forward(request, response);
		}
	}

	private Integer getQueryInteger(Map<String, FileItem> items, String name)
	{
		FileItem item = items.get(name);
		if (item != null && item.isFormField())
			return Integer.parseInt(item.getString());
		else
			return null;
	}

	private String getQueryString(Map<String, FileItem> items, String name)
	{
		FileItem item = items.get(name);
		if (item != null && item.isFormField())
			return item.getString();
		else
			return null;
	}
}
