// DlgIPSet.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgIPSet.h"
#include "afxdialogex.h"
#include "DecodeMsg.h"
#include "SIPCallDlg.h"
// CDlgIPSet dialog

IMPLEMENT_DYNAMIC(CDlgIPSet, CDialogEx)

CDlgIPSet::CDlgIPSet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgIPSet::IDD, pParent)
{

}

CDlgIPSet::~CDlgIPSet()
{

}

void CDlgIPSet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_MAIN, m_ctrlIPMain);
	DDX_Control(pDX, IDC_IPADDRESS_SUB, m_ctrlIPSub);
	DDX_Control(pDX, IDC_LIST1, m_ctrlListIP);
}


BEGIN_MESSAGE_MAP(CDlgIPSet, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgIPSet::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_ADD_SUB, &CDlgIPSet::OnBnClickedBtnAddSub)
	ON_BN_CLICKED(IDC_BTN_DEL_SUB, &CDlgIPSet::OnBnClickedBtnDelSub)	
	ON_BN_CLICKED(IDC_RADIO_MAIN, &CDlgIPSet::OnBnClickedRadioMain)
	ON_BN_CLICKED(IDC_RADIO_SUB, &CDlgIPSet::OnBnClickedRadioSub)
END_MESSAGE_MAP()


// CDlgIPSet message handlers


void CDlgIPSet::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here	
	int nNum = 0;
	SIPData* pData = g_MSG.GetIPData(nNum);	
	int nMainSystem = ((CButton*)GetDlgItem(IDC_RADIO_MAIN))->GetCheck();
	if(nNum > 0 ){
		unsigned  char  *pIP;  	
		DWORD  dwIP;  
		m_ctrlIPMain.GetAddress(dwIP);  
		pIP  =  (unsigned  char*)&dwIP; 
		sprintf(pData[0].ip, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);		
		g_MSG.SaveIPData();
		if (nMainSystem)
		{
			if (nNum == 1)
			{
				MessageBox(L"请把分控IP添加到List中！", L"提醒");
			}
			else{
				g_Config.m_SystemConfig.nMainSystem = nMainSystem;
				g_Config.m_ini.SetMainFlag(nMainSystem);
				CDialogEx::OnOK();
			}
		}
		else{
			g_Config.m_SystemConfig.nMainSystem = nMainSystem;
			g_Config.m_ini.SetMainFlag(nMainSystem);
			CDialogEx::OnOK();
		}
		
	}	
	else{
		MessageBox(L"请设置主分控IP", L"提醒");
	}
	
}


void CDlgIPSet::OnBnClickedBtnAddSub()
{
	// TODO: Add your control notification handler code here
	unsigned  char  *pIP;  	
	DWORD  dwIP;  
	m_ctrlIPSub.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	SIPData data;
	data.type = 2;
	sprintf(data.ip, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	g_MSG.AddIPData(data);

	wchar_t wName[256] = {0};
	MultiByteToWideChar(CP_ACP, 0, data.ip, -1, wName, 256);
	m_ctrlListIP.AddString(wName);
}


void CDlgIPSet::OnBnClickedBtnDelSub()
{
	// TODO: Add your control notification handler code here
	int curSel = m_ctrlListIP.GetCurSel();
	if (curSel != -1)
	{
		CString strName;
		m_ctrlListIP.GetText(curSel, strName);
		char IP[32] = {0};
		WideCharToMultiByte(CP_ACP, 0, strName, -1, IP, 32, NULL, NULL);
		g_MSG.DelIPData(IP);
		m_ctrlListIP.DeleteString(curSel);
	}
}
void CDlgIPSet::LoadIpData()
{
	m_ctrlListIP.ResetContent();
	int nNum = 0;
	SIPData* pData = g_MSG.GetIPData(nNum);
	if (nNum > 0)
	{
		DWORD  dwIP  =  inet_addr(pData[0].ip);
		unsigned  char  *pIP  =  (unsigned  char*)&dwIP;
		m_ctrlIPMain.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));
		for (int i = 1; i < nNum; i++)
		{
			wchar_t wName[256] = {0};
			MultiByteToWideChar(CP_ACP, 0, pData[i].ip, -1, wName, 256);
			m_ctrlListIP.AddString(wName);
		}
	}
}

BOOL CDlgIPSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	
	LoadIpData();
	if (g_Config.m_SystemConfig.nMainSystem)
	{
		((CButton*)GetDlgItem(IDC_RADIO_MAIN))->SetCheck(1);
		SetLayout(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_RADIO_SUB))->SetCheck(1);
		SetLayout(0);
	}	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CDlgIPSet::SetLayout(int bMain)
{
	if (bMain)
	{
		GetDlgItem(IDC_IPADDRESS_SUB)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_ADD_SUB)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DEL_SUB)->EnableWindow(TRUE);
		GetDlgItem(IDC_LIST1)->EnableWindow(TRUE);
	}
	else{
		GetDlgItem(IDC_IPADDRESS_SUB)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ADD_SUB)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_DEL_SUB)->EnableWindow(FALSE);
		GetDlgItem(IDC_LIST1)->EnableWindow(FALSE);
	}
}

void CDlgIPSet::OnBnClickedRadioMain()
{
	// TODO: Add your control notification handler code here	
	SetLayout(1);
	DWORD  dwIP  =  inet_addr(g_MSG.GetHostIP());
	unsigned  char  *pIP  =  (unsigned  char*)&dwIP;
	m_ctrlIPMain.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));	
}


void CDlgIPSet::OnBnClickedRadioSub()
{
	// TODO: Add your control notification handler code here
	SetLayout(0);
}
