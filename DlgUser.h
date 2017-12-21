#pragma once
#include "afxwin.h"
#include "MgrUser.h"

// CDlgUser dialog

class CDlgUser : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgUser)

public:
	CDlgUser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgUser();

// Dialog Data
	enum { IDD = IDD_DIALOG_USER };


	SUsrInfo* m_pUser;
	int m_nNum;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnMPwd();
	afx_msg void OnBnClickedBtnDelUser();
	afx_msg void OnBnClickedBtnAddUser();
	CString m_strUserName;
	CListBox m_listUser;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnPwdReset();
};
