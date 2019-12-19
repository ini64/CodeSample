package com.chzero.socket.working;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.util.Iterator;


public class SocketThread  extends Thread  implements SocketThreadInterface{ 
	final static protected int BUF_SZ = 64 * 1024;
		
	protected String bindingIp = "127.0.0.1";
	protected int bindingPort = 9000;
	
	protected Selector selector = null;
	protected DatagramChannel datagramChannel = null;
	protected ServerSocketChannel serverSocketChannel = null;
	protected InetSocketAddress isa = null;
	protected ServerSocket serverSocket = null;
	
	//protected ByteBuffer buffer;
		
	public SocketThread(String BindingIp, int BindingPort, Selector Select)
	{
		this.setDaemon(true);
		
		selector = Select;
		bindingIp = BindingIp;
		bindingPort = BindingPort;
		
		//buffer = ByteBuffer.allocateDirect(BUF_SZ);
	}
	
	@Override
	public void run()
	{
		startThread();
	}
	
	@Override
	public void startThread() 
	{
		//tcp, udp에서 코딩
		//select();
	}
	
	@Override
	public void select()
	{
		 while (true) 
		 {
			 try{
				selector.select();
				Iterator selectedKeys = selector.selectedKeys().iterator();
				 	
				while (selectedKeys.hasNext()) 
				{
			        SelectionKey key = (SelectionKey) selectedKeys.next();
			        selectedKeys.remove();
			
			        if (!key.isValid()) continue;
			        
			        if (key.isAcceptable()) {
                        accept(key);
                    } else if (key.isReadable()) {
                    	read(key);
                    }
				}
			 }
			 catch (IOException e) { continue; } 
			 catch(ClosedSelectorException e){break;}
		 }
	}
		
	@Override
	public void endThread() 
	{
		try {
			if(null != datagramChannel)datagramChannel.close();
			if(null != serverSocketChannel)serverSocketChannel.close();
			if(null != serverSocket)serverSocket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	@Override
	public boolean read(SelectionKey key)
	{
		//tcp, udp에서 코딩을 해야함
		return  false;
	}
	
	@Override
	public boolean write(ByteBuffer bb)
	{
		//tcp, udp에서 코딩을 해야함
		return  false;
	}
	
	@Override
	public boolean accept(SelectionKey key)
	{
		//tcp, udp에서 코딩
		return false;
	}
}
