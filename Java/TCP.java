package com.chzero.socket.working;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.Vector;

import com.chzero.socket.base.Debug;
import com.chzero.socket.base.IOTcpClient;
import com.chzero.socket.base.IOTcpServer;


public class TCP extends SocketThread {
	
	protected String className = "";
	protected IOTcpServer ioTcpServer = new IOTcpServer();
	protected Vector user = new Vector();
	
	public TCP(String BindingIp, 
			int BindingPort, Selector Select, String ClassName) 
	{
		super(BindingIp, BindingPort, Select);
		className = ClassName;
	}
	
	@Override
	public void startThread()
	{
		Debug.message(className + " �⵿");
		
		try {
			Debug.message(className + "-binding:" + bindingIp +":"+ Integer.toString(bindingPort));
			
			serverSocketChannel = ServerSocketChannel.open();
			
	        //�� ���ŷ ��� ����
			serverSocketChannel.configureBlocking(false);
			
	        // �������� ä�ΰ� ����� ���������� �����´�.
			serverSocket = serverSocketChannel.socket();
			
	        // �־��� �Ķ���Ϳ� �ش缭���� �ּҴ�. ��Ʈ�� ���������� ���ε��Ѵ�.
	        isa = new InetSocketAddress(bindingIp, bindingPort);
	        serverSocket.bind(isa);
	        	        	        
	        // �������� ä���� �����Ϳ� ����Ѵ�.
	        SelectionKey serverKey = serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);
	        
	        //�̺�Ʈ �߻��� ����� ������ ���
	        ioTcpServer.setServerSocketChannel(serverSocketChannel);
	        serverKey.attach(ioTcpServer);
		}
		catch (UnknownHostException e) {
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
		
		super.select();
				
		Debug.message(className + "����");
	}
	@Override
	public void endThread()
	{
		//���μ��� �ݱ�
		ioTcpServer.close();
		
		//Ŭ����� ����
		for(int i = 0 ; i < user.size() ; i++)
		{
			IOTcpClient io = ((IOTcpClient)user.get(i));
			io.close();
		}
		
		super.endThread();
	}
	
	@Override
	public boolean read(SelectionKey key)
	{
		IOTcpClient io = (IOTcpClient)key.attachment();
		
		try {
			io.getBuffer().clear();
			io.getSocketChannel().read(io.getBuffer());
        } 
        catch (IOException ex) 
        {
           	io.close();
           	user.remove(io);
           	
           	return false;
        }
		
		if(io.getBuffer().position() == 0)
		{
			io.close();
           	user.remove(io);
           	return false;
		}
		return true;
	}
	
	@Override
	public boolean write(ByteBuffer bb){

		try {
	        Iterator iter = user.iterator();
	        while (iter.hasNext()) 
	        {
	        	if(bb.position() != 0)bb.flip();
	        	IOTcpClient io = (IOTcpClient)iter.next();
	        	io.getSocketChannel().write(bb);
	        }
        } 
		catch (IOException ex) 
        {
        	Debug.message(className + "-broadcast:  "+ ex.getMessage());
        	return false;
        }
		return true;
	}

	
	@Override
	public boolean accept(SelectionKey key)
	{
        try {
        	//IOTcpServer serverIO = (IOTcpServer)key.attachment();
        	
        	//Ŭ����� ����
        	SocketChannel cleintSocketChannel = ioTcpServer.getServerSocketChannel().accept();
        	if (cleintSocketChannel == null)return false;

        	cleintSocketChannel.configureBlocking(false);
            SelectionKey clientKey = cleintSocketChannel.register(selector, SelectionKey.OP_READ);
            
            IOTcpClient clientIO = new IOTcpClient(cleintSocketChannel);
	        clientKey.attach(clientIO);
            
            user.add(clientIO); // ������ �߰�
            Debug.message(className + "-accept:  "+ cleintSocketChannel.toString() + "Ŭ���̾�Ʈ�� �����߽��ϴ�.");
            
        } catch (Exception e) {
        	Debug.message(className + "-accept:  "+ e.getMessage());
        	return false;
        }
        return true;
	}
}
