package com.chzero.socket.working;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.LinkedList;

import com.chzero.socket.base.Debug;
import com.chzero.socket.base.FileData;
import com.chzero.socket.base.INT;
import com.chzero.socket.packet.TcpSend;


public class FileWorking extends Thread{
	
	protected LinkedList parameters = new LinkedList();
	protected Selector selector = null;
	protected TCP tcp = null;
	protected TcpSend tcpSend = new TcpSend();
	
	protected INT bufferPos = new INT(0);
	
	final static int BUF_SZ = 64 * 1024;
	protected ByteBuffer buffer = ByteBuffer.allocateDirect(BUF_SZ);
	
	public FileWorking(Selector Select)
	{
		Debug.message("FileWorking 기동");
		this.setDaemon(true);
		selector = Select;
	}
	
	@Override
	public void run()
	{
		Select();
	}
	
	public void setTcp(TCP t)
	{
		 tcp = t;
	}
	
	public synchronized void Insert(LinkedList result)
	{
		Iterator it = result.iterator();
		
		while(it.hasNext())
		{
			FileData data = (FileData) it.next();
			parameters.add(data);
		}
		
		WakeUp();
	}
	
	private synchronized FileData Get()
	{
		Iterator it = parameters.iterator();
		if(it.hasNext())
		{
			FileData imageData = (FileData) it.next();
			it.remove();
			
			return imageData;
		}
		return null;
	}
	
	public synchronized void WakeUp()
	{
		selector.wakeup();
	}
	
	public void Select()
	{
		 while (true) 
		 {
			 try{
				selector.select();
				
				FileData imageData = Get();
				
				while(null != imageData)
				{
					imageData.writeImage();
					
					tcpSend.SetCmd(imageData.getCmd());
					tcpSend.SetPoleID(imageData.getPoldId());
					tcpSend.writeBuffer(buffer, bufferPos);
					tcp.write(buffer);
					
					imageData = Get();
				}
			 }
			 catch (IOException e) { continue; } 
			 catch(ClosedSelectorException e){break;}
		 }
		 Debug.message("FileWorking 종료");
	}
}
