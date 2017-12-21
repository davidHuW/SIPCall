#pragma once


// CDlgPwd dialog

class CDlgPwd : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPwd)

public:
	CDlgPwd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPwd();

// Dialog Data
	enum { IDD = IDD_DIALOG_PWD };

	void SetUserName(CString strName);
	char m_szPwd[256];
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CString m_strName;
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	CString m_strPwdCur;
	CString m_strPwdNew;
	CString m_strPwdCon;
};
