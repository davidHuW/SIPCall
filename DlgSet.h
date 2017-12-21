#pragma once
#include "afxcmn.h"


// CDlgSet dialog

class CDlgSet : public CDialog
{
	DECLARE_DYNAMIC(CDlgSet)

public:
	CDlgSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSet();

// Dialog Data
	enum { IDD = IDD_DIALOG_SET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
		
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	int m_nPort;
	CString m_strPath;
	afx_msg void OnBnClickedButton1();
	CIPAddressCtrl m_IPMic;
	CIPAddressCtrl m_IPServer;
	int m_ComPort;
	int m_PortPC;
	CString m_strCallA;
	CString m_strCallB;
	CString m_strOuterGroup;
	CSliderCtrl m_ctrlTime;
	int m_nTime;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	UINT m_nOutPort;
	CIPAddressCtrl m_CtrlOutIP;
	int m_PortEvt;
	CIPAddressCtrl m_IPAux;
	CIPAddressCtrl m_IPMICRecord;
	CString m_szTitle;
	afx_msg void OnBnClickedButtonIpSet();
	int m_PortEvtSend;
	CString m_strSignLeft;
	CString m_strSignRight;
	afx_msg void OnBnClickedButtonNoBc();
	CSliderCtrl m_ctrlSliderVolume;
	afx_msg void OnBnClickedButtonNoCall();
	CIPAddressCtrl m_ctrOutIP2;
	UINT m_nOutIP2;
	CString m_strPathRecord;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonLine();
	CIPAddressCtrl m_IPMonitor;
	afx_msg void OnBnClickedButtonVol();
	afx_msg void OnBnClickedButtonFire();
	int m_PortOuterBCControl;
	CIPAddressCtrl m_IpOuterBCControl;
};
