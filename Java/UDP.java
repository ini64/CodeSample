package com.chzero.socket.working;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.nio.channels.NotYetConnectedException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import com.chzero.socket.base.Debug;
import com.chzero.socket.base.IOUdp;


public class UDP  extends SocketThread implements SocketThreadInterface{
	protected String sendIP = "127.0.0.1";
	protected int sendPort = 9000;
	protected InetSocketAddress sendInetaddr = null;
	protected DatagramChannel sendChannel = null;
	protected String className = "";
	protected IOUdp ioUdp = new IOUdp();
	
	public UDP(String BindingIp, 
			int BindingPort, 
			String SendIP, 
			int SendPort, Selector Select, String ClassName) 
	{
		super(BindingIp, BindingPort, Select);
		
		sendIP = SendIP;
		sendPort = SendPort;
		className = ClassName;
	}
	
	@Override
	public void startThread() 
    {
		Debug.message(className + " 기동");
		
		try {
			sendInetaddr = new InetSocketAddress(sendIP, sendPort);
			sendChannel =  DatagramChannel.open();
			
			Debug.message(className + "-send:" + sendIP +":"+ Integer.toString(sendPort));
		 
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-startServer: sendSocketInit -> "+ e.getMessage());
		}
		catch (SocketException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-startServer: sendSocketInit -> "+ e.getMessage());
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-startServer: sendSocketInit -> "+ e.getMessage());
		}
		
		
		try {
			
			Debug.message(className + "-binding:" + bindingIp +":"+ Integer.toString(bindingPort));
			
			datagramChannel = DatagramChannel.open();
			isa = new InetSocketAddress(bindingIp, bindingPort);
			
			datagramChannel.socket().bind(isa);
			datagramChannel.configureBlocking(false);
	        SelectionKey clientKey = datagramChannel.register(selector, SelectionKey.OP_READ);
	        
	        ioUdp.setInfo(datagramChannel, isa);
	        clientKey.attach(ioUdp);
		
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-startServer: binding -> "+ e.getMessage());
		}
		
		select();
		Debug.message(className + "종료");
    }
	
	@Override
	public void endThread()
	{
		try {
			//바인딩 소켓 제거
			ioUdp.close();
			
			super.endThread();
			if(null != sendChannel)sendChannel.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-endThread:  "+ e.getMessage());
		}
	}
	
	
	@Override
	public boolean read(SelectionKey key)
	{
		try {
			IOUdp io = (IOUdp)key.attachment();
			
			
			io.getBuffer().clear();
			ioUdp.getRecvDatagramChannel().receive(io.getBuffer());
	        
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-read:  "+ e.getMessage());
			return false;
		}
		catch(NotYetConnectedException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-read:  "+ e.getMessage());
			return false;
		}
		return true;
    }

	@Override
	public synchronized boolean write(ByteBuffer bb)
	{
		try {
			if(bb.position() != 0)bb.flip();
			sendChannel.send(bb, sendInetaddr);
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-write:  "+ e.getMessage());
			return false;
		}
		return true;
	}
	
	public synchronized boolean write(ByteBuffer bb, int start, int limit)
	{
		try {
			if(bb.position() != 0)bb.flip();
			
			bb.position(start);
			bb.limit(limit);
			
			sendChannel.send(bb, sendInetaddr);
		}
		catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message(className + "-write:  "+ e.getMessage());
			return false;
		}
		return true;
	}
	
	@Override
	public boolean accept(SelectionKey key)
	{
		return false;
	}
}


