#include "stdafx.h"
#include "fmodplayer.h"
#include "MusicPlay.h"

void CALLBACK MetaSync(HSYNC handle, DWORD channel, DWORD data, DWORD user)
{
	//DoMeta((char*)data);
}

void CALLBACK EndSync(HSYNC handle, DWORD channel, DWORD data, DWORD user)
{
	//MESS(31,WM_SETTEXT,0,"not playing");
	//MESS(30,WM_SETTEXT,0,"");
	//MESS(32,WM_SETTEXT,0,"");
}

void CALLBACK StatusProc(const void *buffer,DWORD length,DWORD user)
{
	if (buffer && !length)
	{
		//MESS(32,WM_SETTEXT,0,buffer); // display connection status
	}
}

MusicPlay::MusicPlay(CWnd* pParent /*=NULL*/)
	: CDialog(MusicPlay::IDD, pParent)
{
	
	m_pBackSkin	= NULL;	

	m_strTime = "";

	m_bMouseOn = FALSE;
}

void MusicPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_BUTTON_SUARAPLUS, m_btnPlus);
	DDX_Control(pDX, IDC_BUTTON_SUARAMIN, m_btnMin);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
}

BEGIN_MESSAGE_MAP(MusicPlay, CDialog)
	ON_WM_PAINT()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SUARAPLUS, &MusicPlay::OnBnClickedButtonSuaraplus)
	ON_BN_CLICKED(IDC_BUTTON_SUARAMIN, &MusicPlay::OnBnClickedButtonSuaramin)	
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &MusicPlay::OnBnClickedButtonClose)
END_MESSAGE_MAP()

void MusicPlay::InitBass()
{	
    //setup output device
	if ( !BASS_Init(-1,44100, 0, NULL,NULL) ) 
	{	
		DestroyWindow();
		return;
	}

	BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST,1); // enable playlist processing
	BASS_SetConfig(BASS_CONFIG_NET_PREBUF,0); // minimize automatic pre-buffering, so we can do it (and display it) instead
}

void MusicPlay::CloseBass()
{	
	BASS_Free();	
}

void MusicPlay::PlayMusic(LPCSTR chMusicName)
{		
	// close old stream
	BASS_StreamFree(hStream); 

	hStream = BASS_StreamCreateURL(chMusicName, 0, BASS_STREAM_STATUS|BASS_STREAM_AUTOFREE, StatusProc, 0);
	      		
	//Timer
	KillTimer(1);
	SetTimer(1, 1000, NULL );
}

void MusicPlay::OnDestroy() 
{		
	KillTimer(2);

	CloseBass();

	if(m_pBackSkin != NULL)
		delete m_pBackSkin;
	m_pBackSkin = NULL;	

	CDialog::OnDestroy();
}

HWND MusicPlay::GetCurPosHandle(POINT point)
{
	return ::WindowFromPoint(point);
}

void MusicPlay::CheckCurPos()
{
	POINT point;
	::GetCursorPos(&point);

	HWND hCur = GetCurPosHandle(point);
	if (hCur == GetSafeHwnd())
	{
		if(m_bMouseOn == FALSE)	m_bMouseOn = TRUE;
        m_btnClose.SetVisibleButton(FALSE); 
	}
	else
	{
		HWND hOwner;	
		do
		{
			hOwner = ::GetParent(hCur);

			if (hOwner == NULL)
			{
				m_bMouseOn = FALSE;
				m_btnClose.SetVisibleButton(TRUE); 
				return;
			}			

			if (hOwner == GetSafeHwnd())
			{
				if(m_bMouseOn == FALSE)
				{
					m_bMouseOn = TRUE;	
					m_btnClose.SetVisibleButton(FALSE);
				}
				return;
			}

			hCur = hOwner;
		}while (hCur);
	}
}

void MusicPlay::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect Rect;
	GetClientRect(&Rect);

	if (!m_bMouseOn && Rect.PtInRect(point))
	{
		m_bMouseOn = TRUE;	
		m_btnClose.SetVisibleButton(FALSE);
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void MusicPlay::OnTimer(UINT nIDEvent) 
{	
	if(nIDEvent == 1)
	{
		CString strState = "";

		//monitor prebuffering progress
		DWORD len = BASS_StreamGetFilePosition(hStream, BASS_FILEPOS_END);
		
		// percentage of buffer filled
		DWORD progress = (BASS_StreamGetFilePosition(hStream, BASS_FILEPOS_DOWNLOAD) -BASS_StreamGetFilePosition(hStream, BASS_FILEPOS_CURRENT)) * 100 / len; 
			
		// over 75% full (or end of download)
		if (progress>75 || !BASS_StreamGetFilePosition(hStream, BASS_FILEPOS_CONNECTED)) 
		{		
			// finished prebuffering, stop monitoring
			KillTimer(1); 
			
			// get the broadcast name and bitrate
			const char *icy = BASS_ChannelGetTags(hStream, BASS_TAG_ICY);
				
			// no ICY tags, try HTTP
			if (!icy) icy = BASS_ChannelGetTags(hStream, BASS_TAG_HTTP);
			
			if (icy) 
			{
				for (;*icy;icy+=strlen(icy)+1) 
				{
					if (!strnicmp(icy,"icy-name:",9))
					{
						strState.Format("%s", icy+9);//MESS(31,WM_SETTEXT,0,icy+9);					
					}

					if (!strnicmp(icy,"icy-br:",7))
					{
						char br[30]="bitrate: ";
						strcat(br,icy+7);
						strState.Format("%s", br);//MESS(32,WM_SETTEXT,0,br);					
					}
				}
			}
			else
			{
				strState.Format("%s", "");//MESS(31,WM_SETTEXT,0,"");			
			}

			// get the stream title and set sync for subsequent titles
			//DoMeta(BASS_ChannelGetTags(chan,BASS_TAG_META));
			BASS_ChannelSetSync(hStream, BASS_SYNC_META, 0, &MetaSync, 0);

			// set sync for end of stream
			BASS_ChannelSetSync(hStream, BASS_SYNC_END, 0, &EndSync, 0);
				
			// play it!
			BASS_ChannelPlay(hStream, FALSE);
		}
		else
		{
			char text[20];
			sprintf(text,"buffering... %d%%",progress);
			strState.Format("%s", text);//MESS(31,WM_SETTEXT,0,text);		
		}

		m_strTime = strState;
		Invalidate();	
	}
	else if(nIDEvent == 2)
	{
		if (m_bMouseOn)	CheckCurPos();
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL MusicPlay::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT MusicPlay::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	CDC* pDC = GetDC();
	if(pDC != NULL)
	{
		DrawSkin(pDC);
		ReleaseDC(pDC);
	}
	MovePlayerLocation();
	return 0;
}

void MusicPlay::OnPaint()
{
	CPaintDC dc(this); 
	DrawSkin(&dc);
}

BOOL MusicPlay::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	
	InitButtons();

	if(!LoadSkin())
	{
		CDialog::OnCancel();
		return FALSE;
	}

	CDC* pDC = GetDC();
	DrawSkin(pDC);
	ReleaseDC(pDC);
			
	InitBass();

	LoadRadioPlayList();

	SetTimer(2, 100, NULL );

	return TRUE; 
}

BOOL MusicPlay::LoadSkin()
{
	m_pBackSkin = new CGdiPlusBitmapResource;
	if(!m_pBackSkin->Load(IDB_PNG_BG, "PNG", AfxGetApp()->m_hInstance))
	{
		delete m_pBackSkin;
		m_pBackSkin = NULL;
		return FALSE;
	}	
	MovePlayerLocation();

	return TRUE;
}

void MusicPlay::DrawSkin(CDC* pDC)
{
	CDC memDC;
	CBitmap bmp;
	CBitmap* bmp_old;

	CRect rect;
	GetWindowRect(&rect);

	int cx = rect.Width();
	int cy = rect.Height();

	memDC.CreateCompatibleDC(pDC);

	BITMAPINFO bmInfo;
	memset(&bmInfo, 0x00, sizeof(BITMAPINFO));
	bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth = cx;
	bmInfo.bmiHeader.biHeight = cy;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = 32;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader.biSizeImage = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biClrImportant = 0;

	LPBYTE pbmpBits = NULL;
	HBITMAP hBitmap = ::CreateDIBSection(pDC->GetSafeHdc(), (LPBITMAPINFO)&bmInfo, DIB_RGB_COLORS, (void **)&pbmpBits, NULL, 0);
	if(hBitmap == NULL)
		bmp.CreateCompatibleBitmap(pDC, cx, cy);
	else
		bmp.Attach(hBitmap);

	bmp_old = memDC.SelectObject(&bmp);
	Graphics gps(memDC.GetSafeHdc());

	gps.DrawImage(
		m_pBackSkin->m_pBitmap, 
		Rect(0, 0, m_pBackSkin->m_pBitmap->GetWidth(), m_pBackSkin->m_pBitmap->GetHeight()), 
		0,
		0, 
		m_pBackSkin->m_pBitmap->GetWidth(), 
		m_pBackSkin->m_pBitmap->GetHeight(),
		UnitPixel);

	
	if(m_btnClose.IsAvailableDraw())
		m_btnClose.OnDrawLayerdWindow(gps);	

	if(m_btnPlus.IsAvailableDraw())
		m_btnPlus.OnDrawLayerdWindow(gps);
	
	if(m_btnMin.IsAvailableDraw())
		m_btnMin.OnDrawLayerdWindow(gps);

	DisplayTime(gps);

	CPoint ptWindowScreenPosition(rect.TopLeft());
	CSize szWindow(cx, cy);

	BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	CPoint ptSrc(0,0);

	HDC hdc = gps.GetHDC();

	BOOL bRet= ::UpdateLayeredWindow(GetSafeHwnd(), 
		pDC->GetSafeHdc(),
		&ptWindowScreenPosition,
		&szWindow,
		hdc,
		&ptSrc,
		0, 
		&blendPixelFunction,
		ULW_ALPHA);

	gps.ReleaseHDC(hdc);

	memDC.SelectObject(bmp_old);
	bmp.DeleteObject();
	memDC.DeleteDC();
}

void MusicPlay::DisplayTime(Graphics& gps)
{	
USES_CONVERSION;

	Gdiplus::Font font1(L"굴림", 9, FontStyleRegular, UnitPixel);
	SolidBrush brush1(Color(234, 137, 6));

	StringFormat stringFormat(StringFormatFlagsDisplayFormatControl);
	stringFormat.SetAlignment(StringAlignmentCenter);	
	stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	
	CRect rect;
    GetClientRect(&rect);
	
	RectF rectF1(REAL(rect.left + 1), REAL(rect.top + 25), REAL(rect.Width()), REAL(30));
	gps.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
	gps.DrawString(A2W(m_strTime), -1, &font1, rectF1, &stringFormat, &brush1);
}

void MusicPlay::MovePlayerLocation()
{	
	int nOffset = 10;
	int cx = m_pBackSkin->m_pBitmap->GetWidth();
	int cy = m_pBackSkin->m_pBitmap->GetHeight();

	RECT rcWorkArea;
	SystemParametersInfo (SPI_GETWORKAREA, 0, &rcWorkArea, 0);
	
	MoveWindow(rcWorkArea.right - cx - nOffset, rcWorkArea.bottom - cy - nOffset,  cx, cy);
	::SetWindowPos(this->m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);	
}

void MusicPlay::InitButtons()
{	
	m_btnPlus.SetImage(MAKEINTRESOURCE(IDB_PNG_PLUS), "PNG", AfxGetApp()->m_hInstance, 30, 2, 1);
	m_btnMin.SetImage(MAKEINTRESOURCE(IDB_PNG_MIN), "PNG", AfxGetApp()->m_hInstance,  50, 2, 1);
	m_btnClose.SetImage(MAKEINTRESOURCE(IDB_PNG_CLOSE), "PNG", AfxGetApp()->m_hInstance, 80, 5, 4);

	m_btnClose.SetVisibleButton(TRUE);
}

void MusicPlay::OnBnClickedButtonClose()
{	
	CDialog::OnOK();
}

void MusicPlay::OnBnClickedButtonSuaramin()
{
	if(m_nMaxCount == 0)
		return;

    m_nCurrentPlay --;

	if(m_nCurrentPlay < 0)
		m_nCurrentPlay = m_nMaxCount - 1;

	m_strCurrentRadio = m_RadioList.GetAt(m_nCurrentPlay);	
	
	PlayMusic((LPCTSTR)m_strCurrentRadio);
}

void MusicPlay::OnBnClickedButtonSuaraplus()
{
	if(m_nMaxCount == 0)
		return;

    m_nCurrentPlay ++;

	if(m_nCurrentPlay > m_nMaxCount-1)
		m_nCurrentPlay = 0;

	m_strCurrentRadio = m_RadioList.GetAt(m_nCurrentPlay);
	
	PlayMusic((LPCTSTR)m_strCurrentRadio);
}

void MusicPlay::LoadRadioPlayList()
{
	m_RadioList.RemoveAll();

	CString strInput = GetOpenRadioListFilePath("RadioList.json");
	if(strInput == "")
	{		
		m_RadioList.Add("http://mukulcast.com");
	}
	else
	{
		CString strName;
		CString strValue;
		CStringArray strTotalApp;
		if(GetSplitAppsTotal(strInput, strTotalApp))
		{		
			for(int i=0; i < strTotalApp.GetCount(); i++)
			{			
				GetSplitString(strTotalApp.GetAt(i), strName, strValue);
				if(lstrcmpi(strName, "url") == 0)
					m_RadioList.Add(strValue);
			}
		}	
		strTotalApp.RemoveAll();
	}

	m_nMaxCount = m_RadioList.GetSize();
	if(m_nMaxCount == 0) return;

	m_strCurrentRadio = m_RadioList.GetAt(0);
		
	m_nCurrentPlay = 0;

	PlayMusic((LPCTSTR)m_strCurrentRadio);
}

CString MusicPlay::GetModulePath()
{
	char cTemp[MAX_PATH];
	char *spzRealDirEnd;
	CString strModulePath;

	GetModuleFileName(NULL, cTemp, MAX_PATH);  
	spzRealDirEnd = strrchr(cTemp, '\\');
	*spzRealDirEnd = '\0';

	strModulePath = (CString)cTemp;

	return strModulePath;	
}

BOOL MusicPlay::GetSplitAppsTotal(CString strInput, CStringArray& strResultArr)
{
	BOOL bFind = FALSE;

	strResultArr.RemoveAll();
	CString strOutput(_T(""));
	int nCountLeft = strInput.Find(_T("{"));
	int nCountRight = strInput.Find(_T("}"));
	while(nCountRight>nCountLeft)
	{
		strOutput = strInput.Mid(nCountLeft+1, nCountRight - nCountLeft - 1);
		strResultArr.Add(strOutput);
		strInput = strInput.Mid(nCountRight+1);
		nCountLeft = strInput.Find(_T("{"));
		nCountRight = strInput.Find(_T("}"));
		bFind = TRUE;
	}	
	return bFind;
}

void MusicPlay::GetSplitString(CString strInput, CString &strName, CString &strValue)
{
	int nCount = strInput.Find(_T(":"));
	int nLength = strInput.GetLength();
	strName = strInput.Left(nCount);
	strValue = strInput.Right(nLength - nCount - 1);

	strName.Replace("\"","");
	strName.TrimLeft();
	strName.TrimRight();

	strValue.Replace("\"","");
	strValue.TrimLeft();
	strValue.TrimRight();
}

CString MusicPlay::GetOpenRadioListFilePath(CString strFileName)
{	
	CString strFileFullPath;
	strFileFullPath.Format("%s\\%s", GetModulePath(), strFileName);
	if(!::PathFileExists(strFileFullPath))
		return ""; 

	wstring content;
	CFile file;
	file.Open(strFileFullPath, CFile::modeRead);
	ReadFile::readFile(file, content);
	file.Close();

	CString strTotalContent(content.c_str());
	return strTotalContent;	
}