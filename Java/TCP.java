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
		Debug.message(className + " 기동");
		
		try {
			Debug.message(className + "-binding:" + bindingIp +":"+ Integer.toString(bindingPort));
			
			serverSocketChannel = ServerSocketChannel.open();
			
	        //논 블로킹 모드 설정
			serverSocketChannel.configureBlocking(false);
			
	        // 서버소켓 채널과 연결된 서버소켓을 가져온다.
			serverSocket = serverSocketChannel.socket();
			
	        // 주어진 파라미터에 해당서버의 주소다. 포트로 서버소켓을 바인드한다.
	        isa = new InetSocketAddress(bindingIp, bindingPort);
	        serverSocket.bind(isa);
	        	        	        
	        // 서버소켓 채널을 셀렉터에 등록한다.
	        SelectionKey serverKey = serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);
	        
	        //이벤트 발생시 사용할 데이터 등록
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
				
		Debug.message(className + "종료");
	}
	@Override
	public void endThread()
	{
		//메인소켓 닫기
		ioTcpServer.close();
		
		//클라소켓 정리
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
        	
        	//클라소켓 생성
        	SocketChannel cleintSocketChannel = ioTcpServer.getServerSocketChannel().accept();
        	if (cleintSocketChannel == null)return false;

        	cleintSocketChannel.configureBlocking(false);
            SelectionKey clientKey = cleintSocketChannel.register(selector, SelectionKey.OP_READ);
            
            IOTcpClient clientIO = new IOTcpClient(cleintSocketChannel);
	        clientKey.attach(clientIO);
            
            user.add(clientIO); // 접속자 추가
            Debug.message(className + "-accept:  "+ cleintSocketChannel.toString() + "클라이언트가 접속했습니다.");
            
        } catch (Exception e) {
        	Debug.message(className + "-accept:  "+ e.getMessage());
        	return false;
        }
        return true;
	}
}
