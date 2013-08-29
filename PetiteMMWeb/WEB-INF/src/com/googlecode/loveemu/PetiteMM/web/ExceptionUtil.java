package com.googlecode.loveemu.PetiteMM.web;

import java.io.PrintWriter;
import java.io.StringWriter;

public class ExceptionUtil
{
	Exception myException;

	/**
	 * 例外処理ユーティリティ コンストラクタ
	 * @param e 処理対象の例外
	 */
	public ExceptionUtil(Exception e)
	{
		myException = e;
	}

	/**
	 * 例外の詳細を printStackTrace と同等の文字列形式で取得する
	 */
	public String getStackTraceString()
	{
		return ExceptionUtil.getStackTraceString(myException);
	}

	/**
	 * 例外の詳細を printStackTrace と同等の文字列形式で取得する
	 * @param e メッセージ取得対象の例外
	 */
	public static String getStackTraceString(Exception e)
	{
		StringWriter sw = new StringWriter();
		e.printStackTrace(new PrintWriter(sw));
		return sw.toString();
	}
}
