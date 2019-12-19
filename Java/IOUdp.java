package com.chzero.socket.base;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.DatagramChannel;

public class IOUdp  extends IO{
	protected DatagramChannel recvDatagramChannel = null;
	
    public IOUdp() 
    {
    	super.setTypeUDP(true);
    }
    
    public void setInfo(DatagramChannel recvChan,  InetSocketAddress sendAddr)
    {
    	recvDatagramChannel = recvChan;
    	setSocketAddr(sendAddr);
    }
    
    @Override
	public void close()
	{
		try {
			recvDatagramChannel.close();
			super.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
		}
	}

	public DatagramChannel getRecvDatagramChannel() {
		return recvDatagramChannel;
	}

	public void setRecvDatagramChannel(DatagramChannel recvDatagramChannel) {
		this.recvDatagramChannel = recvDatagramChannel;
	}
}

