//----------------------------------------------------------------------------------------------------------------------
// 파  일   명 : LuaRender.cpp
//----------------------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "LuaRender.h"
#include "GiSetting.h"
#include "GisMath.h"
#include "GISubSetting.h"
#include "ClassDataPack.h"
#include "GiGeometry.h"

//int g_iScriptError = 0;


//
LuaRender::LuaRender() :
m_bInit(false), m_l(0),
m_pDC(0), m_pOldPen(0), m_pOldBrush(0),
m_pMath(0)
{
}

//
LuaRender::~LuaRender()
{
   Destroy();
}

// 루아 환경을 등록하고, 루아에 LuaRender 클래스의 인스턴스와 멤버 함수를 등록한다.
// 명시적으로 호출하지 않아도 Draw 함수에서 호출함.
void  LuaRender::Init()
{
   if(m_bInit)
      return;
   
   // 루아 환경을 만든다.
   m_l = lua_open();
   ASSERT(m_l);
   

   // 라이브러리 등록
   luaopen_base(m_l);
   luaL_openlibs(m_l);



   // LuaRender 클래스를 Lua 에 추가한다.
   lua_tinker::class_add<LuaRender>(m_l, "LuaRender");


   
   // LuaRender 의 멤버 함수들을 등록한다.------------------------------------------------------------------------------

   
   //-------------------------------------------------------------------------------------------------------------------
   lua_tinker::class_def<LuaRender>(m_l, "TextOut", &LuaRender::TextOut);
   lua_tinker::class_def<LuaRender>(m_l, "TextOutWithSize", &LuaRender::TextOutWithSize);
   lua_tinker::class_def<LuaRender>(m_l, "TextExtent", &LuaRender::TextExtent);

   
   //-------------------------------------------------------------------------------------------------------------------
   lua_tinker::class_def<LuaRender>(m_l, "CreatePen", &LuaRender::CreatePen);
   lua_tinker::class_def<LuaRender>(m_l, "CreateBrush", &LuaRender::CreateBrush);
   lua_tinker::class_def<LuaRender>(m_l, "ReturnPen", &LuaRender::ReturnPen);
   lua_tinker::class_def<LuaRender>(m_l, "ReturnBrush", &LuaRender::ReturnBrush);
   lua_tinker::class_def<LuaRender>(m_l, "ReturnPenBrush", &LuaRender::ReturnPenBrush);
  
   
   //-------------------------------------------------------------------------------------------------------------------
   lua_tinker::class_def<LuaRender>(m_l, "LineTo", &LuaRender::LineTo);
   lua_tinker::class_def<LuaRender>(m_l, "MoveTo", &LuaRender::MoveTo);

   
   //-------------------------------------------------------------------------------------------------------------------
   lua_tinker::class_def<LuaRender>(m_l, "Ellipse", &LuaRender::Ellipse);
   lua_tinker::class_def<LuaRender>(m_l, "Polygon", &LuaRender::Polygon);
   lua_tinker::class_def<LuaRender>(m_l, "Rectangle", &LuaRender::Rectangle);


   //-------------------------------------------------------------------------------------------------------------------
   lua_tinker::class_def<LuaRender>(m_l, "MsgBox", &LuaRender::MsgBox);
   lua_tinker::class_def<LuaRender>(m_l, "SetTextColor", &LuaRender::SetTextColor);
   lua_tinker::class_def<LuaRender>(m_l, "StringSplit", &LuaRender::StringSplit);
   lua_tinker::class_def<LuaRender>(m_l, "Rot", &LuaRender::Rot);
   lua_tinker::class_def<LuaRender>(m_l, "RotSec", &LuaRender::RotSec);
   lua_tinker::class_def<LuaRender>(m_l, "SetRotInfo", &LuaRender::SetRotInfo);

   lua_tinker::class_def<LuaRender>(m_l, "SetRotInfo", &LuaRender::SetRotInfo);
   lua_tinker::class_def<LuaRender>(m_l, "GetLengthToScreenSize", &LuaRender::GetLengthToScreenSize);
    

   // Lua 전역 변수에 LuaRender의 인스턴스를 등록한다.
   lua_tinker::set(m_l, "SDK", this);
   m_bInit = true;
}


void  LuaRender::SetTextColor(int r, int g, int b)
{
	m_geomFont.SetTextColor(r, g, b);
}

// 소멸자에서 호출(정리 함수)
void  LuaRender::Destroy()
{
   SAFE_LUA_CLOSE(m_l);

   ReturnPenBrush();
   
   if(NULL != m_pDC){m_pDC = NULL;}
   //if(NULL != m_pOldPen){m_pOldPen = NULL;}
   //if(NULL != m_pOldBrush){m_pOldBrush = NULL;}
   //if(NULL != m_pOldFont){m_pOldFont = NULL;}

   //SAFE_NULL(m_pDC);
   //SAFE_NULL(m_pOldPen);
   //SAFE_NULL(m_pOldBrush);
   //SAFE_NULL(m_pOldFont);

   //m_mpSettings.clear();
   //m_mpSubSettings.clear();
}


void LuaRender::SetScript(LPCTSTR script)
{
	lua_tinker::dostring(m_l, script);
}

lua_State* LuaRender::GetLuaState()
{
	if(!m_bInit)// 초기화를 명시적으로 하지 않은 경우를 위해 호출한다.
		Init();

	return m_l;
}

// GiScript::vDraw에서 호출
void  LuaRender::Draw(CDC* pDC, GiGeometry * geom, GiGeometryPoints & points, 
					  lua_tinker::table & luaData, ClassDataPack * pack)
{
   if(!m_bInit)// 초기화를 명시적으로 하지 않은 경우를 위해 호출한다.
      Init();
   
   if(!m_l || !pDC)
      return;

   //파이 값으로 변환
   m_symbolAngle = luaData.get<int>(_T("symbol_angle"));
   m_RotSec = (PI / 180.0) * luaData.get<double>("symbol_angle");

	//y축을 기준으로 좌측에 있을때 보정 즉 x축 180도 회전을 할지 말지 유무
	//m_textReverse = luaData.get<int>("text_reverse");

	//거의 모든 심벌의 회전 센터의 geom의 좌표여서 우선 설정
	m_RotCenterX = luaData.get<double>("center_x");
	m_RotCenterY = luaData.get<double>("center_y");
   

   m_geomFont.SetBasicInfo(pack, pDC);

   //auto일 경우 폰트를 180도 회전시키기 때문에 도형의 각과 폰트의 각은 꼭 같지 않다

   CString label_alignment(luaData.get<LPCTSTR>("label_alignment"));
   m_geomFont.CreateFont(luaData.get<int>("symbol_angle"), label_alignment);


   // DC 복사
   m_pDC = pDC;
   m_pack = pack;
   m_pSettings = pack->m_pSetting;
   m_pSubSettings = pack->m_pSubSetting;
   m_pMath = pack->m_pMath;
   

   // 루아 메인 함수 호출(고정된 값) - 모든 스크립트에서 Main 함수 구현해야 된다.
   lua_tinker::table ret = lua_tinker::call<lua_tinker::table>(m_l, "Main", luaData);

   CRect * screenRt = m_pMath->GetScreenRect();

	points.m_drawRect.clear();
   //points.m_rtDrawRect.SetRect(screenRt->right, screenRt->bottom, screenRt->left, screenRt->top);
   
   int returnSize = ret.getn();
   
//    points.m_drawRect.push_back(CPoint((int)m_RotCenterX-5,(int)m_RotCenterY-5));
//    points.m_drawRect.push_back(CPoint((int)m_RotCenterX+5,(int)m_RotCenterY-5));
//    points.m_drawRect.push_back(CPoint((int)m_RotCenterX+5,(int)m_RotCenterY+5));
//    points.m_drawRect.push_back(CPoint((int)m_RotCenterX-5,(int)m_RotCenterY+5));
   for(int i = 0 ; i < returnSize ; i++)
   {
	   CPoint pt((int)(ret.get<lua_tinker::table>(i+1).get<double>("x") + 0.5), 
		      (int)(ret.get<lua_tinker::table>(i+1).get<double>("y") + 0.5) );

		points.m_drawRect.push_back(pt);		
   }
   

   m_geomFont.DeleteFont();
   ReturnPenBrush();
}


void LuaRender::StringSplit(LPCTSTR str, lua_tinker::table luaReturnData)
{
	CString data(str);

	TCHAR seps = _T('\n');

	int iSub = 0;
	CString strTok;


	while(AfxExtractSubString(strTok, str, iSub++, seps))
	{
		luaReturnData.set(iSub, (LPCTSTR)strTok);
	}

}


void LuaRender::SetRotInfo(double centerPtX, double centerPtY)
{
	m_RotCenterX = centerPtX;
	m_RotCenterY = centerPtY;
}

void LuaRender::Rot(lua_tinker::table points, lua_tinker::table luaReturnData)
{
	double fss = sin(m_RotSec);
	double fcs = cos(m_RotSec);

	int pointSize = points.getn();

	D2 dpt;
	
	for(int i = 0 ; i < pointSize ; i++)
	{

		dpt.x = points.get<lua_tinker::table>(i+1).get<double>("x");
		dpt.y = points.get<lua_tinker::table>(i+1).get<double>("y");

		m_pMath->Rotation(dpt, m_RotCenterX, m_RotCenterY, m_RotSec);

		luaReturnData.set(i+1, lua_tinker::table(m_l));
		lua_tinker::table point = luaReturnData.get<lua_tinker::table>(i+1);

		point.set("x", dpt.x);
		point.set("y", dpt.y);
	}
}

void LuaRender::RotSec(lua_tinker::table points, lua_tinker::table luaReturnData, int sec)
{
	double dSec = (PI / 180) * sec;

	double fss = sin(dSec);
	double fcs = cos(dSec);

	int pointSize = points.getn();

	D2 dpt;

	for(int i = 0 ; i < pointSize ; i++)
	{

		dpt.x = points.get<lua_tinker::table>(i+1).get<double>("x");
		dpt.y = points.get<lua_tinker::table>(i+1).get<double>("y");

		m_pMath->Rotation(dpt, m_RotCenterX, m_RotCenterY, m_RotSec);

		luaReturnData.set(i+1, lua_tinker::table(m_l));
		lua_tinker::table point = luaReturnData.get<lua_tinker::table>(i+1);

		point.set("x", dpt.x);
		point.set("y", dpt.y);
	}
}



void  LuaRender::MsgBox(LPCTSTR msg)
{
	AfxMessageBox(msg);
}


// 텍스트 출력
void  LuaRender::TextOut(double x, double y, LPCTSTR text)
{
   if(m_pDC)
   {
	   m_geomFont.TextOut(x, y, text, m_RotCenterX, m_RotCenterY);
   }
}


void  LuaRender::TextOutWithSize(double x, double y, LPCTSTR text, double width, double height)
{
	if(m_pDC)
	{
		m_geomFont.TextOut(x, y, text, m_RotCenterX, m_RotCenterY, width, height);
	}
}


void LuaRender::TextExtent(LPCTSTR cs, lua_tinker::table tableValue)
{
	if(m_pDC)
	{
		CSize si;
		m_geomFont.TextExtent(cs, si);

		tableValue.set("x", si.cx);
		tableValue.set("y", si.cy);
	}
}

// 펜 색상을 R / G / B 값으로 받아 펜 생성
void  LuaRender::CreatePen(int r, int g, int b)
{
	ReturnPen();

	if(m_pDC)
	{
		m_geomPen.CreatePen(m_pDC, &m_pen, &m_pOldPen, m_pack, RGB(r,g,b));
	}
}



// 솔리드 브러시 생성
void  LuaRender::CreateBrush(int r, int g, int b)
{
	ReturnBrush();

	if(m_pDC)
	{
		m_geomBrush.CreateBrush(m_pDC, &m_brush, &m_pOldBrush, m_pack, RGB(r,g,b));
	}
}

// 바로 전 펜으로 되돌림
void  LuaRender::ReturnPen()
{
   if(m_pDC)
   {
      m_geomPen.DeletePen(m_pDC, &m_pen, &m_pOldPen);
   }
}

// 바로 전 브러시로 되돌림
void  LuaRender::ReturnBrush()
{
	if(m_pDC)
	{
		m_geomBrush.DeleteBrush(m_pDC, &m_brush, &m_pOldBrush);
	}
}

// 바로 전 펜 / 브러시로 되돌림
void  LuaRender::ReturnPenBrush()
{
   ReturnPen();
   ReturnBrush();
}


// Draws a line from the current position up to, but not including, a point.
void  LuaRender::LineTo(double x, double y)
{
	if(m_pDC)
	{
		D2 dpt(x, y);

		m_pMath->Rotation(dpt, m_RotCenterX, m_RotCenterY, m_RotSec);

		m_pDC->LineTo((int)(dpt.x + 0.5), (int)(dpt.y + 0.5));
	}
}

// Moves the current position.
void  LuaRender::MoveTo(double x, double y)
{
	if(m_pDC)
	{
		D2 dpt(x, y);

		m_pMath->Rotation(dpt, m_RotCenterX, m_RotCenterY, m_RotSec);

		m_pDC->MoveTo((int)(dpt.x + 0.5), (int)(dpt.y + 0.5));
	}
}


// Draws an ellipse
void  LuaRender::Ellipse(double l, double t, double r, double b)
{
   CRect rect((int)(l + 0.5), (int)(t + 0.5), (int)(r + 0.5), (int)(b + 0.5));
   if(m_pDC)
      m_pDC->Ellipse(&rect);
}

void LuaRender::Rectangle(lua_tinker::table tableValue)
{
	if(m_pDC)
	{
		vector<D2> vec;
		int pointSize = tableValue.getn();

		if(0 == pointSize)return;

		for(int i = 0 ; i < pointSize ; i++)
		{
			vec.push_back(
				D2(tableValue.get<lua_tinker::table>(i+1).get<double>("x"), 
				tableValue.get<lua_tinker::table>(i+1).get<double>("y")) );
		}

		if(0 == m_symbolAngle || 90 == m_symbolAngle || 180 == m_symbolAngle || 270 == m_symbolAngle || 360 == m_symbolAngle)
		{
			POINT pt, pt2;

			pt.x = (int)(vec.at(0).x + 0.5);
			pt.y = (int)(vec.at(0).y + 0.5);

			pt2.x = (int)(vec.at(2).x + 0.5);
			pt2.y = (int)(vec.at(2).y + 0.5);

			CRect rt(pt, pt2);
			m_pDC->Rectangle(&rt);
		}
		else
		{
			m_pMath->Rotation(vec, m_RotCenterX, m_RotCenterY, m_RotSec);

			POINT * pPts  = new POINT[pointSize];

			for(int i = 0 ; i < pointSize ; i++)
			{
				pPts[i].x = (int)(vec.at(i).x + 0.5);
				pPts[i].y = (int)(vec.at(i).y + 0.5);
			}

			m_pDC->Polygon(pPts, pointSize);
			delete [] pPts; 
			pPts = NULL;
			//SAFE_DELETEA(pPts);
		}
	}
}

void  LuaRender::Polygon(lua_tinker::table tableValue)
{
	if(m_pDC)
	{
		vector<D2> vec;
		int pointSize = tableValue.getn();

		if(0 == pointSize)return;

		for(int i = 0 ; i < pointSize ; i++)
		{
			vec.push_back(
							D2(tableValue.get<lua_tinker::table>(i+1).get<double>("x"), 
							   tableValue.get<lua_tinker::table>(i+1).get<double>("y")) );
		}

		m_pMath->Rotation(vec, m_RotCenterX, m_RotCenterY, m_RotSec);
		
		POINT * pPts  = new POINT[pointSize];
		
		for(int i = 0 ; i < pointSize ; i++)
		{
			pPts[i].x = (int)(vec.at(i).x + 0.5);
			pPts[i].y = (int)(vec.at(i).y + 0.5);
		}
		m_pDC->Polygon(pPts, pointSize);
		delete [] pPts;
		pPts = NULL;
	}
}

double  LuaRender::GetLengthToScreenSize(double sizeMiter)
{
	double scale = m_pMath->GetScale();

	D2 ratio;
	m_pMath->GetScaleRatio(scale, ratio);
	double len = sizeMiter * ratio.x;

	if(len < 1.0)return 1.0;

	return len;
}
