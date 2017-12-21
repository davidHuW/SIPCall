// DlgPwdCheck.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgPwdCheck.h"
#include "afxdialogex.h"
#include "SIPCallDlg.h"
// CDlgPwdCheck dialog

IMPLEMENT_DYNAMIC(CDlgPwdCheck, CDialogEx)

CDlgPwdCheck::CDlgPwdCheck(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPwdCheck::IDD, pParent)
	, m_strName(_T(""))
	, m_strPwd(_T(""))
{

}

CDlgPwdCheck::~CDlgPwdCheck()
{
}

void CDlgPwdCheck::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT4, m_strName);
	DDX_Text(pDX, IDC_EDIT_CUR_PWD, m_strPwd);
}


BEGIN_MESSAGE_MAP(CDlgPwdCheck, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgPwdCheck::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgPwdCheck message handlers


void CDlgPwdCheck::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
// 	memset(m_szPwd, 0, 256 * sizeof(char));
	// user
	char szName[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, m_strName, -1, szName, 256, NULL, NULL);
	SUsrInfo info;
	g_User.GetUser(szName, info);
	// pwd cur
	char szPwdCur[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, m_strPwd, -1, szPwdCur, 256, NULL, NULL);
// 	// pwd new
// 	char szPwdNew[256] = {0};
// 	WideCharToMultiByte(CP_ACP, 0, m_strPwdNew, -1, szPwdNew, 256, NULL, NULL);
// 	// pwd confirm
// 	char szPwdCon[256] = {0};
// 	WideCharToMultiByte(CP_ACP, 0, m_strPwdCon, -1, szPwdCon, 256, NULL, NULL);
	if (strncmp(info.pwd, szPwdCur, strlen(info.pwd)) == 0)
	{
		m_bConfirm = true;
		CDialogEx::OnOK();
	}
	else{
		m_bConfirm = false;
		MessageBox(L"密码不正确，无法强制重启！", L"提醒");
	}
	
}
void CDlgPwdCheck::SetUserName(CString strName)
{
	m_strName = strName;
}

BOOL CDlgPwdCheck::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_bConfirm = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
bool CDlgPwdCheck::IsConfirm()
{
	return m_bConfirm;
}