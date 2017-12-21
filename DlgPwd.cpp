// DlgPwd.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgPwd.h"
#include "afxdialogex.h"

#include "SIPCallDlg.h"
// CDlgPwd dialog

IMPLEMENT_DYNAMIC(CDlgPwd, CDialogEx)

CDlgPwd::CDlgPwd(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPwd::IDD, pParent)
	, m_strName(_T(""))
	, m_strPwdCur(_T(""))
	, m_strPwdNew(_T(""))
	, m_strPwdCon(_T(""))
{

}

CDlgPwd::~CDlgPwd()
{
}

void CDlgPwd::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT4, m_strName);
	DDX_Text(pDX, IDC_EDIT_CUR_PWD, m_strPwdCur);
	DDX_Text(pDX, IDC_EDIT_NEW_PWD, m_strPwdNew);
	DDX_Text(pDX, IDC_EDIT_CON_PWD, m_strPwdCon);
}


BEGIN_MESSAGE_MAP(CDlgPwd, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgPwd::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPwd::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgPwd message handlers


void CDlgPwd::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	memset(m_szPwd, 0, 256 * sizeof(char));
	// user
	char szName[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, m_strName, -1, szName, 256, NULL, NULL);
	SUsrInfo info;
	g_User.GetUser(szName, info);
	// pwd cur
	char szPwdCur[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, m_strPwdCur, -1, szPwdCur, 256, NULL, NULL);
	// pwd new
	char szPwdNew[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, m_strPwdNew, -1, szPwdNew, 256, NULL, NULL);
	// pwd confirm
	char szPwdCon[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, m_strPwdCon, -1, szPwdCon, 256, NULL, NULL);
	if (strcmp(info.pwd, szPwdCur) == 0)
	{
		if (strcmp(szPwdNew, szPwdCon) == 0)
		{
			sprintf(m_szPwd, szPwdCon);
		}
		else{
			MessageBox(L"修改密码不一致！", L"提醒");
		}
	}
	else{
		MessageBox(L"用户密码输入错误！", L"提醒");
	}
	CDialogEx::OnOK();
}


void CDlgPwd::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}


BOOL CDlgPwd::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPwd::SetUserName(CString strName)
{
	m_strName = strName;
}
