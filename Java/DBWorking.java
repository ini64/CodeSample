package com.chzero.socket.working;

import java.io.IOException;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.Selector;
import java.sql.SQLException;
import java.util.Iterator;
import java.util.LinkedList;

import com.chzero.cdf.dao.GenericDAOException;
import com.chzero.cdf.model.Parameters;
import com.chzero.socket.base.Debug;
import com.kdn.facility.EventLB;


public class DBWorking extends Thread{
	
	protected EventLB event = new EventLB();
	protected LinkedList parameters = new LinkedList();
	protected Selector selector = null;
	
	public DBWorking(Selector Select)
	{
		Debug.message("DBWorking 기동");
		this.setDaemon(true);
		selector = Select;
	}
	
	@Override
	public void run()
	{
		Select();
	}
	
	public synchronized void Insert(LinkedList result)
	{
		Iterator it = result.iterator();
		
		while(it.hasNext())
		{
			Parameters data = (Parameters) it.next();
			parameters.add(data);
		}
		
		WakeUp();
	}
	
	private synchronized Parameters Get()
	{
		Iterator it = parameters.iterator();
		if(it.hasNext())
		{
			Parameters parameters = (Parameters) it.next();
			it.remove();
			
			return parameters;
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
				
				Parameters parameters = Get();
				
				while(null != parameters)
				{
					
					try {
						event.insert(parameters);
						
					} catch (GenericDAOException e) {
						// TODO Auto-generated catch block
						Debug.message("DBWorking-insert: " + e.getMessage());
					}
					catch (SQLException e) {
						// TODO Auto-generated catch block
						Debug.message("DBWorking-insert: " + e.getMessage());
					}
					catch (NullPointerException e){
						// TODO Auto-generated catch block
						Debug.message("DBWorking-insert: " + e.getMessage());
					}
					 parameters = Get();
				}
			 }
			 catch (IOException e) { continue; } 
			 catch(ClosedSelectorException e){break;}
		 }
		 Debug.message("DBWorking 종료");
	}
}
