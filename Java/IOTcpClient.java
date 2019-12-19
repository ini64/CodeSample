package com.chzero.socket.base;

import java.io.IOException;
import java.net.Socket;
import java.nio.channels.SocketChannel;

public class IOTcpClient extends IO{

	public Socket socket;
	protected String sendPoleId = "";
	protected SocketChannel socketChannel = null;
	
	
    public IOTcpClient(SocketChannel c) {
    	super.setTypeUDP(false);
    	
    	setSocketChannel(c);
    	setSocket(c.socket());
    	setSocketAddr( socket.getLocalSocketAddress() );
    }

	@Override
	public synchronized void close()
	{
		try {
			socketChannel.close();
			socket.close();
			super.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
		}
	}
	
	public Socket getSocket() {
		return socket;
	}

	public void setSocket(Socket socket) {
		this.socket = socket;
	}

	public SocketChannel getSocketChannel() {
		return socketChannel;
	}

	public void setSocketChannel(SocketChannel socketChannel) {
		this.socketChannel = socketChannel;
	}

	public String getSendPoleId() {
		return sendPoleId;
	}

	public void setSendPoleId(String sendPoleId) {
		this.sendPoleId = sendPoleId;
	}
	
	public boolean comparePoleId(String id)
	{
		if(this.sendPoleId.compareTo(id) == 0)return true;
		return false;
	}
}
