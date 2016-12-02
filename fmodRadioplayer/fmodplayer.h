#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"	

class CfmodplayerApp : public CWinAppEx
{
protected:
	ULONG_PTR	m_gdiplusToken;

public:
	CfmodplayerApp();

	public:
	virtual BOOL InitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual int ExitInstance();
	DECLARE_MESSAGE_MAP()
};

extern CfmodplayerApp theApp;