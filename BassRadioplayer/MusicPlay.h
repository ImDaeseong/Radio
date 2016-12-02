#pragma once
#include "afxwin.h"

class MusicPlay : public CDialog
{
public:
	MusicPlay(CWnd* pParent = NULL);  
	
	enum { IDD = IDD_DIALOG_PLAY };

    protected:
	virtual void DoDataExchange(CDataExchange* pDX);  

protected:
    virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();	
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	
private:	
	void InitBass();
	void CloseBass();
	void PlayMusic(LPCSTR chMusicName);	

	HSTREAM hStream;

private:
	void DrawSkin(CDC* pDC);
	BOOL LoadSkin();
	void MovePlayerLocation();
	void InitButtons();
	CGdiPlusBitmapResource* m_pBackSkin;
	void DisplayTime(Graphics& gps);
		
	CString m_strTime;

	int m_nMaxCount;
	int m_nCurrentPlay;
	CString m_strCurrentRadio;
	CStringArray m_RadioList;

	void LoadRadioPlayList();
	CString GetModulePath();
	BOOL GetSplitAppsTotal(CString strInput, CStringArray& strResultArr);
	void GetSplitString(CString strInput, CString &strName, CString &strValue);
	CString GetOpenRadioListFilePath(CString strFileName);

	BOOL m_bMouseOn;
	HWND GetCurPosHandle(POINT point);
	void CheckCurPos();

public:
	CSkinButton m_btnPlus;
	CSkinButton m_btnMin;
	CSkinButton m_btnClose;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonSuaraplus();
	afx_msg void OnBnClickedButtonSuaramin();
};
