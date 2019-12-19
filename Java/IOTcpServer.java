package com.chzero.socket.base;

import java.io.IOException;
import java.net.ServerSocket;
import java.nio.channels.ServerSocketChannel;
;

public class IOTcpServer extends IO{

	public ServerSocket socket;
	protected ServerSocketChannel serverSocketChannel = null;
	
    public IOTcpServer() {
    	super.setTypeUDP(false);
    }

	@Override
	public void close()
	{
		try {
			serverSocketChannel.close();
			socket.close();
			super.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
		}
	}
	
	public void accept(ServerSocketChannel c)
	{
		serverSocketChannel = c;
	}

	public ServerSocketChannel getServerSocketChannel() {
		return serverSocketChannel;
	}

	public void setServerSocketChannel(ServerSocketChannel serverSocketChannel) {
		this.serverSocketChannel = serverSocketChannel;
		setSocket(serverSocketChannel.socket());
    	setSocketAddr( socket.getLocalSocketAddress() );
	}

	public ServerSocket getSocket() {
		return socket;
	}

	public void setSocket(ServerSocket socket) {
		this.socket = socket;
	}
}
