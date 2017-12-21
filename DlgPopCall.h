#pragma once
#include "DataM.h"
#include "afxwin.h"



// CDlgPopCall dialog

class CDlgPopCall : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPopCall)

public:
	CDlgPopCall(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPopCall();
	void SetInfo(SDevice* pDevice);
	void SaveEvt();
private:
	SDevice* m_pDevice;

	int m_nCount;
	bool m_bKill;

// Dialog Data
	enum { IDD = IDD_DIALOG_POP_CALL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CComboBox m_ctrlComboxType;
	afx_msg void OnChangeEditDir();
	afx_msg void OnSelchangeComboType();
	afx_msg void OnChangeEditCarno();
	CString m_strEvtNo;
	CString m_strKM;
	CString m_strDir;
	CString m_strCarNo;
};
