package com.chzero.socket.base;

import java.net.SocketAddress;
import java.nio.ByteBuffer;

public class IO implements IOInterface{
	
	final static int bufferSize = 64 * 1024;
	
	public ByteBuffer buffer;
	public SocketAddress socketAddr;

	
	public IO()
	{
		buffer = ByteBuffer.allocateDirect(bufferSize);
	}
	
	protected boolean typeUDP = true;
	public boolean isTypeUDP() {
		return typeUDP;
	}
	public void setTypeUDP(boolean typeUDP) {
		this.typeUDP = typeUDP;
	}
	public SocketAddress getSocketAddr() {
		return socketAddr;
	}
	public void setSocketAddr(SocketAddress socketAddr) {
		this.socketAddr = socketAddr;
	}
	public static int getBuffersize() {
		return bufferSize;
	}
	public ByteBuffer getBuffer() {
		return buffer;
	}
	
	@Override
	public void close()
	{
		Debug.message(socketAddr.toString() + ": ≈¨∑Œ¡Ó");
	}
	
	
}
