package com.chzero.socket.base;

import java.nio.ByteBuffer;



public class INT {
protected int value;
	
	public INT(int v)
	{
		value = v;
	}
	public INT()
	{
		value = 0;
	}
	
	public void Set(ByteBuffer bb, INT bi)
	{
		value = LittleEndianBits.getInt(bb, bi.Get());
		
		bi.Set( bi.Get() + 2);
		
		bb.position(bi.Get());
	}
	public void Put(ByteBuffer bb, INT bi)
	{
		LittleEndianBits.putInt(bb, bi.Get(), value);
		
		bi.Set( bi.Get() + 2);
		
		bb.position(bi.Get());
	}
	
	public int Get()
	{
		return value;
	}
	
	
	
	public void Set(int v)
	{
		value = v;
	} 
}
