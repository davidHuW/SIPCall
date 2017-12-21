#pragma once


// CDlgPwdCheck dialog

class CDlgPwdCheck : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPwdCheck)

public:
	CDlgPwdCheck(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPwdCheck();
	void SetUserName(CString strName);
	bool IsConfirm();
// Dialog Data
	enum { IDD = IDD_DIALOG_PWD_CHECK };
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_strName;
	CString m_strPwd;
	bool m_bConfirm;
	virtual BOOL OnInitDialog();
};
