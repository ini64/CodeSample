package com.chzero.socket.base;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import com.chzero.cdf.util.EnvLoader;

public class FileData {
	protected byte [] image = null;
	protected String poldId = "";
	protected SHORT cmd = new SHORT(0);
	
	protected String imageDir = "C:/";
		
	
	public FileData()
	{
		EnvLoader env = EnvLoader.getInstance();
		imageDir = env.getString("imgae.write.dir").trim();
	}
	public boolean writeImage()
	{
		if(null == image || poldId.length() == 0)return false;
		
		String fileFullPath = imageDir + "/" + poldId;
		String fileFullPathOld = fileFullPath + "_old.jpg";
		fileFullPath += ".jpg";
				
		try {
			//올드 파일이 있을경우 변경
			File fileOld = new File(fileFullPathOld);
			if(fileOld.exists())
			{
				 for (int i = 0; i < 20; i++) {
					 if(fileOld.delete())break;
					Thread.sleep(50); //처리할 시간적 여유를 주기 위함인듯.
				}
			}
			
			//기존 파일 변경
			File file = new File(fileFullPath);
			if(file.exists())
			{
				 for (int i = 0; i < 20; i++) {
					 if(file.renameTo(fileOld))break;
						Thread.sleep(50);
				}
			}
			
			//파일 저장
			FileOutputStream outFiles = new FileOutputStream(fileFullPath);
			outFiles.write(image);
			outFiles.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Debug.message("ImageData-writeImage-PoleId: " + poldId + " " + e.getMessage());
			return false;
		}
		catch (InterruptedException e) {
			Debug.message("ImageData-writeImage-PoleId: " + poldId + " " + e.getMessage());
			return false;
		}
		return true;
	}

	public byte[] getImage() {
		return image;
	}

	public void setImage(byte[] image) {
		this.image = image;
	}

	public String getPoldId() {
		return poldId;
	}

	public void setPoldId(String poldId) {
		this.poldId = poldId;
	}

	public SHORT getCmd() {
		return cmd;
	}

	public void setCmd(SHORT cmd) {
		this.cmd = cmd;
	}
}
