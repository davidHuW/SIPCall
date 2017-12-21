// DlgLogin.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgLogin.h"
#include "afxdialogex.h"
#include "ZBase64.h"
#include "SIPCallDlg.h"
// CDlgLogin dialog

IMPLEMENT_DYNAMIC(CDlgLogin, CDialogEx)

CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgLogin::IDD, pParent)
	, m_strName(_T(""))
	, m_strPass(_T(""))
{
	m_type = ENUM_TYPE_NONE;
}

CDlgLogin::~CDlgLogin()
{
}

void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USER, m_strName);
	DDX_Text(pDX, IDC_EDIT_PAS, m_strPass);
}


BEGIN_MESSAGE_MAP(CDlgLogin, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CDlgLogin::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CDlgLogin::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgLogin message handlers


void CDlgLogin::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	
	CDialogEx::OnCancel();
}
enAdminType CDlgLogin::GetType(CString strName, CString strPwd)
{
	int nRecNum = 0;
	SUsrInfo* pUserInfo = g_User.GetUser(nRecNum);
	char szName[256] = {0};
	char szPwd[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, strName, -1, szName, 256, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strPwd, -1, szPwd, 256, NULL, NULL);
	if (strlen(szName) && strlen(szPwd))
	{
		for (int i = 0; i < nRecNum; i++)
		{
			if ((strcmp(pUserInfo[i].name, szName) == 0) && 
				(strcmp(pUserInfo[i].pwd, szPwd) == 0))
			{
				return pUserInfo[i].type;
			}
		}
	}
	return ENUM_TYPE_NONE;
}

void CDlgLogin::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_type = GetType(m_strName, m_strPass);
	if(m_type != ENUM_TYPE_NONE){
		CDialogEx::OnOK();
	}	
	else{
		MessageBox(L"请输入正确的用户名和密码！", L"提醒");
	}
}
enAdminType CDlgLogin::GetType()
{
	return m_type;
}
CString CDlgLogin::GetLoginName()
{
	return m_strName;
}