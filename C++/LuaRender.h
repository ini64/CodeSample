//----------------------------------------------------------------------------------------------------------------------
// 파일명 : LuaRender.h
// 용  도 : 루아 스크립트(.lua)를 이용
//----------------------------------------------------------------------------------------------------------------------
#pragma once


#pragma warning(disable : 4996)


#include "stdafx.h"
#include "StlTypes.h"
#include "GdiObject.h"
#include "lua_tinker.h"// 루아에서 클래스 사용을 위해 
#include "GisymbolSize.h"
#include "GiGeometryFont.h"
#include "GiGeometryBrush.h"
#include "GiGeometryPen.h"
#include "GiGeometryPointsData.h"



#define  SAFE_LUA_CLOSE(a) 		if(a) { lua_close((a));    (a) = 0; }// lua_state를 해제


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
   void  Init();// 루아 환경을 등록하고, 루아에 LuaRender 클래스의 인스턴스와 멤버 함수를 등록한다.
                // 명시적으로 호출하지 않아도 Draw 함수에서 호출함.
   void  Destroy();// 소멸자에서 호출(정리 함수)

   lua_State* GetLuaState();

public:
   void LuaRender::SetScript(LPCTSTR script);

   void  Draw(CDC* pDC, GiGeometry * geom, GiGeometryPoints & points, 
	   lua_tinker::table & luaData, ClassDataPack * pack);// GiScript::vDraw에서 호출
   	
   void SetRotInfo(double centerPtX, double centerPtY);

   void  MsgBox(LPCTSTR msg);
   LPCTSTR GetSettingValue(LPCTSTR key);
   void SetTextInfo(int iEscapement);
   void StringSplit(LPCTSTR str, lua_tinker::table luaReturnData);
   void Rot(lua_tinker::table points, lua_tinker::table luaReturnData);
   void RotSec(lua_tinker::table points, lua_tinker::table luaReturnData, int sec);



public:// DrawText
	void  TextOutWithSize(double x, double y, LPCTSTR text, double width, double height);
    void  TextOut(double x, double y, LPCTSTR text);// 텍스트 출력

   void TextExtent(LPCTSTR cs, lua_tinker::table tableValue);


public:// Type-Safe Selection Helpers
   void  CreatePen(int r, int g, int b);// 펜 색상을 R / G / B 값으로 받아 펜 생성
   void  CreateBrush(int r, int g, int b);// 솔리드 브러시 생성

   void  ReturnPen();// 바로 전 펜으로 되돌림
   void  ReturnBrush();// 바로 전 브러시로 되돌림
   void  ReturnPenBrush();// 바로 전 펜 / 브러시로 되돌림


public:// Line-Output Functions
	void  LineTo(double x, double y);
    void  MoveTo(double x, double y);

public:// Ellipse and Polygon Functions
   void  Ellipse(double l, double t, double r, double b);// Draws an ellipse

   //회전 시켜서 폴리곤을 그린다
   void  Polygon(lua_tinker::table tableValue);
   void  Rectangle(lua_tinker::table tableValue);

public:// 루아에서 셋팅의 값이 필요할 때 호출

   void  SetTextColor(int r, int g, int b);

public:
	//거리를 화면 사이즈로 환산해서 알려 준다
	double  GetLengthToScreenSize(double sizeMiter);

private:
      
   //GiSetting*     _GetSettingPtr(int iLayerID);// m_mpSettings에서 레이어 아이디에 해당하는 포인터를 반환
   //GiSubSetting*  _GetSubSettingPtr(int iLayerID);// m_mpSubSettings에서 레이어 아이디에 해당하는 포인터를 반환


private:
	void rotation(D2 & inoutPt);
	void rotation(vector<D2> & inoutVec);

   bool           m_bInit;
   lua_State*     m_l;// 루아 환경 구조체


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
   //map<int, GiSetting*>       m_mpSettings;// GiSetting 포인터 저장소(아이디, 포인터)
   //map<int, GiSubSetting*>    m_mpSubSettings;// GiSubSetting 포인터 저장소(아이디, 포인터)

	GiSetting * m_pSettings;
	GiSubSetting * m_pSubSettings;
    CGisMath * m_pMath;
	int m_symbolAngle;
	ClassDataPack * m_pack;

	GiGeometryFont m_geomFont;
	GiGeometryPen m_geomPen;
	GiGeometryBrush m_geomBrush;
};
