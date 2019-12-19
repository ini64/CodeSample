//----------------------------------------------------------------------------------------------------------------------
// ���ϸ� : LuaRender.h
// ��  �� : ��� ��ũ��Ʈ(.lua)�� �̿�
//----------------------------------------------------------------------------------------------------------------------
#pragma once


#pragma warning(disable : 4996)


#include "stdafx.h"
#include "StlTypes.h"
#include "GdiObject.h"
#include "lua_tinker.h"// ��ƿ��� Ŭ���� ����� ���� 
#include "GisymbolSize.h"
#include "GiGeometryFont.h"
#include "GiGeometryBrush.h"
#include "GiGeometryPen.h"
#include "GiGeometryPointsData.h"



#define  SAFE_LUA_CLOSE(a) 		if(a) { lua_close((a));    (a) = 0; }// lua_state�� ����


class GiSubSetting;
class CGisMath;
class GiSetting;
class ClassDataPack;
class GiGeometry;

class DLL_EXPORT LuaRender : public GiSymbolSize
{
public:
   LuaRender();
   virtual ~LuaRender();


public:
   void  Init();// ��� ȯ���� ����ϰ�, ��ƿ� LuaRender Ŭ������ �ν��Ͻ��� ��� �Լ��� ����Ѵ�.
                // ��������� ȣ������ �ʾƵ� Draw �Լ����� ȣ����.
   void  Destroy();// �Ҹ��ڿ��� ȣ��(���� �Լ�)

   lua_State* GetLuaState();

public:
   void LuaRender::SetScript(LPCTSTR script);

   void  Draw(CDC* pDC, GiGeometry * geom, GiGeometryPoints & points, 
	   lua_tinker::table & luaData, ClassDataPack * pack);// GiScript::vDraw���� ȣ��
   	
   void SetRotInfo(double centerPtX, double centerPtY);

   void  MsgBox(LPCTSTR msg);
   LPCTSTR GetSettingValue(LPCTSTR key);
   void SetTextInfo(int iEscapement);
   void StringSplit(LPCTSTR str, lua_tinker::table luaReturnData);
   void Rot(lua_tinker::table points, lua_tinker::table luaReturnData);
   void RotSec(lua_tinker::table points, lua_tinker::table luaReturnData, int sec);



public:// DrawText
	void  TextOutWithSize(double x, double y, LPCTSTR text, double width, double height);
    void  TextOut(double x, double y, LPCTSTR text);// �ؽ�Ʈ ���

   void TextExtent(LPCTSTR cs, lua_tinker::table tableValue);


public:// Type-Safe Selection Helpers
   void  CreatePen(int r, int g, int b);// �� ������ R / G / B ������ �޾� �� ����
   void  CreateBrush(int r, int g, int b);// �ָ��� �귯�� ����

   void  ReturnPen();// �ٷ� �� ������ �ǵ���
   void  ReturnBrush();// �ٷ� �� �귯�÷� �ǵ���
   void  ReturnPenBrush();// �ٷ� �� �� / �귯�÷� �ǵ���


public:// Line-Output Functions
	void  LineTo(double x, double y);
    void  MoveTo(double x, double y);

public:// Ellipse and Polygon Functions
   void  Ellipse(double l, double t, double r, double b);// Draws an ellipse

   //ȸ�� ���Ѽ� �������� �׸���
   void  Polygon(lua_tinker::table tableValue);
   void  Rectangle(lua_tinker::table tableValue);

public:// ��ƿ��� ������ ���� �ʿ��� �� ȣ��

   void  SetTextColor(int r, int g, int b);

public:
	//�Ÿ��� ȭ�� ������� ȯ���ؼ� �˷� �ش�
	double  GetLengthToScreenSize(double sizeMiter);

private:
      
   //GiSetting*     _GetSettingPtr(int iLayerID);// m_mpSettings���� ���̾� ���̵� �ش��ϴ� �����͸� ��ȯ
   //GiSubSetting*  _GetSubSettingPtr(int iLayerID);// m_mpSubSettings���� ���̾� ���̵� �ش��ϴ� �����͸� ��ȯ


private:
	void rotation(D2 & inoutPt);
	void rotation(vector<D2> & inoutVec);

   bool           m_bInit;
   lua_State*     m_l;// ��� ȯ�� ����ü


private:
   CDC * m_pDC;

   CPen	m_pen;
   CPen * m_pOldPen;

   CBrush m_brush;
   CBrush * m_pOldBrush;

   //CFont*         m_pOldFont;

  

   double m_RotCenterX;
   double m_RotCenterY;
   double m_RotSec;
   int m_textReverse;


private:


private:
   //map<int, GiSetting*>       m_mpSettings;// GiSetting ������ �����(���̵�, ������)
   //map<int, GiSubSetting*>    m_mpSubSettings;// GiSubSetting ������ �����(���̵�, ������)

	GiSetting * m_pSettings;
	GiSubSetting * m_pSubSettings;
    CGisMath * m_pMath;
	int m_symbolAngle;
	ClassDataPack * m_pack;

	GiGeometryFont m_geomFont;
	GiGeometryPen m_geomPen;
	GiGeometryBrush m_geomBrush;
};
