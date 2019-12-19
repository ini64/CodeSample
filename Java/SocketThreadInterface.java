package com.chzero.socket.working;

import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;


public interface SocketThreadInterface {
	public void startThread();
	public void endThread();
	public void select();
	
	public boolean read(SelectionKey key);
	public boolean write(ByteBuffer bb);
	public boolean accept(SelectionKey key);
	
	//public void udpWrite(SelectionKey key) throws IOException;
	//public void tcpRead(SelectionKey key) throws IOException;
	//public void tcpWrite(SelectionKey key) throws IOException;
}
