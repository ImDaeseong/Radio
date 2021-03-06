#include "stdafx.h"
#include "fmodplayer.h"
#include "MusicPlay.h"

MusicPlay::MusicPlay(CWnd* pParent /*=NULL*/)
	: CDialog(MusicPlay::IDD, pParent)
{
	system = 0;
    sound = 0;
    channel = 0;

	result = FMOD_OK;
	openstate = FMOD_OPENSTATE_READY;

	m_pBackSkin	= NULL;	

	m_strTime = "";

	m_fVolume = 0.0;
}

void MusicPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_btnPlay);	
	DDX_Control(pDX, IDC_BUTTON_SUARAPLUS, m_btnPlus);
	DDX_Control(pDX, IDC_BUTTON_SUARAMIN, m_btnMin);
	DDX_Control(pDX, IDC_BUTTON_PLAYSTOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
}

BEGIN_MESSAGE_MAP(MusicPlay, CDialog)
	ON_WM_PAINT()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &MusicPlay::OnBnClickedButtonPlay)	
	ON_BN_CLICKED(IDC_BUTTON_SUARAPLUS, &MusicPlay::OnBnClickedButtonSuaraplus)
	ON_BN_CLICKED(IDC_BUTTON_SUARAMIN, &MusicPlay::OnBnClickedButtonSuaramin)	
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &MusicPlay::OnBnClickedButtonClose)
END_MESSAGE_MAP()

void MusicPlay::InitFMOD()
{	
    unsigned int version;
    void  *extradriverdata = 0;

	result = FMOD::System_Create(&system);
	result = system->getVersion(&version);
	result = system->init(1, FMOD_INIT_NORMAL, extradriverdata);
	result = system->setStreamBufferSize(64*1024, FMOD_TIMEUNIT_RAWBYTES);

	PlayMusic("http://live-radio01.mediahubaustralia.com/2TJW/mp3/");
}

void MusicPlay::CloseFMOD()
{	
	if (channel)
		result = channel->stop();
	
	do
    {
		m_strTime = "Waiting for sound to finish opening before trying to release it....";
		Invalidate();

        Sleep(50);
        
        result = system->update();
        result = sound->getOpenState(&openstate, 0, 0, 0);
    } while (openstate != FMOD_OPENSTATE_READY);

	result = sound->release();
	result = system->close(); 
    result = system->release();
}

void MusicPlay::PlayMusic(LPCSTR chMusicName)
{		
	if( channel != NULL )
		channel->stop();
	
	if( sound != NULL )
		sound->release();
	
    result = system->createSound(chMusicName, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &sound);    
   		
	//Timer
	KillTimer(1);
	SetTimer(1, 1000, NULL );
}

void MusicPlay::Play()
{
	if( channel != NULL )
		channel->setPaused( false );
}

void MusicPlay::Pause()
{
	if( channel != NULL )
	{
		FMOD_RESULT result;
		bool paused;
		result = channel->getPaused(&paused);
		result = channel->setPaused(!paused);
	}
}

void MusicPlay::Stop()
{	
	if (channel)
	{
		result = channel->stop();
	}
}

void MusicPlay::setVolume()
{
	FMOD_RESULT result;

	if (m_fVolume > 1)
		m_fVolume = 1;
	else if (m_fVolume < 0)
		m_fVolume = 0;

	result = channel->setVolume(m_fVolume);
}

void MusicPlay::IncreaseVolume(BOOL bInCrease)
{
	if (bInCrease)
		m_fVolume += 0.1;
	else
		m_fVolume -= 0.1;

	setVolume();
}

void MusicPlay::OnDestroy() 
{		
	CloseFMOD();

	if(m_pBackSkin != NULL)
		delete m_pBackSkin;
	m_pBackSkin = NULL;	

	CDialog::OnDestroy();
}

void MusicPlay::OnTimer(UINT nIDEvent) 
{	
	unsigned int    pos = 0;
    unsigned int    percent = 0;
    bool            playing = false;
    bool            paused = false;
    bool            starving = false;
    const char     *state = "Stopped";
	const int       tagcount = 4;
    int             tagindex = 0;
    char            tagstring[tagcount][128] = { 0 };


	result = channel->isPlaying(&playing);
	result = channel->getPaused(&paused);  


	result = system->update();
	result = sound->getOpenState(&openstate, &percent, &starving, 0);
        
	if (channel)
	{
		FMOD_TAG tag;

		while (sound->getTag(0, -1, &tag) == FMOD_OK)
		{
			if (tag.datatype == FMOD_TAGDATATYPE_STRING)
			{
				sprintf(tagstring[tagindex], "%s = '%s' (%d bytes)", tag.name, (char *)tag.data, tag.datalen);
				tagindex = (tagindex + 1) % tagcount;
			}
			else if (tag.type == FMOD_TAGTYPE_FMOD)
			{
				if (!strcmp(tag.name, "Sample Rate Change"))
				{
					float frequency = *((float *)tag.data);
					result = channel->setFrequency(frequency);
				}
			}
		}
            
		result = channel->getPaused(&paused);            
		result = channel->isPlaying(&playing);            
		result = channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
		result = channel->setMute(starving);
	}
    else
	{
		system->playSound(sound, 0, false, &channel);
	}

	if (openstate == FMOD_OPENSTATE_BUFFERING)
	{
		state = "Buffering...";
	}
	else if (openstate == FMOD_OPENSTATE_CONNECTING)
	{
		state = "Connecting...";
	}
	else if (paused)
	{
		state = "Paused";
	}
	else if (playing)
	{
		state = "Playing";
	}

	CString strTime, strState, strPercent, strTags;
	strTime.Format("Time = %02d:%02d:%02d", pos / 1000 / 60, pos / 1000 % 60, pos / 10 % 100);
    strState.Format("State = %s %s", state, starving ? "(STARVING)" : "");
    strPercent.Format("Buffer Percentage = %d", percent);
    
	strTags ="Tags:";
    for (int i = tagindex; i < (tagindex + tagcount); i++)
    {
        strTags.Format("%s", tagstring[i % tagcount]);
    }

	m_strTime = strState;
	Invalidate();
	

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
			
	InitFMOD();

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

	if(m_btnPlay.IsAvailableDraw())
		m_btnPlay.OnDrawLayerdWindow(gps);	

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

	Gdiplus::Font font1(L"굴림", 11, FontStyleRegular, UnitPixel);
	SolidBrush brush1(Color(234, 137, 6));

	StringFormat stringFormat(StringFormatFlagsDisplayFormatControl);
	stringFormat.SetAlignment(StringAlignmentCenter);	
	stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	
	CRect rect;
    GetClientRect(&rect);
	
	RectF rectF1(REAL(rect.left + 1), REAL(rect.top + 415), REAL(rect.Width()), REAL(30));
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
	m_btnPlay.SetImage(MAKEINTRESOURCE(IDB_PNG_PLAY), "PNG", AfxGetApp()->m_hInstance, 40, 17, 1);
	m_btnPlus.SetImage(MAKEINTRESOURCE(IDB_PNG_PLUS), "PNG", AfxGetApp()->m_hInstance, 15, 17, 1);
	m_btnMin.SetImage(MAKEINTRESOURCE(IDB_PNG_MIN), "PNG", AfxGetApp()->m_hInstance,  65, 17, 1);
	m_btnClose.SetImage(MAKEINTRESOURCE(IDB_PNG_CLOSE), "PNG", AfxGetApp()->m_hInstance, 80, 5, 4);
}

void MusicPlay::OnBnClickedButtonClose()
{	
	CDialog::OnOK();
}

void MusicPlay::OnBnClickedButtonPlay()
{
	if( channel != NULL )
	{
		FMOD_RESULT result;
		bool paused;
		result = channel->getPaused(&paused);
		result = channel->setPaused(!paused);
	}
}

void MusicPlay::OnBnClickedButtonSuaraplus()
{
	IncreaseVolume(TRUE);
}

void MusicPlay::OnBnClickedButtonSuaramin()
{
	IncreaseVolume(FALSE);
}
