package com.chzero.socket.base;

import java.nio.ByteBuffer;



public class SHORT {
protected short value;
	
	public SHORT(int v)
	{
		value = (short)v;
	}
	public SHORT(short v)
	{
		value = v;
	}
	public SHORT()
	{
		value = 0;
	}
	
	public void Set(ByteBuffer bb, INT bi)
	{
		value = LittleEndianBits.getShort(bb, bi.Get());
		bi.Set( bi.Get() + 1);
		
		bb.position(bi.Get());
	}
	public void Put(ByteBuffer bb, INT bi)
	{
		LittleEndianBits.putShort(bb, bi.Get(), value);
		bi.Set( bi.Get() + 1);
		
		bb.position(bi.Get());
	}
	
	public short Get()
	{
		return value;
	}
	
	public String GetString()
	{
		return Short.toString(value);
	}
	
	public void Set(short v)
	{
		value = v;
	} 
	
	public void Set(int v)
	{
		value = (short)v;
	}
}
