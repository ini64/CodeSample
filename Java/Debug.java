package com.chzero.socket.base;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class Debug {
	private Debug(){}
	final static int BUF_SZ = 64 * 1024;
	private static byte [] bytes = new byte[BUF_SZ];
	
	public synchronized static  void message(String Message)
	{
		System.out.println(Message);
	}
	
	/*public static  void message(String Message)
	{
		//System.out.println(Message);
	}*/
	
	public static void writeDebugInfo(String FullPath, ByteBuffer bb) throws FileNotFoundException
	{
		bb.flip();
		
		FileOutputStream outFiles = new FileOutputStream(FullPath);
		bb.get(bytes, 0, bb.limit());
		
		try {
			outFiles.write(bytes, 0, bb.limit());
			outFiles.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message("Debug-writeDebugInfo: " + e.getMessage());
		}
	}
	
	public static void readDebugInfo(String FullPath, ByteBuffer bb) throws FileNotFoundException
	{
		bb.clear();
		
		FileInputStream  outFiles = new FileInputStream (FullPath);
				
		try {
			int size = outFiles.read(bytes);
			outFiles.close();
		
			bb.put(bytes, 0 , size);
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message("Debug-readDebugInfo: " + e.getMessage());
		}
	}

}
