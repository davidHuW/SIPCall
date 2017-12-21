// DlgUser.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgUser.h"
#include "afxdialogex.h"
#include "DlgPwd.h"

#include "SIPCallDlg.h"

// CDlgUser dialog

IMPLEMENT_DYNAMIC(CDlgUser, CDialogEx)

CDlgUser::CDlgUser(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgUser::IDD, pParent)
	, m_strUserName(_T(""))
{
	m_pUser = NULL;
	m_nNum = 0;
}

CDlgUser::~CDlgUser()
{
	char szAppPath[256] = {0};
	WideCharToMultiByte(CP_ACP, 0, g_szAPPPath, -1, szAppPath, 256, NULL, NULL);
	char szFile[256] = {0};
	sprintf(szFile, "%s%s", szAppPath, "usr.dat");
	g_User.SaveUser(szFile);
}

void CDlgUser::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUserName);
	DDX_Control(pDX, IDC_LIST1, m_listUser);
}


BEGIN_MESSAGE_MAP(CDlgUser, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_M_PWD, &CDlgUser::OnBnClickedBtnMPwd)
	ON_BN_CLICKED(IDC_BTN_DEL_USER, &CDlgUser::OnBnClickedBtnDelUser)
	ON_BN_CLICKED(IDC_BTN_ADD_USER, &CDlgUser::OnBnClickedBtnAddUser)
	ON_BN_CLICKED(IDC_BTN_PWD_RESET, &CDlgUser::OnBnClickedBtnPwdReset)
END_MESSAGE_MAP()


// CDlgUser message handlers


void CDlgUser::OnBnClickedBtnMPwd()
{
	// TODO: Add your control notification handler code here
	int curSel = m_listUser.GetCurSel();
	if (curSel != -1)
	{
		CString strName;
		m_listUser.GetText(curSel, strName);
		char szName[256] = {0};
		WideCharToMultiByte(CP_ACP, 0, strName, -1, szName, 256, NULL, NULL);

		CDlgPwd dlg;
		dlg.SetUserName(strName);
		if (dlg.DoModal() == IDOK )
		{
			if (strlen(dlg.m_szPwd) > 0)
			{
				g_User.ModifyPWD(szName, dlg.m_szPwd);
				MessageBox(L"修改密码成功！", L"提醒");
			}
		}	

	}
	
}


void CDlgUser::OnBnClickedBtnDelUser()
{
	// TODO: Add your control notification handler code here
	int curSel = m_listUser.GetCurSel();
	if (curSel != -1)
	{
		CString strName;
		m_listUser.GetText(curSel, strName);

		char szName[256] = {0};
		WideCharToMultiByte(CP_ACP, 0, strName, -1, szName, 256, NULL, NULL);

		int type = g_User.GetUserType(szName);
		if (type == ENUM_TYPE_USER)
		{
			g_User.RemoveUser(szName);
			m_listUser.DeleteString(curSel);
		}
		else{
			MessageBox(L"无法删除用户！", L"提醒");
		}		
	}
}


void CDlgUser::OnBnClickedBtnAddUser()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_strUserName != L"")
	{		
		char szName[256] = {0};
		WideCharToMultiByte(CP_ACP, 0, m_strUserName, -1, szName, 256, NULL, NULL);
		if (g_User.AddUser(szName, szName))
		{
			m_listUser.AddString(m_strUserName);
		}
		
	}		
}


BOOL CDlgUser::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_pUser = g_User.GetUser(m_nNum);
	for (int i = 0; i < m_nNum; i++)
	{
		if (g_dataType == ENUM_TYPE_SUPER)
		{
			wchar_t wName[256] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pUser[i].name, -1, wName, 256);
			m_listUser.AddString(wName);
		}
		else if (g_dataType == ENUM_TYPE_ADMIN)
		{
			if (m_pUser[i].type == ENUM_TYPE_ADMIN  || m_pUser[i].type == ENUM_TYPE_USER)
			{
				wchar_t wName[256] = {0};
				MultiByteToWideChar(CP_ACP, 0, m_pUser[i].name, -1, wName, 256);
				m_listUser.AddString(wName);
			}
		}
		else{
			if (m_pUser[i].type == ENUM_TYPE_USER)
			{
				wchar_t wName[256] = {0};
				MultiByteToWideChar(CP_ACP, 0, m_pUser[i].name, -1, wName, 256);
				m_listUser.AddString(wName);
			}
		}
		
	}
	if (g_dataType == ENUM_TYPE_USER)
	{
		GetDlgItem(IDC_BTN_PWD_RESET)->ShowWindow(SW_HIDE);
	}
	else{
		GetDlgItem(IDC_BTN_PWD_RESET)->ShowWindow(SW_SHOW);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgUser::OnBnClickedBtnPwdReset()
{
	// TODO: Add your control notification handler code here
	int curSel = m_listUser.GetCurSel();
	if (curSel != -1)
	{
		CString strName;
		m_listUser.GetText(curSel, strName);

		char szName[256] = {0};
		WideCharToMultiByte(CP_ACP, 0, strName, -1, szName, 256, NULL, NULL);

		int type = g_User.GetUserType(szName);
		if (type == ENUM_TYPE_USER)
		{
			g_User.ResetPwd(szName);			
		}
		else{
			MessageBox(L"无法重置管理人员密码！", L"提醒");
		}		
	}
}
