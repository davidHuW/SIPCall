// DlgSet.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgSet.h"
#include "SIPCallDlg.h"
#include "DlgIPSet.h"
// CDlgSet dialog



IMPLEMENT_DYNAMIC(CDlgSet, CDialog)


CDlgSet::CDlgSet(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSet::IDD, pParent)	
	, m_nPort(0)
	, m_strPath(_T(""))
	, m_ComPort(0)
	, m_PortPC(0)
	, m_strCallA(_T(""))
	, m_strCallB(_T(""))
	, m_strOuterGroup(_T(""))
	, m_nTime(0)
	, m_nOutPort(0)
	, m_PortEvt(0)
	, m_szTitle(_T(""))
	, m_PortEvtSend(0)
	, m_strSignLeft(_T(""))
	, m_strSignRight(_T(""))
	, m_nOutIP2(0)
	, m_strPathRecord(_T(""))
	, m_PortOuterBCControl(0)
{

}

CDlgSet::~CDlgSet()
{
}

void CDlgSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 0, 65535);
	DDX_Text(pDX, IDC_EDIT2, m_strPath);
	DDX_Control(pDX, IDC_IPADDRESS_MIC, m_IPMic);
	DDX_Control(pDX, IDC_IPADDRESS_SERVER, m_IPServer);
	DDX_Text(pDX, IDC_EDIT_COM_PORT, m_ComPort);
	DDV_MinMaxInt(pDX, m_ComPort, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_PCPORT, m_PortPC);
	DDV_MinMaxInt(pDX, m_PortPC, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_CALL_A, m_strCallA);
	DDX_Text(pDX, IDC_EDIT_CALL_B, m_strCallB);
	DDX_Text(pDX, IDC_EDIT_OUTER_GROUP, m_strOuterGroup);
	DDX_Control(pDX, IDC_SLIDER_TIME, m_ctrlTime);
	DDX_Text(pDX, IDC_STATIC_TIME, m_nTime);
	DDV_MinMaxInt(pDX, m_nTime, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_PORT_OUT, m_nOutPort);
	DDV_MinMaxUInt(pDX, m_nOutPort, 0, 65535);
	DDX_Control(pDX, IDC_IPADDRESS_OUT, m_CtrlOutIP);
	DDX_Text(pDX, IDC_EDIT_PORT_EVENT, m_PortEvt);
	DDV_MinMaxInt(pDX, m_PortEvt, 0, 65535);
	DDX_Control(pDX, IDC_IPADDRESS_AUX, m_IPAux);
	DDX_Control(pDX, IDC_IPADDRESS_MIC_RECORD, m_IPMICRecord);
	DDX_Text(pDX, IDC_EDIT_TITLE, m_szTitle);
	DDX_Text(pDX, IDC_EDIT_PORT_EVENT_SEND, m_PortEvtSend);
	DDV_MinMaxInt(pDX, m_PortEvtSend, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_SIGN_LEFT, m_strSignLeft);
	DDX_Text(pDX, IDC_EDIT_SIGN_RIGHT, m_strSignRight);
	//DDX_Control(pDX, IDC_SLIDER_VOLUME, m_ctrlSliderVolume);
	DDX_Control(pDX, IDC_IPADDRESS_OUT2, m_ctrOutIP2);
	DDX_Text(pDX, IDC_EDIT_PORT_OUT2, m_nOutIP2);
	DDX_Text(pDX, IDC_EDIT3, m_strPathRecord);
	DDX_Control(pDX, IDC_IPADDRESS_MONITOR, m_IPMonitor);
	DDX_Text(pDX, IDC_EDIT_PORT_OUTER_CONTROL, m_PortOuterBCControl);
	DDV_MinMaxInt(pDX, m_PortOuterBCControl, 0, 65535);
	DDX_Control(pDX, IDC_IPADDRESS_OUTER_CONTROL, m_IpOuterBCControl);
}


BEGIN_MESSAGE_MAP(CDlgSet, CDialog)

	ON_BN_CLICKED(IDOK, &CDlgSet::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgSet::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSet::OnBnClickedButton1)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_IP_SET, &CDlgSet::OnBnClickedButtonIpSet)
	ON_BN_CLICKED(IDC_BUTTON_NO_BC, &CDlgSet::OnBnClickedButtonNoBc)
	ON_BN_CLICKED(IDC_BUTTON_NO_CALL, &CDlgSet::OnBnClickedButtonNoCall)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgSet::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_LINE, &CDlgSet::OnBnClickedButtonLine)
	ON_BN_CLICKED(IDC_BUTTON_VOL, &CDlgSet::OnBnClickedButtonVol)
	ON_BN_CLICKED(IDC_BUTTON_FIRE, &CDlgSet::OnBnClickedButtonFire)
END_MESSAGE_MAP()


// CDlgSet message handlers
void CString2Char(CString str, char* value)
{
	WideCharToMultiByte(CP_ACP, 0, str, -1, value, MAX_PATH, NULL, NULL);
}
void CDlgSet::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	//wsprintf(m_pAPP->m_SystemConfig.path, _T("%s"), m_strPath);
	CString2Char(m_strPath, g_Config.m_SystemConfig.path);
	CString2Char(m_strPathRecord, g_Config.m_SystemConfig.pathRecord);
	CString2Char(m_szTitle, g_Config.m_SystemConfig.title);
	CString2Char(m_strSignLeft, g_Config.m_SystemConfig.signL);
	CString2Char(m_strSignRight, g_Config.m_SystemConfig.signR);
	g_Config.m_SystemConfig.portPC = m_PortPC;
	g_Config.m_SystemConfig.portDevice = m_ComPort;
	g_Config.m_SystemConfig.portAudio = m_nPort;
	g_Config.m_SystemConfig.portOut = m_nOutPort;
	g_Config.m_SystemConfig.portOut2 = m_nOutIP2;
	g_Config.m_SystemConfig.portEvtSend = m_PortEvtSend;
	g_Config.m_SystemConfig.portEvtListen = m_PortEvt;
	g_Config.m_SystemConfig.portOuterBCControl = m_PortOuterBCControl;
	if (((CButton*)GetDlgItem(IDC_RADIO_HTTP))->GetCheck())
	{
		g_Config.m_SystemConfig.bHttpConnect = 1;
	}
	else{
		g_Config.m_SystemConfig.bHttpConnect = 0;
	}

	if (((CButton*)GetDlgItem(IDC_CHECK_RECORD))->GetCheck())
	{
		g_Config.m_SystemConfig.nRecord = 1;
	}
	else{
		g_Config.m_SystemConfig.nRecord = 0;
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_JT))->GetCheck())
	{
		g_Config.m_SystemConfig.nJT = 1;
	}
	else{
		g_Config.m_SystemConfig.nJT = 0;
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_POPCALL))->GetCheck())
	{
		g_Config.m_SystemConfig.nPop = 1;
	}
	else{
		g_Config.m_SystemConfig.nPop = 0;
	}
	// mic ip
	unsigned  char  *pIP;  	
	DWORD  dwIP;  
	m_IPMic.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	
	sprintf(g_Config.m_SystemConfig.IP_MIC, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	// server ip
	m_IPServer.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_Server, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	// out ip
	m_CtrlOutIP.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_Out, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	// out2 ip
	m_ctrOutIP2.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_Out2, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	// mic record ip
	m_IPMICRecord.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_MIC_RECORD, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	// aux ip
	m_IPAux.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_AUX, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	// monitor ip
	m_IPMonitor.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_Monitor, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);

	//bc outer control ip
	m_IpOuterBCControl.GetAddress(dwIP);  
	pIP  =  (unsigned  char*)&dwIP;  
	sprintf(g_Config.m_SystemConfig.IP_OUTER_BC_CONTROL, ("%u.%u.%u.%u"),*(pIP+3),  *(pIP+2),  *(pIP+1),  *pIP);
	//
	CString2Char(m_strCallA, g_Config.m_SystemConfig.CallA);
	CString2Char(m_strCallB, g_Config.m_SystemConfig.CallB);
	CString2Char(m_strOuterGroup, g_Config.m_SystemConfig.Outer);
	g_Config.m_SystemConfig.nPlayTime = m_nTime;
	//g_Config.m_SystemConfig.nVolume = m_ctrlSliderVolume.GetPos();
	g_Config.SaveConfig();	
	
	CDlgSet::OnOK();
}

void CDlgSet::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


BOOL CDlgSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetWindowText(_T("设置"));
	m_strPath = g_Config.m_SystemConfig.path;
	m_strPathRecord = g_Config.m_SystemConfig.pathRecord;
	m_ComPort = g_Config.m_SystemConfig.portDevice;	
	m_nPort = g_Config.m_SystemConfig.portAudio;
	m_PortPC = g_Config.m_SystemConfig.portPC;
	m_nTime = min(g_Config.m_SystemConfig.nPlayTime, MAX_PLAY_TIME);
	m_nOutPort = g_Config.m_SystemConfig.portOut;
	m_nOutIP2 = g_Config.m_SystemConfig.portOut2;
	m_PortEvt = g_Config.m_SystemConfig.portEvtListen;
	m_szTitle = g_Config.m_SystemConfig.title;
	m_PortEvtSend =  g_Config.m_SystemConfig.portEvtSend;
	m_strSignLeft = g_Config.m_SystemConfig.signL;
	m_strSignRight = g_Config.m_SystemConfig.signR;
	m_PortOuterBCControl = g_Config.m_SystemConfig.portOuterBCControl;
	if (g_Config.m_SystemConfig.bHttpConnect)
	{
		((CButton*)GetDlgItem(IDC_RADIO_HTTP))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_TCP))->SetCheck(0);
	}
	else{
		((CButton*)GetDlgItem(IDC_RADIO_HTTP))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_TCP))->SetCheck(1);
	}
	if (g_Config.m_SystemConfig.nRecord)
	{
		((CButton*)GetDlgItem(IDC_CHECK_RECORD))->SetCheck(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_CHECK_RECORD))->SetCheck(0);
	}
	if (g_Config.m_SystemConfig.nJT)
	{
		((CButton*)GetDlgItem(IDC_CHECK_JT))->SetCheck(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_CHECK_JT))->SetCheck(0);
	}
	if (g_Config.m_SystemConfig.nPop)
	{
		((CButton*)GetDlgItem(IDC_CHECK_POPCALL))->SetCheck(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_CHECK_POPCALL))->SetCheck(0);
	}
	// mic ip
// 	CString strIP_MIC;
// 	strIP_MIC.Format(_T("%s"),m_pAPP->m_SystemConfig.IP_MIC);
// 	char pIPInfo[32] = {0};
// 	WideCharToMultiByte(CP_ACP, 0, strIP_MIC, -1, pIPInfo, 32, NULL, NULL);
	DWORD  dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_MIC);
	unsigned  char  *pIP  =  (unsigned  char*)&dwIP;
	m_IPMic.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));
	// server ip
// 	CString strIP_server = m_pAPP->m_SystemConfig.IP_Server;
// 	memset(pIPInfo, 0, 32);
// 	WideCharToMultiByte(CP_ACP, 0, strIP_server, -1, pIPInfo, 32, NULL, NULL);
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_Server);
	pIP  =  (unsigned  char*)&dwIP;
	m_IPServer.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));
	// out ip
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_Out);
	pIP  =  (unsigned  char*)&dwIP;
	m_CtrlOutIP.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));

	// out2 ip
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_Out2);
	pIP  =  (unsigned  char*)&dwIP;
	m_ctrOutIP2.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));

	// mic record ip 
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_MIC_RECORD);
	pIP  =  (unsigned  char*)&dwIP;
	m_IPMICRecord.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));
	// aux ip
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_AUX);
	pIP  =  (unsigned  char*)&dwIP;
	m_IPAux.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));

	// monitor ip
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_Monitor);
	pIP  =  (unsigned  char*)&dwIP;
	m_IPMonitor.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));

	//outer bc control ip
	// monitor ip
	dwIP  =  inet_addr(g_Config.m_SystemConfig.IP_OUTER_BC_CONTROL);
	pIP  =  (unsigned  char*)&dwIP;
	m_IpOuterBCControl.SetAddress(*pIP,  *(pIP+1),  *(pIP+2),  *(pIP+3));
	// 
	m_strCallA = g_Config.m_SystemConfig.CallA;
	m_strCallB = g_Config.m_SystemConfig.CallB;
	m_strOuterGroup = g_Config.m_SystemConfig.Outer;
	UpdateData(FALSE);


	m_ctrlTime.SetRange(1, MAX_PLAY_TIME, TRUE);//设置滑动条范围
	m_ctrlTime.SetPos(m_nTime);//设置滑动条位置


	//m_ctrlSliderVolume.SetRange(80, 100, TRUE);
	//m_ctrlSliderVolume.SetPos(g_Config.m_SystemConfig.nVolume);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSet::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	wchar_t szPath[MAX_PATH];     //存放选择的目录路径 
	CString str;

	ZeroMemory(szPath, sizeof(szPath));   

	BROWSEINFO bi;   
	bi.hwndOwner = m_hWnd;   
	bi.pidlRoot = NULL;   
	bi.pszDisplayName = szPath;   
	bi.lpszTitle = L"请选择目录：";   
	bi.ulFlags = 0;   
	bi.lpfn = NULL;   
	bi.lParam = 0;   
	bi.iImage = 0;   
	//弹出选择目录对话框
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);   

	if(lp && SHGetPathFromIDList(lp, szPath))   
	{
		m_strPath = szPath;
	}
	UpdateData(FALSE);
}


void CDlgSet::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default	
	m_nTime =  m_ctrlTime.GetPos();//取得当前位置值
	UpdateData(FALSE);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CDlgSet::OnBnClickedButtonIpSet()
{
	// TODO: Add your control notification handler code here
	CDlgIPSet dlg;
	dlg.DoModal();
}


void CDlgSet::OnBnClickedButtonNoBc()
{
	// TODO: Add your control notification handler code here
	CString strFile = g_szAPPPath + ("NoBroadcast.txt");
	CFileFind fFind;
	BOOL bFind = fFind.FindFile(strFile);
	if (bFind)
	{
		ShellExecute( NULL, _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
	}
	else{
		MessageBox(L"NoBroadcast.txt不存在!", L"提醒");
	}
}


void CDlgSet::OnBnClickedButtonNoCall()
{
	// TODO: Add your control notification handler code here
	CString strFile = g_szAPPPath + ("NoCall.txt");
	CFileFind fFind;
	BOOL bFind = fFind.FindFile(strFile);
	if (bFind)
	{
		ShellExecute( NULL, _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
	}
	else{
		MessageBox(L"NoCall.txt不存在!", L"提醒");
	}
}


void CDlgSet::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	wchar_t szPath[MAX_PATH];     //存放选择的目录路径 
	CString str;

	ZeroMemory(szPath, sizeof(szPath));   

	BROWSEINFO bi;   
	bi.hwndOwner = m_hWnd;   
	bi.pidlRoot = NULL;   
	bi.pszDisplayName = szPath;   
	bi.lpszTitle = L"请选择目录：";   
	bi.ulFlags = 0;   
	bi.lpfn = NULL;   
	bi.lParam = 0;   
	bi.iImage = 0;   
	//弹出选择目录对话框
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);   

	if(lp && SHGetPathFromIDList(lp, szPath))   
	{
		m_strPathRecord = szPath;
	}
	UpdateData(FALSE);
}


void CDlgSet::OnBnClickedButtonLine()
{
	// TODO: Add your control notification handler code here
	CString strFile = g_szAPPPath + ("parking.txt");
	CFileFind fFind;
	BOOL bFind = fFind.FindFile(strFile);
	if (bFind)
	{
		ShellExecute( NULL, _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
	}
	else{
		MessageBox(L"parking.txt不存在!", L"提醒");
	}
}


void CDlgSet::OnBnClickedButtonVol()
{
	// TODO: Add your control notification handler code here
	CString strFile = g_szAPPPath + ("ying.txt");
	CFileFind fFind;
	BOOL bFind = fFind.FindFile(strFile);
	if (bFind)
	{
		ShellExecute( NULL, _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
	}
	else{
		MessageBox(L"ying.txt不存在!", L"提醒");
	}
}


void CDlgSet::OnBnClickedButtonFire()
{
	// TODO: Add your control notification handler code here
	CString strFile = g_szAPPPath + ("fire.txt");
	CFileFind fFind;
	BOOL bFind = fFind.FindFile(strFile);
	if (bFind)
	{
		ShellExecute( NULL, _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
	}
	else{
		MessageBox(L"fire.txt不存在!", L"提醒");
	}
}
