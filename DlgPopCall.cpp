// DlgPopCall.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgPopCall.h"
#include "afxdialogex.h"
#include "SIPCallDlg.h"

#define TIMER_ID_TIME	(90000)
#define TIMER_REVER		(120)




static wchar_t* s_accidentType[] = {
// 	L"撞护栏",
// 	L"着火",
// 	L"翻车",
// 	L"撞车",
// 	L"爆胎",
// 	L"发动机故障",
// 	L"螺丝断",
// 	L"半轴故障"
	L"需要加油求助",
	L"需要拖车求助",
	L"交通事故求助",
	L"内部管理联系",
	L"呼叫无应答",
	L"捣乱",
	L"检测",
	L"其他"
};
// CDlgPopCall dialog

IMPLEMENT_DYNAMIC(CDlgPopCall, CDialogEx)

CDlgPopCall::CDlgPopCall(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPopCall::IDD, pParent)
	, m_strEvtNo(_T(""))
	, m_strKM(_T(""))
	, m_strDir(_T(""))
	, m_strCarNo(_T("苏A88888"))
{
	m_pDevice = NULL;
}

CDlgPopCall::~CDlgPopCall()
{
	
}

void CDlgPopCall::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_ctrlComboxType);
	DDX_Text(pDX, IDC_EDIT_EXTNO, m_strEvtNo);
	DDX_Text(pDX, IDC_EDIT_KM, m_strKM);
	DDX_Text(pDX, IDC_EDIT_DIR, m_strDir);
	DDX_Text(pDX, IDC_EDIT_CARNO, m_strCarNo);
}


BEGIN_MESSAGE_MAP(CDlgPopCall, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgPopCall::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT_DIR, &CDlgPopCall::OnChangeEditDir)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CDlgPopCall::OnSelchangeComboType)
	ON_EN_CHANGE(IDC_EDIT_CARNO, &CDlgPopCall::OnChangeEditCarno)
END_MESSAGE_MAP()


// CDlgPopCall message handlers


void CDlgPopCall::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	SaveEvt();
	CDialogEx::OnOK();
}


BOOL CDlgPopCall::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_nCount = 0;
	m_bKill = false;

	if (m_pDevice)
	{		
		wchar_t wzName[MAX_PATH] = {0};
		wsprintf(wzName, L"保存(%d)", TIMER_REVER - m_nCount);
		SetDlgItemText(IDOK, wzName);

		SetTimer(TIMER_ID_TIME, 1000, 0);
		// 分机号
		wchar_t wzExtNo[32] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_pDevice->sDB.extNo, -1, wzExtNo, 32);
		SetDlgItemText(IDC_EDIT_EXTNO, wzExtNo);
		// 公里号
		wchar_t wzKM[32] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_pDevice->sDB.kmID, -1, wzKM, 32);
		SetDlgItemText(IDC_EDIT_KM, wzKM);
		// 方向
		wchar_t wzDir[MAX_PATH] = {0};
		if (m_pDevice->sDB.flag == 0)
		{
			MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.signL, -1, wzDir, MAX_PATH);			
		}
		else{
			MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.signR, -1, wzDir, MAX_PATH);
		}
		SetDlgItemText(IDC_EDIT_DIR, wzDir);
		int typeSz = sizeof(s_accidentType) / sizeof(char*);
		for (int i = 0; i < typeSz; i++)
		{
			m_ctrlComboxType.AddString(s_accidentType[i]);
		}
		m_ctrlComboxType.SetCurSel(0);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgPopCall::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == TIMER_ID_TIME)
	{
		m_nCount++;
		if (TIMER_REVER - m_nCount > 0)
		{
			wchar_t wzName[MAX_PATH] = {0};
			wsprintf(wzName, L"保存(%d)", TIMER_REVER - m_nCount);
			SetDlgItemText(IDOK, wzName);
		}
		else{
			SaveEvt();
			SendMessage(WM_CLOSE);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CDlgPopCall::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	if (!m_bKill)
	{
		KillTimer(TIMER_ID_TIME);
		m_bKill = true;
	}	
}
void CDlgPopCall::SetInfo(SDevice* pDevice)
{
	m_pDevice = pDevice;
}

void CDlgPopCall::OnChangeEditDir()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
// 	if (!m_bKill)
// 	{
// 		KillTimer(TIMER_ID_TIME);
// 		m_bKill = true;		
// 		SetDlgItemText(IDOK, L"保存");
// 	}	
}


void CDlgPopCall::OnSelchangeComboType()
{
	// TODO: Add your control notification handler code here
// 	if (!m_bKill)
// 	{
// 		KillTimer(TIMER_ID_TIME);
// 		m_bKill = true;
// 		SetDlgItemText(IDOK, L"保存");
// 	}	
}


void CDlgPopCall::OnChangeEditCarno()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
// 	if (!m_bKill)
// 	{
// 		KillTimer(TIMER_ID_TIME);
// 		m_bKill = true;
// 		SetDlgItemText(IDOK, L"保存");
// 	}	
	
}
void CDlgPopCall::SaveEvt()
{
	UpdateData(TRUE);
	CString strEvtFile = g_szAPPPath + L"event";
	strEvtFile += L"\\evt.log";
	HANDLE hFile = CreateFile(strEvtFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL ||
		hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);
	int curSel = m_ctrlComboxType.GetCurSel();
	SYSTEMTIME tm;
	GetSystemTime(&tm);
	wchar_t wzEvt[512] = {0};
	wsprintf(wzEvt, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d]#%s,%s,%s,%s,%s#\r\n", tm.wYear, tm.wMonth, tm.wDay, tm.wHour + 8, tm.wMinute, tm.wSecond, tm.wMilliseconds, m_strEvtNo, m_strKM, m_strDir, s_accidentType[curSel], m_strCarNo);
	char czEvt[512] = {0};
	WideCharToMultiByte(CP_ACP, 0, wzEvt, -1, czEvt, 1024, NULL, NULL);
	DWORD dwWrite = 0;
	WriteFile(hFile, czEvt, strlen(czEvt), &dwWrite, NULL);
	CloseHandle(hFile);
}