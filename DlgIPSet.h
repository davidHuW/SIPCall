#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDlgIPSet dialog

class CDlgIPSet : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgIPSet)

public:
	CDlgIPSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgIPSet();

// Dialog Data
	enum { IDD = IDD_DIALOG_IP };

	void SetLayout(int bMain);
	void LoadIpData();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CIPAddressCtrl m_ctrlIPMain;
	CIPAddressCtrl m_ctrlIPSub;
	CListBox m_ctrlListIP;
	afx_msg void OnBnClickedBtnAddSub();
	afx_msg void OnBnClickedBtnDelSub();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadioMain();
	afx_msg void OnBnClickedRadioSub();
};
