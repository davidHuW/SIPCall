// DlgSend.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgSend.h"
//#include "afxdialogex.h"
#include "SIPCallDlg.h"

// CDlgSend dialog

static CString s_strPath = _T("D:\\KuGou\\阿牛 - 桃花朵朵开.mp3");
static CString s_strStreamPath = _T("D:\\msg.txt");
static int s_CurSel = ENUM_MODEL_FILE;

IMPLEMENT_DYNAMIC(CDlgSend, CDialogEx)

CDlgSend::CDlgSend(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSend::IDD, pParent)

{
	m_strPath = s_strPath;
	m_strStreamPath = s_strStreamPath;
	
}

CDlgSend::~CDlgSend()
{
	//m_tts.UnInit();
	
}

void CDlgSend::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
	DDX_Control(pDX, IDC_COMBOX_SRC, m_cbSRC);
	DDX_Text(pDX, IDC_EDIT_PATH2, m_strStreamPath);
}


BEGIN_MESSAGE_MAP(CDlgSend, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_FILE, &CDlgSend::OnBnClickedBtnFile)
	ON_BN_CLICKED(IDC_BTN_MIC, &CDlgSend::OnBnClickedBtnMic)
	ON_BN_CLICKED(IDC_BTN_PATH, &CDlgSend::OnBnClickedBtnPath)
	ON_CBN_SELCHANGE(IDC_COMBOX_SRC, &CDlgSend::OnSelchangeComboxSrc)
	ON_BN_CLICKED(IDC_BTN_MIC_CLOSE, &CDlgSend::OnBnClickedBtnMicClose)
	ON_BN_CLICKED(IDC_BTN_FILE_STOP2, &CDlgSend::OnBnClickedBtnFileStop2)
	ON_BN_CLICKED(IDC_BTN_FILE2, &CDlgSend::OnBnClickedBtnFile2)
	ON_BN_CLICKED(IDC_BTN_PATH2, &CDlgSend::OnBnClickedBtnPath2)
	ON_BN_CLICKED(IDC_BTN_FILE_STOP, &CDlgSend::OnBnClickedBtnFileStop)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgSend message handlers


void CDlgSend::OnBnClickedBtnFile()
{
	// TODO: Add your control notification handler code here
	// music play
#if 0
	m_pAPP->m_ComMsg.CreateSocket("192.168.1.106", 6006);
	BYTE sub = 2;
	m_pAPP->m_ComMsg.SendOpenCall(sub);

	CString str1, str2;
	char pSendBuf[1024] = {0};
	int id = 8002;
	int outer = 204;
	sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\
		<Transfer attribute=\"Connect\">\
		<ext id=\"%d\"/>\
		<outer to=\"%d\"/>\
		</Transfer>\
", id, outer);
	//TCHAR wchar[1024] = {0};
	//MultiByteToWideChar(CP_UTF8, 0, pSend, -1, wchar, 1024);
	//CString strSend = pSend;
	m_http.HTTP_POST_DATA(_T("http://192.168.1.240"), pSendBuf, str1, str2);
#endif
	
	int nNum = 0;
	m_pTunnelInfo = g_data.GetData(nNum);
	if (m_curIdx < 0 || m_curIdx >= nNum)
	{
		return;
	}
	STunnel* pCurTunnel = m_pTunnelInfo + m_curIdx;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
	{
		if (pCurTunnel->vecDeviceUp[i].Check && 
			pCurTunnel->vecDeviceUp[i].bcStatus == ENUM_DEVICE_STATUS_OK)
		{

			USHORT port = g_Config.m_SystemConfig.portDevice;
			
			g_MSG.SendOpenBroadcast(pCurTunnel->vecDeviceUp[i].sDB.extAddr, pCurTunnel->vecDeviceUp[i].sDB.ip, port/*6006*//*port*//*m_pAPP->m_SystemConfig.portDevice*/);
			g_Play.SetIP(pCurTunnel->vecDeviceUp[i].sDB.ip);
			g_Play.SetBroadType(ENUM_BROAD_SINGLE);
			g_Play.SetModel(ENUM_MODEL_FILE);
			char pFile[MAX_PATH] = {0};
			WideCharToMultiByte(CP_ACP, 0, m_strPath, -1, pFile, MAX_PATH, NULL, NULL);
			//g_Play.play(pFile);
			AddMsgRecord(pCurTunnel->vecDeviceUp[i], ENUM_MODEL_FILE);
			pCurTunnel->vecDeviceUp[i].bcStatus = ENUM_DEVICE_STATUS_RUN;
		}
	}
	for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
	{
		if (pCurTunnel->vecDeviceDown[i].Check && 
			pCurTunnel->vecDeviceDown[i].bcStatus == ENUM_DEVICE_STATUS_OK)
		{

			g_MSG.SendOpenBroadcast(pCurTunnel->vecDeviceDown[i].sDB.extAddr, pCurTunnel->vecDeviceDown[i].sDB.ip, g_Config.m_SystemConfig.portDevice);
			g_Play.SetIP(pCurTunnel->vecDeviceDown[i].sDB.ip);
			g_Play.SetBroadType(ENUM_BROAD_SINGLE);
			g_Play.SetModel(ENUM_MODEL_FILE);
			char pFile[MAX_PATH] = {0};
			WideCharToMultiByte(CP_ACP, 0, m_strPath, -1, pFile, MAX_PATH, NULL, NULL);
			//g_Play.play(pFile);
			AddMsgRecord(pCurTunnel->vecDeviceDown[i], ENUM_MODEL_FILE);
			pCurTunnel->vecDeviceDown[i].bcStatus = ENUM_DEVICE_STATUS_RUN;
		}
	}
	
	m_pAPP->StartTimerPlay();
	m_pAPP->Status2Button();
	g_RunPlay = true;
	s_CurSel = m_nSel;
	
	AutoStartStopStatus();
}


void CDlgSend::OnBnClickedBtnMic()
{
	// TODO: Add your control notification handler code here
	// mic 
#if 0
	m_msg1.CreateSocket("192.168.1.106", 65276);
	m_msg1.SendTempGroup();

	m_msg.CreateSocket("192.168.1.200", 65276);
	m_msg.SendOpenMic();

	m_pAPP->m_ComMsg.CreateSocket("192.168.1.106", 6006);
	BYTE sub = 2;
	m_pAPP->m_ComMsg.SendOpenBroadcast(sub);
#endif

// 	m_Play.SetBroadType(ENUM_BROAD_GROUP);
//  	m_Play.SetModel(ENUM_MODEL_MIC);	
//  	m_Play.play("");

	int nNum = 0;
	m_pTunnelInfo = g_data.GetData(nNum);
	if (m_curIdx < 0 || m_curIdx >= nNum)
	{
		return;
	}


	STunnel* pCurTunnel = m_pTunnelInfo + m_curIdx;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
	{
		if (pCurTunnel->vecDeviceUp[i].Check && 
			pCurTunnel->vecDeviceUp[i].bcStatus == ENUM_DEVICE_STATUS_OK)
		{
// 			CDecodeMsg msg;
// 			msg.CreateSocket(pCurTunnel->vecDeviceUp[i].sDB.ip, m_pAPP->m_SystemConfig.portDevice);
// 			msg.SendOpenBroadcast(pCurTunnel->vecDeviceUp[i].sDB.extAddr);
			g_MSG.SendOpenBroadcast(pCurTunnel->vecDeviceUp[i].sDB.extAddr, pCurTunnel->vecDeviceUp[i].sDB.ip, g_Config.m_SystemConfig.portDevice);
// 			CDecodeMsg msg1;
// 			msg1.CreateSocket(pCurTunnel->vecDeviceUp[i].sDB.ip, m_pAPP->m_SystemConfig.portAudio);
// 			msg1.SendTempGroup();
			g_MSG.SendTempGroup(pCurTunnel->vecDeviceUp[i].sDB.ip, g_Config.m_SystemConfig.portAudio);
// 			char pFile[MAX_PATH] = {0};
// 			WideCharToMultiByte(CP_ACP, 0, m_strPath, -1, pFile, MAX_PATH, NULL, NULL);
// 			m_pAPP->m_Play.play(pFile);
			AddMsgRecord(pCurTunnel->vecDeviceUp[i], ENUM_MODEL_MIC);
			pCurTunnel->vecDeviceUp[i].bcStatus = ENUM_DEVICE_STATUS_RUN;
		}
	}
	for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
	{
		if (pCurTunnel->vecDeviceDown[i].Check && 
			pCurTunnel->vecDeviceDown[i].bcStatus == ENUM_DEVICE_STATUS_OK)
		{
// 			CDecodeMsg msg;
// 			msg.CreateSocket(pCurTunnel->vecDeviceDown[i].sDB.ip, m_pAPP->m_SystemConfig.portDevice);
// 			msg.SendOpenBroadcast(pCurTunnel->vecDeviceDown[i].sDB.extAddr);
			g_MSG.SendOpenBroadcast(pCurTunnel->vecDeviceDown[i].sDB.extAddr, pCurTunnel->vecDeviceDown[i].sDB.ip, g_Config.m_SystemConfig.portDevice);
// 			CDecodeMsg msg1;
// 			msg1.CreateSocket(pCurTunnel->vecDeviceDown[i].sDB.ip, m_pAPP->m_SystemConfig.portAudio);
// 			msg1.SendTempGroup();
			g_MSG.SendTempGroup(pCurTunnel->vecDeviceDown[i].sDB.ip, g_Config.m_SystemConfig.portAudio);
// 			char pFile[MAX_PATH] = {0};
// 			WideCharToMultiByte(CP_ACP, 0, m_strPath, -1, pFile, MAX_PATH, NULL, NULL);
// 			m_pAPP->m_Play.play(pFile);
			AddMsgRecord(pCurTunnel->vecDeviceDown[i], ENUM_MODEL_MIC);
			pCurTunnel->vecDeviceDown[i].bcStatus = ENUM_DEVICE_STATUS_RUN;
		}
	}

	// mic 广播
	//m_pAPP->m_MicMsg.SendOpenMic();
// 	CString strIP =  m_pAPP->m_SystemConfig.IP_MIC;
 	//char ip[32] = {0};
// 	WideCharToMultiByte(CP_ACP, 0, strIP, -1, ip, 32, NULL, NULL);
	//TcharToChar(m_pAPP->m_SystemConfig.IP_MIC, ip);
	USHORT portAudio = g_Config.m_SystemConfig.portAudio;
	g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_MIC, portAudio);
	//m_pAPP->StartTimerPlay();
	m_pAPP->Status2Button();
	g_RunPlay = true;
	s_CurSel = m_nSel;
	AutoStartStopStatus();
}


BOOL CDlgSend::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	
	SetWindowText(L"广播");
	m_cbSRC.AddString(L"文件");
	m_cbSRC.AddString(L"语音合成");
	m_cbSRC.AddString(L"寻呼话筒");
	m_cbSRC.SetCurSel(s_CurSel);
	m_nSel = s_CurSel;
// 	SetFile(true);
// 	SetMIC(false);
// 	SetStream(false);
	AutoStartStopStatus();
	UpdateData(FALSE);
	//m_tts.Init();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSend::OnBnClickedBtnPath()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		NULL, 
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("MP3 Files (*.mp3)|*.mp3|MP2 Files (*.mp2)|*.mp2|WAV Files (*.wav)|*.wav|WMA Files (*.wma)|*.wma||"),
		NULL);
	if(dlg.DoModal()==IDOK)
	{
		s_strPath = m_strPath = dlg.GetPathName(); //文件名保存在了FilePathName里
		UpdateData(FALSE);
	}
	
}


void CDlgSend::OnSelchangeComboxSrc()
{
	// TODO: Add your control notification handler code here
	
	// 获取组合框控件的列表框中选中项的索引   
	m_nSel = m_cbSRC.GetCurSel();
// 	if (m_nSel == ENUM_MODEL_FILE)
// 	{
// 		SetFile(true);
// 		SetMIC(false);
// 		SetStream(false);
// 	}
// 	else if (m_nSel == ENUM_MODEL_STREAM)
// 	{
// 		SetFile(false);
// 		SetMIC(false);
// 		SetStream(true);
// 	}
// 	else{
// 		SetFile(false);
// 		SetMIC(true);
// 		SetStream(false);
// 	}
	AutoStartStopStatus();
	UpdateData(FALSE);
}
void CDlgSend::SetFile(bool flag)
{
	((CButton*)GetDlgItem(IDC_BTN_FILE))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_BTN_FILE_STOP))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_BTN_PATH))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_EDIT_PATH))->EnableWindow(flag);
	
	
}
void CDlgSend::SetStream(bool flag)
{
	((CButton*)GetDlgItem(IDC_BTN_FILE2))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_BTN_FILE_STOP2))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_BTN_PATH2))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_EDIT_PATH2))->EnableWindow(flag);
	
}
void CDlgSend::SetMIC(bool flag)
{
	((CButton*)GetDlgItem(IDC_BTN_MIC))->EnableWindow(flag);
	((CButton*)GetDlgItem(IDC_BTN_MIC_CLOSE))->EnableWindow(flag);
	
}

void CDlgSend::SetBroadcastType(enBroadcastType enType)
{
	g_Play.SetBroadType(enType);
}


void CDlgSend::OnBnClickedBtnMicClose()
{
	// TODO: Add your control notification handler code here
#if 0
	m_msg.SendCloseMic();
	BYTE sub = 2;
	m_pAPP->m_ComMsg.SendCloseBroadcast(sub);
#endif
	ClosePlay();
	m_pAPP->Status2Button();
	//m_pAPP->m_MSG.SendCloseMic(g_Config.m_SystemConfig.IP_MIC, g_Config.m_SystemConfig.portAudio);
	g_RunPlay = false;
	//m_pAPP->KillTimerPlay();
	AutoStartStopStatus();
	m_pAPP->CancleCheck();
	CDlgSend::OnCancel();
	
}
void CDlgSend::SetCurTunnelIdx(int curIdx)
{
	m_curIdx = curIdx;
}

void CDlgSend::OnBnClickedBtnFileStop2()
{
	// TODO: Add your control notification handler code here
	//ClosePlay();
	m_pAPP->KillTimerPlay();
	//m_pAPP->Status2Button();
	g_RunPlay = false;
	AutoStartStopStatus();
	//m_pAPP->CancleCheck();
	CDlgSend::OnCancel();
}


void CDlgSend::OnBnClickedBtnFile2()
{
	// TODO: Add your control notification handler code here
	// tts
	int nNum = 0;
	m_pTunnelInfo = g_data.GetData(nNum);
	if (m_curIdx < 0 || m_curIdx >= nNum)
	{
		return;
	}
	char szWave[MAX_PATH] = {0};
	WideCharToMultiByte(CP_ACP, 0, g_ttsFilePath, -1, szWave, MAX_PATH, NULL, NULL);
	// txt to buf

	HANDLE hFile = CreateFile(m_strStreamPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	DWORD dwSize = GetFileSize(hFile, NULL);
	char* pBuf = (char*)malloc((dwSize + 1) * sizeof(char));
	memset(pBuf, 0, (dwSize + 1) * sizeof(char));
	wchar_t* pwBuf = (wchar_t*)malloc((dwSize + 1) * sizeof(wchar_t));
	memset(pwBuf, 0, (dwSize + 1) * sizeof(wchar_t));
	DWORD dwRead;
	ReadFile(hFile, pBuf, dwSize, &dwRead, NULL);
	MultiByteToWideChar(CP_ACP, 0, pBuf, dwRead, pwBuf, (dwSize + 1));
	// tts
	

	//strncpy(szWave, m_pAPP->m_strAPPPath, strlen(m_pAPP->m_strAPPPath));
	

	wchar_t strWave[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, szWave, -1, strWave, MAX_PATH);


	CVoiceTTS tts;
	tts.Init();
	tts.TTS(strWave, pwBuf);
	tts.UnInit();
	// free
	if (pBuf != NULL)
	{
		free(pBuf);
		pBuf = NULL;
	}
	if (pwBuf != NULL)
	{
		free(pwBuf);
		pwBuf = NULL;
	}
	CloseHandle(hFile);
	
	//send
	STunnel* pCurTunnel = m_pTunnelInfo + m_curIdx;
	

	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
	{
		if (pCurTunnel->vecDeviceUp[i].Check && 
			pCurTunnel->vecDeviceUp[i].bcStatus == ENUM_DEVICE_STATUS_OK)
		{
// 			CDecodeMsg msg;
// 			msg.CreateSocket(pCurTunnel->vecDeviceUp[i].sDB.ip, m_pAPP->m_SystemConfig.portDevice);
// 			msg.SendOpenBroadcast(pCurTunnel->vecDeviceUp[i].sDB.extAddr);
			g_MSG.SendOpenBroadcast(pCurTunnel->vecDeviceUp[i].sDB.extAddr, pCurTunnel->vecDeviceUp[i].sDB.ip, g_Config.m_SystemConfig.portDevice);
			g_Play.SetIP(pCurTunnel->vecDeviceUp[i].sDB.ip);
			g_Play.SetBroadType(ENUM_BROAD_SINGLE);
			g_Play.SetModel(ENUM_MODEL_FILE);
			
			//g_Play.play(szWave);
			AddMsgRecord(pCurTunnel->vecDeviceUp[i], ENUM_MODEL_STREAM);
			pCurTunnel->vecDeviceUp[i].bcStatus = ENUM_DEVICE_STATUS_RUN;
			
		}
	}
	for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
	{
		if (pCurTunnel->vecDeviceDown[i].Check &&
			pCurTunnel->vecDeviceDown[i].bcStatus == ENUM_DEVICE_STATUS_OK)
		{
// 			CDecodeMsg msg;
// 			msg.CreateSocket(pCurTunnel->vecDeviceDown[i].sDB.ip, m_pAPP->m_SystemConfig.portDevice);
// 			msg.SendOpenBroadcast(pCurTunnel->vecDeviceDown[i].sDB.extAddr);
			g_MSG.SendOpenBroadcast(pCurTunnel->vecDeviceDown[i].sDB.extAddr, pCurTunnel->vecDeviceDown[i].sDB.ip, g_Config.m_SystemConfig.portDevice);
			g_Play.SetIP(pCurTunnel->vecDeviceDown[i].sDB.ip);
			g_Play.SetBroadType(ENUM_BROAD_SINGLE);
			g_Play.SetModel(ENUM_MODEL_FILE);
			
			//g_Play.play(szWave);
			AddMsgRecord(pCurTunnel->vecDeviceDown[i], ENUM_MODEL_STREAM);
			pCurTunnel->vecDeviceDown[i].bcStatus = ENUM_DEVICE_STATUS_RUN;
		}
	}

	m_pAPP->StartTimerPlay();
	m_pAPP->Status2Button();
	g_RunPlay = true;
	s_CurSel = m_nSel;
	AutoStartStopStatus();
}
void CDlgSend::AddMsgRecord(SDevice& stDevice, enPlaySrc model)
{
	SRecord stRecord;
	CharToTchar(stDevice.sDB.ip, stRecord.IP);
	CharToTchar(stDevice.sDB.tunnelName, stRecord.tunnel);
	CharToTchar(stDevice.sDB.kmID, stRecord.kmid);
	CharToTchar(stDevice.sDB.extNo, stRecord.sip);
	CharToTchar(stDevice.sDB.Name, stRecord.noid);//??????????
	stRecord.addr = stDevice.sDB.extAddr;
	memset(stRecord.desc, 0, 32 * sizeof(TCHAR));
	switch(model){
	case ENUM_MODEL_FILE:
		CharToTchar("本地文件",stRecord.desc);
		break;
	case ENUM_MODEL_MIC:
		CharToTchar("网络寻呼话筒",stRecord.desc);
		break;
	case ENUM_MODEL_STREAM:
		CharToTchar("语音合成",stRecord.desc);
		break;
	case ENUM_MODEL_AUX:
		CharToTchar("外部设备",stRecord.desc);
		break;
	case ENUM_MODEL_FILE_LIST:
		CharToTchar("列表文件",stRecord.desc);
		break;
	}
	g_data.InsertMsg(stRecord);
}

void CDlgSend::OnBnClickedBtnPath2()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		NULL, 
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("txt Files (*.txt)|*.txt|"),
		NULL);
	if(dlg.DoModal()==IDOK)
	{
		s_strStreamPath = m_strStreamPath = dlg.GetPathName(); //文件名保存在了FilePathName里
		UpdateData(FALSE);
	}
}


void CDlgSend::OnBnClickedBtnFileStop()
{
	// TODO: Add your control notification handler code here
	//ClosePlay();
	m_pAPP->KillTimerPlay();
	//m_pAPP->Status2Button();
	g_RunPlay = false;
	AutoStartStopStatus();
	//m_pAPP->CancleCheck();
	CDlgSend::OnCancel();
}
void CDlgSend::ClosePlay()
{
	if (m_nSel == ENUM_MODEL_MIC)
	{
		// mic 广播	
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendCloseMic(g_Config.m_SystemConfig.IP_MIC, portAudio);
		
	}

	int nNum = 0;
	m_pTunnelInfo = g_data.GetData(nNum);
	if (m_curIdx < 0 || m_curIdx >= nNum)
	{
		return;
	}


	STunnel* pCurTunnel = m_pTunnelInfo + m_curIdx;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
	{
		if (pCurTunnel->vecDeviceUp[i].bcStatus == ENUM_DEVICE_STATUS_RUN)
		{

			g_MSG.SendCloseBroadcast(pCurTunnel->vecDeviceUp[i].sDB.extAddr, pCurTunnel->vecDeviceUp[i].sDB.ip, g_Config.m_SystemConfig.portDevice);

			pCurTunnel->vecDeviceUp[i].bcStatus = ENUM_DEVICE_STATUS_OK;
		}
	}
	for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
	{
		if (pCurTunnel->vecDeviceDown[i].bcStatus == ENUM_DEVICE_STATUS_RUN)
		{
			g_MSG.SendCloseBroadcast(pCurTunnel->vecDeviceDown[i].sDB.extAddr, pCurTunnel->vecDeviceDown[i].sDB.ip, g_Config.m_SystemConfig.portDevice);

			pCurTunnel->vecDeviceDown[i].bcStatus = ENUM_DEVICE_STATUS_OK;
		}
	}
	
	g_RunPlay = false;

	
}
void CDlgSend::AutoStartStopStatus()
{
	bool bRun = g_RunPlay;
	if (bRun)
	{
		SetStream(false);
		SetMIC(false);
		SetFile(false);

		if (s_CurSel == ENUM_MODEL_FILE)
		{
			((CButton*)GetDlgItem(IDC_BTN_FILE))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP))->EnableWindow(true);
			
		}
		else if (s_CurSel == ENUM_MODEL_MIC)
		{
			((CButton*)GetDlgItem(IDC_BTN_MIC))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_BTN_MIC_CLOSE))->EnableWindow(true);

		}
		else{
			((CButton*)GetDlgItem(IDC_BTN_FILE2))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP2))->EnableWindow(true);
		}
	}
	else{
		if (m_nSel == ENUM_MODEL_FILE)
		{
			SetStream(false);
			SetMIC(false);
			SetFile(true);
			((CButton*)GetDlgItem(IDC_BTN_FILE))->EnableWindow(true);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP))->EnableWindow(false);

		}
		else if (m_nSel == ENUM_MODEL_MIC)
		{
			SetStream(false);
			SetMIC(true);
			SetFile(false);
			((CButton*)GetDlgItem(IDC_BTN_MIC))->EnableWindow(true);
			((CButton*)GetDlgItem(IDC_BTN_MIC_CLOSE))->EnableWindow(false);

		}
		else{
			SetStream(true);
			SetMIC(false);
			SetFile(false);
			((CButton*)GetDlgItem(IDC_BTN_FILE2))->EnableWindow(true);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP2))->EnableWindow(false);
		}
	}
#if 0
	if (s_CurSel == ENUM_MODEL_FILE)
	{
		if (bRun)
		{
			((CButton*)GetDlgItem(IDC_BTN_FILE))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP))->EnableWindow(true);
		}
		else{
			((CButton*)GetDlgItem(IDC_BTN_FILE))->EnableWindow(true);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP))->EnableWindow(false);
		}
	}
	else if (s_CurSel == ENUM_MODEL_MIC)
	{
		if (bRun)
		{
			((CButton*)GetDlgItem(IDC_BTN_MIC))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_BTN_MIC_CLOSE))->EnableWindow(true);
		}
		else{
			((CButton*)GetDlgItem(IDC_BTN_MIC))->EnableWindow(true);
			((CButton*)GetDlgItem(IDC_BTN_MIC_CLOSE))->EnableWindow(false);
		}
	}
	else{
		if (bRun)
		{
			((CButton*)GetDlgItem(IDC_BTN_FILE2))->EnableWindow(false);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP2))->EnableWindow(true);
		}
		else{
			((CButton*)GetDlgItem(IDC_BTN_FILE2))->EnableWindow(true);
			((CButton*)GetDlgItem(IDC_BTN_FILE_STOP2))->EnableWindow(false);
		}
	}
#endif
// 	((CButton*)GetDlgItem(IDC_BTN_FILE))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_BTN_FILE_STOP))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_BTN_PATH))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_EDIT_PATH))->EnableWindow(flag);
// 
// 
// 	((CButton*)GetDlgItem(IDC_BTN_FILE2))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_BTN_FILE_STOP2))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_BTN_PATH2))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_EDIT_PATH2))->EnableWindow(flag);
// 
// 	((CButton*)GetDlgItem(IDC_BTN_MIC))->EnableWindow(flag);
// 	((CButton*)GetDlgItem(IDC_BTN_MIC_CLOSE))->EnableWindow(flag);

}

void CDlgSend::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	
}
