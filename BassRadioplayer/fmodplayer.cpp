#include "stdafx.h"
#include "fmodplayer.h"
#include "MusicPlay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CfmodplayerApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CfmodplayerApp::CfmodplayerApp()
{
}

CfmodplayerApp theApp;

BOOL CfmodplayerApp::InitInstance()
{
	HANDLE hMutex = ::CreateMutex(NULL, FALSE, "bassRadioplayerInstance");
	if (ERROR_ALREADY_EXISTS == ::GetLastError())
		return FALSE;
	
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	MusicPlay dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	return FALSE;
}

BOOL CfmodplayerApp::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case	WM_SYSKEYDOWN:
		if (pMsg->wParam == VK_F4)
		{
			return TRUE;
		}
	default:
		break;
	}		
	return CWinApp::PreTranslateMessage(pMsg);
}

int CfmodplayerApp::ExitInstance() 
{	
	GdiplusShutdown(m_gdiplusToken);
	
	return CWinApp::ExitInstance();
}