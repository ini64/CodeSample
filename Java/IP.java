package com.chzero.socket.base;

import java.nio.ByteBuffer;


public class IP {
protected String value;

private SHORT ip1 = new SHORT();
private SHORT ip2 = new SHORT();

private SHORT ip3 = new SHORT();
private SHORT ip4 = new SHORT();


	public IP()
	{
		value = "";
	}
	
	public IP(String v)
	{
		value = v;
	}
	
	public void Set(ByteBuffer bb, INT bi)
	{
		
		ip1.Set(bb, bi);
		ip2.Set(bb, bi);
		ip3.Set(bb, bi);
		ip4.Set(bb, bi);
		
		value = Short.toString(ip1.Get()) + "." + Short.toString(ip2.Get())  + "." + Short.toString(ip3.Get()) + "." + Short.toString(ip4.Get());
		
		bb.position(bi.Get());
	}
	public void Put(ByteBuffer bb, INT bi)
	{
		String [] array = value.split("\\.");
		if(array.length > 3)
		{
			ip1.Set( Short.parseShort(array[0]) );
			ip2.Set( Short.parseShort(array[1]) );
			ip3.Set( Short.parseShort(array[2]) );
			ip4.Set( Short.parseShort(array[3]) );
		}
		
		ip1.Put(bb, bi);
		ip2.Put(bb, bi);
		ip3.Put(bb, bi);
		ip4.Put(bb, bi);
		
		bb.position(bi.Get());
	}
	
	public String Get()
	{
		return value;
	}
	
	public void Set(String v)
	{
		value = v;
	} 
}
