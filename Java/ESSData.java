package com.chzero.socket.base;

import java.nio.ByteBuffer;



public class ESSData {

	protected STRING sid = new STRING(1);
	protected SHORT battV = new SHORT();
	
	public ESSData(String s, short b)
	{
		sid.Set(s);
		battV.Set(b);
	}
	
	public ESSData()
	{
	}

	
	public void Set(ByteBuffer bb, INT bi)
	{
		sid.Set(bb, bi);
		battV.Set(bb, bi);
		
		bb.position(bi.Get());
	}
	public void Put(ByteBuffer bb, INT bi)
	{
		sid.Put(bb, bi);
		battV.Put(bb, bi);
		
		bb.position(bi.Get());
	}
	
	public String GetSid()
	{
		return sid.Get();
	}
	
	public short GetBattV()
	{
		return battV.Get();
	}
	
	public void Set(String s, short b)
	{
		sid.Set(s);
		battV.Set(b);
	} 
}
