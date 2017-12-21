#pragma once
#include "afxwin.h"
#include "AudioPlay.h"
#include "DecodeMsg.h"
#include "HttpRecv.h"
#include "DataM.h"
#include "VoiceTTS.h"
// CDlgSend dialog

class CDlgSend : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSend)

public:
	CDlgSend(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSend();

// Dialog Data
	enum { IDD = IDD_DIALOG_SEND };
public:
	void SetBroadcastType(enBroadcastType enType);
private:
	void SetFile(bool flag);
	void SetStream(bool flag);
	void SetMIC(bool flag);

	//CDecodeMsg m_msg;
	//CDecodeMsg m_msg1;
	
	//CHttpRecv m_http;
public:
	void SetCurTunnelIdx(int curIdx);
	void AddMsgRecord(SDevice& stDevice, enPlaySrc model);
	void ClosePlay();
	void AutoStartStopStatus();
private:
	int m_curIdx;
	STunnel* m_pTunnelInfo;
	int m_nSel;   
	//CVoiceTTS m_tts;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strPath;
	afx_msg void OnBnClickedBtnFile();
	afx_msg void OnBnClickedBtnMic();
	CComboBox m_cbSRC;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnPath();
	afx_msg void OnSelchangeComboxSrc();
	afx_msg void OnBnClickedBtnMicClose();
	afx_msg void OnBnClickedBtnFileStop2();
	afx_msg void OnBnClickedBtnFile2();
	afx_msg void OnBnClickedBtnPath2();
	CString m_strStreamPath;
	afx_msg void OnBnClickedBtnFileStop();
	afx_msg void OnDestroy();
};

extern CString g_ttsFilePath;
extern bool g_RunPlay;