#pragma once

#include "DataM.h"
// CDlgLogin dialog

class CDlgLogin : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgLogin)

public:
	CDlgLogin(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLogin();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOGIN };

	enAdminType GetType(CString strName, CString strPwd);

	enAdminType m_type;
	enAdminType GetType();
	CString GetLoginName();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CString m_strName;
	CString m_strPass;
};
