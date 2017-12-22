// SIPCallDlg.cpp : implementation file

#include "stdafx.h"
//#include "vld.h"
#include "SIPCall.h"
#include "SIPCallDlg.h"
#include "VoiceTTS.h"
#include "DlgSet.h"
#include "DlgRecord.h"
//#include "DlgSend.h"
#include "DlgDB.h"
#include "tinyxml.h"
#include "DlgLogin.h"
#include "ZBase64.h"
#include "DlgBC.h"
#include "DlgUser.h"
#include "DlgPwdCheck.h"
#include"tlhelp32.h"

#include "LayoutConfig.h"
#include "DlgMonitor.h"
#include "DlgPopCall.h"
#if 0
#include "pcap.h"
#pragma comment(lib,"wpcap.lib")
#endif

#pragma region GLOBLE
static bool s_bOne = true;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define DEBUG_FILE (0)
#else
#define DEBUG_FILE (0)
#endif

#ifdef _DEBUG
#define DEBUG_MEMORY (0)
#else
#define DEBUG_MEMORY (0)
#endif

#define BEAT_SEND_CLOSE (1)
#define BUTTON_PICTURE_MODEL (1)

#define TIMER_ID_CHECK			(100)		// 心跳包
#define TIMER_ID_UPDATE_STATUS	(101)		// 心跳1分钟后更新状态
#define TIMER_ID_REFLASH		(102)		// 刷新
#define TIMER_ID_PLAY			(103)		// 播放
#define TIMER_ID_CALL			(104)		// 拨打电话
#define TIMER_ID_PC_BEAT		(105)		// PC心跳（用于使用状态更新，有延迟）
#define TIMER_ID_PC_BEAT_FAST	(106)		// PC实时更新心跳
#define TIMER_ID_INFO_SHOW      (107)       // 主呼时定时检测解码板是否回应

#define TIMER_INTER_500		(500)
#define TIMER_INTER_1000	(1000)
#define TIMER_INTER_3000	(3000)
#define TIMER_INTER_5000	(5000)
#define TIMER_INTER_30000	(30*1000)
#define TIMER_INTER_60000	(60*1000)
#define TIMER_INTER_120000	(2*60*1000)
#define TIMER_INTER_300000	(5*60*1000)
#define TIMER_INTER_1800000	(30*60*1000)
#define TIMER_INTER_3000000	(50*60*1000)

#define CALL_TIME (2016)
#define INVALID_TIME_VALUE	(60*1000)

CSIPCallDlg* m_pAPP = NULL;
CString g_ttsFilePath = L"";

CConfig g_Config;
CLog2File g_Log;
CAudioPlay g_Play;
CDataM g_data;
CDecodeMsg g_MSG;
enAdminType g_dataType;
bool g_RunPlay = false;
CString g_szAPPPath  = L"";

char* g_dwLocalIp = NULL;
CMgrUser g_User;
CMgrPlay g_MgrPlay;
// CAboutDlg dialog used for App About
bool g_bOpenMic = false;
bool g_bOpenAux = false;
bool g_bOpenAPP = true;				// 正常打开程序，非重启打开
bool g_bExitApp = false;

bool g_bExitSipgw = false;         // sip代理网管是否关闭
bool g_bExitAudiogw = false;
bool g_bExitCallgw = false;
HANDLE g_hSipgwExitEvent = NULL;
HANDLE g_hCallRecordAgentEvent = NULL;
HANDLE g_hAudioRecordAgentEvent = NULL;
HANDLE g_hSipgwPacketEvent = NULL;
static bool s_ExitOpt = false;		// 强制重启操作
static bool s_bManulBeat = false;	// 手动巡检开关
static bool s_bBeatStop = false;	// 停止巡检
static bool s_SipgwSend = false;
static bool s_OmSend = false;
static int s_UsingNetwork = -1;
static CString s_strSoftVer = L"1.0.103";
static CString s_strSoftDate = L"2017/11/07";
static int s_bFirst = 0;  //消息机制s_bFirst = (int)wParam
static DWORD s_callEvtTime = 0;	//最新电话事件的产生时间
static DWORD s_bcEvtTime = 0;		//最新广播事件的产生时间
static int s_pic_bk_x = 0;
static int s_pic_bk_y = 0;
static int s_pic_bk_w = 0;
static int s_pic_bk_h = 0;
#pragma endregion GLOBLE
#if 0

UINT CheckUsingNetworkProc(LPVOID lp);
UINT CheckSipgwUdpPacketProc(LPVOID lp);
#endif
#pragma region ABOUTGLG
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	CString m_strSoftVer;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	 m_strSoftVer = L"软件版本：" + s_strSoftVer; 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_SOFT_VER, m_strSoftVer);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()
#pragma endregion ABOUTGLG

// CSIPCallDlg dialog
CSIPCallDlg::CSIPCallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSIPCallDlg::IDD, pParent)
{
	//_CrtSetBreakAlloc(1035); 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_nPageTunnel = 0;
	m_nPageDevice = 0;

	m_pTunnelInfo = NULL;	  //CK注释，隧道结构体初始化NULL!!!
	m_nTunnelNum = 0;
	m_nCurTunnelIdx = 0;

	m_bCall = false;
	m_bPause = false;
	g_Logic.m_UpdateDB = false;
	g_Logic.m_SendBC = false;
	g_Log.clearAll();
	g_Log.addConfig(LOG_TYPE, "log_debug.txt");
	g_Log.addConfig(LOG_TYPE_8, "log_msg.txt");
	//g_Log.addConfig(LOG_TYPE_HTTP, "log_http.txt");
	//g_Log.output(LOG_TYPE, "\r\n");
	m_pGroupCall = NULL;
	m_nGroupCall = 0;

	m_pIPGroupFile = NULL;
	m_nIPGroupFile = 0;

	m_pBCYing = NULL;
	m_nBCYing = 0;

	m_vecCallEvent.reserve(300);
	m_vecCallEvt.reserve(300);
	m_vecNoBCData.reserve(50);
	m_vecNoCallData.reserve(50);
	m_vecBeatData.reserve(300);
	m_vecOutData.reserve(50);
	m_vecStausData.reserve(300);
	m_vecLineData.reserve(50);
	m_vecDeciveCMD.reserve(10);
}

void CSIPCallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CALL2, m_PictureB);
	DDX_Control(pDX, IDC_STATIC_CALL1, m_PictureA);
	DDX_Control(pDX, IDC_BUTTON1, m_pBtnTunnel[0]);
	DDX_Control(pDX, IDC_BUTTON2, m_pBtnTunnel[1]);
	DDX_Control(pDX, IDC_BUTTON3, m_pBtnTunnel[2]);
	DDX_Control(pDX, IDC_BUTTON4, m_pBtnTunnel[3]);
	DDX_Control(pDX, IDC_BUTTON5, m_pBtnTunnel[4]);
	DDX_Control(pDX, IDC_BUTTON6, m_pBtnTunnel[5]);
	DDX_Control(pDX, IDC_BUTTON7, m_pBtnTunnel[6]);
	DDX_Control(pDX, IDC_BUTTON8, m_pBtnTunnel[7]);
	DDX_Control(pDX, IDC_BUTTON9, m_pBtnTunnel[8]);
	DDX_Control(pDX, IDC_BUTTON10, m_pBtnTunnel[9]);


	DDX_Control(pDX, IDC_PIC_PE_1, m_pIMG_PE_U[0]);
	DDX_Control(pDX, IDC_PIC_PE_2, m_pIMG_PE_U[1]);
	DDX_Control(pDX, IDC_PIC_PE_3, m_pIMG_PE_U[2]);
	DDX_Control(pDX, IDC_PIC_PE_4, m_pIMG_PE_U[3]);
	DDX_Control(pDX, IDC_PIC_PE_5, m_pIMG_PE_U[4]);
	DDX_Control(pDX, IDC_PIC_PE_6, m_pIMG_PE_U[5]);
	DDX_Control(pDX, IDC_PIC_PE_7, m_pIMG_PE_U[6]);
	DDX_Control(pDX, IDC_PIC_PE_8, m_pIMG_PE_U[7]);
	DDX_Control(pDX, IDC_PIC_PE_9, m_pIMG_PE_U[8]);
	DDX_Control(pDX, IDC_PIC_PE_10, m_pIMG_PE_U[9]);
	DDX_Control(pDX, IDC_PIC_PE_11, m_pIMG_PE_U[10]);
	DDX_Control(pDX, IDC_PIC_PE_12, m_pIMG_PE_U[11]);
	DDX_Control(pDX, IDC_PIC_PE_13, m_pIMG_PE_U[12]);
	DDX_Control(pDX, IDC_PIC_PE_14, m_pIMG_PE_U[13]);
	DDX_Control(pDX, IDC_PIC_PE_15, m_pIMG_PE_U[14]);
	DDX_Control(pDX, IDC_PIC_PE_16, m_pIMG_PE_U[15]);
	DDX_Control(pDX, IDC_PIC_PE_17, m_pIMG_PE_U[16]);
	DDX_Control(pDX, IDC_PIC_PE_18, m_pIMG_PE_U[17]);
	DDX_Control(pDX, IDC_PIC_PE_19, m_pIMG_PE_U[18]);
	DDX_Control(pDX, IDC_PIC_PE_20, m_pIMG_PE_U[19]);
	
	DDX_Control(pDX, IDC_PIC_PE_51, m_pIMG_PE_D[0]);
	DDX_Control(pDX, IDC_PIC_PE_52, m_pIMG_PE_D[1]);
	DDX_Control(pDX, IDC_PIC_PE_53, m_pIMG_PE_D[2]);
	DDX_Control(pDX, IDC_PIC_PE_54, m_pIMG_PE_D[3]);
	DDX_Control(pDX, IDC_PIC_PE_55, m_pIMG_PE_D[4]);
	DDX_Control(pDX, IDC_PIC_PE_56, m_pIMG_PE_D[5]);
	DDX_Control(pDX, IDC_PIC_PE_57, m_pIMG_PE_D[6]);
	DDX_Control(pDX, IDC_PIC_PE_58, m_pIMG_PE_D[7]);
	DDX_Control(pDX, IDC_PIC_PE_59, m_pIMG_PE_D[8]);
	DDX_Control(pDX, IDC_PIC_PE_60, m_pIMG_PE_D[9]);
	DDX_Control(pDX, IDC_PIC_PE_61, m_pIMG_PE_D[10]);
	DDX_Control(pDX, IDC_PIC_PE_62, m_pIMG_PE_D[11]);
	DDX_Control(pDX, IDC_PIC_PE_63, m_pIMG_PE_D[12]);
	DDX_Control(pDX, IDC_PIC_PE_64, m_pIMG_PE_D[13]);
	DDX_Control(pDX, IDC_PIC_PE_65, m_pIMG_PE_D[14]);
	DDX_Control(pDX, IDC_PIC_PE_66, m_pIMG_PE_D[15]);
	DDX_Control(pDX, IDC_PIC_PE_67, m_pIMG_PE_D[16]);
	DDX_Control(pDX, IDC_PIC_PE_68, m_pIMG_PE_D[17]);
	DDX_Control(pDX, IDC_PIC_PE_69, m_pIMG_PE_D[18]);
	DDX_Control(pDX, IDC_PIC_PE_70, m_pIMG_PE_D[19]);

	DDX_Control(pDX, IDC_PIC_ARROW_LEFT, m_PicArrowL);
	DDX_Control(pDX, IDC_PIC_ARROW_RIGHT, m_PicArrowR);
	DDX_Control(pDX, IDC_STATIC_IMAGE, m_pictureBK);
	DDX_Control(pDX, IDC_STATIC_TUNNEL_TITLE, m_ctrlStaticTunnelTitle);
}

BEGIN_MESSAGE_MAP(CSIPCallDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()

	ON_COMMAND(ID_MENU1_SET, &CSIPCallDlg::OnSet)
	ON_COMMAND(ID_MENU_CALL, &CSIPCallDlg::OnRecordCall)
	ON_COMMAND(ID_MENU_MSG, &CSIPCallDlg::OnRecordMsg)
	ON_COMMAND(ID_MENU_LOG, &CSIPCallDlg::OnRecordLog)
	ON_COMMAND(ID_MENU_DB, &CSIPCallDlg::OnRecordDB)

	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CSIPCallDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_PRE, &CSIPCallDlg::OnBnClickedButtonPre)
	ON_BN_CLICKED(BTN_IMAGE_NEXT, &CSIPCallDlg::OnBnClickedButtonDeviceNext)
	ON_BN_CLICKED(BTN_IMAGE_PRE, &CSIPCallDlg::OnBnClickedButtonDevicePre)
	ON_BN_CLICKED(IDC_BUTTON1, &CSIPCallDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSIPCallDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSIPCallDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CSIPCallDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CSIPCallDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CSIPCallDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CSIPCallDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CSIPCallDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CSIPCallDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CSIPCallDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BTN_SEND_BC, &CSIPCallDlg::OnBnClickedBtnSendBc)
	ON_BN_CLICKED(IDC_CHECK_SEND_BC, &CSIPCallDlg::OnBnClickedCheckSendBc)

	ON_WM_TIMER()
	ON_COMMAND_RANGE(BTN_CREATE_START, BTN_CREATE_END, OnCreateButtonClick)	

	ON_COMMAND(ID_MENU_ABOUT_INFO, &CSIPCallDlg::OnMenuAboutInfo)
	ON_COMMAND(ID_MENU_HELP, &CSIPCallDlg::OnMenuHelp)
	ON_BN_CLICKED(IDC_BTN_CONFIG, &CSIPCallDlg::OnBnClickedBtnConfig)
	ON_BN_CLICKED(IDC_BTN_STOP, &CSIPCallDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CSIPCallDlg::OnBnClickedBtnPlay)
	ON_UPDATE_COMMAND_UI(ID_MENU_USER, &CSIPCallDlg::OnUpdateMenuUser)
	ON_COMMAND(ID_MENU_ONEKEYCLOSE, &CSIPCallDlg::OnMenuOnekeyclose)
	ON_WM_CLOSE()
	ON_COMMAND(ID_MENU_MAN_BEAT, &CSIPCallDlg::OnMenuManBeat)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_MENU_VIDEO, &CSIPCallDlg::OnMenuVideo)
	ON_BN_CLICKED(IDC_BTN_TTS, &CSIPCallDlg::OnBnClickedBtnTts)
	
	ON_COMMAND(ID_MENU_MONITOR, &CSIPCallDlg::OnMenuMonitor)
	ON_COMMAND(ID_MENU_MONITOR_CLOSE, &CSIPCallDlg::OnMenuMonitorClose)
	ON_BN_CLICKED(IDC_CHECK_UP_ALL, &CSIPCallDlg::OnBnClickedCheckUpAll)
	ON_BN_CLICKED(IDC_CHECK_DOWN_ALL, &CSIPCallDlg::OnBnClickedCheckDownAll)
	ON_MESSAGE(RECV_MSG_CALL_REQ,&CSIPCallDlg::OnRecvMsgCallReqFromMasterSystem)
	ON_MESSAGE(RECV_MSG_E1,&CSIPCallDlg::OnRecvMsgE1)
	ON_MESSAGE(RECV_MSG_E2,&CSIPCallDlg::OnRecvMsgE2)
	ON_MESSAGE(RECV_MSG_E3,&CSIPCallDlg::OnRecvMsgE3)
	ON_MESSAGE(RECV_MSG_E4,&CSIPCallDlg::OnRecvMsgE4)
	ON_MESSAGE(RECV_MSG_CALL,&CSIPCallDlg::OnRecvMsgCallFromDevice)
	ON_MESSAGE(RECV_MSG_CALL_OK,&CSIPCallDlg::OnRecvMsgCallOnlineFromOM)
	ON_MESSAGE(RECV_MSG_CALL_OFFLINE,&CSIPCallDlg::OnRecvMsgCallOfflineFromOM)
	ON_MESSAGE(RECV_MSG_CALL_IDLE,&CSIPCallDlg::OnRecvMsgCallIdleFromOM)
	ON_MESSAGE(RECV_MSG_CALL_RING,&CSIPCallDlg::OnRecvMsgCallRingFromOM)
	ON_MESSAGE(RECV_MSG_CALL_RUN,&CSIPCallDlg::OnRecvMsgCallRunningFromOM)
	ON_MESSAGE(RECV_MSG_CALL_HANGUP,&CSIPCallDlg::OnRecvMsgCallHungUpFromOM)
	ON_MESSAGE(RECV_MSG_CALL_NOANSWER,&CSIPCallDlg::OnRecvMsgCallNoAnswerFromOM)
	ON_MESSAGE(RECV_MSG_BC_CLOSE,&CSIPCallDlg::OnRecvMsgBCOption)
	ON_MESSAGE(RECV_MSG_BEAT_OK,&CSIPCallDlg::OnRecvMsgHeartBeatReplyFromDevice)
	ON_MESSAGE(RECV_MSG_DEVICESTATUS_MASTERSYSTEM_REQ,&CSIPCallDlg::OnRecvMsgDeviceStatusReqFromMasterSystem)
	ON_MESSAGE(RECV_MSG_DEVICESTATUS_BRANCHSYSTEM_RESPONSE,&CSIPCallDlg::OnRecvMsgDeviceStatusResponseFromBranchSystem)
	ON_MESSAGE(RECV_MSG_DEVICESTATUS_MASTERSYSTEM_ACK,&CSIPCallDlg::OnRecvMsgDeviceStatusAckFromMasterSystem)
	ON_MESSAGE(RECV_MSG_CALL_STATUS,&CSIPCallDlg::OnRecvMsgCallStatusFromBranch)
	ON_MESSAGE(RECV_MSG_BROADCAST_STATUS,&CSIPCallDlg::OnRecvMsgBroadcastStatusFromBranch)
	ON_MESSAGE(RECV_MSG_BC_REQ,&CSIPCallDlg::OnRecvMsgBroadcastReqFromMaster)
	ON_MESSAGE(NO_E1_ANSWER,&CSIPCallDlg::OnRecvNoE1Answer)
	ON_MESSAGE(NO_E2_ANSWER,&CSIPCallDlg::OnRecvNoE2Answer)
	ON_MESSAGE(NO_E3_ANSWER,&CSIPCallDlg::OnRecvNoE3Answer)
	ON_MESSAGE(NO_E4_ANSWER,&CSIPCallDlg::OnRecvNoE4Answer)
	ON_MESSAGE(NO_RING_EVENT,&CSIPCallDlg::OnRecvNoRingEvt)
	ON_MESSAGE(RECV_BRANCH_OM_STATUS,&CSIPCallDlg::OnRecvBranchOMDown)
	ON_MESSAGE(SIPGW_EXIT,&CSIPCallDlg::OnRecvSipgwExit)
	ON_MESSAGE(AUDIO_RECORD_EXIT,&CSIPCallDlg::OnRecvAudioRecordAgentExit)
	ON_MESSAGE(CALL_RECORD_EXIT,&CSIPCallDlg::OnRecvCallRecordAgentExit)
	ON_COMMAND(ID_OpenAudioRecord, &CSIPCallDlg::OnOpenaudiorecord)
	ON_COMMAND(ID_OpenCallRecord, &CSIPCallDlg::OnOpencallrecord)
	ON_COMMAND(ID_CloseAudioRecord, &CSIPCallDlg::OnCloseaudiorecord)
	ON_COMMAND(ID_CloseCallRecord, &CSIPCallDlg::OnClosecallrecord)
	ON_MESSAGE(RECV_MSG_DEVICE_BEAT_REQ,&CSIPCallDlg::OnRecvMsgDeviceBeatReq)
	ON_MESSAGE(NO_PACKETS,&CSIPCallDlg::OnRecvMsgNoPackets)
	ON_MESSAGE(RECV_MSG_OUTER_BC_OPT_START,&CSIPCallDlg::OnRecvOuterBCStart)
	ON_MESSAGE(RECV_MSG_OUTER_BC_OPT_CLOSE,&CSIPCallDlg::OnRecvOuterBCClose)
END_MESSAGE_MAP()


DWORD GetProcessidFromName(CString processName,CString &processPath)
{
	PROCESSENTRY32 pe;
	DWORD idx = 0;
	HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	BOOL ret=Process32First( hSnapshot,&pe);
	if(!ret){
		return 0;
	}
	while(1)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==FALSE)
			break;
		if(processName.CompareNoCase(pe.szExeFile)==0)
		{
			//id=pe.th32ProcessID;
			//获取当前进程所在路径
// 			mehandle=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe.th32ProcessID);
// 			me.dwSize=sizeof(MODULEENTRY32);
// 			Module32First(mehandle,&me);
// 			processPath=me.szExePath;
			idx++;
		}
	}
	CloseHandle(hSnapshot);
	return idx;

}
UINT CheckSipgwEXE(LPVOID lpParam)
{
	//CSIPCallDlg* SipObj = (CSIPCallDlg*)lpParam;
	g_hSipgwExitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	while (!g_bExitApp)
	{
		CString strPath;
		if(GetProcessidFromName(L"SRSipgw.exe", strPath)==0)
		{
			if(m_pAPP->m_hWnd){
				SendMessage(m_pAPP->m_hWnd,SIPGW_EXIT,0,0);
				Sleep(5000);
			}
		}
		else
		{
			if(m_pAPP->m_hWnd){
				SendMessage(m_pAPP->m_hWnd,SIPGW_EXIT,1,0);
				Sleep(2000);
			}
		}
	}
	SetEvent(g_hSipgwExitEvent); 
	return 1;
}
UINT CheckAudioServerEXE(LPVOID lpParam)
{
	//CSIPCallDlg* SipObj = (CSIPCallDlg*)lpParam;
	g_hAudioRecordAgentEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	while (!g_bExitApp)
	{
		CString strPath;
		if(GetProcessidFromName(L"AudioServer.exe", strPath)==0)
		{
			if(m_pAPP->m_hWnd){
				SendMessage(m_pAPP->m_hWnd,AUDIO_RECORD_EXIT,0,0);
				Sleep(20000);
			}
		}
		else
		{
			if(m_pAPP->m_hWnd){
				SendMessage(m_pAPP->m_hWnd,AUDIO_RECORD_EXIT,1,0);
				Sleep(10000);
			}
		}
	}
	SetEvent(g_hAudioRecordAgentEvent); 
	return 1;
}
UINT CheckCallRecordEXE(LPVOID lpParam)
{
	g_hCallRecordAgentEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	while (!g_bExitApp)
	{
		CString strPath;
		if(GetProcessidFromName(L"pbxrecord.exe", strPath)==0)
		{
			if(m_pAPP->m_hWnd){
				SendMessage(m_pAPP->m_hWnd,CALL_RECORD_EXIT,0,0);
				Sleep(20000);
			}
		}
		else
		{
			if(m_pAPP->m_hWnd){
				SendMessage(m_pAPP->m_hWnd,CALL_RECORD_EXIT,1,0);
				Sleep(10000);
			}
		}
	}
	SetEvent(g_hCallRecordAgentEvent);
	return 1;
}
// CSIPCallDlg message handlers

BOOL CSIPCallDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);		// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon	
// 	SEventInfo evt;
// 	m_http.DecodeEvent("xxx", evt);
	CString strPath;
	if (GetProcessidFromName(L"SIPCall.exe", strPath) > 1)
	{
		MessageBox(L"软件正在运行，请关闭后再打开！", L"提醒");
		//SendMessage(WM_CLOSE);
		//return FALSE;
		exit(1);
	}
	if (GetProcessidFromName(L"SRSipgw.exe", strPath) == 0)
	{
		MessageBox(L"缺少代理网关 SRSipgw.exe, 请检查！", L"提醒");
		//SendMessage(WM_CLOSE);
		//return FALSE;
		exit(1);
	}
	// TODO: Add extra initialization here
	//使关闭按钮无效
	//pSysMenu->EnableMenuItem(SC_CLOSE,MF_DISABLED);
	BtnInit();
	s_ExitOpt = false;

	m_pAPP = this;		 
	InitAPPPath();	
	
	if (IsReboot())
	{
		g_bOpenAPP = false;
		ClearReboot();		
	}
	else{
		g_bOpenAPP = true;	
	}
	
	// user
	char szUser[256]= {0};
	WideCharToMultiByte(CP_ACP, 0, g_szAPPPath + L"usr.dat", -1, szUser, 256, NULL, NULL);
	g_User.ReadUser(szUser);
	g_MSG.ReadIPData();
	ReadNoBCFile();
	ReadNoCallFile();
	ReadDeviceLineFile();
	ReadGroupFile();
	ReadIPGroupFile();
	ReadDevCmdFile();
	CDlgLogin login;
	login.DoModal(); //弹出登录窗口13
	g_dataType = login.GetType();
	//g_dataType = ENUM_TYPE_SUPER;
	if (g_dataType == ENUM_TYPE_NONE){
		//exit(1);	
		SendMessage(WM_CLOSE);
	}	
	else{
		//m_hbrush=CreateSolidBrush(RGB(0,0,0));
		memset(m_szLoginName, 0, MAX_PATH * sizeof(char));
		WideCharToMultiByte(CP_ACP, 0, login.GetLoginName(), -1, m_szLoginName, MAX_PATH, NULL, NULL);
		m_MgrPic.LoadPic();
		m_editFont1.CreatePointFont(70, L"宋体");
		m_editFont_title.CreatePointFont(200, L"微软雅黑");
		g_Config.ReadConfig();
		//Logic
		InitSocket();  //初始化创建3Socket
		ReadDB();

		// tip start
		// 按钮提示
		m_Mytip.Create(this);  	
		m_Mytip.SetDelayTime(200); //设置延迟
		m_Mytip.SetDelayTime(TTDT_AUTOPOP, 8000);
		m_Mytip.SetTipTextColor( RGB(0,0,255) ); //设置提示文本的颜色
		m_Mytip.SetTipBkColor( RGB(255,255,255)); //设置提示框的背景颜色
		m_Mytip.Activate(TRUE); //设置是否启用提示
		// tip end
		
		m_nCurTunnelIdx = 0;
		m_nPageDevice = 0;

		int cx = GetSystemMetrics(SM_CXFULLSCREEN);
		int cy = GetSystemMetrics(SM_CYFULLSCREEN);
		
		CRect rt;
		SystemParametersInfo(SPI_GETWORKAREA,0,&rt,0);
		cy = rt.bottom;
		MoveWindow(0, 0, cx, cy);
		//软件左上角标题格式
		CString strSoftTitle = L"";
		strSoftTitle.Format(L"紧急电话广播系统V%s_%s", s_strSoftVer, s_strSoftDate);
		SetWindowText(strSoftTitle);
		//ShowWindow(SW_SHOWMAXIMIZED/*SW_MAXIMIZE*/);
		//CRect rect;
		//GetClientRect(&rect);
		m_width = cx/*rect.right - rect.left*/;
		m_height = cy/*rect.bottom - rect.top*/;
		
		SetLayout();

		if (m_nTunnelNum == 0)
		{		
			if(g_dataType == ENUM_TYPE_SUPER){
				MessageBox(L"数据库无数据，请先初始化数据！", L"提醒");
				CDlgDB dlg;
				dlg.DoModal();			
			}	
			else{
				MessageBox(L"数据库无数据，请联系管理员初始化数据！", L"提醒");

			}
			SendMessage(WM_CLOSE);
		}

		SendMessage(SEND_MSG_BEAT, 1, 0);	 //Caution 100!!!
		//SetTimer(TIMER_ID_CHECK, TIMER_INTER_300000, 0);
		SetTimer(TIMER_ID_CHECK, TIMER_INTER_1800000, 0);
		SetTimer(TIMER_ID_REFLASH, TIMER_INTER_1000, 0);
		SetTimer(TIMER_ID_CALL, TIMER_INTER_1000, 0);
		SetTimer(TIMER_ID_PLAY, TIMER_INTER_3000, 0);	
		//SetTimer(TIMER_ID_PC_BEAT_FAST, TIMER_INTER_3000, 0);
#if 0
		ReflashCallThread();
#else 
		m_CallA.status = ENUM_DEVICE_STATUS_OK;
		m_CallB.status = ENUM_DEVICE_STATUS_OK;
#endif
		m_http.CreateTCPRecvThread();

		if (g_dataType == ENUM_TYPE_SUPER)
		{
			
		}
		else if (g_dataType == ENUM_TYPE_ADMIN)
		{
			AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(ID_MENU1_SET,MF_GRAYED);
		}
		else if (g_dataType == ENUM_TYPE_USER)
		{
			AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(ID_MENU1_SET,MF_GRAYED);
			AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(ID_MENU_ONEKEYCLOSE, MF_GRAYED);
		}

		if (g_Config.m_SystemConfig.nJT == 0)
		{
			m_Menu.RemoveMenu(ID_MENU_MONITOR, MF_BYCOMMAND);
			m_Menu.RemoveMenu(ID_MENU_MONITOR_CLOSE, MF_BYCOMMAND);
		}
		// 检测版本
		//CheckVersion();
		// 初始化录音
		g_Play.RecordInit();
		AfxBeginThread(CheckSipgwEXE,NULL);
		//AfxBeginThread(CheckUsingNetworkProc,NULL);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CSIPCallDlg::CheckVersion()
{
	g_data.ReadLog(ENUM_SORT_ALL);
	int nLogNum = 0;
	g_data.GetRecordLog(nLogNum);

	SYSTEMTIME tm;
	GetLocalTime(&tm);
	TCHAR windowText[512] = {0};
	GetWindowText(windowText, 512);
	if ((nLogNum > 10000) || (tm.wYear > 2016) || (tm.wYear == 2016 && tm.wMonth < 1))
	{
		MessageBox(L"环境变量为0x000000，需要设置，请关闭！", L"提醒");
		OnResetApp();
	}
}
void CSIPCallDlg::ReadDB()
{
	m_DBLock.Lock();
	g_data.ReadDB();
	m_pTunnelInfo = g_data.GetData(m_nTunnelNum);
	m_DBLock.Unlock();
}
void CSIPCallDlg::UpdateData()
{
	m_DBLock.Lock();
	g_data.DB2Data();
	m_pTunnelInfo = g_data.GetData(m_nTunnelNum);
	m_DBLock.Unlock();

	DB2TunnelBtn();
	CheckTunnelPreNext();

	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void  CSIPCallDlg::getCurDeviceNum(int curTunnelIdx, int& upNum, int& downNum)
{
	upNum = downNum = 0;
	if(m_pTunnelInfo == NULL || m_nTunnelNum == 0){
		return;
	}
	if (curTunnelIdx < 0 || curTunnelIdx >= m_nTunnelNum)
	{
		return;
	}
	downNum = m_pTunnelInfo[curTunnelIdx].vecDeviceDown.size();
	upNum = m_pTunnelInfo[curTunnelIdx].vecDeviceUp.size();
}
void CSIPCallDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CSIPCallDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		
#if 1
		CPaintDC dc(this);
		HBITMAP   m_hBitmap = NULL; 	
		//m_hBitmap   =   ::LoadBitmap(::GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BITMAP2)); 
		m_hBitmap = m_MgrPic.GetBitmap(_T("texture\\bk.bmp"));

		if (m_hBitmap)
		{
			BITMAP bm;
			GetObject( m_hBitmap, sizeof(BITMAP), &bm);

			CDC  dcMem;   //定义一个工具箱（设备上下文）  
			dcMem.CreateCompatibleDC(&dc);///建立关联DC   

			/*CBitmap  *pbmpOld=*/dcMem.SelectObject(m_hBitmap);   //保存原有CDC对象，并选入新CDC对象入DC  
			dc.SetStretchBltMode(STRETCH_HALFTONE/*COLORONCOLOR*/);//防止bmp图片失真  
			dc.StretchBlt(s_pic_bk_x, s_pic_bk_y , s_pic_bk_w, s_pic_bk_h , &dcMem,0,0,  bm.bmWidth,bm.bmHeight,SRCCOPY);  

			wchar_t wName[128] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[m_nCurTunnelIdx].Name, -1, wName, 128);
			
			((CButton*)GetDlgItem(IDC_STATIC_TUNNEL_TITLE))->SetWindowText(wName);

			// （个人建议把,rect.Width(),rect.Height()这两个数据 换成你的图片的大小，前提是图片足够大，这样图片不容易失真。关于图片失真，参考：http://blog.csdn.net/abidepan/article/details/7963929 ）  
			//dcMem.SelectObject(pbmpOld);  
			//::DeleteObject(m_hBitmap);
			
			dcMem.DeleteDC(); 
		}
		
#endif
		RepaintTunnelBtn();
		CDialog::OnPaint();
	}
}
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSIPCallDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CSIPCallDlg::SetLayout()
{
	m_Menu.LoadMenu(IDR_MENU1);  
	SetMenu(&m_Menu);
	
	for (int i = 0; i < BTN_NUM_TUNNEL; i++)
	{
		m_pBtnTunnel[i].SetFlat();
		m_pBtnTunnel[i].SetAlign(CButtonST::ST_ALIGN_OVERLAP);
	}
	
	SetTitleLayout();	//标题布局

	SetTunnelBtnLayout(); //隧道按键布局
	SetDeviceLayout();	  //设备布局
	SetCallLayout();   //电话布局

	DB2TunnelBtn();
	CheckTunnelPreNext();

	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();

	Call2Btn();
}
void CSIPCallDlg::SetTitleLayout()
{
	int padW = 10;
	int padH = 2;
	int XCoord = m_width / 4;
	int YCoord = padH;
	int btnInter = 10;
	int w = m_width / 2;
	
	CStatic* pWnd = (CStatic*)GetDlgItem(IDC_STATIC_TITLE);	
	pWnd->SetWindowPos( NULL, XCoord, YCoord, w, 30, SWP_NOZORDER );

	TCHAR wzTitle[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.title, -1, wzTitle, MAX_PATH);
	SetDlgItemText(IDC_STATIC_TITLE, wzTitle);
	
	pWnd->SetFont(&m_editFont_title);
}
void CSIPCallDlg::SetTunnelNameLayout()
{
	int padW = 10;
	int padH = 5;
	int XCoord = m_width / 4;
	int YCoord = 280;
	int btnInter = 10;
	int w = m_width / 2;

	CStatic* pWnd = (CStatic*)GetDlgItem(IDC_STATIC_TUNNEL_TITLE);	
	pWnd->SetWindowPos( NULL, XCoord, YCoord, w, 40, SWP_NOZORDER );

	TCHAR wzTitle[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.title, -1, wzTitle, MAX_PATH);
	SetDlgItemText(IDC_STATIC_TUNNEL_TITLE, wzTitle);

	pWnd->SetFont(&m_editFont_title);
}
void CSIPCallDlg::SetTunnelBtnLayout()
{	
	//
	int padW = 10;
	int padH = 10;
	int XCoord = padW;
	int YCoord = LAYOUT_TOP_TUNNEL;
	int btnInter = 10;
	int w = m_width - 2 * padW;
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_STATIC_TUNNEL);	
	pWnd->SetWindowPos( NULL, XCoord, YCoord, w, 70, SWP_NOZORDER );
	// PRE
	int btnPreW = 40;
	int btnH = 35;	
	pWnd = GetDlgItem(IDC_BUTTON_PRE);	
	int XCoordPre = XCoord + padW;
	int YCoordPre = YCoord + 20;
	pWnd->SetWindowPos( NULL, XCoordPre,YCoordPre, btnPreW, btnH, SWP_NOZORDER );
	
	// tunnel button
	int btnNum = BTN_NUM_TUNNEL;	
	int btnW  = (m_width - 4 * padW - 2 * btnPreW - (btnNum + 1) * btnInter) / btnNum;
	int XCoordBtn  = XCoordPre +  btnPreW + btnInter;
	int YCoordBtn =  YCoordPre;
	for (int i = 0; i < btnNum; i++)
	{
		pWnd = GetDlgItem(IDC_BUTTON1 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, YCoordBtn, btnW, btnH, SWP_NOZORDER );
	}

	// NEXT
	pWnd = GetDlgItem(IDC_BUTTON_NEXT);	
	pWnd->SetWindowPos( NULL, m_width - 2 * padW - btnPreW, YCoordPre, btnPreW, btnH, SWP_NOZORDER );
	
	//CheckBtnPreNext();
}
void CSIPCallDlg::SetDeviceLayout()
{
	//
	int padW = 10;
	int padH = 10;
	int XCoord = padW;
	int YCoord = LAYOUT_TOP_DEVICE;
	int btnInter = 10;
	int w = m_width - 2 * padW;
	CWnd *pWnd;


	pWnd = GetDlgItem(IDC_STATIC_CUR_TUNNEL);	
	pWnd->SetWindowPos( NULL, XCoord, YCoord, w, TOP_DEVICE_BOTTOM - LAYOUT_TOP_DEVICE, SWP_NOZORDER );

	// PRE
	int XCoordPre = XCoord + padW;
	int YCoordPre = YCoord + 30;
	int btnPreW = 30;
	int btnH = 30;	
	CRect rectPre(XCoordPre, YCoordPre, XCoordPre + btnPreW, YCoordPre + 340);	
	m_pbtnImgPre = NewMyButton(BTN_IMAGE_PRE, rectPre, 0, L"<");
	pWnd = GetDlgItem(BTN_IMAGE_PRE);		
	pWnd->SetWindowPos( NULL, XCoordPre,YCoordPre, btnPreW, 340, SWP_NOZORDER );

	// call button
	int btnNum = BTN_NUM_CALL;	
	int btnW  = (m_width - 4 * padW - 2 * btnPreW - (btnNum + 1) * btnInter) / btnNum;
	// 上电话
	int XCoordBtn  = XCoordPre +  btnPreW + btnInter;
	//int YCoordBtn =  TOP_CALL_UP/*YCoordPre + 40*/;
	for (int i = 0; i < btnNum; i++)
	{
		CRect rectCall(0, 0, 40, 40);	
		m_pbtnCall1[i] = NewMyButton(BTN_IMAGE_BASEID_1_0 + i, rectCall, 0);
		
		pWnd = GetDlgItem(BTN_IMAGE_BASEID_1_0 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_CALL_UP, 40/*btnW*/, 40/*btnH*/, SWP_NOZORDER );
		SetButtonBMP((CButton*)pWnd, _T("texture\\call4_40.bmp"));
		
	}
	// 上广播
	XCoordBtn  = XCoordPre +  btnPreW + btnInter;
	//YCoordBtn =  TOP_BC_UP/*YCoordPre*//* + 50*/;
	for (int i = 0; i < btnNum; i++)
	{
		CRect rectCall1(0, 0, 40, 20);	
		m_pbtnMsg1[i] = NewCheckBox(CHECK_BOX_BASEID_1 + i, rectCall1, 0);
		m_pbtnMsg1[i]->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_BC_UP, 40/*btnW*/, 20/*btnH*/, SWP_NOZORDER );
		SetButtonBMP(m_pbtnMsg1[i], _T("texture\\m4_20.bmp"));
	}

	// 上标签
	XCoordBtn  = XCoordPre +  btnPreW + btnInter;	
	for (int i = 0; i < btnNum; i++)
	{
		CRect rectCall(0, 0, 50, 20);	
		m_pStatic1[i] = NewStatic(BTN_STATIC_BASEID_1 + i, rectCall, 0);

		pWnd = GetDlgItem(BTN_STATIC_BASEID_1 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_NAME_UP, 50, 20, SWP_NOZORDER );

		CRect rectPE(0, 0, 40, 20);			
		pWnd = GetDlgItem(IDC_PIC_PE_1 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_PERSON_UP, 40, 20, SWP_NOZORDER );


		CRect rectCall1(0, 0, 45, 15);	
		m_pStatic3[i] = NewStatic(BTN_STATIC_BASEID_11 + i, rectCall1, 1, L"");
		
		pWnd = GetDlgItem(BTN_STATIC_BASEID_11 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_ERROR_UP, 45/*btnW*/, 15/*btnH*/, SWP_NOZORDER );

		CRect rectCallKM(0, 0, 80, 20);	
		m_pStaticKM1[i] = NewStatic(BTN_STATIC_BASEID_KM1 + i, rectCallKM, 0);

		pWnd = GetDlgItem(BTN_STATIC_BASEID_KM1 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_KM_UP, 80, 20, SWP_NOZORDER );
	}

	// send broadcast
	//GetDlgItem(IDC_BTN_SEND_BC)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.5, YCoordPre + 110, 100, 30, SWP_NOZORDER );
	// send broadcast checkbox
	
	if (true/*g_Config.m_SystemConfig.nMainSystem*/)
	{
		SetDlgItemText(IDC_CHECK_SEND_BC, L"全选广播");
		GetDlgItem(IDC_CHECK_UP_ALL)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.2,TOP_BUTTON_BOTTOM, 60, 40, SWP_NOZORDER );
		GetDlgItem(IDC_CHECK_DOWN_ALL)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.25,TOP_BUTTON_BOTTOM, 60, 40, SWP_NOZORDER );
		GetDlgItem(IDC_CHECK_SEND_BC)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.3,TOP_BUTTON_BOTTOM, 100, 40, SWP_NOZORDER );		
		GetDlgItem(IDC_BTN_PLAY)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.4, TOP_BUTTON_BOTTOM, 100, 40, SWP_NOZORDER );
		GetDlgItem(IDC_BTN_STOP)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.5, TOP_BUTTON_BOTTOM, 100, 40, SWP_NOZORDER );
		GetDlgItem(IDC_BTN_CONFIG)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.6, TOP_BUTTON_BOTTOM, 100, 40, SWP_NOZORDER );
		GetDlgItem(IDC_BTN_TTS)->SetWindowPos( NULL,(m_width - 2 * padW) * 0.7, TOP_BUTTON_BOTTOM, 100, 40, SWP_NOZORDER );
	}
	else{
		GetDlgItem(IDC_CHECK_UP_ALL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_DOWN_ALL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_SEND_BC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_PLAY)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_STOP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_CONFIG)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_TTS)->ShowWindow(SW_HIDE);
	}
	
	//int offsetXia = 90;
	// 下电话
	XCoordBtn  = XCoordPre +  btnPreW + btnInter;
	//YCoordBtn =  YCoordPre + 190 + offsetXia;
	for (int i = 0; i < btnNum; i++)
	{
		CRect rectCall(0, 0, 40, 40);	
		m_pbtnCall2[i] = NewMyButton(BTN_IMAGE_BASEID_2_0 + i, rectCall, 0);

		pWnd = GetDlgItem(BTN_IMAGE_BASEID_2_0 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_CALL_DOWN, 40/*btnW*/, 40/*btnH*/, SWP_NOZORDER );
		SetButtonBMP((CButton*)pWnd, _T("texture\\call4_40.bmp"));
	}
	// 下广播
	XCoordBtn  = XCoordPre +  btnPreW + btnInter;
	//YCoordBtn =  YCoordPre + 150 + offsetXia;
	for (int i = 0; i < btnNum; i++)
	{
		CRect rectCall2(0, 0, 40, 20);	
		m_pbtnMsg2[i] = NewCheckBox(CHECK_BOX_BASEID_2 + i, rectCall2, 0);
		m_pbtnMsg2[i]->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_BC_DOWN, 40/*btnW*/, 20/*btnH*/, SWP_NOZORDER );
		SetButtonBMP(m_pbtnMsg2[i], _T("texture\\m4_20.bmp"));
	}

	// 下标签
	XCoordBtn  = XCoordPre +  btnPreW + btnInter;
	//YCoordBtn =  YCoordPre + 235 + offsetXia;
	for (int i = 0; i < btnNum; i++)
	{
		CRect rectCall(0, 0, 50, 20);	
		m_pStatic2[i] = NewStatic(BTN_STATIC_BASEID_2 + i, rectCall, 0);

		pWnd = GetDlgItem(BTN_STATIC_BASEID_2 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_NAME_DOWN, 50, 20, SWP_NOZORDER );
		
		CRect rectPE(0, 0, 40, 20);			
		pWnd = GetDlgItem(IDC_PIC_PE_51 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_PERSON_DOWN, 40, 20, SWP_NOZORDER );

		CRect rectCall1(0, 0, 45, 15);	
		m_pStatic4[i] = NewStatic(BTN_STATIC_BASEID_21 + i, rectCall1, 1, L"");

		pWnd = GetDlgItem(BTN_STATIC_BASEID_21 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_ERROR_DOWN, 45, 15, SWP_NOZORDER );

		CRect rectCallKM(0, 0, 80, 20);	
		m_pStaticKM2[i] = NewStatic(BTN_STATIC_BASEID_KM2 + i, rectCallKM, 0);

		pWnd = GetDlgItem(BTN_STATIC_BASEID_KM2 + i);	
		pWnd->SetWindowPos( NULL, XCoordBtn + (btnW + btnInter) * i, TOP_KM_DOWN, 80, 20, SWP_NOZORDER );
	}

	// NEXT
	CRect rectNext(m_width - 2 * padW - btnPreW, YCoordPre, m_width - 2 * padW, YCoordPre + 340);	
	m_pbtnImgNext = NewMyButton(BTN_IMAGE_NEXT, rectNext, 0, L">");
	pWnd = GetDlgItem(BTN_IMAGE_NEXT);	
	pWnd->SetWindowPos( NULL, m_width - 2 * padW - btnPreW, YCoordPre, btnPreW, 340, SWP_NOZORDER );

	//CheckBtnImgPreNext();
	// arrow 
	m_PicArrowL.SetWindowPos( NULL, XCoordPre +  btnPreW + btnInter, 280, 178, 80, SWP_NOZORDER );
	HBITMAP hbitmapArrowL = m_MgrPic.GetBitmap(L"texture\\arrowl.bmp");
	if (hbitmapArrowL != NULL)
	{				
		m_PicArrowL.SetBitmap(hbitmapArrowL);
	}
	
	YCoord = 280;
	int picW = 178;
	int picH = 80;
	m_PicArrowR.SetWindowPos( NULL, m_width - 2 * padW - btnPreW - padW - picW, YCoord, padW, picH, SWP_NOZORDER );
	HBITMAP hbitmapArrowR = m_MgrPic.GetBitmap(L"texture\\arrowr.bmp");
	if (hbitmapArrowR != NULL)
	{				
		m_PicArrowR.SetBitmap(hbitmapArrowR);
	}

	// cur tunnel
	XCoord = m_width / 8 * 3 ;
	w = m_width / 4;
	pWnd = GetDlgItem(IDC_STATIC_TUNNEL_TITLE);	
	pWnd->SetWindowPos( NULL, XCoord, YCoord, w, picH, SWP_NOZORDER );

	SetDlgItemText(IDC_STATIC_TUNNEL_TITLE, L"隧道名称");
	pWnd->SetFont(&m_editFont_title);
	//m_ctrlStaticTunnelTitle.SetTransparent(TRUE);
	//pWnd->ShowWindow(SW_HIDE);

	pWnd = GetDlgItem(IDC_STATIC_IMAGE);	

	s_pic_bk_y = TOP_PERSON_UP + 20 + 2;
	s_pic_bk_h = TOP_PERSON_DOWN - s_pic_bk_y - 2;

	CRect left;
	GetDlgItem(BTN_IMAGE_PRE)->GetWindowRect(&left);

	CRect right;
	GetDlgItem(BTN_IMAGE_NEXT)->GetWindowRect(&right);
	s_pic_bk_x = left.right + 30;
	s_pic_bk_w = right.left - 30 - s_pic_bk_x;
	//pWnd->SetWindowPos( NULL, s_pic_bk_x, s_pic_bk_y, s_pic_bk_w, s_pic_bk_h,SWP_NOZORDER );

	// sign
	pWnd = GetDlgItem(IDC_STATIC_SIGN_LEFT);
	pWnd->SetWindowPos( NULL, s_pic_bk_x + s_pic_bk_w * 0.1 /*XCoordPre +  btnPreW + btnInter + padW + picW*/, YCoord, picW, picH, SWP_NOZORDER );
	/*TCHAR wzSignL[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[m_nCurTunnelIdx].SignL, -1, wzSignL, MAX_PATH);*/
	SetDlgItemText(IDC_STATIC_SIGN_LEFT, m_pTunnelInfo[m_nCurTunnelIdx].SignL);	
	pWnd->SetFont(&m_editFont_title);
	//pWnd->ShowWindow(SW_HIDE);
	//s_pic_bk_x = XCoordPre +  btnPreW + btnInter + padW + picW;

	pWnd = GetDlgItem(IDC_STATIC_SIGN_RIGHT);
	pWnd->SetWindowPos( NULL, s_pic_bk_x + s_pic_bk_w * 0.9/*m_width - 2 * padW - btnPreW - 2* (padW + picW)*/, YCoord, picW, picH, SWP_NOZORDER );
	/*TCHAR wzSignR[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[m_nCurTunnelIdx].SignR, -1, wzSignR, MAX_PATH);*/
	SetDlgItemText(IDC_STATIC_SIGN_RIGHT, m_pTunnelInfo[m_nCurTunnelIdx].SignR);
	//pWnd->ShowWindow(SW_HIDE);
	pWnd->SetFont(&m_editFont_title);
	//s_pic_bk_w = m_width - 2 * padW - btnPreW - 2* (padW + picW) + picW - s_pic_bk_x;
}
CFont* SetFont(CString _strFontName, UINT _nFontSize)
{
	int     nCount;
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	//设置字体样式
	nCount  = sizeof(lf.lfFaceName)/sizeof(TCHAR);
	_tcscpy_s(lf.lfFaceName, nCount, _strFontName);
	lf.lfHeight  = _nFontSize;
	lf.lfWeight  = 2;
	lf.lfCharSet = GB2312_CHARSET;

	CFont* pFont = new CFont;
	pFont->CreateFontIndirect(&lf);
	return pFont;
}
void CSIPCallDlg::SetCallLayout()
{
	int padW = 10;
	int padH = 10;
	int XCoord = padW;
	int YCoord = padH + TOP_CALL_LAYOUT;
	int btnInter = 10;
	int w = m_width - 2 * padW;
	CWnd *pWnd;
	//  A
	pWnd = GetDlgItem(IDC_STATIC_ACALL);		
	pWnd->SetWindowPos( NULL, XCoord, YCoord, w / 3 - 20, m_height - 3*padH - YCoord -20, SWP_NOZORDER );

	// 名称
	pWnd = GetDlgItem(IDC_STATIC_NAMA1);
	pWnd->SetWindowPos( NULL, XCoord + w / 6 + 20, YCoord + 40, 150, 20, SWP_NOZORDER );
	// 时间
	pWnd = GetDlgItem(IDC_STATIC_TIME1);
	pWnd->SetWindowPos( NULL, XCoord + w / 6 + 20, YCoord + 60, 150, 20, SWP_NOZORDER );

	// 电话图标
	pWnd = GetDlgItem(IDC_STATIC_CALL1);
	pWnd->SetWindowPos( NULL, XCoord + w / 6 - 80, YCoord + 25, 80, 80, SWP_NOZORDER );

// 	pWnd = GetDlgItem(IDC_BUTTON_STOP1);
// 	pWnd->SetWindowPos( NULL, XCoord + w / 4 - 60, m_height - 120, 100, 40, SWP_NOZORDER );
	//  B
	pWnd = GetDlgItem(IDC_STATIC_BCALL);	
	pWnd->SetWindowPos( NULL, XCoord + w * 2 / 3 + 20 , YCoord, w / 3-20, m_height - 3*padH - YCoord - 20/*250*/, SWP_NOZORDER );


	pWnd = GetDlgItem(IDC_STATIC_NAMA2);
	pWnd->SetWindowPos( NULL, XCoord + w * 5 / 6 + 20, YCoord + 40, 150, 20, SWP_NOZORDER );

	pWnd = GetDlgItem(IDC_STATIC_TIME2);
	pWnd->SetWindowPos( NULL, XCoord + w  * 5 / 6 + 20, YCoord + 60, 150, 20, SWP_NOZORDER );
	//pWnd->ShowWindow(SW_HIDE);

	pWnd = GetDlgItem(IDC_STATIC_CALL2);
	pWnd->SetWindowPos( NULL, XCoord + w  * 5 / 6 - 80, YCoord + 25, 80, 80, SWP_NOZORDER );

// 	pWnd = GetDlgItem(IDC_BUTTON_STOP2);
// 	pWnd->SetWindowPos( NULL, XCoord + w  * 0.75 - 60, m_height - 120, 100, 40, SWP_NOZORDER );

	SetWaitLayout();
}
void CSIPCallDlg::SetWaitLayout()
{
	int padW = 10;
	int padH = 10;
	int XCoord = padW;
	int YCoord = padH + TOP_CALL_LAYOUT;
	int w = m_width - 2 * padW;
	CWnd *pWnd;
	for (int i = 0; i < BTN_NUM_WAIT; i++)
	{
		CRect rectCall(0, 0, 100, 20);	
		m_pStatic5[i] = NewStatic(BTN_STATIC_WAIT_BASEID + i, rectCall, 0);

		pWnd = GetDlgItem(BTN_STATIC_WAIT_BASEID + i);	
		pWnd->SetWindowPos( NULL, w / 3, YCoord + i * 25,  w / 3 - 20, 20, SWP_NOZORDER );
	}

	GetDlgItem(IDC_STATIC_MSG)->SetWindowPos(NULL,w*2/5,YCoord - 15,w*2/5 - 20,20,SWP_NOZORDER);
}
void CSIPCallDlg::ShowInfo(CString str)
{
	//MessageBox(str);
	GetDlgItem(IDC_STATIC_MSG)->ShowWindow(SW_SHOW);
	SetDlgItemText(IDC_STATIC_MSG,str);
	SetTimer(TIMER_ID_INFO_SHOW,TIMER_INTER_5000,0);
}
CButton* CSIPCallDlg::NewMyButton(int nID, CRect rect, int nStyle, CString Caption)
{
// 	CString m_Caption;
// 	m_Caption.LoadString( nID ); //取按钮标题
	CButton *p_Button = new CButton();
	ASSERT_VALID(p_Button);
	p_Button->Create( Caption, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP/* |  BF_FLAT*/ , rect, this, nID ); //创建按钮
	if (nStyle == 0)
	{
		//pButton->SetFont(&m_editFont1); // 设置新字体  
	}
	else if (nStyle == 1)
	{
		p_Button->SetFont(&m_editFont1); // 设置新字体  
	}
	return p_Button;
}
CButton*  CSIPCallDlg::NewCheckBox(int nID,CRect rect,int nStyle, CString Caption)
{
	CButton *pButton = new CButton();
	ASSERT_VALID(pButton);
	pButton->Create(Caption,WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX | BS_BITMAP,rect,this,nID);
	return pButton;

}
CButton* CSIPCallDlg::NewBMPStatic(int nID,CRect rect,int nStyle, CString Caption)
{

// 	CStatic *pButton = new CStatic();
// 	ASSERT_VALID(pButton);
// 	pButton->Create(Caption,WS_CHILD|WS_VISIBLE|SS_CENTER,rect,this,nID);	
// 	pButton->ModifyStyle(0xf,SS_BITMAP|SS_CENTERIMAGE); 
// 	return (CButton*)pButton;

	CButtonST *p_Button = new CButtonST();
	ASSERT_VALID(p_Button);
	p_Button->Create( Caption, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_BITMAP/* |  BF_FLAT*/ , rect, this, nID ); //创建按钮
	if (nStyle == 0)
	{
		//pButton->SetFont(&m_editFont1); // 设置新字体  
	}
	else if (nStyle == 1)
	{
		p_Button->SetFont(&m_editFont1); // 设置新字体  
	}
	p_Button->SetFlat();
	return p_Button;
}
CStatic* CSIPCallDlg::NewStatic(int nID,CRect rect,int nStyle, CString Caption)
{
	CStatic *pButton = new CStatic();
	ASSERT_VALID(pButton);
	pButton->Create(Caption,WS_CHILD|WS_VISIBLE|SS_CENTER,rect,this,nID);

	if (nStyle == 0)
	{
		//pButton->SetFont(&m_editFont1); // 设置新字体  
	}
	else if (nStyle == 1)
	{
		pButton->SetFont(&m_editFont1); // 设置新字体  
	}
	pButton->ModifyStyle(0,SS_NOTIFY);

	

	return pButton;
}
void  CSIPCallDlg::SetButtonBMP(CButton* pButton, CString strSrc)
{
#if BUTTON_PICTURE_MODEL
	if (pButton == NULL)
	{
		return;
	}
	//pButton->SetButtonStyle(BS_BITMAP,true);
#if 0

#else
	HBITMAP hbitmap = m_MgrPic.GetBitmap(strSrc);
	if (hbitmap != NULL)
	{
		pButton->SetBitmap(hbitmap);
	}
#endif
#endif
}
void CSIPCallDlg::ReleaseButton(CButton* pButton)
{
	if(pButton != NULL){
		delete pButton;
		pButton = NULL;
	}
}
void CSIPCallDlg::BtnInit()
{
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		m_pbtnCall1[i] = NULL;
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		m_pbtnMsg1[i] = NULL;
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		m_pStatic1[i] = NULL;
		m_pStatic2[i] = NULL;
		m_pStatic3[i] = NULL;
		m_pStatic4[i] = NULL;
		m_pStaticKM1[i] = NULL;
		m_pStaticKM2[i] = NULL;
// 		m_pIMG_PE_U[i] = NULL;
// 		m_pIMG_PE_D[i] = NULL;
	}

	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		m_pbtnCall2[i] = NULL;
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		m_pbtnMsg2[i] = NULL;
	}
	m_pbtnImgPre = NULL;
	m_pbtnImgNext = NULL;

	for(int i = 0; i < BTN_NUM_WAIT; i++){
		m_pStatic5[i] = NULL;
	}
}
void CSIPCallDlg::BtnUninit()
{
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_pbtnCall1[i] != NULL)
		{
			delete m_pbtnCall1[i];
			m_pbtnCall1[i] = NULL;
		}
		
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_pbtnMsg1[i] != NULL)
		{
			delete m_pbtnMsg1[i];
			m_pbtnMsg1[i] = NULL;
		}
		
	}

	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_pbtnCall2[i] != NULL)
		{
			delete m_pbtnCall2[i];
			m_pbtnCall2[i] = NULL;
		}

	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_pbtnMsg2[i] != NULL)
		{
			delete m_pbtnMsg2[i];
			m_pbtnMsg2[i] = NULL;
		}

	}
	if(m_pbtnImgPre != NULL){
		delete m_pbtnImgPre;
		m_pbtnImgPre = NULL;
	}

	if(m_pbtnImgNext != NULL){
		delete m_pbtnImgNext;
		m_pbtnImgNext = NULL;
	}
	// lable
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_pStatic1[i] != NULL)
		{
			delete m_pStatic1[i];
			m_pStatic1[i] = NULL;
		}
		if (m_pStatic2[i] != NULL)
		{
			delete m_pStatic2[i];
			m_pStatic2[i] = NULL;
		}
		if (m_pStatic3[i] != NULL)
		{
			delete m_pStatic3[i];
			m_pStatic3[i] = NULL;
		}
		if (m_pStatic4[i] != NULL)
		{
			delete m_pStatic4[i];
			m_pStatic4[i] = NULL;
		}
		if (m_pStaticKM1[i] != NULL)
		{
			delete m_pStaticKM1[i];
			m_pStaticKM1[i] = NULL;
		}
		if (m_pStaticKM2[i] != NULL)
		{
			delete m_pStaticKM2[i];
			m_pStaticKM2[i] = NULL;
		}
	}
	for (int i = 0; i < BTN_NUM_WAIT; i++)
	{
		if (m_pStatic5[i] != NULL)
		{
			delete m_pStatic5[i];
			m_pStatic5[i] = NULL;
		}
	}
}
DWORD CloseProcessidFromName(CString processName)
{
	PROCESSENTRY32 pe;
	DWORD idx = 0;
	HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	pe.dwSize=sizeof(PROCESSENTRY32);
	BOOL ret=Process32First( hSnapshot,&pe);
	if(!ret){
		return 0;
	}
	while(1)
	{
		pe.dwSize=sizeof(PROCESSENTRY32);
		if(Process32Next(hSnapshot,&pe)==FALSE)
			break;
		if(processName.CompareNoCase(pe.szExeFile)==0)
		{
			//id=pe.th32ProcessID;
			//获取当前进程所在路径
			// 			mehandle=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe.th32ProcessID);
			// 			me.dwSize=sizeof(MODULEENTRY32);
			// 			Module32First(mehandle,&me);
			// 			processPath=me.szExePath;
			//从快照进程中获取该进程的PID(即任务管理器中的PID)  
			DWORD dwProcessID = pe.th32ProcessID;  
			HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE,FALSE, dwProcessID);  
			::TerminateProcess(hProcess,0);  
			CloseHandle(hProcess);  

			idx++;
		}
	}
	CloseHandle(hSnapshot);
	return idx;

}
void CSIPCallDlg::ReleaseExit()
{
	g_bExitApp = true;
	if(g_hSipgwExitEvent){
		WaitForSingleObject(g_hSipgwExitEvent, INFINITE);
		CloseHandle(g_hSipgwExitEvent);
	}
	
	// 结束定时器
	KillTimer(TIMER_ID_CHECK);
	KillTimer(TIMER_ID_REFLASH);	
	KillTimer(TIMER_ID_PLAY);
	KillTimer(TIMER_ID_CALL);
	KillTimer(TIMER_ID_PC_BEAT);
	KillTimer(TIMER_ID_PC_BEAT_FAST);
	StopBC(true);
	Sleep(1000);
	if (s_ExitOpt)
	{
		OneKeyClose();	
	}


	Sleep(100);
	m_HeatLock.Lock();
	m_HeatLock.Unlock();
	
	if (!m_vecCallMsg.empty())
	{
		m_vecCallMsg.clear();
	}

	BtnUninit();
	if (!m_vecCallEvt.empty())
	{
		m_vecCallEvt.clear();
	}

	if (!m_vecCallEvent.empty())
	{
		m_vecCallEvent.clear();
	}

	if (!m_vecNoBCData.empty())
	{
		m_vecNoBCData.clear();
	}
	if (!m_vecNoCallData.empty())
	{
		m_vecNoCallData.clear();
	}
	if (!m_vecBeatData.empty())
	{
		m_vecBeatData.clear();
	}
	if (!m_vecOutData.empty())
	{
		m_vecOutData.clear();
	}
	if (!m_vecStausData.empty())
	{
		m_vecStausData.clear();
	}
	if (!m_vecLineData.empty())
	{
		m_vecLineData.clear();
	}
	if (!m_vecDeciveCMD.empty())
	{
		for(int i = 0; i < m_vecDeciveCMD.size(); i++){
			m_vecDeciveCMD[i].vecDevice.clear();
		}
		m_vecDeciveCMD.clear();
	}
	if (m_pGroupCall != NULL)
	{
		free(m_pGroupCall);
		m_pGroupCall = NULL;
	}
	m_nGroupCall = 0;

	if (m_pBCYing != NULL)
	{
		free(m_pBCYing);
		m_pBCYing = NULL;
	}
	m_nBCYing = 0;

	if (m_pIPGroupFile != NULL)
	{
		free(m_pIPGroupFile);
		m_pIPGroupFile = NULL;
	}
	m_nIPGroupFile = 0;
}
void CSIPCallDlg::OnDestroy()
{
	CDialog::OnDestroy();
	// TODO: Add your message handler code here
	ReleaseExit();
}
void CSIPCallDlg::OnSet()
{
	CDlgSet dlg;
	if (dlg.DoModal() == IDOK)
	{
		MessageBox(L"配置已保存，系统将重新登录！", L"提醒");
		OnResetApp();
	}
}
void CSIPCallDlg::OnRecordCall()
{
	CDlgRecord dlg;
	dlg.SetRecord(1);
	dlg.DoModal();
}
void CSIPCallDlg::OnRecordMsg()
{
	CDlgRecord dlg;
	dlg.SetRecord(2);
	dlg.DoModal();
}
void CSIPCallDlg::OnRecordLog()
{
	CDlgRecord dlg;
	dlg.SetRecord(3);
	dlg.DoModal();
}
void CSIPCallDlg::OnRecordDB()
{
	if (g_Logic.m_SendBC || 
		m_CallA.status == ENUM_DEVICE_STATUS_RUN ||
		m_CallB.status == ENUM_DEVICE_STATUS_RUN)
	{
		MessageBox(L"系统正在操作，请稍后修改数据!", L"提醒");
	}
	else{
		g_Logic.m_UpdateDB = true;
		CDlgDB dlg;	
		dlg.DoModal();
		//SendMessage(SEND_MSG_BEAT, 0, 0);
		OnMenuManBeat();
	}
}
void CSIPCallDlg::OnBnClickedButtonNext()
{
	// TODO: Add your control notification handler code here	
	if (m_nPageTunnel + BTN_NUM_TUNNEL > m_nTunnelNum - 1)
	{
		
	}
	else{
		m_nPageTunnel += BTN_NUM_TUNNEL;
	}

	DB2TunnelBtn();
	CheckTunnelPreNext();

	m_nCurTunnelIdx = m_nPageTunnel;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButtonPre()
{
	// TODO: Add your control notification handler code here
	
	if (m_nPageTunnel - BTN_NUM_TUNNEL < 0)
	{
		m_nPageTunnel = 0;
	}
	else{
		m_nPageTunnel -= BTN_NUM_TUNNEL;
	}	
	DB2TunnelBtn();
	CheckTunnelPreNext();
	//CheckBtnImgPreNext();
	m_nCurTunnelIdx = m_nPageTunnel;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::CheckTunnelPreNext()
{
	if (m_nPageTunnel == 0)
	{
		((CButton*)GetDlgItem(IDC_BUTTON_PRE))->EnableWindow(FALSE);
	}
	else{
		((CButton*)GetDlgItem(IDC_BUTTON_PRE))->EnableWindow(TRUE);
	}
	if (m_nPageTunnel + BTN_NUM_TUNNEL >= m_nTunnelNum)//??????????
	{
		((CButton*)GetDlgItem(IDC_BUTTON_NEXT))->EnableWindow(FALSE);
	}
	else{
		((CButton*)GetDlgItem(IDC_BUTTON_NEXT))->EnableWindow(TRUE);
	}
}
void CSIPCallDlg::OnBnClickedButtonDeviceNext()
{
	// TODO: Add your control notification handler code here	
	int upSz, downSz;
	getCurDeviceNum(m_nCurTunnelIdx, upSz, downSz);
	int maxSz = max(upSz, downSz);
	if (m_nPageDevice + BTN_NUM_CALL > maxSz - 1)
	{

	}
	else{
		m_nPageDevice += BTN_NUM_CALL;
	}
	
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButtonDevicePre()
{
	// TODO: Add your control notification handler code here

	if (m_nPageDevice - BTN_NUM_CALL < 0)
	{
		m_nPageDevice = 0;
	}
	else{
		m_nPageDevice -= BTN_NUM_CALL;
	}
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::CheckDevicePreNext()
{
	int upSz, downSz;
	getCurDeviceNum(m_nCurTunnelIdx, upSz, downSz);
	int maxSz = max(upSz, downSz);
	if (m_nPageDevice == 0)
	{
		((CButton*)GetDlgItem(BTN_IMAGE_PRE))->EnableWindow(FALSE);
	}
	else{
		((CButton*)GetDlgItem(BTN_IMAGE_PRE))->EnableWindow(TRUE);
	}
	if (m_nPageDevice + BTN_NUM_MSG >= maxSz)//??????????
	{
		((CButton*)GetDlgItem(BTN_IMAGE_NEXT))->EnableWindow(FALSE);
	}
	else{
		((CButton*)GetDlgItem(BTN_IMAGE_NEXT))->EnableWindow(TRUE);
	}
}
void CSIPCallDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 0;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 1;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 2;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 3;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 4;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 5;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 6;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();

}
void CSIPCallDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 7;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);	
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 8;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::OnBnClickedButton10()
{
	// TODO: Add your control notification handler code here
	m_nCurTunnelIdx = m_nPageTunnel + 9;
	m_nPageDevice = 0;
	//InitSel(m_nCurTunnelIdx);
	DB2DeviceBtn(m_nCurTunnelIdx);
	CheckDevicePreNext();
}
void CSIPCallDlg::InitSel(int curTunnelIdx, int check)
{
	if (curTunnelIdx < 0 || curTunnelIdx >= m_nTunnelNum)
	{
		return;
	}
	m_DBLock.Lock();
	for (int j = 0; j < m_pTunnelInfo[curTunnelIdx].vecDeviceUp.size(); j++)
	{
		m_pTunnelInfo[curTunnelIdx].vecDeviceUp[j].Check = check;
	}
	for (int j = 0; j < m_pTunnelInfo[curTunnelIdx].vecDeviceDown.size(); j++)
	{
		m_pTunnelInfo[curTunnelIdx].vecDeviceDown[j].Check = check;			
	}
	m_DBLock.Unlock();

}
void CSIPCallDlg::OnBnClickedBtnSendBc()
{	
	if (m_CallA.status == ENUM_DEVICE_STATUS_RUN || m_CallB.status == ENUM_DEVICE_STATUS_RUN)
	{
		MessageBox(L"系统正在语音通话,请稍后再试!", L"提醒");
		return;
	}
	if (IsDeviceOK(m_nCurTunnelIdx))
	{
	}
	else{		
		MessageBox(L"未指定设备或设备出现故障!", L"提醒");
	}
}
void CSIPCallDlg::OnBnClickedCheckSendBc()
{
	// TODO: Add your control notification handler code here
	
	int check = ((CButton*)GetDlgItem(IDC_CHECK_SEND_BC))->GetCheck();
	if (m_nCurTunnelIdx < 0 || m_nCurTunnelIdx >= m_nTunnelNum)
	{
		return;
	}
	bool bOffline = false;
	for (int j = 0; j < m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp.size(); j++)
	{
		if(!m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[j].sDB.IsVisible()){
			m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[j].Check = 0;
			continue;
		}
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[j].Check = check;
	}
	for (int j = 0; j < m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown.size(); j++)
	{
		if(!m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[j].sDB.IsVisible()){
			m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[j].Check = 0;
			continue;
		}
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[j].Check = check;			
	}
	DB2DeviceBtn(m_nCurTunnelIdx);	
}
void CSIPCallDlg::InitSocket()
{
	g_MSG.CreateSocket();	
	g_MSG.CreatePCSocket();
	g_MSG.CreateOuterSocket();
}
void CSIPCallDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == TIMER_ID_CHECK)
	{
		SendMessage(SEND_MSG_BEAT, 0, 0);
	}
	else if(nIDEvent == TIMER_ID_REFLASH){		
		//刷新button
		RepaintTunnelBtn();
		//
		Call2Btn();
		//Invalidate();
		// 自动去掉故障信息
		UpdateErrorMsg();
		// 显示等待电话信息
		DisplayCallMsg();
	}
	else if (nIDEvent == TIMER_ID_PLAY) //广播检测
	{
		//KillTimerPlay();
		//g_RunPlay = false;
		if (TimeStopBC())
		{
			Status2Button();
			TRACE("timer stop bc....\r\n");
		}
		int time = GetTickCount() - s_bcEvtTime;
		if (time > 2000)
		{
			UpdateBCMsg();
		}
	}
	else if (nIDEvent == TIMER_ID_CALL)
	{
		// 自动去掉ring状态
		//UpdateCallInfo();
		int time = GetTickCount() - s_callEvtTime;
		if (time > 2000)
		{
			UpdateCallMsg();
			CallFromMsg();
		}	
	}
	else if (nIDEvent == TIMER_ID_PC_BEAT)
	{
		SendMessage(SEND_MSG_PC_BEAT, 0, 0);
	}
	else if (nIDEvent == TIMER_ID_PC_BEAT_FAST)
	{
		SendMessage(SEND_MSG_PC_BEAT_FAST, 0, 0);
	}else if(nIDEvent == TIMER_ID_INFO_SHOW)
	{
		KillTimer(nIDEvent);
		GetDlgItem(IDC_STATIC_MSG)->ShowWindow(SW_HIDE);
	}
	CDialog::OnTimer(nIDEvent);
}
bool CSIPCallDlg::Device2LogRecord(SDevice& stDevice, SRecord& stRecord)
{	
	CharToTchar(stDevice.sDB.ip, stRecord.IP);
	CharToTchar(stDevice.sDB.tunnelName, stRecord.tunnel);
	CharToTchar(stDevice.sDB.kmID, stRecord.kmid);
	CharToTchar(stDevice.sDB.extNo, stRecord.sip);
	CharToTchar(stDevice.sDB.Name, stRecord.noid);//?????????
	stRecord.addr = stDevice.sDB.extAddr;
	switch(stDevice.errorCode){
	case ENUM_ERROR_STATUS_BUTTON:
		wsprintf(stRecord.desc, _T("按钮故障"));		
		break;
	case ENUM_ERROR_STATUS_DOOR:
		wsprintf(stRecord.desc, _T("门开故障")); 		
		break;
	case ENUM_ERROR_STATUS_MIC:
		wsprintf(stRecord.desc, _T("喇叭麦克故障"));		
		break;
	case ENUM_ERROR_STATUS_UNKNOWN:
		wsprintf(stRecord.desc, _T("巡检失败")); 		
		break;
	case ENUM_ERROR_STATUS_DOOR_BTN:
		wsprintf(stRecord.desc, _T("门开、按钮故障")); 		
		break;
	case ENUM_ERROR_STATUS_MIC_DOOR:
		wsprintf(stRecord.desc, _T("喇叭麦克、门开故障")); 
		break;
	case ENUM_ERROR_STATUS_MIC_BTN:
		wsprintf(stRecord.desc, _T("喇叭麦克、按钮故障")); 		
		break;
	case ENUM_ERROR_STATUS_MIC_DOOR_BTN:
		wsprintf(stRecord.desc, _T("喇叭麦克、门开、按钮故障")); 

		break;
	case ENUM_ERROR_STATUS_NONE:
		wsprintf(stRecord.desc, _T("正常")); 		
		break;
	}	

	if (wcslen(stRecord.desc))
	{
		return true;
	}
	else{
		return false;
	}
}
void CSIPCallDlg::AddLogRecord(SDevice& stDevice)
{
#if 0		
#else
	SRecord stRecord;
	if (Device2LogRecord(stDevice, stRecord))
	{
		g_data.InsertLog(stRecord);
	}	
#endif
}
void CSIPCallDlg::AddLogDB()
{
	m_DBLock.Lock();
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			
			AddLogRecord(m_pTunnelInfo[i].vecDeviceUp[j]);

		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			
			AddLogRecord(m_pTunnelInfo[i].vecDeviceDown[j]);
		}		
	}
	m_DBLock.Unlock();
}
void CSIPCallDlg::AddLogDB(int extAddr)
{
	m_DBLock.Lock();
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				AddLogRecord(m_pTunnelInfo[i].vecDeviceUp[j]);
			}			

		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				AddLogRecord(m_pTunnelInfo[i].vecDeviceDown[j]);
			}
		}		
	}
	m_DBLock.Unlock();
}
UINT ReflashCallProc(LPVOID pParam)  
{  
	CSIPCallDlg* pAPP = (CSIPCallDlg*)pParam;
	if (pAPP == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 

	enDeviceStatus enCallAStatus = pAPP->GetCallStatus(g_Config.m_SystemConfig.CallA);
	if (enCallAStatus == ENUM_DEVICE_STATUS_OK)
	{
		pAPP->m_CallA.status = ENUM_DEVICE_STATUS_OK;
	}
	enDeviceStatus enCallBStatus = pAPP->GetCallStatus(g_Config.m_SystemConfig.CallB);
	if (enCallBStatus == ENUM_DEVICE_STATUS_OK)
	{
		pAPP->m_CallB.status = ENUM_DEVICE_STATUS_OK;
	}

	return 1;	
} 
void CSIPCallDlg::ReflashCallThread()
{
	AfxBeginThread(ReflashCallProc, this);
}
UINT PopCallProc(LPVOID pParam)  
{  
	SDevice* pDevice = (SDevice*)pParam;
	if (pDevice == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
	CDlgPopCall dlg;
	dlg.SetInfo(pDevice);
	dlg.DoModal();
	return 1;
}
void CSIPCallDlg::PopCallThread(SDevice* pDevice)
{
	if (g_Config.m_SystemConfig.nPop)
	{
		AfxBeginThread(PopCallProc, pDevice);
	}
}
bool CSIPCallDlg::IsDeviceLocalController(char* dwIp)
{
	if(g_Config.m_SystemConfig.nMainSystem&&strcmp(dwIp,g_dwLocalIp)==0)
	{
		return true;
	}
	else if(!g_Config.m_SystemConfig.nMainSystem)
	{
		return true;
	}else
	{
		return false;
	}
}
bool CSIPCallDlg::IsNeedSendBeat(SDevice* pDevice)
{
	if (pDevice)
	{
		if (pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN ||
			pDevice->callStatus == ENUM_DEVICE_STATUS_RUN ||
			pDevice->callStatus == ENUM_DEVICE_STATUS_RING ||
			pDevice->bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING||
			(!pDevice->sDB.IsVisible()))
		{
			return false;
		}
		else{
			return true;
		}
	}
	return false;
}
bool BeatSleepThread(DWORD dwTime, bool bAutoExit)
{
	if (!bAutoExit)
	{
		Sleep(dwTime);
		return true;
	}
	else{
		Sleep(0);
		int count = dwTime;
		for (int i = 0; i < count; i ++)
		{
			if (g_bExitApp || s_bBeatStop){
				return false;
			}
			else{
				Sleep(1);
			}

		}
		return true;
	}	
}
void CSIPCallDlg::SendBeatContinue()
{
	for (int idx = 0; idx < 2; idx++)
	{
		BeatSleepThread(5000, true);
		for (int i = 0; i < m_vecBeatData.size(); i++)
		{
			if (g_bExitApp || s_bBeatStop)
			{
				return;
			}
			m_BeatDataLock.Lock();
			bool bReply = m_vecBeatData[i].bReply;
			m_BeatDataLock.Unlock();
			if (!bReply)
			{
				g_MSG.SendHeartBeat(m_vecBeatData[i].extAddr, m_vecBeatData[i].ip, g_Config.m_SystemConfig.portDevice);
				Sleep(20);
			}
		}
	}
}
void CSIPCallDlg::SendBeat(int bFirst)
{
	bool bClose = false;		//是否关闭分机电话和广播
	if (bFirst)				//第一次运行程序
	{
		if (g_bOpenAPP)		//正常打开
		{
			bClose = false;
		}
		else{				//重启打开
			bClose = true;
		}
	}
	else{// 心跳
		bClose = false;
	}
	
	// 清除数据
	m_BeatDataLock.Lock();
	if (!m_vecBeatData.empty())
	{
		m_vecBeatData.clear();
	}
	m_BeatDataLock.Unlock();

	m_HeatLock.Lock();
	bool bSendAuto = true;
	//如果是主控巡检，下发给分控进行巡检
	bool bSend2BranchController = false;
	if(g_Config.m_SystemConfig.nMainSystem)
	{
		bSend2BranchController = true;
	}
	// 分控改为不直接巡检发送心跳
	//if (/*(!g_Config.m_SystemConfig.nMainSystem) &&*/
	//	(!s_bManulBeat))
	//{
	//	bSendAuto = false;
	//}
	//手动巡检强制关闭电话和广播
	if (s_bManulBeat)
	{
		bClose = true;
		s_bManulBeat = false; // 手动巡检开关，恢复关闭
		s_bBeatStop = false;	//
	}	
	
	if (bSendAuto)
	{
		for (int i = 0; i < m_nTunnelNum; i++)
		{
			int upSize = m_pTunnelInfo[i].vecDeviceUp.size();
			for (int j = 0; j < upSize; j++)
			{
				if (g_bExitApp || s_bBeatStop)
				{
					//m_DBLock.Unlock();
					m_HeatLock.Unlock();				
					return;
				}
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceUp.begin() + j;
				SDevice* pDevice = &(*vecIter);
				if(bSend2BranchController&&!IsDeviceLocalController(pDevice->dwIP))
				{
					//分控设备，发到分控进行巡检
					g_MSG.SendDeviceStatusReq(*pDevice,0);
					continue;
				}
				if(!IsNeedSendBeat(pDevice)){
					continue;
				}
#if BEAT_SEND_CLOSE

				if(bClose){
					if (m_pTunnelInfo[i].vecDeviceUp[j].bcStatus == ENUM_DEVICE_STATUS_OK ||
						m_pTunnelInfo[i].vecDeviceUp[j].bcStatus == ENUM_DEVICE_STATUS_UNKNOWN)
					{
						//g_Log.output(LOG_TYPE, "SendCloseBroadcast,BEAT_SEND_CLOSE:%d\r\n",m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr);
						g_MSG.SendCloseBroadcast(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice, true);
						Sleep(20);
					}				
					if (m_pTunnelInfo[i].vecDeviceUp[j].callStatus == ENUM_DEVICE_STATUS_OK ||
						m_pTunnelInfo[i].vecDeviceUp[j].callStatus == ENUM_DEVICE_STATUS_UNKNOWN)
					{
						g_MSG.SendHangUp(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
						Sleep(20);					
					}
				}
#endif
				m_BeatDataLock.Lock();
				SBeatData data;
				data.extAddr = m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr;
				data.dwTime = GetTickCount();
#if _A_
				data.bReply = true;
#else
				data.bReply = false;
#endif
				sprintf(data.ip, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip);
				m_vecBeatData.push_back(data);
				m_BeatDataLock.Unlock();
				// 发送心跳
				g_MSG.SendHeartBeat(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
				Sleep(20);
#if _A_
				m_pTunnelInfo[i].vecDeviceUp[j].bcStatus = ENUM_DEVICE_STATUS_OK;
				m_pTunnelInfo[i].vecDeviceUp[j].callStatus = ENUM_DEVICE_STATUS_OK;				
#endif
			}
			int downSz = m_pTunnelInfo[i].vecDeviceDown.size();
			for (int j = 0; j < downSz; j++)
			{
				if (g_bExitApp ||  s_bBeatStop)
				{
					//m_DBLock.Unlock();
					m_HeatLock.Unlock();
					return;
				}
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceDown.begin() + j;
				SDevice* pDevice = &(*vecIter);
				if(bSend2BranchController&&!IsDeviceLocalController(pDevice->dwIP))
				{
					//分控设备，发到分控进行巡检
					g_MSG.SendDeviceStatusReq(*pDevice,0);
					continue;
				}
				if(!IsNeedSendBeat(pDevice)){
					continue;
				}
#if BEAT_SEND_CLOSE
				if (bClose)
				{
					if (m_pTunnelInfo[i].vecDeviceDown[j].bcStatus == ENUM_DEVICE_STATUS_OK ||
						m_pTunnelInfo[i].vecDeviceDown[j].bcStatus == ENUM_DEVICE_STATUS_UNKNOWN)
					{
						//g_Log.output(LOG_TYPE, "SendCloseBroadcast,BEAT_SEND_CLOSE:%d\r\n",m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr);
						g_MSG.SendCloseBroadcast(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice, true);
						Sleep(20);
					}

					if (m_pTunnelInfo[i].vecDeviceDown[j].callStatus == ENUM_DEVICE_STATUS_OK ||
						m_pTunnelInfo[i].vecDeviceDown[j].callStatus == ENUM_DEVICE_STATUS_UNKNOWN)
					{
						g_MSG.SendHangUp(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
						Sleep(20);
					}
				}
#endif
				m_BeatDataLock.Lock();
				SBeatData data;
				data.extAddr = m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr;
				data.dwTime = GetTickCount();
#if _A_
				data.bReply = true;
#else
				data.bReply = false;
#endif
				sprintf(data.ip, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip);
				m_vecBeatData.push_back(data);
				m_BeatDataLock.Unlock();
				// 发送心跳
				g_MSG.SendHeartBeat(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
				Sleep(20);
#if _A_
				m_pTunnelInfo[i].vecDeviceDown[j].bcStatus = ENUM_DEVICE_STATUS_OK;
				m_pTunnelInfo[i].vecDeviceDown[j].callStatus = ENUM_DEVICE_STATUS_OK;
#endif
			}			
		}
		// 丢包再发心跳2次
		SendBeatContinue();
		// 推送给分控状态
		//SendSatus2DeviceThread();
// 		if (g_Config.m_SystemConfig.nMainSystem)
// 		{			
// 			SendSatus2DeviceThread();
// 		}
		
	}
	
	//m_DBLock.Unlock();
	if (bFirst){ // 如果第一次，过30s发送PC心跳
		if(!BeatSleepThread(30*1000, true)){
			m_HeatLock.Unlock();
			return; 
		}
		//SendMessage(SEND_MSG_PC_BEAT, 0, 0);
		//SetTimer(TIMER_ID_PC_BEAT, TIMER_INTER_60000, 0);
	}
	else{
		if(!BeatSleepThread(5000/*30*1000*/, true)){
			m_HeatLock.Unlock();
			return; 
		}
	}
	if (bSendAuto)
	{
		// 更新Log
		UpdateDeviceLog();
	}
	m_HeatLock.Unlock();
}
UINT SendBCProc(LPVOID pParam)  
{  
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
// 	int bFirst = 0;
// 	memcpy(&bFirst, pParam, sizeof(int));
	m_pAPP->SendBeat(s_bFirst);
	g_Log.output(LOG_TYPE, "SendBeat........%d\r\n", s_bFirst);
	return 1;	
} 
void CSIPCallDlg::SendSatus2Device()
{
	if (!m_vecStausData.empty())
	{
		m_vecStausData.clear();
	}
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		int upSize = m_pTunnelInfo[i].vecDeviceUp.size();
		for (int j = 0; j < upSize; j++)
		{
			if (g_bExitApp || s_bBeatStop)
			{			
				return;
			}
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceUp.begin() + j;
			SDevice* pDevice = &(*vecIter);
			
 			m_StausDataLock.Lock();
			SBeatData data;
			data.extAddr = m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr;
			data.dwTime = GetTickCount();
			data.bReply = false;
			sprintf(data.ip, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip);
			m_vecStausData.push_back(data);
 			m_StausDataLock.Unlock();
			// 推送状态
			g_MSG.SendDeviceStatus(pDevice);
			Sleep(50);
		}
		int downSz = m_pTunnelInfo[i].vecDeviceDown.size();
		for (int j = 0; j < downSz; j++)
		{
			if (g_bExitApp ||  s_bBeatStop)
			{
				return;
			}
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceDown.begin() + j;
			SDevice* pDevice = &(*vecIter);
 			m_StausDataLock.Lock();
			SBeatData data;
			data.extAddr = m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr;
			data.dwTime = GetTickCount();
			data.bReply = false;
			sprintf(data.ip, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip);
			m_vecStausData.push_back(data);
 			m_StausDataLock.Unlock();
			// 推送状态
			g_MSG.SendDeviceStatus(pDevice);
			Sleep(50);
		}			
	}
	// 再发送2次
	for (int idx = 0; idx < 2; idx++)
	{
		BeatSleepThread(5000, true);
		for (int i = 0; i < m_vecStausData.size(); i++)
		{
			if (g_bExitApp || s_bBeatStop)
			{
				return;
			}
			m_StausDataLock.Lock();
			bool bReply = m_vecStausData[i].bReply;
			m_StausDataLock.Unlock();
			if (!bReply)
			{
				SDevice* pDevice = GetDevice(m_vecStausData[i].extAddr);
				g_MSG.SendDeviceStatus(pDevice);				
				Sleep(50);
			}
		}
	}
}
UINT SendSatus2SubSystemProc(LPVOID pParam)  
{  
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 	
	m_pAPP->SendSatus2Device();
	
	return 1;	
} 
void CSIPCallDlg::SendSatus2DeviceThread()
{
	AfxBeginThread(SendSatus2SubSystemProc, this);
}
void CSIPCallDlg::SendStatus2Subsystem(BYTE* pBuffer, int size)
{
	BYTE all = pBuffer[8];
	if (all)
	{
		SendSatus2DeviceThread();
	}
	else{
		BYTE extAddr = pBuffer[2];
		SDevice* pDevice = GetDevice(extAddr);
		g_MSG.SendDeviceStatus(pDevice);
	}
}
void CSIPCallDlg::SendBeatThead(int bFirst)
{
	AfxBeginThread(SendBCProc, &bFirst);
}
UINT SendPCBeatFastProc(LPVOID pParam) 
{
	g_MSG.SendPCBeat();
	return 1;
}
//---KEY---每一个窗体对象都使用窗体过程函数（WindowProc）来处理接收到的各种消息。
LRESULT CSIPCallDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch(message){
	#pragma region Send_MSG_BEAT
case SEND_MSG_BEAT:	   //CK注释，心跳包30分钟巡检一次？！！！
		if (/*m_bPause || */g_Logic.m_UpdateDB)	 //初始状态false
		{
			break;				//	CK注释，修改DB数据则Break 
		}		
		{
			s_bFirst = (int)wParam;
			if (/*g_Config.m_SystemConfig.nMainSystem ||*/ s_bManulBeat || s_bFirst)
			{
				//Caution 200!!!
				SendBeatThead(s_bFirst);  //直接调用 F-SendBCProc
			}
			// 分控第一次向主控请求状态
			/*if ((!g_Config.m_SystemConfig.nMainSystem) || s_bFirst) 
			{
				SDevice stDevice;
				g_MSG.SendDeviceStatusReq(stDevice, 0);
			}*/
		}
		//SetTimer(TIMER_ID_UPDATE_STATUS, TIMER_INTER_30000, 0);
		break;
#pragma endregion Send_MSG_BEAT
	case SEND_MSG_PC_BEAT:	 //暂时用不到
		{
			g_data.SelectDeviceBeat();
			g_data.SendDeviceBeat();
		}
		break;
	case SEND_MSG_PC_BEAT_FAST:	 //暂时用不到
		{
			AfxBeginThread(SendPCBeatFastProc, this);
		}
		break;
//	#pragma region Device_BEAT_Reply
//case RECV_MSG_BEAT_OK:	 //检测到BEAT OK
//		
//		//AddLogDB(wParam);
//		break;
//#pragma endregion Device_BEAT_Reply
	#pragma region Device_Beat_Reply_Error
case RECV_MSG_BEAT_ERROR:	//检测到BEAT ERROR
		if (g_Logic.m_UpdateDB)
		{
			break;			 //	  修改DB数据则Break
		}	
		if (!IsBCRun(wParam))
		{
			UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_OK);	
		}		
		if (!IsCallRun(wParam))
		{
			UpdateStatus_CALL(wParam, ENUM_DEVICE_STATUS_ERROR);
		}
		UpdateErrorCode(wParam, ENUM_DEVICE_STATUS_ERROR, (enErrorCode)(lParam));
		//AddLogDB(wParam);
		break;
#pragma endregion Device_Beat_Reply_Error
	case RECV_MSG_BC_OPEN:
		UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_RUN);
		break;
	#pragma region BC_CLOSE
case RECV_MSG_BC_CLOSE:	 //CK注释，接收广播关的消息
		{
			int extAddr = (int)wParam;	//消息响应机制
			SDevice* pDevice = GetDevice(extAddr); //获取设备地址
			if (pDevice)
			{
				if ( pDevice->IsBcPlay() == ENUM_BC_OPT_LOCAL)	//本地操作
				{
					bool bmic;
					bool baux;
					StopOneDevice(pDevice, bmic, baux, false, false);
					AutoCloseMicAux(bmic, baux);
				}
				else{
					UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_OK);	
				}
			}			
		}		
		break;
#pragma endregion BC_CLOSE
	case RECV_MSG_BC_OPT_CLOSE:	 //广播操作关闭
		{
			int extAddr = (int)wParam;
			SDevice* pDevice = GetDevice(extAddr);
			if (pDevice)
			{
				bool bmic;
				bool baux;
				StopOneDevice(pDevice, bmic, baux, false, false);
				AutoCloseMicAux(bmic, baux);
			}
		}
		break;
#pragma region DOOR_DOOR_OPEN_CLOSE
	case RECV_MSG_DOOR_RT_OPEN:	//CK注释，RT real time 门开
		if (g_Logic.m_UpdateDB)
		{
			break;
		}
		UpdateDoorRT(wParam, 1, GetTickCount());				
		break;
	case RECV_MSG_DOOR_RT_CLOSE:  //CK注释，门关
		if (g_Logic.m_UpdateDB)
		{
			break;
		}
		UpdateDoorRT(wParam, 0, 0);
		break;
#pragma endregion DOOR_DOOR_OPEN_CLOSE
	case WM_MSG_COMPLETED:
		TRACE(L".........WM_MSG_COMPLETED........%d, %d\r\n", wParam, lParam);
		CompltePlayBC(wParam);
		Status2Button();		
		break;
	case RECV_MSG_STOP_MIC:
	#pragma region STOP_AUX
case RECV_MSG_STOP_AUX:
		{
			SDevice* pDevice = (SDevice*)lParam;
			if (pDevice)
			{
				MsgStopPlayDevice(pDevice);
				Status2Button();	
			}
		}
		break;
#pragma endregion STOP_AUX
	#pragma region DEVICE_BEAT    status from branch controller pc
case RECV_MSG_DEVICE_BEAT:
		{
			int extAddr = (int)wParam;
			SDevice* pDevice = GetDevice(extAddr);
			if (pDevice)
			{
				g_MSG.SendDeviceBeatReply(extAddr, pDevice->bcStatus, pDevice->callStatus);
			}
		}
		break;
#pragma endregion DEVICE_BEAT
	case RECV_MSG_DEVICE_BEAT_REPLY:
		g_data.UpdateReplay(wParam, (SBeatReply*)lParam);		
		break;
	case RECV_MSG_DEVICE_UPDATE_STATUS:
		UpdateStatusFromPCBeat((SBeatReply*)lParam);
		break;
	#pragma region Auto_BC_Open
case RECV_MSG_AUTO_BC_OPEN_RERECV:
		{
			int groupID = (int)wParam;
			SDeviceCmd* pDeviceCmd = GetDevCmd(groupID);
			g_Log.output(LOG_TYPE, "RECV_MSG_AUTO_BC_OPEN_RERECV.........\r\n");
			if (pDeviceCmd)
			{
				for (int i = 0; i < pDeviceCmd->vecDevice.size(); i++)
				{
					int extAddr = pDeviceCmd->vecDevice[i];
					SDevice* pDevice = GetDevice(extAddr);
					if (pDevice)
					{
						g_Log.output(LOG_TYPE, "RECV_MSG_AUTO_BC_OPEN_RERECV, ext=%d.........\r\n", extAddr);
						CMD_PlayOneDevice(pDevice, pDeviceCmd->file);						
					}
				}
			}
		}
		break;
#pragma endregion Auto_BC_Open
	#pragma region Auto_BC_Close
case RECV_MSG_AUTO_BC_CLOSE_RERECV:
		{
			int groupID = (int)wParam;
			SDeviceCmd* pDeviceCmd = GetDevCmd(groupID);
			if (pDeviceCmd)
			{
				for (int i = 0; i < pDeviceCmd->vecDevice.size(); i++)
				{
					int extAddr = pDeviceCmd->vecDevice[i];
					SDevice* pDevice = GetDevice(extAddr);
					if(pDevice){
						if ( pDevice->IsBcPlay() == ENUM_BC_OPT_LOCAL)
						{
							bool bmic;
							bool baux;
							StopOneDevice(pDevice, bmic, baux, true, false);
							AutoCloseMicAux(bmic, baux);
						}
						else{
							UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_OK);	
						}
						// 自动归位
						pDevice->Check = 0;
						pDevice->playSrc = ENUM_MODEL_NONE;
						pDevice->playMode = ENUM_PLAY_MODEL_NONE;
						memset(pDevice->playFile, 0, MAX_PATH);
					}	
				}
			}		
		}
		break;
#pragma endregion Auto_BC_Close
//case SIPGW_EXIT:
//		{
//			int sipgwFlag = (int)wParam;
//			if(sipgwFlag == 0)
//			{
//				if(!g_bExitSipgw){
//					MessageBox(L"sipgw has been exit...");
//					EnableCall(false);
//					if(!g_Config.m_SystemConfig.nMainSystem)
//					{
//						g_MSG.SendBranchOMDown2Master(2);
//
//					}
//					g_bExitSipgw = true;
//				}
//			}
//			else
//			{
//				if(g_bExitSipgw){
//					EnableCall(true);
//					g_bExitSipgw = false;
//					g_MSG.SendBranchOMDown2Master(1);
//				}
//			}
//		}
//		break;
	}
//	MessageBox(L"WND！", L"提醒");	
	return CDialog::WindowProc(message, wParam, lParam);
}
void CSIPCallDlg::TakeDeviceByOMStatus(char* szIp,BYTE status)
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (!m_pTunnelInfo[i].vecDeviceUp[j].sDB.IsVisible())
			{
				continue;
			}
			if (strcmp(m_pTunnelInfo[i].vecDeviceUp[j].dwIP,szIp)==0)
			{
				m_pTunnelInfo[i].vecDeviceUp[j].callStatus = 
					(status==1?ENUM_DEVICE_STATUS_OK:ENUM_DEVICE_OM_DOWN);
				Status2Button(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr);
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (!m_pTunnelInfo[i].vecDeviceDown[j].sDB.IsVisible())
			{
				continue;
			}
			if (strcmp(m_pTunnelInfo[i].vecDeviceDown[j].dwIP,szIp)==0)
			{
				m_pTunnelInfo[i].vecDeviceDown[j].callStatus = 
					((status == 1)?ENUM_DEVICE_STATUS_OK:ENUM_DEVICE_OM_DOWN);
				Status2Button(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr);
			}		
		}		
	}
}
void CSIPCallDlg::EnableCall(bool bCanCall)
{
	if(g_bExitSipgw&!bCanCall)
	{
		//上次更新是不可用,本次更新不可用
		return ;
		
	}else if(g_bExitSipgw&bCanCall)
	{
		//上次更新不可用、本次更新可用
		for (int i = 0; i < BTN_NUM_MSG; i++)
		{
			//((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->EnableWindow(TRUE);			
			//((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->EnableWindow(TRUE);
			((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i))->EnableWindow(TRUE);
			((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i))->EnableWindow(TRUE);
			//1.向主控、分控报告分机电话可用
		}
	}
	else if (!g_bExitSipgw&!bCanCall)
	{
		//上次更新可用、本次更新不可用
		for (int i = 0; i < BTN_NUM_MSG; i++)
		{
			//((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->EnableWindow(FALSE);			
			//((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->EnableWindow(FALSE);
			((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i))->EnableWindow(FALSE);
			((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i))->EnableWindow(FALSE);
			//1.删除通话事件
			//2.向主控、分控报告分机电话不可用
		}
	}else 
	{
		//上次更新可用、本次更新可用
		return ;
	}
}
void CSIPCallDlg::UpdateStatusFromPCBeat(SBeatReply* pBeat)
{
	if (pBeat == NULL)
	{
		return;
	}
	int extAddr = pBeat->extAddr;
	SDevice* pDevice = GetDevice(extAddr);
	if (pDevice)
	{
		pDevice->bcStatus = pBeat->bc;
		if (pDevice->callStatus == ENUM_DEVICE_STATUS_UNKNOWN)
		{
			pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
		}		
		Status2Button(extAddr);
	}
}
void CSIPCallDlg::RemoveRing(int id)
{
	if (m_CallA.status == ENUM_DEVICE_STATUS_RING)
	{
		if (m_CallA.id == id)
		{
			m_CallA.status = m_CallA.preStatus;
			m_CallA.clearCall();
		}		
	}
	if (m_CallB.status == ENUM_DEVICE_STATUS_RING)
	{
		if(m_CallB.id == id){
			m_CallB.status = m_CallB.preStatus;
			m_CallB.clearCall();
		}		
	}
}
SDevice* CSIPCallDlg::GetDeviceFromEvent(SEventInfo* pEvtInfo)
{	
	if (pEvtInfo == NULL)
	{
		return NULL;
	}
	return GetDevice(pEvtInfo->from);
}
void CSIPCallDlg::EventChangeBtn(SDevice* pDevice, enRecvMsg msg)
{	
	if (pDevice != NULL)
	{				
		if (msg == RECV_MSG_CALL_RUN /*|| msg == RECV_MSG_CALL_RING*/)
		{
			pDevice->callStatus = ENUM_DEVICE_STATUS_RUN;
		}
		else if (msg == RECV_MSG_CALL_HANGUP){
			if(pDevice->errorCode!=ENUM_ERROR_STATUS_NONE)
				pDevice->callStatus = ENUM_DEVICE_STATUS_ERROR;
			else
				pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
		}		
		else if (msg == RECV_MSG_CALL_OK){
			if(pDevice->errorCode!=ENUM_ERROR_STATUS_NONE)
				pDevice->callStatus = ENUM_DEVICE_STATUS_ERROR;
			else
				pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
		}	
		else if (msg == RECV_MSG_CALL_RING)
		{
			pDevice->callStatus = ENUM_DEVICE_STATUS_RING;
		}
		else if (msg == RECV_MSG_CALL_IDLE)
		{
			if(pDevice->errorCode!=ENUM_ERROR_STATUS_NONE)
				pDevice->callStatus = ENUM_DEVICE_STATUS_ERROR;
			else
				pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
		}
		else if (msg == RECV_MSG_CALL_OFFLINE)
		{
			pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
			pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
		}
		Status2Button(pDevice->sDB.extAddr);
	}
}
bool CSIPCallDlg::TimeStopDevice(SDevice* pDevice)
{
	bool flag = false;
	if(pDevice->playSrc == ENUM_MODEL_TTS ||
	   pDevice->playSrc == ENUM_MODEL_FILE ||
	   pDevice->playSrc == ENUM_MODEL_FILE_LIST)
	{
		if ((GetTickCount() - pDevice->playTime) >= (g_Config.m_SystemConfig.nPlayTime * 3600 * 1000))//超时
		{
			flag = true;	
			pDevice->enStopFlag = ENUM_STOP_MSG_TIME;
			if (pDevice->threadID)
			{
				//等待关闭回复指令
				SetBCStatus(pDevice->sDB.extAddr,ENUM_BC_WAIT_E4);
				g_Play.stop(pDevice->playParam);
				if(IsDeviceLocalController(pDevice->dwIP)){
					g_MSG.SendDeviceBroadcastStatus2Master(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_IS_USABLE);
					g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr,pDevice->dwIP,g_Config.m_SystemConfig.portDevice,false);
				}
				else{
					g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_CLOSE_REQ);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_BC_CLOSE);
				}
			}			
		}
	}	
	return flag;
}
void CSIPCallDlg::EndPlay(SDevice* pDevice)
{
	pDevice->threadID = 0;
	pDevice->playTime = 0;
	pDevice->playFlag = 0;
	pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
	pDevice->Check = 0;

	// 关广播
	// ....
	SetBCStatus(pDevice->sDB.extAddr,ENUM_BC_WAIT_E4);

	if(IsDeviceLocalController(pDevice->dwIP)){
		//g_Log.output(LOG_TYPE, "SendCloseBroadcast,EndPlay:%d\r\n",pDevice->sDB.extAddr);
		g_MSG.SendDeviceBroadcastStatus2Master(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_IS_USABLE);
		g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice, false);
	}else
	{
		//g_Log.output(LOG_TYPE, "SendBroadcastReq,EndPlay:%d\r\n",pDevice->sDB.extAddr);
		g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_CLOSE_REQ);
		g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
	}
}
bool CSIPCallDlg::StartPlay(SDevice* pDevice)
{
	//g_Log.output(LOG_TYPE, "StartPlay.....FILE:%s\r\n", pDevice->playFile);

	//创建线程，play函数中调用lc_play开始播放
	pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
	
	if (!pDevice->threadID)
	{
		return false;
	}
	else{
		g_Log.output(LOG_TYPE, "StartPlay.....FILE:%s, success\r\n", pDevice->playFile);
		if(IsDeviceLocalController(pDevice->dwIP)){
			USHORT port = g_Config.m_SystemConfig.portDevice;
			g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, port);	
			//g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			if(pDevice->cmd_open_bc_recv)
				pDevice->bcStatus = ENUM_DEVICE_STATUS_OUTER_USING;
			else
				pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
			pDevice->playTime = GetTickCount();
			pDevice->playFlag = true;
			if(g_Config.m_SystemConfig.nMainSystem)
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			if(!g_Config.m_SystemConfig.nMainSystem)
				g_MSG.SendDeviceBroadcastStatus2Master(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_IS_USING);
			return true;
		}else{
			g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_OPEN_REQ);
			if(g_Config.m_SystemConfig.nMainSystem)
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			if(pDevice->cmd_open_bc_recv)
				pDevice->bcStatus = ENUM_DEVICE_STATUS_OUTER_USING;
			else
				pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
			pDevice->playTime = GetTickCount();
			pDevice->playFlag = true;
			return true;
		}
	}	
}
bool CSIPCallDlg::MsgStopPlayDevice(SDevice* pDevice)
{	
	bool flag = false;
	if (pDevice == NULL)
	{
		return false;
	}
	switch(pDevice->enStopFlag){
	case ENUM_STOP_MSG_NONE:// 自动播放结束
		{
			g_Log.output(LOG_TYPE, "自动播放结束.......\r\n");
			if (pDevice->playSrc == ENUM_MODEL_FILE){ // 单文件
				if (pDevice->playMode == ENUM_PLAY_MODEL_ONE_ONCE){// 播放一次
					EndPlay(pDevice);
					flag = true;				
					g_Log.output(LOG_TYPE, "play file stop....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
				} 
				else{// 重复播放
					if(pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN||
						pDevice->bcStatus == ENUM_DEVICE_STATUS_OUTER_USING){
						g_Log.output(LOG_TYPE, "play file play ....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
						pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);					
						pDevice->playFlag = true;
						if(pDevice->cmd_open_bc_recv)
							pDevice->bcStatus = ENUM_DEVICE_STATUS_OUTER_USING;
						else
							pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
					}
				}
			}
			else if (pDevice->playSrc == ENUM_MODEL_FILE_LIST){// 列表文件可以单曲播放、顺序播放
				if (pDevice->playMode == ENUM_PLAY_MODEL_ONE_ONCE){
					EndPlay(pDevice);
					flag = true;				
					g_Log.output(LOG_TYPE, "play filelist stop....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
				}
				else if(pDevice->playMode == ENUM_PLAY_MODEL_ONE_CONTINUE){//单曲循环				
					g_Log.output(LOG_TYPE, "play filelist replay....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
					if(pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN){
						pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
						//pDevice->playTime = GetTickCount();
						pDevice->playFlag = true;
						pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
					}
				}			
				else if (pDevice->playMode == ENUM_PLAY_MODEL_LIST){//列表				
					pDevice->curPlayIdx++;
					if (pDevice->curPlayIdx >= g_data.GetPlayListSz() || 
						pDevice->curPlayIdx < 0)
					{
						pDevice->curPlayIdx = 0;
					}
					char* pFile = g_data.GetPlayFile(pDevice->curPlayIdx);
					if (pFile&&pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN)
					{
						sprintf(pDevice->playFile, pFile);
						pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
						//pDevice->playTime = GetTickCount();
						pDevice->playFlag = true;
						pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;

						g_Log.output(LOG_TYPE, "play filelist stop start....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
					}
					
				}
				else if (pDevice->playMode == ENUM_PLAY_MODEL_LIST_ONCE)
				{
					pDevice->curPlayIdx++;					
					if (pDevice->curPlayIdx >= g_data.GetPlayListSz() )
					{			
						EndPlay(pDevice);
						flag = true;
					}
					else{
						if (pDevice->curPlayIdx < 0)
						{
							pDevice->curPlayIdx = 0;
						}
						char* pFile = g_data.GetPlayFile(pDevice->curPlayIdx);
						if (pFile&&pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN)
						{
							sprintf(pDevice->playFile, pFile);
							pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
							//pDevice->playTime = GetTickCount();
							pDevice->playFlag = true;
							pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;

							g_Log.output(LOG_TYPE, "play filelist stop start....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
						}
					}		
				}
			}
			else if (pDevice->playSrc == ENUM_MODEL_TTS){ // TTS
				if (pDevice->playMode == ENUM_PLAY_MODEL_ONE_ONCE){// 播放一次
					EndPlay(pDevice);
					flag = true;				
					g_Log.output(LOG_TYPE, "play file stop....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
				} 
				else{// 重复播放
					if(pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN){
						g_Log.output(LOG_TYPE, "play file replay....%d: %s.....\r\n", pDevice->sDB.extAddr,  pDevice->playFile);
						pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
						//pDevice->playTime = GetTickCount();
						pDevice->playFlag = true;
						pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
					}
				}
			}
		}
		break;
	case ENUM_STOP_MSG_CLICK:// 强制停止
		{
			g_Log.output(LOG_TYPE, "点击停止.......\r\n");
			EndPlay(pDevice);
		}
		break;
	case ENUM_STOP_MSG_TIME://  时间停止
		{
			g_Log.output(LOG_TYPE, "时间停止.......\r\n");
			EndPlay(pDevice);
		}
		break;
	case ENUM_STOP_MSG_CHANGE:// 播放切换
		{
			g_Log.output(LOG_TYPE, "播放切换，ext[%d].......\r\n", pDevice->sDB.extAddr);
			enBCOptStatus opt = pDevice->IsBcPlay();
			if (opt == ENUM_BC_OPT_NONE)
			{
				g_Log.output(LOG_TYPE, "播放切换, ENUM_BC_OPT_NONE.......\r\n");
			}
			else if (opt == ENUM_BC_OPT_REMOTE)
			{
				//发送远程关闭指令
				g_Log.output(LOG_TYPE, "ENUM_SEND_BC_OPT_CLOSE: 播放切换 dlg: %d\r\n", pDevice->sDB.extAddr);
				//g_MSG.SendBCStatus(ENUM_SEND_BC_OPT_CLOSE, pDevice->sDB.extAddr);
				g_Log.output(LOG_TYPE, "播放切换, ENUM_BC_OPT_REMOTE.......\r\n");
			}	
			else if (opt == ENUM_BC_OPT_LOCAL)
			{		
				g_Log.output(LOG_TYPE, "播放切换, ENUM_BC_OPT_LOCAL.......\r\n");
				// 更新
				enPlaySrc oldSrc = pDevice->playSrc;
				g_data.Config2Device(pDevice);
				enPlaySrc newSrc = pDevice->playSrc;
				if ((oldSrc == ENUM_MODEL_MIC ||oldSrc == ENUM_MODEL_AUX) &&
					(newSrc == ENUM_MODEL_FILE ||newSrc == ENUM_MODEL_FILE_LIST || newSrc == ENUM_MODEL_TTS))
				{
					// 开始计时
					pDevice->playTime = GetTickCount();
				}
				g_Log.output(LOG_TYPE, "播放切换, old[%d], new[%d].......\r\n", (int)oldSrc, (int)newSrc);
				// 开启
				if (pDevice->playSrc == ENUM_MODEL_FILE){ // 单文件
					g_Log.output(LOG_TYPE, "ENUM_STOP_MSG_CHANGE: ENUM_MODEL_FILE, %s\r\n", pDevice->playFile);
					pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
					//pDevice->playTime = GetTickCount();
					pDevice->playFlag = true;
					pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
				}
				else if (pDevice->playSrc == ENUM_MODEL_FILE_LIST){// 列表文件可以单曲播放、顺序播放
					g_Log.output(LOG_TYPE, "ENUM_STOP_MSG_CHANGE: ENUM_MODEL_FILE_LIST, %s\r\n", pDevice->playFile);
					if (pDevice->curPlayIdx >= g_data.GetPlayListSz() || 
						pDevice->curPlayIdx < 0)
					{
						pDevice->curPlayIdx = 0;
					}
					char* pFile = g_data.GetPlayFile(pDevice->curPlayIdx);
					if (pFile)
					{
						sprintf(pDevice->playFile, pFile);
						g_Log.output(LOG_TYPE, "ENUM_STOP_MSG_CHANGE: ENUM_MODEL_FILE_LIST, %d, %s\r\n", pDevice->sDB.extAddr, pDevice->playFile);
						pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
						//pDevice->playTime = GetTickCount();
						pDevice->playFlag = true;
						pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
					}						
					else{
						EndPlay(pDevice);
					}					
				}
				else if (pDevice->playSrc == ENUM_MODEL_TTS){ // TTS
					g_Log.output(LOG_TYPE, "ENUM_STOP_MSG_CHANGE: ENUM_MODEL_TTS, %s\r\n", pDevice->playFile);
					//m_pAPP->Text2Speach(pDevice->playFile);
					//char szWave[MAX_PATH] = {0};
					if (strstr(pDevice->playFile + strlen(pDevice->playFile) - 4, ".txt"))
					{
						WideCharToMultiByte(CP_ACP, 0, m_pAPP->Txt2TTSPath(pDevice->playFile)/*g_ttsFilePath*/, -1, pDevice->playFile/*szWave*/, MAX_PATH, NULL, NULL);
					}
					
					pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
					//pDevice->playTime = GetTickCount();
					pDevice->playFlag = true;
					pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
				}
				else if (pDevice->playSrc == ENUM_MODEL_MIC)
				{
					g_Log.output(LOG_TYPE, "ENUM_STOP_MSG_CHANGE: ENUM_MODEL_MIC\r\n");
					//g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr,pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
					//g_MSG.SendCmd(pDevice->sDB.extAddr, ENUM_CMD_CODE_BC_OPEN);
					g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);				
					m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_MIC);

					if (!g_bOpenMic)
					{
						g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_MIC_RECORD, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);
						g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_Monitor, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);
						USHORT portAudio = g_Config.m_SystemConfig.portAudio;
						g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_MIC, portAudio, false);
						g_Play.RecordStart();
					}
				}
				else if (pDevice->playSrc == ENUM_MODEL_AUX)
				{
					g_Log.output(LOG_TYPE, "ENUM_STOP_MSG_CHANGE: ENUM_MODEL_AUX\r\n");
					//g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr,pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
					//g_MSG.SendCmd(pDevice->sDB.extAddr, ENUM_CMD_CODE_BC_OPEN);
					g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_AUX);				
					m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_AUX);

					if (!g_bOpenAux)
					{
						USHORT portAudio = g_Config.m_SystemConfig.portAudio;
						g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_AUX, portAudio, true);
					}
				}
			}
			
		}
		break;
	}
	// 发送停止消息
// 	if (m_MonitorDevice.pDevice == pDevice)
// 	{
// 		if (IsMonitor())
// 		{			
// 			g_Play.stop(m_MonitorDevice.param);	
// 		}
// 	}
	// 恢复
	pDevice->enStopFlag = ENUM_STOP_MSG_NONE;
	
	return flag;
}
bool CSIPCallDlg::DevicePlay(SDevice* pDevice, bool bOpenAudio)
{
	if(pDevice == NULL){
		return false;
	}
	pDevice->threadID = g_Play.play(pDevice->playFile, pDevice->playParam, pDevice->sDB.ip);
	if (pDevice->threadID)
	{
		if (bOpenAudio)
		{
			g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);	
		}
		pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
		pDevice->playTime = GetTickCount();
		pDevice->playFlag = true;
		return true;
	}
	else{
		return false;
	}
}
void CSIPCallDlg::DeviceStop(SDevice* pDevice, bool bCloseAudio)
{
	if(pDevice == NULL){
		return;
	}
	if (pDevice->threadID>0)
	{
		g_Play.stop(pDevice->playParam);	
		pDevice->threadID = 0;
		pDevice->playFlag = false;
		pDevice->playTime = 0;
		pDevice->Check = 0;
		g_Log.output(LOG_TYPE,"device stop...\r\n");
	}	
}
void CSIPCallDlg::DevicePause(SDevice* pDevice)
{
	if(pDevice == NULL){
		return;
	}
	if(pDevice->threadID)
	{
		g_Play.pausePlay(pDevice->playParam);
	}
}
void CSIPCallDlg::DeviceResume(SDevice* pDevice)
{
	if(pDevice == NULL){
		return;
	}
	if (pDevice->threadID)
	{
		g_Play.continuePlay(pDevice->playParam);
	}
}
int CSIPCallDlg::GetDeviceBCStatus(SDevice* pDevice)
{
	if (!pDevice)
	{
		return -1;
	}
	if (pDevice->threadID)
	{
		return g_Play.status(pDevice->playParam);
	}else{
		return -1;
	}
}
bool CSIPCallDlg::TimeStopBC()
{
	bool bAutoStop = false;
	for (int m = 0; m < m_nTunnelNum; m++)
	{
		STunnel* pCurTunnel = m_pTunnelInfo + m;
		if (!pCurTunnel->vecDeviceUp.empty())
		{
			vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceUp.begin();
			SDevice* pDevice = &(*vecIter);	
			for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
			{		
				if (pDevice[i].IsCheck()&&!IsCallRun(pDevice[i].sDB.extAddr))
				{
					enBCOptStatus opt = pDevice[i].IsBcPlay();
					if (opt == ENUM_BC_OPT_NONE)
					{

					}
					else if (opt == ENUM_BC_OPT_LOCAL){
						if (TimeStopDevice(pDevice + i))
						{
							bAutoStop = true;
						}
					}										
				}
			}
		}
		if (!pCurTunnel->vecDeviceDown.empty())
		{
			vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceDown.begin();
			SDevice* pDevice = &(*vecIter);
			for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
			{
				if (pDevice[i].IsCheck())
				{
					enBCOptStatus opt = pDevice[i].IsBcPlay();
					if (opt == ENUM_BC_OPT_NONE)
					{

					}
					else if (opt == ENUM_BC_OPT_REMOTE)
					{
						//发送远程关闭指令
						//g_Log.output(LOG_TYPE, "ENUM_SEND_BC_OPT_CLOSE: TimeStopBC down: %d\r\n", pDevice[i].sDB.extAddr);
						//g_MSG.SendBCStatus(ENUM_SEND_BC_OPT_CLOSE, pDevice[i].sDB.extAddr);
					}	
					else if (opt == ENUM_BC_OPT_LOCAL){
						if (TimeStopDevice(pDevice + i))
						{
							bAutoStop = true;
						}	
					}
				}
			}
		}	
	}
	return bAutoStop;
}
void CSIPCallDlg::CompltePlayBC(int threadID)
{
	if (threadID <= 0)
	{
		return;
	}
	// 如果是监听广播的，进行监听广播操作
// 	if (StopMonitorFromMsg(threadID))
// 	{
// 		return;
// 	}
	for (int m = 0; m < m_nTunnelNum; m++)
	{
		STunnel* pCurTunnel = m_pTunnelInfo + m;
		vector<SDevice>::iterator vecIter;
		SDevice* pDevice = NULL;	
		if (!pCurTunnel->vecDeviceUp.empty())
		{
			vecIter = pCurTunnel->vecDeviceUp.begin();
			pDevice = &(*vecIter);	
			for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
			{		
				if (threadID == pDevice[i].threadID)
				{
					MsgStopPlayDevice(pDevice + i);			
					break;
				}
			}
		}
		
		if (!pCurTunnel->vecDeviceDown.empty())
		{
			vecIter = pCurTunnel->vecDeviceDown.begin();
			pDevice = &(*vecIter);
			for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
			{
				if (threadID == pDevice[i].threadID)
				{
					MsgStopPlayDevice(pDevice + i);			
					break;
				}	
			}
		}		
	}
}
void CSIPCallDlg::UpdateStatus()
{	
	Status2Button();
}
SDevice* CSIPCallDlg::GetDevice(int extAddr) //CK注释,GetDevice函数重载
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (!m_pTunnelInfo[i].vecDeviceUp[j].sDB.IsVisible())
			{
				continue;
			}
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceUp.begin() + j;
				return &(*vecIter) ;
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (!m_pTunnelInfo[i].vecDeviceDown[j].sDB.IsVisible())
			{
				continue;
			}
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceDown.begin() + j;
				return &(*vecIter) ;
			}			
		}		
	}
	return NULL;
}
SDevice* CSIPCallDlg::GetDevice(char* extNo) //CK注释,GetDevice函数重载
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (!m_pTunnelInfo[i].vecDeviceUp[j].sDB.IsVisible())
			{
				continue;
			}
			if (strcmp(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extNo, extNo) == 0)
			{
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceUp.begin() + j;
				return &(*vecIter) ;
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (!m_pTunnelInfo[i].vecDeviceDown[j].sDB.IsVisible())
			{
				continue;
			}
			if (strcmp(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extNo, extNo) == 0) //编号相等
			{
				 //CK注释，Iterator模式是运用于聚合对象的一种模式，
				//通过运用该模式，使得我们可以在不知道对象内部表示的情况下，
				//按照一定顺序（由iterator提供的方法）访问聚合对象中的各个元素。
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceDown.begin() + j;
				return &(*vecIter) ;
			}			
		}		
	}
	return NULL;
}
SDevice* CSIPCallDlg::GetDevice(DWORD dwIP)//CK注释,GetDevice函数重载
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			/*if (m_pTunnelInfo[i].vecDeviceUp[j].dwIP == dwIP)
			{
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceUp.begin() + j;
				return &(*vecIter) ;
			}*/			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			/*if (m_pTunnelInfo[i].vecDeviceDown[j].dwIP == dwIP)
			{
				vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceDown.begin() + j;
				return &(*vecIter) ;
			}	*/		
		}		
	}
	return NULL;
}
void CSIPCallDlg::UpdateStatus_BC(int extAddr, enDeviceStatus status)
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				if(m_pTunnelInfo[i].vecDeviceUp[j].bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING&&
					status == ENUM_DEVICE_STATUS_RUN)
				{

				}
				else
					m_pTunnelInfo[i].vecDeviceUp[j].bcStatus = status;
				Status2Button(extAddr);
				TRACE("!!!!up button,broadcast status:...[%d],[%d]\r\n",m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr,extAddr);
				return;
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				if(m_pTunnelInfo[i].vecDeviceDown[j].bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING &&
					status == ENUM_DEVICE_STATUS_RUN)
				{

				}
				else
					m_pTunnelInfo[i].vecDeviceDown[j].bcStatus = status;
				Status2Button(extAddr);
				return;
			}			
		}		
	}
}
void CSIPCallDlg::UpdateStatus_CALL(int extAddr, enDeviceStatus status)
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				m_pTunnelInfo[i].vecDeviceUp[j].callStatus = status;
				Status2Button(extAddr);
				return;
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				m_pTunnelInfo[i].vecDeviceDown[j].callStatus = status;				
				Status2Button(extAddr);
				return;
			}			
		}		
	}
}
void CSIPCallDlg::UpdateErrorCode(int extAddr, enDeviceStatus status, enErrorCode errorCode)
{	
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{				
				m_pTunnelInfo[i].vecDeviceUp[j].errorCode = errorCode;	
				SeatBeatReply(extAddr);
				return;			
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{				
				
				m_pTunnelInfo[i].vecDeviceDown[j].errorCode = errorCode;
				
				SeatBeatReply(extAddr);
				return;
			}			
		}		
	}	
}
bool CSIPCallDlg::IsCallRun(int extAddr)
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				if (m_pTunnelInfo[i].vecDeviceUp[j].callStatus == ENUM_DEVICE_STATUS_RUN)
				{
					return true;
				}
				else{
					return false;
				}
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				if (m_pTunnelInfo[i].vecDeviceDown[j].callStatus == ENUM_DEVICE_STATUS_RUN)
				{
					return true;
				}
				else {
					return false;
				}
			}
		}		
	}
	return false;
}
bool CSIPCallDlg::IsBCRun(int extAddr)
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				if (m_pTunnelInfo[i].vecDeviceUp[j].bcStatus == ENUM_DEVICE_STATUS_RUN)
				{
					return true;
				}
				else{
					return false;
				}
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				if (m_pTunnelInfo[i].vecDeviceDown[j].bcStatus == ENUM_DEVICE_STATUS_RUN)
				{
					return true;
				}
				else {
					return false;
				}
			}
		}		
	}
	return false;
}
void CSIPCallDlg::UpdateDoorRT(int extAddr, int flag, DWORD dwTime)
{
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr == extAddr)
			{
				
				m_pTunnelInfo[i].vecDeviceUp[j].doorRTTime = dwTime;
				m_pTunnelInfo[i].vecDeviceUp[j].doorRTFlag = flag;
				Status2Button();
				TRACE("door rt ....\r\n");
				return;
			}			
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr == extAddr)
			{
				
				m_pTunnelInfo[i].vecDeviceDown[j].doorRTTime = dwTime;
				m_pTunnelInfo[i].vecDeviceDown[j].doorRTFlag = flag;
				Status2Button();
				return;
			}			
		}		
	}
}
void CSIPCallDlg::Status2Button()
{
	int upSz, downSz;
	getCurDeviceNum(m_nCurTunnelIdx, upSz, downSz);
	// bc
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{			
			bool bZoom = IsNoCall(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip);
			Status2ButtonBc(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bcStatus, ((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i)), bZoom);			
			// tip
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp.begin() + m_nPageDevice + i;
			SDevice* pDevice = 	&(*vecIter);
			AddTipBC(pDevice, CHECK_BOX_BASEID_1 + i);
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RUN||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_OUTER_USING)
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->SetCheck(0);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->EnableWindow(FALSE);
			}
			else{
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->SetCheck(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].Check);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->EnableWindow(TRUE);
			}
		}		
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			bool bZoom = IsNoCall(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip);
			Status2ButtonBc(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bcStatus, ((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i)), bZoom);
			//((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].Check);	
			// tip
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown.begin() + m_nPageDevice + i;
			SDevice* pDevice = 	&(*vecIter);
			AddTipBC(pDevice, CHECK_BOX_BASEID_2 + i);
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RUN||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_OUTER_USING)
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(0);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->EnableWindow(FALSE);
			}
			else{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].Check);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->EnableWindow(TRUE);
			}
		}		
	}
	// call
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			Status2ButtonCall(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].callStatus, ((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i)), m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bOutTunnel);
		}		
	}
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			Status2ButtonCall(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].callStatus, ((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i)), m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bOutTunnel);
		}		
	}

	// status static 
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			
			Status2ErrorMsg(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].doorRTFlag, BTN_STATIC_BASEID_11 + i);
		}	
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			Status2ErrorMsg(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].doorRTFlag, BTN_STATIC_BASEID_21 + i);
		}		
	}
}
void CSIPCallDlg::Status2Button(int extAddr)
{
	int upSz, downSz;
	getCurDeviceNum(m_nCurTunnelIdx, upSz, downSz);
	// bc
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{			
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.extAddr != extAddr)
			{
				continue;
			}
			Status2ButtonBc(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bcStatus, ((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i)));			

			// tip
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp.begin() + m_nPageDevice + i;
			SDevice* pDevice = 	&(*vecIter);
			AddTipBC(pDevice, CHECK_BOX_BASEID_1 + i);
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RUN||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_OUTER_USING)
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->SetCheck(0);
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->EnableWindow(FALSE);
			}
			else{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->SetCheck(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].Check);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->EnableWindow(TRUE);
			}
			break;
		}		
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.extAddr != extAddr)
			{
				continue;
			}

			Status2ButtonBc(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bcStatus, ((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i)));
			//((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].Check);	
			// tip
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown.begin() + m_nPageDevice + i;
			SDevice* pDevice = 	&(*vecIter);
			AddTipBC(pDevice, CHECK_BOX_BASEID_2 + i);
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RUN||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].callStatus == ENUM_DEVICE_STATUS_RING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING||
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bcStatus == ENUM_DEVICE_STATUS_OUTER_USING)
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(0);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->EnableWindow(FALSE);
			}
			else{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].Check);			
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->EnableWindow(TRUE);
			}
			break;
		}		
	}
	// call
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.extAddr != extAddr)
			{
				continue;
			}
			Status2ButtonCall(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].callStatus, ((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i)), m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].bOutTunnel);
			break;
		}		
	}
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.extAddr != extAddr)
			{
				continue;
			}
			Status2ButtonCall(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].callStatus, ((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i)), m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].bOutTunnel);
			break;
		}		
	}

	// status static 
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.extAddr != extAddr)
			{
				continue;
			}
			Status2ErrorMsg(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].doorRTFlag, BTN_STATIC_BASEID_11 + i);
			break;
		}
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.extAddr != extAddr)
			{
				continue;
			}
			Status2ErrorMsg(m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].doorRTFlag, BTN_STATIC_BASEID_21 + i);
			break;
		}		
	}
}
void CSIPCallDlg::Status2ErrorMsg(int flag, UINT ID)
{
	if(flag == 1)
	{
		SetDlgItemText(ID,L"门被打开");
	}
	else{
		SetDlgItemText(ID,L"");
	}
}
void CSIPCallDlg::Status2ButtonBc(enDeviceStatus status, CButton* pButton, bool bZoom)
{
	if (bZoom)
	{
		CString strFile = _T("texture\\m4_80.bmp");
		switch(status){
		case ENUM_DEVICE_STATUS_OK:
			strFile = _T("texture\\m2_80.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_RUN:
			strFile = _T("texture\\m1_80.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_ERROR:
			strFile = _T("texture\\m2_80.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_UNKNOWN:
			strFile = _T("texture\\m4_80.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_REMOTE_USING:
			strFile = _T("texture\\m3_80.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		case ENUM_DEVICE_STATUS_OUTER_USING:
			strFile = _T("texture\\m5_80.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		}	
	}
	else{
		CString strFile = _T("texture\\m4_20.bmp");
		switch(status){
		case ENUM_DEVICE_STATUS_OK:
			strFile = _T("texture\\m2_20.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_RUN:
			strFile = _T("texture\\m1_20.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_ERROR:
			strFile = _T("texture\\m2_20.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_UNKNOWN:
			strFile = _T("texture\\m4_20.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_REMOTE_USING:
			strFile = _T("texture\\m3_20.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		case ENUM_DEVICE_STATUS_OUTER_USING:
			strFile = _T("texture\\m5_20.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		}	
	}
}
void CSIPCallDlg::Status2ButtonCall(enDeviceStatus status, CButton* pButton, bool bOut)
{
	if (bOut)
	{
		CString strFile = _T("texture\\call41_40.bmp");	
		switch(status){
		case ENUM_DEVICE_STATUS_OK:
			strFile = _T("texture\\call21_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_RUN:
			strFile = _T("texture\\call11_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_ERROR:
			strFile = _T("texture\\call31_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_UNKNOWN:
			strFile = _T("texture\\call41_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_RING:
			strFile = _T("texture\\call51_40.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		case ENUM_DEVICE_STATUS_REMOTE_USING:
			strFile = _T("texture\\call11_40.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		case ENUM_DEVICE_OM_DOWN:
			strFile = _T("texture\\call61_40.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		}
	}
	else{
		CString strFile = _T("texture\\call4_40.bmp");	
		switch(status){
		case ENUM_DEVICE_STATUS_OK:
			strFile = _T("texture\\call2_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_RUN:
			strFile = _T("texture\\call1_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_ERROR:
			strFile = _T("texture\\call3_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_UNKNOWN:
			strFile = _T("texture\\call4_40.bmp");
			SetButtonBMP(pButton, strFile);
			break;
		case ENUM_DEVICE_STATUS_RING:
			strFile = _T("texture\\call5_40.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		case ENUM_DEVICE_STATUS_REMOTE_USING:
			strFile = _T("texture\\call1_40.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		case ENUM_DEVICE_OM_DOWN:
			strFile = _T("texture\\call6_40.bmp");
			SetButtonBMP(pButton,strFile);
			break;
		}
	}
}
void CSIPCallDlg::OnCreateButtonClick(UINT nID)
{
	if(nID < BTN_CREATE_START || nID > BTN_CREATE_END)
	{
		return;
	}
	else
	{
		if (nID >= CHECK_BOX_BASEID_1 && nID < CHECK_BOX_BASEID_1 + BTN_NUM_MSG) // 上广播
		{
			m_pTunnelInfo[m_nCurTunnelIdx].
				vecDeviceUp[m_nPageDevice + nID - CHECK_BOX_BASEID_1].
				Check = ((CButton*)GetDlgItem(nID))->GetCheck();			
		}
		else if (nID >= CHECK_BOX_BASEID_2 && nID < CHECK_BOX_BASEID_2 + BTN_NUM_MSG)// 下广播
		{
			m_pTunnelInfo[m_nCurTunnelIdx].
				vecDeviceDown[m_nPageDevice + nID - CHECK_BOX_BASEID_2].
				Check = ((CButton*)GetDlgItem(nID))->GetCheck();		
		}
		else if (nID >= BTN_IMAGE_BASEID_1_0 && nID < BTN_IMAGE_BASEID_1_0 + BTN_NUM_CALL)// 上电话
		{
// 			if (g_Config.m_SystemConfig.nMainSystem)
// 			{
				// 插入电话
				m_MsgLock.Lock();
				if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + nID - BTN_IMAGE_BASEID_1_0].callStatus == ENUM_DEVICE_STATUS_OK ||
					m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + nID - BTN_IMAGE_BASEID_1_0].callStatus == ENUM_DEVICE_STATUS_ERROR)
				{
					SCallMsg msg;
					msg.extAddr = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + nID - BTN_IMAGE_BASEID_1_0].sDB.extAddr;
					msg.timestamp = GetTickCount();
					if(IsDeviceLocalController(GetDevice(msg.extAddr)->dwIP))
						msg.type = 1;
					else
						msg.type = 4;
					msg.status = ENUM_CALL_NONE;
					if (!IsExistCall(msg.extAddr))
					{
						m_vecCallMsg.push_back(msg);
						//SetTimer(TIMER_ID_CHECK_E1,TIMER_INTER_5000,0);
					}				
				}			
				m_MsgLock.Unlock();
//			}
		}
		else if (nID >= BTN_IMAGE_BASEID_2_0 && nID < BTN_IMAGE_BASEID_2_0 + BTN_NUM_CALL)// 下电话
		{
// 			if (g_Config.m_SystemConfig.nMainSystem)
// 			{
				// 插入电话
				m_MsgLock.Lock();
				if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + nID - BTN_IMAGE_BASEID_2_0].callStatus == ENUM_DEVICE_STATUS_OK ||
					m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + nID - BTN_IMAGE_BASEID_2_0].callStatus == ENUM_DEVICE_STATUS_ERROR||
					m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + nID -BTN_IMAGE_BASEID_2_0].callStatus == ENUM_DEVICE_STATUS_REMOTE_USING)
				{
					SCallMsg msg;
					msg.extAddr = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + nID - BTN_IMAGE_BASEID_2_0].sDB.extAddr;
					msg.timestamp = GetTickCount();
					if(IsDeviceLocalController(GetDevice(msg.extAddr)->dwIP))
						msg.type = 1;
					else
						msg.type = 4;
					if (!IsExistCall(msg.extAddr))
					{
						m_vecCallMsg.push_back(msg);
					}
				}			
				m_MsgLock.Unlock();
//			}
			
		}	
	}
}
void CSIPCallDlg::OnErrorMsgDBClick(UINT nID)
{
	if (nID >= BTN_STATIC_BASEID_11 && nID < BTN_STATIC_BASEID_11 + BTN_NUM_MSG){
		SetDlgItemText(nID, L"");
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + nID - BTN_STATIC_BASEID_11].doorRTFlag = 0;
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + nID - BTN_STATIC_BASEID_11].doorRTTime = 0;
	}
	else if (nID >= BTN_STATIC_BASEID_21 && nID < BTN_STATIC_BASEID_21 + BTN_NUM_MSG){
		SetDlgItemText(nID, L"");
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + nID - BTN_STATIC_BASEID_21].doorRTFlag = 0;
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + nID - BTN_STATIC_BASEID_21].doorRTTime = 0;
	}
}
bool CSIPCallDlg::IsExistCall(int extAddr)
{
	if (m_vecCallMsg.empty())
	{
		return false;
	}
	for (int i = 0; i < m_vecCallMsg.size(); i++)
	{
		if (m_vecCallMsg[i].extAddr == extAddr)
		{
			return true;
		}
	}
	return false;
}
void CSIPCallDlg::AnswerCall(int extAddr)
{	
	// 插入电话
	m_MsgLock.Lock();
	g_Log.output(LOG_TYPE, "AnswerCall: %d\r\n", extAddr);
	SCallMsg msg;
	msg.extAddr = extAddr;
	msg.timestamp = GetTickCount();
	msg.type = 2;
	msg.status = ENUM_CALL_NONE;
	//该分机未使用，OM(SIP代理)未离线
	if (!IsExistCall(extAddr)&&!g_bExitSipgw)
	{
		m_vecCallMsg.push_back(msg);
		//g_Log.output(LOG_TYPE, "AnswerCall: push_back: %d\r\n", extAddr);
	}
	if (/*false*/true)
	{
		SDevice* pDevice = GetDevice(extAddr);
		if (pDevice)
		{
			g_MSG.SendCallAnswer(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
		}
	}
	m_MsgLock.Unlock();
}
#pragma region ClickCall
#if 0
int CSIPCallDlg::ClickCall(SDevice* pDevice, int type)
{
	g_Log.output(LOG_TYPE, "ClickCall start............\r\n");	
	if(IsDeviceLocalController(pDevice->dwIP))
	{
		if ((m_CallA.status == ENUM_DEVICE_STATUS_OK) &&
			(m_CallB.status != ENUM_DEVICE_STATUS_OK))
		{				
			m_CallA.timestamp = GetTickCount();
			sprintf(m_CallA.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
			m_CallA.extAddr = pDevice->sDB.extAddr;			
		}
		else if ((m_CallB.status == ENUM_DEVICE_STATUS_OK) &&
			(m_CallA.status != ENUM_DEVICE_STATUS_OK))
		{			
			m_CallB.timestamp = GetTickCount();
			sprintf(m_CallB.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
			m_CallB.extAddr = pDevice->sDB.extAddr;					
		}
		else if ((m_CallB.status == ENUM_DEVICE_STATUS_OK) &&
			(m_CallA.status == ENUM_DEVICE_STATUS_OK))
		{
			// A			
			m_CallA.timestamp = GetTickCount();
			sprintf(m_CallA.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
			m_CallA.extAddr = pDevice->sDB.extAddr;

			// B		
			m_CallB.timestamp = GetTickCount();
			sprintf(m_CallB.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
			m_CallB.extAddr = pDevice->sDB.extAddr;			
		}
		else{
			return 0;
		}

#if DEBUG_FILE
		return 1;
#endif
		/*if(type == 2)
		{*/
		char* pIP = pDevice->sDB.ip;
		BYTE sub = pDevice->sDB.extAddr;
		char* pExtNo = pDevice->sDB.extNo;
		//pDevice->callStatus = ENUM_DEVICE_STATUS_RUN;
		if(IsBCRun(pDevice->sDB.extAddr))
		{
			DevicePause(pDevice);
			Sleep(300);
		}
		g_MSG.SendOpenCall(sub, pIP, g_Config.m_SystemConfig.portDevice);
		/*g_Log.output(LOG_TYPE,"bc status   ... %d\r\n",g_Play.status(pDevice->playParam));
		Sleep(1000);*/
		//}

		// 添加
		SCallEvt callEvt;
		strncpy(callEvt.extNo, pDevice->sDB.extNo, strlen(pDevice->sDB.extNo));	
		callEvt.extAddr = pDevice->sDB.extAddr;
		callEvt.dwTime = GetTickCount();
		// del 2017/08/13 m_vecCallEvt.push_back(callEvt);
		g_Log.output(LOG_TYPE, "m_vecCallEvt.push: %d\r\n", callEvt.extAddr);
		// 拨号
		CString str1, str2;
		char pSendBuf[1024] = {0};		
		if (type == 2){
			char* pGroupCall = GetGroup(pDevice->sDB.extAddr);
			if (pGroupCall)
			{
				sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Transfer attribute=\"Connect\">\r\n  <ext id=\"%s\"/>\r\n<outer to=\"%s\"/>\r\n</Transfer>", pDevice->sDB.extNo, pGroupCall);
			}
			else{
				sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Transfer attribute=\"Connect\">\r\n  <ext id=\"%s\"/>\r\n<outer to=\"%s\"/>\r\n</Transfer>", pDevice->sDB.extNo, g_Config.m_SystemConfig.Outer);
			}
		}
		else{
			sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Transfer attribute=\"Connect\">\r\n  <ext id=\"%s\"/>\r\n<outer to=\"%s\"/>\r\n</Transfer>", pDevice->sDB.extNo, g_Config.m_SystemConfig.Outer);
		}

		g_Log.output(LOG_TYPE, pSendBuf);
		TCHAR ipserver[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.IP_Server, -1, ipserver, MAX_PATH);
		CString IP_Server = _T("");
		IP_Server.Format(_T("http://%s"), ipserver/*g_Config.m_SystemConfig.IP_Server*/);
		if (g_Config.m_SystemConfig.bHttpConnect)
		{
			m_http.HTTP_POST_DATA(IP_Server, pSendBuf/*, str1, str2*/);
		}
		else{
			m_http.TCP_POST_DATA(/*g_Config.m_SystemConfig.IP_Server, g_Config.m_SystemConfig.portEvtSend, */pSendBuf);
		}

		s_callEvtTime = GetTickCount();

		if(!g_Config.m_SystemConfig.nMainSystem&&(type == 1||type == 2))
			g_MSG.SendDeviceCallStatus(pDevice->sDB.extAddr,pDevice->dwIP,1);
		return 1;
	}else 
	{
		if (IsBCRun(pDevice->sDB.extAddr))
		{
			DevicePause(pDevice);
			Sleep(300);
		}
		g_Log.output(LOG_TYPE,"localip:%s,,deviceip:%s\n",g_dwLocalIp,pDevice->dwIP);
		g_Log.output(LOG_TYPE,"bc status   ... %d\r\n",g_Play.status(pDevice->playParam));
		g_MSG.SendCallReq(pDevice->sDB.extAddr,pDevice->dwIP,1);		//发送请求通话指令
		return 0;
	}
}
#endif
#pragma endregion ClickCall
void CSIPCallDlg::DB2TunnelBtn()
{
	for(int i = 0; i < BTN_NUM_TUNNEL; i++){
		if (m_nPageTunnel + i < m_nTunnelNum)
		{
			CString strName = L"";
			TCHAR wName[1024] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[m_nPageTunnel + i].Name, -1, wName, 1024);

			SetDlgItemText(IDC_BUTTON1 + i, wName);
			((CButton*)GetDlgItem(IDC_BUTTON1 + i))->ShowWindow(SW_SHOW);
		}
		else{
			((CButton*)GetDlgItem(IDC_BUTTON1 + i))->ShowWindow(SW_HIDE);
		}
	}
}
int CSIPCallDlg::GetDeviceCol(int idx, int up, int down)
{
	int col = 0;
	int maxCol = max(up, down);
	int colNum = maxCol - idx;
	if (colNum > BTN_NUM_CALL)
	{
		return BTN_NUM_CALL;
	}
	else{
		return colNum;
	}
}
void CSIPCallDlg::SetDevicePos(int yoffset, int id, int idx, int col, int ctrlW)
{		
	if (idx >= col)
	{
		return;
	}
	CWnd* pWnd = GetDlgItem(id);
	if (pWnd)
	{
		CRect rect;
		pWnd->GetWindowRect(&rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;
		
		CRect left;
		GetDlgItem(BTN_IMAGE_PRE)->GetWindowRect(&left);
		
		CRect right;
		GetDlgItem(BTN_IMAGE_NEXT)->GetWindowRect(&right);
		
		int allW = right.left - left.right;
#if 0
		int nInter = (allW - col * w) / (col + 1);		
		pWnd->SetWindowPos( NULL, left.right + nInter + idx * (nInter + w), yoffset, w, h, SWP_NOZORDER );
#else
		int padW = 30;
		float nInter = 0;
		if (col > 1)
		{
			nInter = (allW - col * ctrlW - 2* padW) * 1.0 / (col - 1);
		}
		int xCoord = left.right + idx * (nInter + ctrlW) + padW;
		if ((id >= BTN_STATIC_BASEID_KM1) && (id <= BTN_STATIC_BASEID_KM1 + BTN_NUM_CALL) ||
			(id >= BTN_STATIC_BASEID_KM2) && (id <= BTN_STATIC_BASEID_KM2 + BTN_NUM_CALL) ||
			(id >= BTN_STATIC_BASEID_1) && (id <= BTN_STATIC_BASEID_1 + BTN_NUM_CALL) ||
			(id >= BTN_STATIC_BASEID_2) && (id <= BTN_STATIC_BASEID_2 + BTN_NUM_CALL) ||
			(id >= BTN_STATIC_BASEID_11) && (id <= BTN_STATIC_BASEID_11 + BTN_NUM_CALL) ||
			(id >= BTN_STATIC_BASEID_21) && (id <= BTN_STATIC_BASEID_21 + BTN_NUM_CALL))
		{
			int xw = w / 2 - ctrlW / 2;
			xCoord -= xw;
		}
		
		pWnd->SetWindowPos( NULL, xCoord, yoffset, w, h, SWP_NOZORDER );
#endif
		
	}
}
void CSIPCallDlg::AddTipBC(SDevice* pDevice, int nCtrlID)
{
	if(pDevice == NULL){
		return;
	}
	// tip
	wchar_t strTip[256] = {0};
	enBCOptStatus opt = pDevice->IsBcPlay();
	if (opt == ENUM_BC_OPT_REMOTE)
	{
		MultiByteToWideChar(CP_ACP, 0, "远程音源", -1, strTip, 256);
	}
	else if (opt == ENUM_BC_OPT_LOCAL)
	{
		MultiByteToWideChar(CP_ACP, 0, PlaySrc2String(*pDevice), -1, strTip, 256);
	}

	m_Mytip.AddTool( GetDlgItem(nCtrlID),  strTip); 
}
void CSIPCallDlg::DB2DeviceBtn(int nCurTunnelIdx)
{
	if (nCurTunnelIdx < 0 || nCurTunnelIdx >= m_nTunnelNum)
	{
		return;
	}
	int upSz, downSz;
	getCurDeviceNum(nCurTunnelIdx, upSz, downSz);
	
	
	wchar_t wName[128] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].Name, -1, wName, 128);
	CString strName = _T("");
	strName.Format(_T("当前隧道：%s"), wName);
	((CButton*)GetDlgItem(IDC_STATIC_CUR_TUNNEL))->SetWindowText(strName);
	
	// 是否包含没有电话
	bool bNoCall = false;
	// 计算排布几个按钮
	int countCol = GetDeviceCol(m_nPageDevice, upSz, downSz);
	//call
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_nPageDevice + i < upSz)
		{		
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i))->ShowWindow(SW_HIDE);
				continue;
			}
			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip))
			{
				((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i))->ShowWindow(SW_HIDE);
				bNoCall = true;
			}
			else{
				((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i))->ShowWindow(SW_SHOW);
				SetDevicePos(TOP_CALL_UP, BTN_IMAGE_BASEID_1_0 + i, i, countCol);
				// tip
				wchar_t strTip[128] = {0};
				MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.desc, -1, strTip, 128);
				m_Mytip.AddTool( GetDlgItem(BTN_IMAGE_BASEID_1_0 + i),  strTip); 
			}
						
		}
		else{
			((CButton*)GetDlgItem(BTN_IMAGE_BASEID_1_0 + i))->ShowWindow(SW_HIDE);
		}
	}
	for (int i = 0; i < BTN_NUM_CALL; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i))->ShowWindow(SW_HIDE);
				continue;
			}
			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip))
			{
				((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i))->ShowWindow(SW_HIDE);
				bNoCall = true;
			}
			else{
				((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i))->ShowWindow(SW_SHOW);
				SetDevicePos(TOP_CALL_DOWN, BTN_IMAGE_BASEID_2_0 + i, i, countCol);
				// tip
				wchar_t strTip[128] = {0};
				MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.desc, -1, strTip, 128);
				m_Mytip.AddTool( GetDlgItem(BTN_IMAGE_BASEID_2_0 + i),  strTip); 
			}		
		}
		else{
			((CButton*)GetDlgItem(BTN_IMAGE_BASEID_2_0 + i))->ShowWindow(SW_HIDE);
		}
	}
	int upcheck = 1;
	if (upSz <= 0)
	{
		upcheck = 0;
	}
	//broadcast
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->ShowWindow(SW_HIDE);
				continue;
			}
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->SetCheck(
				m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].Check);
		
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->ShowWindow(SW_SHOW);
			if (m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].Check == 0)
			{
				upcheck = 0;
			}
			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip))
			{
				// 设置大图标				
				GetDlgItem(CHECK_BOX_BASEID_1 + i)->SetWindowPos(NULL, 0, 0, 100, 80, SWP_NOZORDER );
				SetDevicePos(TOP_BC_UP_BC, CHECK_BOX_BASEID_1 + i, i, countCol, CTRL_BTN_BC_W);
			}
			else{
				// 小图标
				GetDlgItem(CHECK_BOX_BASEID_1 + i)->SetWindowPos(NULL, 0, 0, 40, 20, SWP_NOZORDER );
				SetDevicePos(TOP_BC_UP, CHECK_BOX_BASEID_1 + i, i, countCol);
			}
		
			// 没有广播的隐藏显示
			if (IsNoBC(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip))
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->ShowWindow(SW_HIDE);
			}
			else{
				// tip
				wchar_t strTip[256] = {0};
				enBCOptStatus opt = m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].IsBcPlay();
				if (opt == ENUM_BC_OPT_REMOTE)
				{
					MultiByteToWideChar(CP_ACP, 0, "远程音源", -1, strTip, 256);
				}
				else if (opt == ENUM_BC_OPT_LOCAL)
				{
					MultiByteToWideChar(CP_ACP, 0, PlaySrc2String(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i]), -1, strTip, 256);
				}
				
				m_Mytip.AddTool( GetDlgItem(CHECK_BOX_BASEID_1 + i),  strTip); 
			}
		}
		else{
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_1 + i))->ShowWindow(SW_HIDE);
			
		}
	}
	int downcheck = 1;
	if (downSz <= 0)
	{
		downcheck = 0;
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->ShowWindow(SW_HIDE);
				continue;
			}
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->SetCheck(
				m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].Check);
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->ShowWindow(SW_SHOW);
			if (m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].Check == 0)
			{
				downcheck = 0;
			}
			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip))
			{
				// 设置大图标				
				GetDlgItem(CHECK_BOX_BASEID_2 + i)->SetWindowPos(NULL, 0, 0, 100, 80, SWP_NOZORDER );
				//SetDevicePos(TOP_BC_DOWN_BC, CHECK_BOX_BASEID_2 + i, i, countCol);
				SetDevicePos(TOP_BC_DOWN_BC, CHECK_BOX_BASEID_2 + i, i, countCol, CTRL_BTN_BC_W);
			}
			else{
				// 小图标
				GetDlgItem(CHECK_BOX_BASEID_2 + i)->SetWindowPos(NULL, 0, 0, 40, 20, SWP_NOZORDER );
				SetDevicePos(TOP_BC_DOWN, CHECK_BOX_BASEID_2 + i, i, countCol);
			}
			//SetDevicePos(TOP_BC_DOWN, CHECK_BOX_BASEID_2 + i, i, countCol);
			// 没有广播的隐藏显示
			if (IsNoBC(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip))
			{
				((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->ShowWindow(SW_HIDE);
			}
			else{
				// tip
				wchar_t strTip[256] = {0};
				enBCOptStatus opt = m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].IsBcPlay();
				if (opt == ENUM_BC_OPT_REMOTE)
				{
					MultiByteToWideChar(CP_ACP, 0, "远程音源", -1, strTip, 256);
				}
				else if (opt == ENUM_BC_OPT_LOCAL)
				{
					MultiByteToWideChar(CP_ACP, 0, PlaySrc2String(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i]), -1, strTip, 256);
				}
				m_Mytip.AddTool( GetDlgItem(CHECK_BOX_BASEID_2 + i),  strTip); 
			}
		}
		else{
			((CButton*)GetDlgItem(CHECK_BOX_BASEID_2 + i))->ShowWindow(SW_HIDE);
		}
	}
	// static	
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_1 + i))->ShowWindow(SW_HIDE);
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM1 + i))->ShowWindow(SW_HIDE);				
				continue;
			}

			wchar_t wCharName[256] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.Name, -1, wCharName, 256);
			SetDlgItemText(BTN_STATIC_BASEID_1 + i, wCharName);

			wchar_t wzKMName[256] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.kmID, -1, wzKMName, 256);
			SetDlgItemText(BTN_STATIC_BASEID_KM1 + i, wzKMName);

			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip))
			{
				GetDlgItem(BTN_STATIC_BASEID_1 + i)->SetWindowPos(NULL, 0, 0, 100, 20, SWP_NOZORDER );
				SetDevicePos(TOP_NAME_UP_BC, BTN_STATIC_BASEID_1 + i, i, countCol, CTRL_BTN_BC_W);
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM1 + i))->ShowWindow(SW_HIDE);
			}
			else{
				GetDlgItem(BTN_STATIC_BASEID_1 + i)->SetWindowPos(NULL, 0, 0, 50, 20, SWP_NOZORDER );
				SetDevicePos(TOP_NAME_UP, BTN_STATIC_BASEID_1 + i, i, countCol);
				SetDevicePos(TOP_KM_UP, BTN_STATIC_BASEID_KM1 + i, i, countCol);
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM1 + i))->ShowWindow(SW_SHOW);
			}
	
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_1 + i))->ShowWindow(SW_SHOW);	
		}
		else{
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_1 + i))->ShowWindow(SW_HIDE);
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM1 + i))->ShowWindow(SW_HIDE);
			
		}
	}
	// pe
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{			
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(IDC_PIC_PE_1 + i))->ShowWindow(SW_HIDE);							
				continue;
			}
			if (IsExistPE(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip))
			{
				

				CButton* pButton = (CButton*)GetDlgItem(IDC_PIC_PE_1 + i);
				if (pButton)
				{
					pButton->ShowWindow(SW_SHOW);
					pButton->SetWindowPos(NULL, 0, 0, 40, 20, SWP_NOZORDER );
					SetDevicePos(TOP_PERSON_UP, IDC_PIC_PE_1 + i, i, countCol);

					//
					HBITMAP hbitmapB = m_MgrPic.GetBitmap(_T("texture\\line_u.png"));
					if (hbitmapB != NULL)
					{						
						m_pIMG_PE_U[i].SetBitmap(hbitmapB);
					}
				}
			}
			else{
				((CButton*)GetDlgItem(IDC_PIC_PE_1 + i))->ShowWindow(SW_HIDE);
			}
		}
		else{
			((CButton*)GetDlgItem(IDC_PIC_PE_1 + i))->ShowWindow(SW_HIDE);
		}
	}

	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_2 + i))->ShowWindow(SW_HIDE);
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM2 + i))->ShowWindow(SW_HIDE);		
				continue;
			}

			wchar_t wCharName[256] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.Name, -1, wCharName, 256);
			SetDlgItemText(BTN_STATIC_BASEID_2 + i, wCharName);
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_2 + i))->ShowWindow(SW_SHOW);

			wchar_t wzKMName[256] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.kmID, -1, wzKMName, 256);
			SetDlgItemText(BTN_STATIC_BASEID_KM2 + i, wzKMName);
			
			//SetDevicePos(TOP_NAME_DOWN, BTN_STATIC_BASEID_2 + i, i, countCol);
			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip))
			{
				GetDlgItem(BTN_STATIC_BASEID_2 + i)->SetWindowPos(NULL, 0, 0, 100, 20, SWP_NOZORDER );
				SetDevicePos(TOP_NAME_DOWN_BC, BTN_STATIC_BASEID_2 + i, i, countCol, CTRL_BTN_BC_W);
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM2 + i))->ShowWindow(SW_HIDE);
			}
			else{
				GetDlgItem(BTN_STATIC_BASEID_2 + i)->SetWindowPos(NULL, 0, 0, 50, 20, SWP_NOZORDER );
				SetDevicePos(TOP_NAME_DOWN, BTN_STATIC_BASEID_2 + i, i, countCol);
				SetDevicePos(TOP_KM_DOWN, BTN_STATIC_BASEID_KM2 + i, i, countCol);
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM2 + i))->ShowWindow(SW_SHOW);
			}
		}
		else{
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_2 + i))->ShowWindow(SW_HIDE);
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_KM2 + i))->ShowWindow(SW_HIDE);
		}
	}
	// status static 
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_11 + i))->ShowWindow(SW_HIDE);
				continue;
			}

			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].sDB.ip))
			{
				GetDlgItem(BTN_STATIC_BASEID_11 + i)->SetWindowPos(NULL, 0, 0, 100, 20, SWP_NOZORDER );
				SetDevicePos(TOP_ERROR_UP_BC, BTN_STATIC_BASEID_11 + i, i,countCol, CTRL_BTN_BC_W);
			}
			else{
				GetDlgItem(BTN_STATIC_BASEID_11 + i)->SetWindowPos(NULL, 0, 0, 45, 15, SWP_NOZORDER );
				SetDevicePos(TOP_ERROR_UP, BTN_STATIC_BASEID_11 + i, i,countCol);
			}
			//SetDevicePos(TOP_ERROR_UP, BTN_STATIC_BASEID_11 + i, i,countCol);
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_11 + i))->ShowWindow(SW_SHOW);
		}
		else{
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_11 + i))->ShowWindow(SW_HIDE);
		}
	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{	
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(BTN_STATIC_BASEID_21 + i))->ShowWindow(SW_HIDE);
				continue;
			}
#if 0
			SetDevicePos(TOP_ERROR_DOWN, BTN_STATIC_BASEID_21 + i, i, countCol);
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_21 + i))->ShowWindow(SW_SHOW);
#else
			if (IsNoCall(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip))
			{
				GetDlgItem(BTN_STATIC_BASEID_21 + i)->SetWindowPos(NULL, 0, 0, 100, 20, SWP_NOZORDER );
				SetDevicePos(TOP_ERROR_DOWN_BC, BTN_STATIC_BASEID_21 + i, i,countCol, CTRL_BTN_BC_W);
			}
			else{
				GetDlgItem(BTN_STATIC_BASEID_21 + i)->SetWindowPos(NULL, 0, 0, 45, 15, SWP_NOZORDER );
				SetDevicePos(TOP_ERROR_DOWN, BTN_STATIC_BASEID_21 + i, i,countCol);
			}
			//SetDevicePos(TOP_ERROR_UP, BTN_STATIC_BASEID_11 + i, i,countCol);
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_21 + i))->ShowWindow(SW_SHOW);
#endif
		}
		else{
			((CButton*)GetDlgItem(BTN_STATIC_BASEID_21 + i))->ShowWindow(SW_HIDE);
		}
	}
	// pe
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{			
			if (!m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.IsVisible())
			{
				((CButton*)GetDlgItem(IDC_PIC_PE_51 + i))->ShowWindow(SW_HIDE);
				continue;
			}
			if (IsExistPE(m_pTunnelInfo[nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].sDB.ip))
			{
				CButton* pButton = (CButton*)GetDlgItem(IDC_PIC_PE_51 + i);
				if (pButton)
				{
					pButton->ShowWindow(SW_SHOW);
					pButton->SetWindowPos(NULL, 0, 0, 40, 20, SWP_NOZORDER );
					SetDevicePos(TOP_PERSON_DOWN, IDC_PIC_PE_51 + i, i, countCol);
					//
					HBITMAP hbitmapB = m_MgrPic.GetBitmap(_T("texture\\line_d.png"));
					if (hbitmapB != NULL)
					{						
						m_pIMG_PE_D[i].SetBitmap(hbitmapB);
					}
				}
			}
			else{
				((CButton*)GetDlgItem(IDC_PIC_PE_51 + i))->ShowWindow(SW_HIDE);
			}
		}
		else{
			((CButton*)GetDlgItem(IDC_PIC_PE_51 + i))->ShowWindow(SW_HIDE);
		}
	}

	int arrowTop = TOP_ARROW_UP;
	if (bNoCall)
	{
		arrowTop = TOP_ARROW_UP_BC;
		s_pic_bk_y = TOP_PERSON_UP_BC + 20 + 2 ;		
		s_pic_bk_h = TOP_PERSON_DOWN_BC - s_pic_bk_y - 2;
	}
	else{
		s_pic_bk_y = TOP_PERSON_UP + 20 + 2;
		s_pic_bk_h = TOP_PERSON_DOWN - s_pic_bk_y - 2;
	}
	{// change
		CRect posRect;
		int picW = 178;
		int picH = 80;
// 		if (bNoCall)
// 		{
// 			picH = 110;
// 		}
		int padW = 10;
		int padH = 10;		
		int btnInter = 10;
		int btnPreW = 30;
		int XCoord = padW;
		int YCoord = padH + 110;
		int XCoordPre = XCoord + padW;
		int YCoordPre = YCoord + 30;
		GetDlgItem(IDC_PIC_ARROW_LEFT)->GetWindowRect(&posRect);
		m_PicArrowL.SetWindowPos(NULL, XCoordPre +  btnPreW + btnInter, arrowTop, picW, picH, SWP_NOZORDER );

		GetDlgItem(IDC_PIC_ARROW_RIGHT)->GetWindowRect(&posRect);
		m_PicArrowR.SetWindowPos(NULL, m_width - 2 * padW - btnPreW - padW - picW, arrowTop, picW, picH, SWP_NOZORDER );

		GetDlgItem(IDC_STATIC_SIGN_LEFT)->GetWindowRect(&posRect);
		GetDlgItem(IDC_STATIC_SIGN_LEFT)->SetWindowPos(NULL, s_pic_bk_x + s_pic_bk_w * 0.1+2/*XCoordPre +  btnPreW + btnInter + padW + picW*/, arrowTop, picW, picH, SWP_NOZORDER );


		GetDlgItem(IDC_STATIC_SIGN_RIGHT)->GetWindowRect(&posRect);
		GetDlgItem(IDC_STATIC_SIGN_RIGHT)->SetWindowPos(NULL, s_pic_bk_x + s_pic_bk_w * 0.9-2-picW/*m_width - 2 * padW - btnPreW - 2* (padW + picW)*/, arrowTop, picW, picH, SWP_NOZORDER );

				
		GetDlgItem(IDC_STATIC_TUNNEL_TITLE)->SetWindowPos(NULL, m_width / 8 * 3, arrowTop, m_width / 4, picH, SWP_NOZORDER );

		/*TCHAR wzSignL[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[m_nCurTunnelIdx].SignL, -1, wzSignL, MAX_PATH);
		TCHAR wzSignR[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[m_nCurTunnelIdx].SignR, -1, wzSignR, MAX_PATH);*/
		GetDlgItem(IDC_STATIC_SIGN_LEFT)->SetWindowText(m_pTunnelInfo[m_nCurTunnelIdx].SignL);
		GetDlgItem(IDC_STATIC_SIGN_RIGHT)->SetWindowText(m_pTunnelInfo[m_nCurTunnelIdx].SignR);
	}
	Status2Button();
	// send bc check
	if (upcheck == 1)
	{
		((CButton*)GetDlgItem(IDC_CHECK_UP_ALL))->SetCheck(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_CHECK_UP_ALL))->SetCheck(0);
	}
	if (downcheck == 1)
	{
		((CButton*)GetDlgItem(IDC_CHECK_DOWN_ALL))->SetCheck(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_CHECK_DOWN_ALL))->SetCheck(0);
	}

	if (upcheck == 1 &&
		downcheck == 1)
	{
		((CButton*)GetDlgItem(IDC_CHECK_SEND_BC))->SetCheck(1);
	}
	else{
		((CButton*)GetDlgItem(IDC_CHECK_SEND_BC))->SetCheck(0);
	}
	
#if 0

#else
	Invalidate(FALSE);
#endif
}
bool CSIPCallDlg::IsDeviceOK(int curTunnelIdx)
{	
	for (int i = 0; i < m_pTunnelInfo[curTunnelIdx].vecDeviceUp.size(); i++)
	{
		if (m_pTunnelInfo[curTunnelIdx].vecDeviceUp[i].Check && 
			(true/*m_pTunnelInfo[curTunnelIdx].vecDeviceUp[i].status == ENUM_DEVICE_STATUS_OK
			||m_pTunnelInfo[curTunnelIdx].vecDeviceUp[i].status == ENUM_DEVICE_STATUS_RUN*/))
		{
			return true;
		}
	}
	for (int i = 0; i < m_pTunnelInfo[curTunnelIdx].vecDeviceDown.size(); i++)
	{
		if (m_pTunnelInfo[curTunnelIdx].vecDeviceDown[i].Check	&& 
			(true/*m_pTunnelInfo[curTunnelIdx].vecDeviceDown[i].status == ENUM_DEVICE_STATUS_OK ||
			m_pTunnelInfo[curTunnelIdx].vecDeviceDown[i].status == ENUM_DEVICE_STATUS_RUN*/))
		{
			return true;
		}
	}

	return false;
}
int CSIPCallDlg::IsCallReady()
{
	if (m_CallA.status == ENUM_DEVICE_STATUS_UNKNOWN && 
		m_CallB.status == ENUM_DEVICE_STATUS_UNKNOWN)
	{
		return 0; //不在线
	}
	else if (m_CallA.status == ENUM_DEVICE_STATUS_RUN && 
		m_CallB.status == ENUM_DEVICE_STATUS_RUN)
	{
		return 1;// 忙
	}	
	else if ((m_CallA.status == ENUM_DEVICE_STATUS_OK ) &&
		     (m_CallB.status != ENUM_DEVICE_STATUS_OK))
	{
		return 2; // A
	}
	else if ((m_CallB.status == ENUM_DEVICE_STATUS_OK) &&
			 (m_CallA.status != ENUM_DEVICE_STATUS_OK))
	{
		return 3; // B
	}
	else if ((m_CallB.status == ENUM_DEVICE_STATUS_OK) &&
			 (m_CallA.status == ENUM_DEVICE_STATUS_OK))
	{
		return 4; // A && B
	}
	else{ //故障
		return -1;
	}
}
void CSIPCallDlg::Call2Btn()
{
	// A	
	if (m_CallA.status == ENUM_DEVICE_STATUS_RUN)
	{
		wchar_t wName[256] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_CallA.szName, -1, wName, 256);
		SetDlgItemText(IDC_STATIC_NAMA1, wName/*m_CallA.Name*/);

		CString strTime = L"";
		long espTime = (GetTickCount() - m_CallA.timestamp) / 1000;
		strTime.Format(L"%02d:%02d", espTime / 60, espTime % 60);
		SetDlgItemText(IDC_STATIC_TIME1, strTime);		
	}
	else if (m_CallA.status == ENUM_DEVICE_STATUS_RING)
	{		
		wchar_t wName[256] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_CallA.szName, -1, wName, 256);
		SetDlgItemText(IDC_STATIC_NAMA1, wName/*m_CallA.Name*/);

		SetDlgItemText(IDC_STATIC_TIME1, L"呼叫中....");		
	}
	else if (m_CallA.status == ENUM_DEVICE_STATUS_OK)
	{		
		//SetDlgItemText(IDC_STATIC_NAMA1, L"ZTE01:192.168.100.100");
		SetDlgItemText(IDC_STATIC_NAMA1, L"");
		SetDlgItemText(IDC_STATIC_TIME1, L"空闲中....");
	}
	else{
		SetDlgItemText(IDC_STATIC_NAMA1, L"");
		SetDlgItemText(IDC_STATIC_TIME1, L"");
	}
	CString strSrcA = _T("texture\\call1.bmp");
	if(m_CallA.status == ENUM_DEVICE_STATUS_RUN){
		strSrcA = _T("texture\\call1.bmp");		
	}
	else if(m_CallA.status == ENUM_DEVICE_STATUS_OK){
		strSrcA = _T("texture\\call2.bmp");		
	}
	else if (m_CallA.status == ENUM_DEVICE_STATUS_RING)
	{
		strSrcA = _T("texture\\call5.bmp");
	}
	else{
		strSrcA = _T("texture\\call4.bmp");		
	}

	HBITMAP hbitmapA = m_MgrPic.GetBitmap(strSrcA);
	if (hbitmapA != NULL)
	{
		m_PictureA.SetBitmap(hbitmapA);
	}

	// B
	if (m_CallB.status == ENUM_DEVICE_STATUS_RUN)
	{
		wchar_t wName[256] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_CallB.szName, -1, wName, 256);
		SetDlgItemText(IDC_STATIC_NAMA2, wName/*m_CallB.Name*/);

		CString strTime = L"";
		long espTime = (GetTickCount() - m_CallB.timestamp) / 1000;
		strTime.Format(L"%02d:%02d", espTime / 60, espTime % 60);
		SetDlgItemText(IDC_STATIC_TIME2, strTime);
	}
	else if (m_CallB.status == ENUM_DEVICE_STATUS_RING)
	{		
		wchar_t wName[256] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_CallB.szName, -1, wName, 256);
		SetDlgItemText(IDC_STATIC_NAMA2, wName/*m_CallB.Name*/);

		SetDlgItemText(IDC_STATIC_TIME2, L"呼叫中....");
	}
	else if (m_CallB.status == ENUM_DEVICE_STATUS_OK)
	{		
		//SetDlgItemText(IDC_STATIC_NAMA2, L"ZTE01:192.168.100.100");
		SetDlgItemText(IDC_STATIC_NAMA2, L"");
		SetDlgItemText(IDC_STATIC_TIME2, L"空闲中....");
	}
	else{
		SetDlgItemText(IDC_STATIC_NAMA2, L"");
		SetDlgItemText(IDC_STATIC_TIME2, L"");
	}
	CString strSrcB = _T("texture\\call1.bmp");
	if(m_CallB.status == ENUM_DEVICE_STATUS_RUN){
		strSrcB = _T("texture\\call1.bmp");		
	}
	else if(m_CallB.status == ENUM_DEVICE_STATUS_OK){
		strSrcB = _T("texture\\call2.bmp");		
	}
	else if(m_CallB.status == ENUM_DEVICE_STATUS_RING){
		strSrcB = _T("texture\\call5.bmp");		
	}
	else{
		strSrcB = _T("texture\\call4.bmp");		
	}
	HBITMAP hbitmapB = m_MgrPic.GetBitmap(strSrcB);
	if (hbitmapB != NULL)
	{
		m_PictureB.SetBitmap(hbitmapB);
	}
}
void CSIPCallDlg::InitAPPPath()
{
	//memset(m_strAPPPath, 0, MAX_PATH * sizeof(char));
	TCHAR szPath[MAX_PATH];   
	GetModuleFileName(NULL, szPath, MAX_PATH);
	
	CString PathName(szPath);
	g_szAPPPath = g_ttsFilePath = PathName.Left(PathName.ReverseFind(_T('\\')) + 1);

	CreateDirectory(g_szAPPPath + L"music", NULL);
	CreateDirectory(g_szAPPPath + L"record", NULL);
	CreateDirectory(g_szAPPPath + L"help", NULL);
	CreateDirectory(g_szAPPPath + L"event", NULL);

	g_ttsFilePath += _T("output.wav");
	//WideCharToMultiByte(CP_ACP, 0, strFile, -1, m_strAPPPath, MAX_PATH, NULL, NULL );
}
void  CSIPCallDlg::OnResetApp()
{
	//ExitProcess(-1);
	SendMessage(WM_CLOSE);
	ShellExecute( NULL, _T("open"), g_szAPPPath + ("Launch.exe"), NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数

	//
	return;
	PROCESS_INFORMATION   info;
	STARTUPINFO startup;
	wchar_t szPath[128];
	wchar_t *szCmdLine;
	GetModuleFileName(AfxGetApp()-> m_hInstance,   szPath,   sizeof(szPath));
	szCmdLine   =   GetCommandLine();
	GetStartupInfo(&startup);
	BOOL   bSucc   =   CreateProcess(szPath,   szCmdLine,   NULL,   NULL,
		FALSE,   NORMAL_PRIORITY_CLASS,   NULL,   NULL,   &startup,   &info);
	if(bSucc)
	{
		CWnd *pWnd   =   AfxGetMainWnd();
		if(pWnd   !=   NULL)
		{
			pWnd-> PostMessage(WM_CLOSE,   0,   0);
		}
		else{
			ExitProcess(-1);
		}
	}
	else{
		ExitProcess(-1);
	}
}
static bool s_play = false;
BOOL CSIPCallDlg::UpdateSysTime()
{
	// 这个IP是中国大陆时间同步服务器地址，可自行修改
	SOCKET soc=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	struct sockaddr_in addrSrv;
	addrSrv.sin_addr.S_un.S_addr=inet_addr("210.72.145.44");
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(123);

	NTP_Packet NTP_Send,NTP_Recv; 
	NTP_Send.Control_Word   =   htonl(0x0B000000);   
	NTP_Send.root_delay        =   0;   
	NTP_Send.root_dispersion   =   0;   
	NTP_Send.reference_identifier    =   0;   
	NTP_Send.reference_timestamp    =   0;   
	NTP_Send.originate_timestamp    =   0;   
	NTP_Send.receive_timestamp        =   0;   
	NTP_Send.transmit_timestamp_seconds        =   0;   
	NTP_Send.transmit_timestamp_fractions   =   0; 
	if(SOCKET_ERROR==sendto(soc,(const char*)&NTP_Send,sizeof(NTP_Send),
		0,(struct sockaddr*)&addrSrv,sizeof(addrSrv)))
	{
		closesocket(soc);
		return FALSE;
	}
	int sockaddr_Size =sizeof(addrSrv);
	if(SOCKET_ERROR==recvfrom(soc,(char*)&NTP_Recv,sizeof(NTP_Recv),
		0,(struct sockaddr*)&addrSrv,&sockaddr_Size))
	{
		closesocket(soc);
		return FALSE;
	}
	closesocket(soc);
	

	SYSTEMTIME    newtime;
	float        Splitseconds;
	struct        tm    *lpLocalTime;
	time_t        ntp_time;

	// 获取时间服务器的时间
	ntp_time    = ntohl(NTP_Recv.transmit_timestamp_seconds)-2208988800;
	lpLocalTime = localtime(&ntp_time);
	if(lpLocalTime == NULL)
	{
		return FALSE;
	}

	// 获取新的时间
	newtime.wYear      =lpLocalTime->tm_year+1900;
	newtime.wMonth     =lpLocalTime->tm_mon+1;
	newtime.wDayOfWeek =lpLocalTime->tm_wday;
	newtime.wDay       =lpLocalTime->tm_mday;
	newtime.wHour      =lpLocalTime->tm_hour;
	newtime.wMinute    =lpLocalTime->tm_min;
	newtime.wSecond    =lpLocalTime->tm_sec;

	// 设置时间精度
	Splitseconds=(float)ntohl(NTP_Recv.transmit_timestamp_fractions);
	Splitseconds=(float)0.000000000200 * Splitseconds;
	Splitseconds=(float)1000.0 * Splitseconds;
	newtime.wMilliseconds   =   (unsigned   short)Splitseconds;

	// 修改本机系统时间
	//SetLocalTime(&newtime);
	return TRUE;
}
BOOL CSIPCallDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_MOUSEMOVE){
		m_Mytip.RelayEvent(pMsg);
	}
	if(pMsg->message==WM_LBUTTONDBLCLK)
	{
		int nBtnID =  GetWindowLong(pMsg->hwnd, GWL_ID); 
		if(nBtnID >= BTN_STATIC_BASEID_11 && nBtnID<= BTN_CREATE_END)
		{
			OnErrorMsgDBClick(nBtnID) ;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
static bool s_bDebug = true;
UINT CallStatusProc(LPVOID pParam){
	CSIPCallDlg* pApp = (CSIPCallDlg*)pParam;
	if (pApp == NULL)
	{
		return 0;
	}
	while (s_bDebug)
	{
		pApp->GetCallStatus(g_Config.m_SystemConfig.CallA);
		pApp->GetCallStatus(g_Config.m_SystemConfig.CallB);
	}
	return 1;
}
enDeviceStatus CSIPCallDlg::GetCallStatus(char* id)
{
#if DEBUG_MEMORY
	return ENUM_DEVICE_STATUS_OK;
#endif

#if 1
	CString str1, str2;
	char pSendBuf[1024] = {0};			

	sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Control attribute=\"Query\">\r\n  <ext id=\"%s\"/>\r\n</Control>", id);
	g_Log.output(LOG_TYPE, "OnMenuCallStatus....(%s)\r\n", id);
	TCHAR ipserver[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.IP_Server, -1, ipserver, MAX_PATH);
	CString IP_Server = _T("");
	IP_Server.Format(_T("http://%s"), ipserver/*g_Config.m_SystemConfig.IP_Server*/);
	if (g_Config.m_SystemConfig.bHttpConnect)
	{
		m_http.HTTP_POST_DATA(IP_Server, pSendBuf/*, str1, str2*/);
	}
	else{
		m_http.TCP_POST_DATA(/*g_Config.m_SystemConfig.IP_Server, g_Config.m_SystemConfig.portEvtSend,*/ pSendBuf);
	}
	
	char pRecvBuf[1024] = {0};	
	WideCharToMultiByte(CP_ACP, 0, str1, -1, pRecvBuf, 1024, NULL, NULL);
	g_Log.output(LOG_TYPE, "HTTP_POST_DATA....(%s)\r\n%s\r\n...recv End\r\n", id, pRecvBuf);
#endif

	//创建一个XML的文档对象。
	TiXmlDocument *myDocument = new TiXmlDocument();
	myDocument->Parse(/*pRecvTemp*/pRecvBuf);
	if(myDocument == NULL){
		return ENUM_DEVICE_STATUS_UNKNOWN;
	}
	//获得根元素，即Persons。
	TiXmlElement *RootElement = myDocument->RootElement();
	if (RootElement != NULL)
	{
		TiXmlElement * pExtElement = RootElement->FirstChildElement("ext");
		if (pExtElement != NULL)
		{			
			TiXmlElement * pstateElement = pExtElement->FirstChildElement("state");
			if(pstateElement != NULL){
				//g_Log.output(LOG_TYPE,"%s, %s\n", pstateElement->Value(), pstateElement->GetText());
				char* pGetText = (char*)(pstateElement->GetText());
				if (strncmp(pGetText, "active", strlen("active"))== 0)	{
					TiXmlElement * poutElement = pstateElement->NextSiblingElement("outer");
					if(poutElement != NULL){
						TiXmlElement * poutStateElement = poutElement->FirstChildElement("state");
						if (poutStateElement != NULL)
						{
							char* pGetText1 = (char*)(poutStateElement->GetText());
							if(strncmp(pGetText1,"talk", strlen("talk"))== 0){
								return ENUM_DEVICE_STATUS_RUN;
							}

						}
					}
					return ENUM_DEVICE_STATUS_UNKNOWN/*ENUM_DEVICE_STATUS_BUSY*/;
					
				}
				else if (strcmp(pGetText, "ready")== 0)
				{
					return ENUM_DEVICE_STATUS_OK;
				}
				else{
					return ENUM_DEVICE_STATUS_UNKNOWN;
				}		
			}
		}
	}
	return ENUM_DEVICE_STATUS_UNKNOWN;
}
void CSIPCallDlg::UpdateErrorMsg()
{
	int upSz, downSz;
	getCurDeviceNum(m_nCurTunnelIdx, upSz, downSz);	

	// status static 
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < upSz)
		{
			DWORD time = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].doorRTTime;
			if (GetTickCount() - time > TIMER_INTER_60000)
			{
				SetDlgItemText(BTN_STATIC_BASEID_11 + i, L"");
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].doorRTFlag = 0;
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].doorRTTime = 0;
			}			
		}

	}
	for (int i = 0; i < BTN_NUM_MSG; i++)
	{
		if (m_nPageDevice + i < downSz)
		{
			DWORD time = m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].doorRTTime;
			if (GetTickCount() - time > TIMER_INTER_60000)
			{
				SetDlgItemText(BTN_STATIC_BASEID_21 + i, L"");
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].doorRTFlag = 0;
				m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[m_nPageDevice + i].doorRTTime = 0;
			}				
		}
	}
}
void CSIPCallDlg::UpdateCallMsg()
{
	m_MsgLock.Lock();
	for(int i = 0; i < m_vecCallMsg.size(); ){
		DWORD dwTime = GetTickCount();
		if ((dwTime - m_vecCallMsg[i].timestamp) > INVALID_TIME_VALUE)
		{
			//SendMessage(NO_E1_ANSWER,m_vecCallMsg[i].extAddr,m_vecCallMsg[i].type);
			m_vecCallMsg.erase(m_vecCallMsg.begin() + i);
		}
		else if(dwTime - m_vecCallMsg[i].timestamp > 20*1000 && 
			m_vecCallMsg[i].status == ENUM_CALL_WAIT_RING)
		{
			SendMessage(NO_RING_EVENT,m_vecCallMsg[i].extAddr,m_vecCallMsg[i].type);
			m_vecCallMsg.erase(m_vecCallMsg.begin()+i);
			g_Log.output(LOG_TYPE,"call msg list size after no ring:%d!!!!\r\n",m_vecCallMsg.size());
		}
		else if(dwTime - m_vecCallMsg[i].timestamp > 6000 && 
			/*(m_vecCallMsg[i].type == 1||m_vecCallMsg[i].type == 3) &&*/ 
			m_vecCallMsg[i].status == ENUM_CALL_WAIT_E1)
		{
			SendMessage(NO_E1_ANSWER,m_vecCallMsg[i].extAddr,(LPARAM)&m_vecCallMsg[i]);
			if(m_vecCallMsg[i].nSend>=2)
				m_vecCallMsg.erase(m_vecCallMsg.begin() + i);
			else
				m_vecCallMsg[i].nSend++;
			g_Log.output(LOG_TYPE,"call msg list size after no e1:%d!!!!\r\n",m_vecCallMsg.size());
		}else if(dwTime - m_vecCallMsg[i].timestamp >3000&& 
			m_vecCallMsg[i].status == ENUM_CALL_WAIT_E2){
				if(m_vecCallMsg[i].nSend>=2){
					SendMessage(NO_E2_ANSWER,m_vecCallMsg[i].extAddr,m_vecCallMsg[i].type);
					m_vecCallMsg.erase(m_vecCallMsg.begin()+i);
				}else{
					m_vecCallMsg[i].nSend++;
					//间隔2S发两次关电话指令
				}
				g_Log.output(LOG_TYPE,"call msg list size after no e2:%d!!!!\r\n",m_vecCallMsg.size());
		}else if(dwTime - m_vecCallMsg[i].timestamp > 3000&& 
			m_vecCallMsg[i].type == 5){
				SendMessage(NO_E1_ANSWER,m_vecCallMsg[i].extAddr,(LPARAM)&m_vecCallMsg[i]);
				if(m_vecCallMsg[i].nSend>=2)
					m_vecCallMsg.erase(m_vecCallMsg.begin() + i);
				else
					m_vecCallMsg[i].nSend++;
				g_Log.output(LOG_TYPE,"call msg list size after no e1 by divert open:%d!!!!\r\n",m_vecCallMsg.size());
		}else{
			i++;
		}
	}
	m_MsgLock.Unlock();
}
SCallMsg* CSIPCallDlg::GetCallMsg(int extAddr)
{
	m_MsgLock.Lock();
	SCallMsg* pCallMsg = NULL;
	vector<SCallMsg>::iterator msgiter = m_vecCallMsg.begin();
	//g_Log.output(LOG_TYPE,"msg num:%d\n",m_vecCallMsg.size());
	for(; msgiter != m_vecCallMsg.end();){
		if ((*msgiter).extAddr == extAddr)
		{
			pCallMsg = &*(msgiter);
			break;
		}
		else{
			msgiter++;
		}
	}
	m_MsgLock.Unlock();
	return pCallMsg;
}
void CSIPCallDlg::SetCallMsgStatus(int extAddr,enCallStatus callstatus)
{
	m_MsgLock.Lock();
	for ( unsigned int i=0;i<m_vecCallMsg.size();i++)
	{
		if (m_vecCallMsg[i].extAddr == extAddr)
		{
			m_vecCallMsg[i].timestamp = GetTickCount();
			m_vecCallMsg[i].status = callstatus;
			break;
		}
	}
	m_MsgLock.Unlock();
}
void CSIPCallDlg::DeleteCallMsg(int extAddr)
{
	m_MsgLock.Lock();
	vector<SCallMsg>::iterator msgiter = m_vecCallMsg.begin();
	//g_Log.output(LOG_TYPE,"msg num:%d\n",m_vecCallMsg.size());
	for(; msgiter != m_vecCallMsg.end();){
		if ((*msgiter).extAddr == extAddr)
		{
			g_Log.output(LOG_TYPE,"msg before erase num:%d,extAddr:%d\r\n",m_vecCallMsg.size(),extAddr);
			m_vecCallMsg.erase(msgiter);
			break;
		}
		else{
			msgiter++;
		}
	}
	m_MsgLock.Unlock();
}
void CSIPCallDlg::CallFromMsg()
{
	m_MsgLock.Lock();
	if (!m_vecCallMsg.empty())
	{
		for (unsigned int i=0;i<m_vecCallMsg.size();i++)
		{
			vector<SCallMsg>::iterator vecIter = m_vecCallMsg.begin() + i;
			bool bDelete = Call(&(*vecIter));
			if (bDelete)
			{
				m_vecCallMsg.erase(m_vecCallMsg.begin());
			}	
		}
	}
	m_MsgLock.Unlock();
}
void CSIPCallDlg::DisplayCallMsg()
{
	m_MsgLock.Lock();
	for (int i = 0; i < BTN_NUM_WAIT; i ++)
	{
		if (i >= m_vecCallMsg.size())
		{
			GetDlgItem(BTN_STATIC_WAIT_BASEID + i)->ShowWindow(SW_HIDE);
		}
		else{
			GetDlgItem(BTN_STATIC_WAIT_BASEID + i)->ShowWindow(SW_SHOW);
			int extAddr = m_vecCallMsg[i].extAddr;
			SDevice* pDevice = GetDevice(extAddr);
			if (pDevice&&m_vecCallMsg[i].type == 2)
			{
				char szName[MAX_PATH] = {0};
				sprintf_s(szName, "%s的%s号分机正在呼叫，请及时处理", pDevice->sDB.tunnelName, pDevice->sDB.Name);
				wchar_t wName[MAX_PATH] = {0};
				MultiByteToWideChar(CP_ACP, 0, szName, -1, wName, MAX_PATH);
				SetDlgItemText(BTN_STATIC_WAIT_BASEID + i, wName);
			}	
		}
	}
	m_MsgLock.Unlock();
}
bool CSIPCallDlg::Call(SCallMsg* pCallMsg)
{
	if (pCallMsg == NULL)
	{
		// 删除
		return true;
	}
	int extAddr = pCallMsg->extAddr;
	int type = pCallMsg->type;

	int ret = IsCallReady();
	if (ret == 2 || ret == 3 || ret == 4)
	{
		/*if(type == 4)
		{
		if(pCallMsg->status == ENUM_CALL_WAIT_OPEN_ACK)
		return false;
		else
		pCallMsg->status = ENUM_CALL_WAIT_OPEN_ACK;
		}else 
		{*/
		if(pCallMsg->status >= ENUM_CALL_WAIT_E1)
			return false;
		else{
			pCallMsg->status = ENUM_CALL_WAIT_E1;
		}
		//}
		SDevice* pDevice = GetDevice(extAddr);
		if (pDevice == NULL)
		{
			// 删除
			return true;
		}
		/*
		处理点击分机进行通话的消息：
		首先向分机发0x01 请求打开解码板
		*/
		if(IsDeviceLocalController(pDevice->dwIP))
		{
			if ((m_CallA.status == ENUM_DEVICE_STATUS_OK) &&
				(m_CallB.status != ENUM_DEVICE_STATUS_OK))
			{				
				m_CallA.timestamp = GetTickCount();
				sprintf(m_CallA.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
				m_CallA.extAddr = pDevice->sDB.extAddr;			
			}
			else if ((m_CallB.status == ENUM_DEVICE_STATUS_OK) &&
				(m_CallA.status != ENUM_DEVICE_STATUS_OK))
			{			
				m_CallB.timestamp = GetTickCount();
				sprintf(m_CallB.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
				m_CallB.extAddr = pDevice->sDB.extAddr;					
			}
			else if ((m_CallB.status == ENUM_DEVICE_STATUS_OK) &&
				(m_CallA.status == ENUM_DEVICE_STATUS_OK))
			{
				// A			
				m_CallA.timestamp = GetTickCount();
				sprintf(m_CallA.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
				m_CallA.extAddr = pDevice->sDB.extAddr;

				// B		
				m_CallB.timestamp = GetTickCount();
				sprintf(m_CallB.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
				m_CallB.extAddr = pDevice->sDB.extAddr;			
			}
			else{
				return false;
			}

			if(IsBCRun(pDevice->sDB.extAddr))
			{
				DevicePause(pDevice);
				Sleep(300);
			}
			g_MSG.SendOpenCall(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);

			pCallMsg->status = ENUM_CALL_WAIT_E1;
			pCallMsg->timestamp = GetTickCount();
			s_callEvtTime = GetTickCount();

			if(!g_Config.m_SystemConfig.nMainSystem&&(type == 1||type == 2))
				g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_OPENED);
			return false;
		}else 
		{
			if (IsBCRun(pDevice->sDB.extAddr))
			{
				DevicePause(pDevice);
				Sleep(300);
			}
			g_Log.output(LOG_TYPE,"localip:%s,,deviceip:%s\n",g_dwLocalIp,pDevice->dwIP);
			g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_OPEN_REQ);		//发送请求通话指令
			pCallMsg->timestamp = GetTickCount();
			return false;
		}
	}
	else{
		// 等待空闲
		return false;
	}
	// 删除
	return true;
}
void CSIPCallDlg::UpdateCallInfo()
{
	return;
	SEventInfo sEvent;
	if (m_CallA.status == ENUM_DEVICE_STATUS_RING && 
		m_CallB.status == ENUM_DEVICE_STATUS_RING && 
		m_CallA.extAddr == m_CallB.extAddr)
	{
		DWORD dwTime = GetTickCount();
		if (dwTime - m_CallA.timestamp > INVALID_TIME_VALUE)
		{			
			sEvent.id = m_CallA.id;
			SendMessage(RECV_MSG_CALL_HANGUP, atoi(g_Config.m_SystemConfig.CallA), (LPARAM)(&sEvent));
		}	
	}
	else{
		// A
		if (m_CallA.status == ENUM_DEVICE_STATUS_RUN)
		{

		}
		else if(m_CallA.status == ENUM_DEVICE_STATUS_RING){
			DWORD dwTime = GetTickCount();
			if (dwTime - m_CallA.timestamp > INVALID_TIME_VALUE)
			{
				//  			m_CallA.status = m_CallA.preStatus;
				//  			m_CallA.clearCall();
				sEvent.id = m_CallA.id;
				SendMessage(RECV_MSG_CALL_HANGUP, atoi(g_Config.m_SystemConfig.CallA), (LPARAM)(&sEvent));
			}
		}
		else{
			DWORD dwTime = GetTickCount();
			if (dwTime - m_CallA.timestamp > INVALID_TIME_VALUE)
			{			
				m_CallA.clearCall();			
			}
		}
		// B
		if (m_CallB.status == ENUM_DEVICE_STATUS_RUN)
		{

		}
		else if (m_CallB.status == ENUM_DEVICE_STATUS_RING)	{
			DWORD dwTime = GetTickCount();
			if (dwTime - m_CallB.timestamp > INVALID_TIME_VALUE)
			{
				// 			m_CallB.status = m_CallB.preStatus;
				// 			m_CallB.clearCall();
				sEvent.id = m_CallB.id;
				SendMessage(RECV_MSG_CALL_HANGUP, atoi(g_Config.m_SystemConfig.CallB), (LPARAM)(&sEvent));
			}
		}
		else {
			DWORD dwTime = GetTickCount();
			if (dwTime - m_CallB.timestamp > INVALID_TIME_VALUE)
			{
				m_CallB.clearCall();
			}
		}
	}
}
void CSIPCallDlg::OnMenuAboutInfo()
{
	// TODO: Add your command handler code here
	CAboutDlg dlg;
	dlg.DoModal();
}
void CSIPCallDlg::OnMenuHelp()
{
	// TODO: Add your command handler code here
	CString strDir = g_szAPPPath + L"help";	
	ShellExecute(NULL, L"open", strDir, NULL, NULL, SW_SHOW);
}
void CSIPCallDlg::OnBnClickedBtnConfig()
{
	// TODO: Add your control notification handler code here
	CDlgBC dlgBC;
	dlgBC.SetDlg(true);
	dlgBC.SetCurTunnelIdx(m_nCurTunnelIdx);	
	if(IsCurBcRun(m_nCurTunnelIdx)){
		if (dlgBC.DoModal() == IDOK)
		{

		}
	}
	else{
		MessageBox(L"当前隧道没有广播在使用，请稍后再配置！", L"提醒");
	}
}
void CSIPCallDlg::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	StopBC();	
	Status2Button();
	((CButton*)GetDlgItem(IDC_CHECK_SEND_BC))->SetCheck(0);
}
void CSIPCallDlg::OnBnClickedBtnPlay()
{
	// TODO: Add your control notification handler code here
	
	// 0:未选择 1：选择+配置 2：选择+未配置
	int  flag = GetDeviceConfig();
	if (flag == 2/*||flag == 1*/)
	{
		//CDlgBC 设置音源、音频选项等
		CDlgBC dlg;
		dlg.SetDlg(false);
		dlg.SetCurTunnelIdx(m_nCurTunnelIdx);	
		
		if (dlg.DoModal() == IDOK)
		{		
			//PlayBC 创建线程播放音源
			PlayBC();
			//更新广播状态
			Status2Button();
		}		
	}
	else if (flag == 0)
	{
		MessageBox(L"未选择设备！", L"提醒");
	}
	else{
		PlayBC();
		Status2Button();
	}
	
}
void CSIPCallDlg::OnUpdateMenuUser(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CDlgUser dlg;
	dlg.DoModal();
}
void CSIPCallDlg::AddMsgRecord(SDevice& stDevice, enPlaySrc model)
{
	SRecord stRecord;
	CharToTchar(stDevice.sDB.ip, stRecord.IP);
	CharToTchar(stDevice.sDB.tunnelName, stRecord.tunnel);
	CharToTchar(stDevice.sDB.kmID, stRecord.kmid);
	CharToTchar(stDevice.sDB.extNo, stRecord.sip);
	CharToTchar(stDevice.sDB.Name, stRecord.noid);//??????????
	stRecord.addr = stDevice.sDB.extAddr;
	memset(stRecord.desc, 0, 32 * sizeof(TCHAR));
#if 0
#else
	CharToTchar(PlaySrc2String(model), stRecord.desc);
#endif
	g_data.InsertMsg(stRecord);
}
char* CSIPCallDlg::PlaySrc2String(enPlaySrc src)
{
	switch(src){
	case ENUM_MODEL_FILE:
		return "本地文件";		
	case ENUM_MODEL_MIC:
		return "网络寻呼话筒";		
	case ENUM_MODEL_TTS:
		return "语音合成";
	case ENUM_MODEL_FILE_LIST:
		return "列表文件";
	case ENUM_MODEL_AUX:
		return "外部设备";
	default:
		return "";
	}	
}
char* CSIPCallDlg::PlaySrc2String(SDevice& stDevice)
{
	memset(m_szTip, 0, MAX_PATH);
	enPlaySrc src = stDevice.playSrc;
	switch(src){
	case ENUM_MODEL_FILE:
		sprintf(m_szTip, "本地文件:");		
		break;
	case ENUM_MODEL_MIC:
		sprintf(m_szTip, "网络寻呼话筒:");
		break;
	case ENUM_MODEL_TTS:
		sprintf(m_szTip, "语音合成:");
		break;
	case ENUM_MODEL_FILE_LIST:
		sprintf(m_szTip, "列表文件:");
		break;
	case ENUM_MODEL_AUX:
		sprintf(m_szTip, "外部设备:");
		break;
	}	
	int nLen = strlen(stDevice.playFile);
	if (nLen > 0)
	{
		int i = 0;
		for (i = nLen - 1; i > 0; i--)
		{
			if (stDevice.playFile[i] == '\\')
			{
				break;
			}
		}
		strncat(m_szTip, stDevice.playFile + i + 1, nLen - 1 - i);
	}
	
	return m_szTip;
}
void CSIPCallDlg::StopOneDevice(SDevice* pDevice, bool& bMIC, bool& bAux, bool bCmd, bool bExit)
{	
	bMIC = false;
	bAux = false;
	if (!bCmd)
	{
		if (!bExit && pDevice->IsOffline()){
			return;
		}
		if (!bExit && (!pDevice->IsCheck())){
			return;
		}
		if(!bExit && pDevice->bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING)
		{
			return ;
		}
		if(!bExit && (pDevice->callStatus == ENUM_DEVICE_STATUS_RUN
			||pDevice->callStatus == ENUM_DEVICE_STATUS_RING))
			return ;
		if(!bExit && IsExistCall(pDevice->sDB.extAddr))
			return ;
	}
	pDevice->Check = 0;
	g_Log.output(LOG_TYPE, "stop*ip:[%s],play[%d],thread[%d],time[%d],status[%d]\r\n", pDevice->sDB.ip, (int)(pDevice->playFlag), pDevice->threadID, pDevice->playTime, (int)(pDevice->bcStatus));
	
	enBCOptStatus opt = pDevice->IsBcPlay();
	if (opt == ENUM_BC_OPT_NONE)
	{
		return;
	}
	//关闭指令，等待BC关闭回复  0XE4
	SetBCStatus(pDevice->sDB.extAddr,ENUM_BC_WAIT_E4);
	pDevice->enStopFlag = ENUM_STOP_MSG_CLICK;
	if (pDevice->playSrc == ENUM_MODEL_FILE ||
		pDevice->playSrc == ENUM_MODEL_FILE_LIST ||
		pDevice->playSrc == ENUM_MODEL_TTS){	
		// 发送停止消息		
		if (pDevice->threadID > 0)
		{
			g_Play.stop(pDevice->playParam);
			if(IsDeviceLocalController(pDevice->dwIP)){
				g_MSG.SendDeviceBroadcastStatus2Master(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_IS_USABLE);
				g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,false);
			}
			else{
				g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_CLOSE_REQ);
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
			}
		}
		else{ 
			EndPlay(pDevice);
		}
	}	
	else if (pDevice->playSrc == ENUM_MODEL_MIC){	
		bMIC = true;
		EndPlay(pDevice);
	}
	else if (pDevice->playSrc == ENUM_MODEL_AUX){
		bAux = true;
		EndPlay(pDevice);
	}
}
void CSIPCallDlg::StopBC(bool bExitApp)
{
	if (m_pTunnelInfo != NULL)
	{
		STunnel* pCurTunnel = m_pTunnelInfo + m_nCurTunnelIdx;
		vector<SDevice>::iterator vecIter;
		SDevice* pDevice = NULL;
		bool bCloseMic = false;
		bool bCloseAux = false;
		if (!pCurTunnel->vecDeviceUp.empty())
		{
			vecIter = pCurTunnel->vecDeviceUp.begin();
			pDevice = &(*vecIter);
			for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
			{		
				bool bmic;
				bool baux;
				StopOneDevice(pDevice + i, bmic, baux, false, bExitApp); 
				if (bmic)
				{
					bCloseMic = true;
				}
				if (baux)
				{
					bCloseAux = true;
				}
			}
		}
		if (!pCurTunnel->vecDeviceDown.empty())
		{
			vecIter = pCurTunnel->vecDeviceDown.begin();
			pDevice = &(*vecIter);
			for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
			{
				bool bmic;
				bool baux;
				StopOneDevice(pDevice + i, bmic, baux, false, bExitApp); 
				if (bmic)
				{
					bCloseMic = true;
				}
				if (baux)
				{
					bCloseAux = true;
				}
			}
		}
		AutoCloseMicAux(bCloseMic, bCloseAux);
	}
}
void CSIPCallDlg::AutoCloseMicAux(bool bCloseMic, bool bCloseAux)
{
	if (bCloseMic || bCloseAux)
	{
		bool bAuxRun = false;
		bool bMicRun = false;
		GetTunnelPlayStatus(bMicRun, bAuxRun);
		if ((!bMicRun) && bCloseMic)
		{
			USHORT portAudio = g_Config.m_SystemConfig.portAudio;
			g_MSG.SendCloseMic(g_Config.m_SystemConfig.IP_MIC, portAudio);		
			g_Play.RecordStop();
		}
		if ((!bAuxRun) && bCloseAux)
		{
			USHORT portAudio = g_Config.m_SystemConfig.portAudio;
			g_MSG.SendCloseAux(g_Config.m_SystemConfig.IP_AUX, portAudio);
		}
	}	
}
void CSIPCallDlg::Text2Speach(char* textFile)
{
	char szWave[MAX_PATH] = {0};
	WideCharToMultiByte(CP_ACP, 0,  m_pAPP->Txt2TTSPath(textFile), -1, szWave, MAX_PATH, NULL, NULL);
	// txt to buf

	HANDLE hFile = CreateFileA(textFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
}
// 0:未选择 1：选择+配置 2：选择+未配置
int CSIPCallDlg::GetDeviceConfig()
{
	STunnel* pCurTunnel = m_pTunnelInfo + m_nCurTunnelIdx;
	vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceUp.begin();
	SDevice* pDevice = &(*vecIter);
	int check = 0;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
	{				
		if (pDevice[i].IsCheck())
		{
			check = 1;
			if (!pDevice[i].IsConfig())
			{				
				return 2;
			}
		}		
	}
	if (!pCurTunnel->vecDeviceDown.empty())
	{
		vecIter = pCurTunnel->vecDeviceDown.begin();
		pDevice = &(*vecIter);
		for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
		{
			if (pDevice[i].IsCheck())
			{
				check = 1;
				if (!pDevice[i].IsConfig())
				{
					return 2;
				}
			}
		}
	}
		
	if (check)
	{
		return 1;
	}
	else{
		return 0;
	}
	
}
void CSIPCallDlg::AutoSelAllCheckBox()
{
	int upSz, downSz;
	getCurDeviceNum(m_nCurTunnelIdx, upSz, downSz);
	int count = 0;
	// bc
	for (int i = 0; i < upSz; i++)
	{
		if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].Check)
		{
			count++;
		}
			
	}
	for (int i = 0; i < downSz; i++)
	{
		if (m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[m_nPageDevice + i].Check)
		{
			count++;
		}
	}
	if (count == (upSz + downSz)){

	}
	else{

	}	
}
void CSIPCallDlg::OneKeyClose()
{
	for (int i = 0; i < m_nTunnelNum; i ++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			g_Log.output(LOG_TYPE, "SendCloseBroadcast,OneKeyClose:%d\r\n",m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr);
			g_MSG.SendCloseBroadcast(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice, false);
			Sleep(300);
			g_MSG.SendHangUp(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			g_Log.output(LOG_TYPE, "SendCloseBroadcast,OneKeyClose:%d\r\n",m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr);
			g_MSG.SendCloseBroadcast(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice, false);
			Sleep(300);
			g_MSG.SendHangUp(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
		}
	}
	USHORT portAudio = g_Config.m_SystemConfig.portAudio;
	g_MSG.SendCloseMic(g_Config.m_SystemConfig.IP_MIC, portAudio);
	g_MSG.SendCloseAux(g_Config.m_SystemConfig.IP_AUX, portAudio);
}
void CSIPCallDlg::OnMenuOnekeyclose()
{
	// TODO: Add your command handler code here
	//OnDestroy();
	CDlgPwdCheck dlg;
	wchar_t wzName[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_szLoginName, -1, wzName, MAX_PATH);
	dlg.SetUserName(wzName);
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.IsConfirm())
		{
			SetReboot();
			ShellExecute( NULL, _T("open"), g_szAPPPath + ("Launch.exe"), NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
			s_ExitOpt = true;
			SendMessage(WM_CLOSE);
			
		}	
	}
}
void CSIPCallDlg::AutoChangePage(int extAddr)
{
	int nPageTunnel = 0;
	int nPageDevice = 0;
	int nTunnelIdx = 0;
	if (GetDeviceIndex(extAddr, nPageDevice, nTunnelIdx))
	{
		int preTunnelIdx = m_nCurTunnelIdx;
		int prePageDevice = m_nPageDevice;
		int prePageTunnel = m_nPageTunnel;

		m_nCurTunnelIdx = nTunnelIdx;
		m_nPageDevice = nPageDevice;
		m_nPageTunnel = m_nCurTunnelIdx / BTN_NUM_TUNNEL * BTN_NUM_TUNNEL;

		DB2TunnelBtn();
		CheckTunnelPreNext();

		DB2DeviceBtn(m_nCurTunnelIdx);
		CheckDevicePreNext();		
	}
}
bool CSIPCallDlg::GetDeviceIndex(int extAddr, int& nPageDevice, int& nTunnelIndex)
{
	bool flag = false;
	int i = 0;
	int j = 0;
	for (i = 0; i < m_nTunnelNum; i ++)
	{
		for (j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (extAddr == m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr)
			{
				flag = true;
				nTunnelIndex = i;				
				nPageDevice = j / BTN_NUM_MSG * BTN_NUM_MSG;
				break;
			}
		}
		for (j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (extAddr == m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr)
			{
				flag = true;
				nTunnelIndex = i ;				
				nPageDevice = j / BTN_NUM_MSG * BTN_NUM_MSG;
				break;
			}
		}
	}	
	return flag;
}
void CSIPCallDlg::PlayOneDevice(SDevice* pDevice, bool& bMIC, bool& bAux, bool bCmd)
{
	bMIC = false;
	bAux = false;
	if (!bCmd)
	{
		if (pDevice->IsOffline()){
			return;
		}
		if (!pDevice->IsCheck()){
			return;
		}
		if (!pDevice->sDB.IsVisible())
		{
			return;
		}
		if(IsExistCall(pDevice->sDB.extAddr))
			return ;
	}
	g_Log.output(LOG_TYPE, "PlayOneDevice...IsBcPlay\r\n");
	enBCOptStatus opt = pDevice->IsBcPlay();	
	if (opt == ENUM_BC_OPT_REMOTE || 
		opt == ENUM_BC_OPT_LOCAL)
	{
		return;
	}	
	g_Log.output(LOG_TYPE, "PlayOneDevice...ENUM_MODEL_FILE\r\n");

	/************************************************************************/
	/* 开启广播前生成广播开消息，待收到广播开回应0xE3后，视广播开启成功；
	   定时检测该广播事件的回应，若超时未收到广播开回应，关闭广播，显示分机状态不可用。
	 */
	/************************************************************************/
	SBCMsg bcMsg;
	bcMsg.extAddr = pDevice->sDB.extAddr;
	bcMsg.status = ENUM_BC_WAIT_E3;
	bcMsg.timestamp = GetTickCount();
	if(IsDeviceLocalController(pDevice->dwIP))
		bcMsg.type = 0;
	else
		bcMsg.type = 1;
	if(pDevice->cmd_open_bc_recv)
		bcMsg.type = 2;
	AddBCMsg(bcMsg);
	
	// play
	if (pDevice->playSrc == ENUM_MODEL_FILE){
		if (StartPlay(pDevice))
		{
			if (!bCmd)
			{
				m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_FILE);
			}
		}		
	}
	else if (pDevice->playSrc == ENUM_MODEL_FILE_LIST){
		char* pFile = g_data.GetPlayFile(pDevice->curPlayIdx);
		if (pFile)
		{
			sprintf(pDevice->playFile, pFile);

			if (StartPlay(pDevice))
			{
				m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_FILE_LIST);
			}	
		}		
		else{
			return;
		}
	}
	else if (pDevice->playSrc == ENUM_MODEL_MIC){
		if(IsDeviceLocalController(pDevice->dwIP)){
			g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr,pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);				
			g_MSG.SendDeviceBroadcastStatus2Master(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_IS_USING);
		}else
		{
			g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_OPEN_REQ);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);				
		}
		m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_MIC);
		bMIC = true;
		pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;		
		pDevice->playFlag = true;
	}
	else if (pDevice->playSrc == ENUM_MODEL_AUX){
		if(IsDeviceLocalController(pDevice->dwIP))
		{
			g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr,pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_AUX);				
			g_MSG.SendDeviceBroadcastStatus2Master(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_IS_USING);
		}else
		{
			g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_OPEN_REQ);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_AUX);				
		}
		m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_AUX);
		bAux = true;
		pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;		
		pDevice->playFlag = true;
	}
	else if (pDevice->playSrc == ENUM_MODEL_TTS)	{

		//WideCharToMultiByte(CP_ACP, 0, g_ttsFilePath, -1, pDevice->playFile/*szWave*/, MAX_PATH, NULL, NULL);
		if (StartPlay(pDevice))
		{
			m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_TTS);
		}	
	}
}
void CSIPCallDlg::PlayBC()
{		
	STunnel* pCurTunnel = m_pTunnelInfo + m_nCurTunnelIdx;
	vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceUp.begin();
	SDevice* pDevice = &(*vecIter);
	bool bOpenMic = false;
	bool bOpenAux = false;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i ++)
	{		
		bool bmic;
		bool baux;
		PlayOneDevice(pDevice + i, bmic, baux, false); 
		if (bmic)
		{
			bOpenMic = true;
		}
		if (baux)
		{
			bOpenAux = true; 
		}
	}
	if (!pCurTunnel->vecDeviceDown.empty())
	{
		vecIter = pCurTunnel->vecDeviceDown.begin();
		pDevice = &(*vecIter);
		for(int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
		{
			bool bmic;
			bool baux;
			PlayOneDevice(pDevice + i, bmic, baux, false);  
			if (bmic)
			{
				bOpenMic = true;
			}
			if (baux)
			{
				bOpenAux = true; 
			}
		}
	}
	
	if (bOpenMic)
	{
		g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_MIC_RECORD, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);
		g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_Monitor, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_MIC, portAudio, false);
		g_Play.RecordStart();	
	}
	
	if (bOpenAux)
	{
		g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_MIC_RECORD, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_AUX);
		g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_Monitor, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_AUX);
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_AUX, portAudio, true);
	}

	if ((!bOpenAux) && (!bOpenMic))
	{

	}
}
void CSIPCallDlg::RepaintTunnelBtn()
{	
	//int idx = m_nCurTunnelIdx % BTN_NUM_TUNNEL;
	//GetDlgItem(IDC_BUTTON11)->SetFocus();
	//TRACE("Hight, %d\r\n", idx);
	for (int i = 0; i < BTN_NUM_TUNNEL; i++)
	{
		int tunnelIdx = m_nPageTunnel + i;
		bool bRun = IsTunnelRun(tunnelIdx);
		if (tunnelIdx == m_nCurTunnelIdx)
		{
			m_pBtnTunnel[i].SetBkColor(RGB(255,128,64));
		}
		else if (bRun/*idx == i*/)
		{				
			//m_pBtnTunnel[i]->SetState(TRUE);
			m_pBtnTunnel[i].SetBkColor(RGB(91,170,249));
		}
		else{
			m_pBtnTunnel[i].SetBkColor(RGB(216,235,253));
			//m_pBtnTunnel[i]->SetState(FALSE);
		}	
		m_pBtnTunnel[i].DrawBorder();
	}	
}
bool CSIPCallDlg::IsTunnelRun(int nTunnelIdx)
{
	if (nTunnelIdx < 0 || nTunnelIdx >= m_nTunnelNum)
	{
		return false;
	}

	for (int j = 0; j < m_pTunnelInfo[nTunnelIdx].vecDeviceUp.size(); j++)
	{
		if (m_pTunnelInfo[nTunnelIdx].vecDeviceUp[j].bcStatus == ENUM_DEVICE_STATUS_RUN ||
			m_pTunnelInfo[nTunnelIdx].vecDeviceUp[j].callStatus == ENUM_DEVICE_STATUS_RUN)
		{
			return true;
		}
	}
	for (int j = 0; j < m_pTunnelInfo[nTunnelIdx].vecDeviceDown.size(); j++)
	{
		if (m_pTunnelInfo[nTunnelIdx].vecDeviceDown[j].bcStatus == ENUM_DEVICE_STATUS_RUN ||
			m_pTunnelInfo[nTunnelIdx].vecDeviceDown[j].callStatus == ENUM_DEVICE_STATUS_RUN)
		{
			return true;
		}
	}
	return false;
}
bool CSIPCallDlg::IsTTSRun()
{
	for (int nTunnelIdx = 0; nTunnelIdx < m_nTunnelNum; nTunnelIdx++)
	{
		for (int j = 0; j < m_pTunnelInfo[nTunnelIdx].vecDeviceUp.size(); j++)
		{
			if ((m_pTunnelInfo[nTunnelIdx].vecDeviceUp[j].IsBcPlay() == ENUM_BC_OPT_LOCAL) &&
				(m_pTunnelInfo[nTunnelIdx].vecDeviceUp[j].playSrc == ENUM_MODEL_TTS))
			{
				return true;

			}
		}
		for (int j = 0; j < m_pTunnelInfo[nTunnelIdx].vecDeviceDown.size(); j++)
		{
			if ((m_pTunnelInfo[nTunnelIdx].vecDeviceDown[j].IsBcPlay()  == ENUM_BC_OPT_LOCAL) &&
				(m_pTunnelInfo[nTunnelIdx].vecDeviceDown[j].playSrc == ENUM_MODEL_TTS))
			{
				return true;

			}
		}
	}
	

	return false;
}
void CSIPCallDlg::GetTunnelPlayStatus(bool& bmic, bool& baux)
{
	bmic = baux = false;
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceUp[j].IsBcPlay() == ENUM_BC_OPT_LOCAL )
			{
				if (m_pTunnelInfo[i].vecDeviceUp[j].playSrc == ENUM_MODEL_MIC)
				{
					bmic = true;
				}
				else if (m_pTunnelInfo[i].vecDeviceUp[j].playSrc == ENUM_MODEL_AUX)
				{
					baux = true;
				}
				if (bmic && baux)
				{
					return;
				}
			}
		}
		for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
		{
			if (m_pTunnelInfo[i].vecDeviceDown[j].IsBcPlay() == ENUM_BC_OPT_LOCAL )
			{
				if (m_pTunnelInfo[i].vecDeviceDown[j].playSrc == ENUM_MODEL_MIC)
				{
					bmic = true;
				}
				else if (m_pTunnelInfo[i].vecDeviceDown[j].playSrc == ENUM_MODEL_AUX)
				{
					baux = true;
				}
				if (bmic && baux)
				{
					return;
				}
			}
		}
	}
}
bool CSIPCallDlg::IsCurBcRun(int nTunnelIdx)
{
	if (nTunnelIdx < 0 || nTunnelIdx >= m_nTunnelNum)
	{
		return false;
	}

	for (int j = 0; j < m_pTunnelInfo[nTunnelIdx].vecDeviceUp.size(); j++)
	{
		if (m_pTunnelInfo[nTunnelIdx].vecDeviceUp[j].bcStatus == ENUM_DEVICE_STATUS_RUN)
		{
			return true;
		}
	}
	for (int j = 0; j < m_pTunnelInfo[nTunnelIdx].vecDeviceDown.size(); j++)
	{
		if (m_pTunnelInfo[nTunnelIdx].vecDeviceDown[j].bcStatus == ENUM_DEVICE_STATUS_RUN)
		{
			return true;
		}
	}
	return false;
}
void CSIPCallDlg::Device2PlayDeviceAttr(SDevice* pDevice, SPlayDeviceAttr* pPlayDeviceAttr)
{	
	pPlayDeviceAttr->attr.enSrc = pDevice->playSrc;
	pPlayDeviceAttr->attr.enModel = pDevice->playMode;
	pPlayDeviceAttr->attr.curPlayIndex = pDevice->curPlayIdx;
	memcpy(pPlayDeviceAttr->attr.pFile,pDevice->playFile, MAX_PATH * sizeof(char));
	pPlayDeviceAttr->extAddr = pDevice->sDB.extAddr;
	//pPlayDeviceAttr->ip = pDevice->dwIP;
	pPlayDeviceAttr->ip = inet_addr(pDevice->sDB.ip);
}
void CSIPCallDlg::DeleteCallAnswer(int extAddr)
{
	for (int i = 0; i < m_vecCallEvt.size(); i++)
	{
		if (m_vecCallEvt[i].extAddr == extAddr)
		{
			m_vecCallEvt.erase(m_vecCallEvt.begin() + i);
		}
	}
}
void CSIPCallDlg::CloseCallWithoutAnswer(SDevice* pDevice)
{
	if (pDevice)
	{
		if(IsDeviceLocalController(pDevice->dwIP)){
			g_Log.output(LOG_TYPE, "HANG UP %d............\r\n", pDevice->sDB.extAddr);
			g_MSG.SendHangUp(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);	
			pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
			Status2Button(pDevice->sDB.extAddr);
		}
		else{
			g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_CLOSE_REQ);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);
		}	
		DeleteCallAnswer(pDevice->sDB.extAddr);	
	}
}
SEventInfo*  CSIPCallDlg::IsEventTypeExist(char* pExtNo, enEventStatus type)
{
	for(int i = 0; i < m_vecCallEvent.size(); i++){
		if (strncmp(pExtNo, m_vecCallEvent[i].extNo, strlen(pExtNo)) == 0)
		{
			if (m_vecCallEvent[i].type == type)
			{
				vector<SEventInfo>::iterator vecIter = m_vecCallEvent.begin() + i;
				return &(*vecIter);
			}			
		}
	}
	return NULL;
}
void CSIPCallDlg::DeleteEvent(char* pExtNo)
{
	SEventInfo* pEvtRing = IsEventTypeExist(pExtNo, ENUM_EVENT_RING);
	if (pEvtRing)
	{
		// 删除相关的线路事件
		DeleteEvent(pEvtRing->id);
	}
	for(int i = 0; i < m_vecCallEvent.size(); i++){
		if ((strncmp(pExtNo, m_vecCallEvent[i].extNo, max(1,strlen(pExtNo))) == 0) ||
			(strncmp(pExtNo, m_vecCallEvent[i].from, max(1,strlen(pExtNo))) == 0) )
		{
			m_vecCallEvent.erase(m_vecCallEvent.begin() + i);		
		}
	}
}
void CSIPCallDlg::DeleteEvent(int id)
{
	for(int i = 0; i < m_vecCallEvent.size(); i++){
		if (m_vecCallEvent[i].id == id)
		{
			m_vecCallEvent.erase(m_vecCallEvent.begin() + i);		
		}
	}
}
void CSIPCallDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (s_ExitOpt)
	{			
		CDialog::OnClose();
	}
	else{
		bool bPlay = false;
		for (int i = 0; i < m_nTunnelNum; i++)
		{
			if (IsTunnelRun(i))
			{
				bPlay = true;
			}
		}
		if(bPlay){
			MessageBox(L"当前正在通话或广播, 请稍后关闭！", L"提醒");
		}
		else{				
			CDialog::OnClose();
		}
	}
}
void CSIPCallDlg::SetReboot()
{
	HANDLE hFile = CreateFile(g_szAPPPath + L"reboot.tmp", GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{		
		return;
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
}
bool CSIPCallDlg::IsReboot()
{
	HANDLE hFile = CreateFile(g_szAPPPath + L"reboot.tmp", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{		
		return false;
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	return true;
}
void CSIPCallDlg::ClearReboot()
{
	DeleteFile(g_szAPPPath + L"reboot.tmp");
}
void del_sp(char *src)    // 删除C风格字符串中的空格
{
	char * fp = src;
	while (*src) {
		if (*src != ' ') { // 如果不是空格就复制
			*fp = *src;
			fp++;
		}
		src++;
	}
	*fp = '\0' ; //封闭字符串
}
void CSIPCallDlg::ReadNoBCFile()
{
	if (!m_vecNoBCData.empty())
	{
		m_vecNoBCData.clear();
	}
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"NoBroadcast.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					// 查找ip
					char* pListData = strstr(line, "ip=");
					if (pListData)
					{
						char* ip1 = strstr(pListData, "=") + 1;
						char* ip2 = strstr(ip1, ".") + 1;
						char* ip3 = strstr(ip2, ".") + 1;
						char* ip4 = strstr(ip3, ".") + 1;
						if (ip1 && ip2 && ip3 && ip4)
						{
							BYTE btIP1 = atoi(ip1);
							BYTE btIP2 = atoi(ip2);
							BYTE btIP3 = atoi(ip3);
							BYTE btIP4 = atoi(ip4);
							if ((btIP1 >= 0 && btIP1 <= 255) && 
								(btIP2 >= 0 && btIP2 <= 255) && 
								(btIP3 >= 0 && btIP3 <= 255) && 
								(btIP4 >= 0 && btIP4 <= 255))
							{
								SNoIPFileData data;	
								memset(data.ip, 0, 32);
								sprintf(data.ip, "%d.%d.%d.%d", btIP1, btIP2, btIP3, btIP4);
								m_vecNoBCData.push_back(data);
							}
							
						}
					}
				}
			}
			else
			{
				break;
			}
			j++; 
		} 
		fclose( stream ); 
	}
}
bool CSIPCallDlg::IsNoBC(char* ip)
{
	if (m_vecNoBCData.empty())
	{
		return false;
	}
	for (int i = 0; i < m_vecNoBCData.size(); i++){
		if (strcmp(m_vecNoBCData[i].ip, ip) == 0)
		{
			return true;
		}
	}
	return false;
}
void CSIPCallDlg::ReadNoCallFile()
{
	if (!m_vecNoCallData.empty())
	{
		m_vecNoCallData.clear();
	}
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"NoCall.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					// 查找ip
					char* pListData = strstr(line, "ip=");
					if (pListData)
					{
						char* ip1 = strstr(pListData, "=") + 1;
						char* ip2 = strstr(ip1, ".") + 1;
						char* ip3 = strstr(ip2, ".") + 1;
						char* ip4 = strstr(ip3, ".") + 1;
						if (ip1 && ip2 && ip3 && ip4)
						{
							BYTE btIP1 = atoi(ip1);
							BYTE btIP2 = atoi(ip2);
							BYTE btIP3 = atoi(ip3);
							BYTE btIP4 = atoi(ip4);
							if ((btIP1 >= 0 && btIP1 <= 255) && 
								(btIP2 >= 0 && btIP2 <= 255) && 
								(btIP3 >= 0 && btIP3 <= 255) && 
								(btIP4 >= 0 && btIP4 <= 255) )
							{
								SNoIPFileData data;	
								memset(data.ip, 0, 32);
								sprintf(data.ip, "%d.%d.%d.%d", btIP1, btIP2, btIP3, btIP4);
								m_vecNoCallData.push_back(data);
							}

						}
					}
				}
			}
			else
			{
				break;
			}

			j++; 
		} 
		fclose( stream ); 
	}
}
bool CSIPCallDlg::IsNoCall(char* ip)
{
	if (m_vecNoCallData.empty())
	{
		return false;
	}
	for (int i = 0; i < m_vecNoCallData.size(); i++){
		if (strcmp(m_vecNoCallData[i].ip, ip) == 0)
		{
			return true;
		}
	}
	return false;
}
void CSIPCallDlg::ReadGroupFile()
{
	if (m_pGroupCall == NULL)
	{
		m_pGroupCall = (SGroupCall*)malloc(300* sizeof(SGroupCall));
		memset(m_pGroupCall, 0, 300*sizeof(SGroupCall));
	}
	m_nGroupCall = 0;
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"group.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					memset(m_pGroupCall + m_nGroupCall, 0, sizeof(SGroupCall));
					m_pGroupCall[m_nGroupCall].extAddr = atoi(line); 
					// 查找ip
					char* pListData = strstr(line, "=");
					char* pListData2 = strstr(line, "#");
					if (pListData && pListData2)
					{
						strncpy(m_pGroupCall[m_nGroupCall].group, pListData + 1,  pListData2 - pListData - 1);
						m_nGroupCall++;
					}
				}
			}
			else
			{
				break;
			}
			j++; 
		} 

		fclose( stream ); 
	}
}
char* CSIPCallDlg::GetGroup(int extAddr)
{
	for (int i = 0; i < m_nGroupCall; i++)
	{
		if (m_pGroupCall[i].extAddr == extAddr)
		{
			return m_pGroupCall[i].group;
		}
	}
	return NULL;
}
// 读声音文件
void CSIPCallDlg::ReadYingFile()
{
	if (m_pBCYing == NULL)
	{
		m_pBCYing = (SBCYing*)malloc(300* sizeof(SBCYing));
		memset(m_pBCYing, 0, 300*sizeof(SBCYing));
	}
	m_nBCYing = 0;
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"ying.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					memset(m_pBCYing + m_nBCYing, 0, sizeof(SBCYing));
					m_pBCYing[m_nBCYing].extAddr = atoi(line); 
					// 查找ip
					char* pListData = strstr(line, "=");
					char* pListData2 = strstr(line, "#");
					if (pListData && pListData2)
					{
						m_pBCYing[m_nBCYing].ying= min(max(1, atoi(pListData + 1)), 100);
						m_nBCYing++;
					}
				}
			}
			else
			{
				break;
			}
			j++; 
		} 

		fclose( stream ); 
	}
}
int CSIPCallDlg::GetYing(int extAddr)
{
	for (int i = 0; i < m_nBCYing; i++)
	{
		if (m_pBCYing[i].extAddr == -1)
		{
			return m_pBCYing[i].ying;
		}
	}

	for (int i = 0; i < m_nBCYing; i++)
	{
		if (m_pBCYing[i].extAddr == extAddr)
		{
			return m_pBCYing[i].ying;
		}
	}
	return 100;
}
bool less_groupid(const SFire& item1, const SFire& item2)
{
	return item1.groupID < item2.groupID;	
}
// 读指令文件
void CSIPCallDlg::ReadDevCmdFile()
{	
	if (!m_vecDeciveCMD.empty())
	{
		for(int i = 0; i < m_vecDeciveCMD.size(); i++){
			m_vecDeciveCMD[i].vecDevice.clear();
		}
		m_vecDeciveCMD.clear();
	}
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"fire.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	vector<SFire> vecFile;
	vecFile.reserve(300);
	int j=0; 	
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					
					// 删除空格
					del_sp(line);
					
					
					// 查找ip
					char* pListData = strstr(line, "=");
					char* pListData1 = strstr(line, "=addr=");
					char* pListData2 = strstr(line, "=path");
					char* pListData3 = strstr(line, "path=");
					char* pListData4 = strstr(line, "#");
					if (pListData && pListData1 && pListData2 && pListData3 && pListData4)
					{
						SFire sFire;
						memset(&sFire, 0, sizeof(SFire));
						sFire.groupID = atoi(line); 						
						memcpy(sFire.groupIP, pListData + 1, pListData1 - pListData - 1);
						sFire.extAddr = atoi(pListData1 + 6);
						memcpy(sFire.file, pListData3 + 5, pListData4 - pListData3 - 5);						
						vecFile.push_back(sFire);
					}
				}
			}
			else
			{
				break;
			}

			j++; 
		} 
		fclose( stream ); 
	}
	if (!vecFile.empty())
	{
		sort(vecFile.begin(), vecFile.end(), less_groupid);
		int groupid = -1;
		for (int i = 0; i < vecFile.size(); i++)
		{
			if (groupid == vecFile[i].groupID)
			{				
				m_vecDeciveCMD[m_vecDeciveCMD.size() - 1].vecDevice.push_back(vecFile[i].extAddr);
				
			}
			else{
				groupid = vecFile[i].groupID;
				SDeviceCmd stDevCmd;
				stDevCmd.groupID = groupid;
				memset(stDevCmd.file, 0, MAX_PATH);
				memcpy(stDevCmd.file, vecFile[i].file, strlen(vecFile[i].file));
				memset(stDevCmd.groupIP, 0, 32);
				memcpy(stDevCmd.groupIP, vecFile[i].groupIP, strlen(vecFile[i].groupIP));
				stDevCmd.vecDevice.push_back(vecFile[i].extAddr);
				m_vecDeciveCMD.push_back(stDevCmd);
			}
		}
		vecFile.clear();
	}
}
SDeviceCmd* CSIPCallDlg::GetDevCmd(int groupID)
{	
	for (int i = 0; i < m_vecDeciveCMD.size(); i++)
	{
		if (m_vecDeciveCMD[i].groupID == groupID)
		{
			vector<SDeviceCmd>::iterator vecIter = m_vecDeciveCMD.begin() + i;			
			return &(*vecIter);
		}
	}
	return NULL;
}
void CSIPCallDlg::ReadIPGroupFile()
{
	if (m_pIPGroupFile == NULL)
	{
		m_pIPGroupFile = (SKeyValue*)malloc(300* sizeof(SKeyValue));
		memset(m_pIPGroupFile, 0, 300*sizeof(SKeyValue));
	}
	m_nIPGroupFile = 0;
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"IPGroupBC.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					memset(m_pIPGroupFile + m_nIPGroupFile, 0, sizeof(SKeyValue));
					m_pIPGroupFile[m_nIPGroupFile].key = atoi(line); 
					// 查找ip
					char* pListData = strstr(line, "=");
					char* pListData2 = strstr(line, "#");
					if (pListData && pListData2)
					{
						m_pIPGroupFile[m_nIPGroupFile].value=atoi(pListData+1);//, pListData + 1,  pListData2 - pListData - 1);
						m_nIPGroupFile++;
					}
				}
			}
			else
			{
				break;
			}
			j++; 
		} 
		fclose( stream ); 
	}
}
void CSIPCallDlg::ReadDeviceLineFile()	  //读取停车带
{
	if (!m_vecLineData.empty())
	{
		m_vecLineData.clear();
	}
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"parking.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					// 查找ip
					char* pListData = strstr(line, "ip=");
					if (pListData)
					{
						char* ip1 = strstr(pListData, "=") + 1;
						char* ip2 = strstr(ip1, ".") + 1;
						char* ip3 = strstr(ip2, ".") + 1;
						char* ip4 = strstr(ip3, ".") + 1;
						if (ip1 && ip2 && ip3 && ip4)
						{
							BYTE btIP1 = atoi(ip1);
							BYTE btIP2 = atoi(ip2);
							BYTE btIP3 = atoi(ip3);
							BYTE btIP4 = atoi(ip4);
							if ((btIP1 >= 0 && btIP1 <= 255) && 
								(btIP2 >= 0 && btIP2 <= 255) && 
								(btIP3 >= 0 && btIP3 <= 255) && 
								(btIP4 >= 0 && btIP4 <= 255))
							{
								SNoIPFileData data;	
								memset(data.ip, 0, 32);
								sprintf(data.ip, "%d.%d.%d.%d", btIP1, btIP2, btIP3, btIP4);
								m_vecLineData.push_back(data);
							}

						}
					}
				}
			}
			else
			{
				break;
			}
			j++; 
		} 

		fclose( stream ); 
	}
}
bool CSIPCallDlg::IsExistPE(char* ip)
{
	if (m_vecLineData.empty())
	{
		return false;
	}
	for (int i = 0; i < m_vecLineData.size(); i++){
		if (strcmp(m_vecLineData[i].ip, ip) == 0)
		{
			return true;
		}
	}
	return false;
}
void CSIPCallDlg::ReadDeviceOutFile()
{
	if (!m_vecOutData.empty())
	{
		m_vecOutData.clear();
	}
	FILE *stream = NULL; 
	char line[1024] = {0}; 
	char szFileName[MAX_PATH] = {0};
	CString strFile =  g_szAPPPath + L"dongwai.txt";
	WideCharToMultiByte(CP_ACP, 0, strFile, -1, szFileName, MAX_PATH, NULL, NULL);
	int j=0; 
	if( (stream = fopen(szFileName, "r " )) != NULL ) 
	{ 
		while (!feof(stream)) 
		{ 
			memset(line, 0, 1024 * sizeof(char));
			char *ch = fgets( line, 1024, stream );//读一行	
			if (ch)
			{
				if ((strlen(ch) > 0) && (j > 0))
				{
					// 删除空格
					del_sp(line);
					// 查找ip
					char* pListData = strstr(line, "ip=");
					if (pListData)
					{
						char* ip1 = strstr(pListData, "=") + 1;
						char* ip2 = strstr(ip1, ".") + 1;
						char* ip3 = strstr(ip2, ".") + 1;
						char* ip4 = strstr(ip3, ".") + 1;
						if (ip1 && ip2 && ip3 && ip4)
						{
							BYTE btIP1 = atoi(ip1);
							BYTE btIP2 = atoi(ip2);
							BYTE btIP3 = atoi(ip3);
							BYTE btIP4 = atoi(ip4);
							if ((btIP1 >= 0 && btIP1 <= 255) && 
								(btIP2 >= 0 && btIP2 <= 255) && 
								(btIP3 >= 0 && btIP3 <= 255) && 
								(btIP4 >= 0 && btIP4 <= 255))
							{
								SNoIPFileData data;	
								memset(data.ip, 0, 32);
								sprintf(data.ip, "%d.%d.%d.%d", btIP1, btIP2, btIP3, btIP4);
								m_vecOutData.push_back(data);
							}

						}
					}
				}
			}
			else
			{
				break;
			}
			j++; 
		} 
		fclose( stream ); 
	}
}
bool CSIPCallDlg::IsDeviceOut(char* ip)
{
	if (m_vecOutData.empty())
	{
		return false;
	}
	for (int i = 0; i < m_vecOutData.size(); i++){
		if (strcmp(m_vecOutData[i].ip, ip) == 0)
		{
			return true;
		}
	}
	return false;
}
void CSIPCallDlg::SeatBeatReply(int extAddr)
{
	m_BeatDataLock.Lock();

	bool bFind = false;			
	for (int i = 0; i < m_vecBeatData.size(); i++)
	{
		if (m_vecBeatData[i].extAddr == extAddr)
		{
			m_vecBeatData[i].bReply = true;
			bFind = true;
		}
	}
	if (!bFind)
	{
		AddLogDB(extAddr);
	}
	m_BeatDataLock.Unlock();
}
void CSIPCallDlg::UpdateDeviceLog()
{
	m_BeatDataLock.Lock();
	if (!m_vecBeatData.empty())
	{
		vector<SRecord> vecRecord;
		vecRecord.reserve(500);
		for(int i = 0; i < m_vecBeatData.size(); i++){
			SDevice* pDevice = GetDevice(m_vecBeatData[i].extAddr);
			if (pDevice)
			{
				if (!m_vecBeatData[i].bReply)
				{
					// 防止巡检的时候正好打开
					if (IsNeedSendBeat(pDevice))
					{
						pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
						pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
						pDevice->errorCode = ENUM_ERROR_STATUS_UNKNOWN;	
					}				
				}
				// 写记录
				SRecord stRecord;
				if (Device2LogRecord(*pDevice, stRecord))
				{
					vecRecord.push_back(stRecord);
				}
			}
		}
		if (!vecRecord.empty())
		{
			// 批量写入
			g_data.InsertLogs(vecRecord);
			// 释放
			vecRecord.clear();
		}
		m_vecBeatData.clear();
		// 更新界面
		UpdateStatus();
		TRACE("update device log.....\r\n");
	}
	m_BeatDataLock.Unlock();
}
bool CSIPCallDlg::SleepThread(DWORD dwTime, bool bAutoExit)
{
	if (!bAutoExit)
	{
		Sleep(dwTime);
		return true;
	}
	else{
		Sleep(0);
		int count = dwTime;
		for (int i = 0; i < count; i ++)
		{
			if (g_bExitApp){
				return false;
			}
			else{
				Sleep(1);
			}
		}
		return true;
	}	
}
void CSIPCallDlg::OnSendOpenCam(SDevice* pDevice)
{
	if (pDevice == NULL)
	{
		return;
	}
	if(strlen(pDevice->sDB.ip_cam) < 6){
		return;
	}
//	szIP = "192.168.1.10";
	CWnd *pWnd=CWnd::FindWindow(NULL,_T("视频准备中...")); // 查找窗口

	if(pWnd==NULL){

		//AfxMessageBox("Unable to find DataRecv.");
		ShellExecute( NULL, _T("open"), g_szAPPPath + ("RTSP\\RTSP.exe"), NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
		Sleep(1000);
		pWnd=CWnd::FindWindow(NULL,_T("视频准备中..."));
		if (pWnd == NULL)
		{
			return;
		}
	}
	
	char pSendData[256] = {0};
	sprintf(pSendData, "*CPMD,CAM_OPEN,%s,%s,%s;", pDevice->sDB.tunnelName, pDevice->sDB.Name, pDevice->sDB.ip_cam);
	COPYDATASTRUCT cpd; // 给COPYDATASTRUCT结构赋值

	cpd.dwData = 0;

	cpd.cbData = strlen(pSendData);

	cpd.lpData = (void*)pSendData;

	pWnd->SendMessage(WM_COPYDATA,NULL,(LPARAM)&cpd); // 发送

}
void CSIPCallDlg::OnSendCloseCam(SDevice* pDevice)
{
	if (pDevice == NULL)
	{
		return;
	}
	if(strlen(pDevice->sDB.ip_cam) < 6){
		return;
	}
//	szIP = "192.168.1.10";
	char windowName[256] = {0};
	sprintf(windowName, "%s,%s,%s", pDevice->sDB.tunnelName, pDevice->sDB.Name, pDevice->sDB.ip_cam);
	wchar_t wzIP[256] = {0};
	MultiByteToWideChar(CP_ACP, 0, windowName, -1, wzIP, 256);
	CWnd *pWnd=CWnd::FindWindow(NULL, wzIP); // 查找窗口

	if(pWnd==NULL){
		return;
	}
	char pSendData[256] = {0};
	sprintf(pSendData, "*CPMD,CAM_CLOSE,%s;", windowName);
	
	COPYDATASTRUCT cpd; // 给COPYDATASTRUCT结构赋值

	cpd.dwData = 0;

	cpd.cbData = strlen(pSendData);

	cpd.lpData = (void*)pSendData;

	pWnd->SendMessage(WM_COPYDATA,NULL,(LPARAM)&cpd); // 发送
}
void CSIPCallDlg::OnMenuManBeat()
{
	// TODO: Add your command handler code here
	s_bBeatStop = true;
	s_bManulBeat = true;
	SendMessage(SEND_MSG_BEAT, 0, 0);
}
HBRUSH CSIPCallDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch(nCtlColor)
	{
	case CTLCOLOR_STATIC: //对所有静态文本控件的设置
		{
			if (pWnd->GetDlgCtrlID() == IDC_STATIC_SIGN_LEFT ||
				pWnd->GetDlgCtrlID() == IDC_STATIC_SIGN_RIGHT ||
				pWnd->GetDlgCtrlID() == IDC_STATIC_TUNNEL_TITLE)
			{
				pDC->SetBkMode(TRANSPARENT); 
				return HBRUSH(GetStockObject(HOLLOW_BRUSH)); // 必须
			}
		}
	default:
		return CDialog::OnCtlColor(pDC,pWnd, nCtlColor);
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
BOOL DirectoryExist1(CString Path)
{
	WIN32_FIND_DATA fd;
	BOOL ret = FALSE;
	HANDLE hFind = FindFirstFile(Path, &fd);
	if ((hFind != INVALID_HANDLE_VALUE) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//目录存在
		ret = TRUE;
	}
	FindClose(hFind);
	return ret;
}
void CSIPCallDlg::OnMenuVideo()
{
	// TODO: Add your command handler code here
	CString strVideo = g_szAPPPath + L"video";
	if (DirectoryExist1(strVideo))
	{
		ShellExecute(NULL, L"open", strVideo, NULL, NULL, SW_SHOW); 
	}
	else{
		MessageBox(L"视频记录不存在！", L"提醒");
	}
}
void CSIPCallDlg::OnBnClickedBtnTts()
{
	// TODO: Add your control notification handler code here
	CDlgBC dlg;
	CString strTTS = dlg.GetTTSFile();
	if((strTTS == "") ||
		(!IsTTSRun())){
		MessageBox(L"当前未播放文本内容！", L"提醒");
	}
	else{
		ShellExecute( NULL, _T("open"), strTTS, NULL, NULL, SW_SHOWNORMAL ); // cmd为命令行参数
	}

}
void CSIPCallDlg::OnMenuMonitor()
{
	// TODO: Add your command handler code here
	
	CDlgMonitor dlg;
	if (m_MonitorDevice.pDevice)
	{
		dlg.SetMonitorInfo(m_MonitorDevice.pDevice->sDB.tunnelName, m_MonitorDevice.pDevice->sDB.Name);
	}	
	if (dlg.DoModal() == IDOK)
	{
		SDevice* pDevice = dlg.GetDevice();

		if (pDevice && (pDevice->IsBcPlay() == ENUM_BC_OPT_LOCAL))
		{
#if _DEBUG
			PopCallThread(pDevice);
#endif
			if (pDevice->playSrc == ENUM_MODEL_FILE ||
				pDevice->playSrc == ENUM_MODEL_FILE_LIST ||
				pDevice->playSrc == ENUM_MODEL_TTS){
				
				if (m_MonitorDevice.pDevice == pDevice){// 还是监听上次的

				}
				else{// 监听不同对象
					if (m_MonitorDevice.pDevice != NULL){// 上次存在监听，则停止监听
						EndMonitor();
					}	
					Sleep(500);
					// 赋值新的监听
					m_MonitorDevice.pDevice = pDevice;
					// 开启新监听
					StartMonitor();
				}		
			}

		}
	}
	
}
int CSIPCallDlg::StartMonitor()
{	
	if (m_MonitorDevice.pDevice != NULL)
	{
		return lc_addip(&(m_MonitorDevice.pDevice->playParam), inet_addr( g_Config.m_SystemConfig.IP_Monitor));		
	}
	return 0;
}
void CSIPCallDlg::EndMonitor()
{
	if (m_MonitorDevice.pDevice)
	{
		if (m_MonitorDevice.pDevice->threadID > 0)
		{
			lc_delip(&(m_MonitorDevice.pDevice->playParam), inet_addr(g_Config.m_SystemConfig.IP_Monitor));
		}
	}
	m_MonitorDevice.pDevice = NULL;	
// 	m_MonitorDevice.bMonitor = false;
}
bool CSIPCallDlg::IsMonitorDevice(char* ip)
{
	if (m_MonitorDevice.pDevice == NULL)
	{
		return false;
	}
	else{
		if (strncmp(m_MonitorDevice.pDevice->sDB.ip, ip, strlen(m_MonitorDevice.pDevice->sDB.ip)) == 0)
		{
			return true;
		}
		return false;
	}	
}
void CSIPCallDlg::OnMenuMonitorClose()
{
	// TODO: Add your command handler code here
	EndMonitor();
	Sleep(200);
}
CString CSIPCallDlg::Txt2TTSPath(char* czTextPath)
{
	int nLen = strlen(czTextPath);
	int i = nLen - 1;
	for ( ; i > 0; i--)
	{
		if (czTextPath[i] == '\\')
		{
			break;
		}
	}
	char czTitle[MAX_PATH] = {0};
	memcpy(czTitle, czTextPath + i + 1, nLen - 1 - i);
	int nTitleLen = strlen(czTitle);
	if (nTitleLen > strlen(".txt"))
	{
		czTitle[nTitleLen - 4] = '.';
		czTitle[nTitleLen - 3] = 'w';
		czTitle[nTitleLen - 2] = 'a';
		czTitle[nTitleLen - 1] = 'v';
	}
	wchar_t wzTitle[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, czTitle, -1, wzTitle, MAX_PATH);
	m_strTTSFile.Format(L"%s%s\\%s", g_szAPPPath, L"music", wzTitle);
	return m_strTTSFile;
}
void CSIPCallDlg::OnBnClickedCheckUpAll()
{
	// TODO: Add your control notification handler code here
	int check = ((CButton*)GetDlgItem(IDC_CHECK_UP_ALL))->GetCheck();
	if (m_nCurTunnelIdx < 0 || m_nCurTunnelIdx >= m_nTunnelNum)
	{
		return;
	}
	
	for (int j = 0; j < m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp.size(); j++)
	{
		if(!m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[j].sDB.IsVisible()){
			m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[j].Check = 0;
			continue;
		}
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceUp[j].Check = check;
	}
	DB2DeviceBtn(m_nCurTunnelIdx);	
}
void CSIPCallDlg::OnBnClickedCheckDownAll()
{
	// TODO: Add your control notification handler code here
	int check = ((CButton*)GetDlgItem(IDC_CHECK_DOWN_ALL))->GetCheck();
	if (m_nCurTunnelIdx < 0 || m_nCurTunnelIdx >= m_nTunnelNum)
	{
		return;
	}
	
	for (int j = 0; j < m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown.size(); j++)
	{
		if(!m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[j].sDB.IsVisible()){
			m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[j].Check = 0;
			continue;
		}
		m_pTunnelInfo[m_nCurTunnelIdx].vecDeviceDown[j].Check = check;			
	}
	DB2DeviceBtn(m_nCurTunnelIdx);	
}
void CSIPCallDlg::CMD_PlayOneDevice(SDevice* pDevice, char* filePath)
{	
	if (pDevice->IsOffline()){
		return;
	}
	enBCOptStatus opt = pDevice->IsBcPlay();
	if (opt == ENUM_BC_OPT_NONE)
	{
		pDevice->Check = 1;
		pDevice->playSrc = ENUM_MODEL_FILE;
		pDevice->playMode = ENUM_PLAY_MODEL_ONE_CONTINUE;
		pDevice->cmd_open_bc_recv = 1;
		memset(pDevice->playFile, 0, MAX_PATH);
		memcpy(pDevice->playFile, filePath, MAX_PATH);
		bool bMic = false;
		bool bAux = false;
		g_Log.output(LOG_TYPE, "RECV_MSG_AUTO_BC_OPEN_RERECV, FILE=%s.........\r\n", pDevice->playFile);
		PlayOneDevice(pDevice, bMic, bAux, true);

		return;
	}
	else if (opt == ENUM_BC_OPT_REMOTE)
	{
		//发送远程关闭指令
		g_Log.output(LOG_TYPE, "ENUM_SEND_BC_OPT_CLOSE: StopOneDevice dlg: %d\r\n", pDevice->sDB.extAddr);
		//g_MSG.SendBCStatus(ENUM_SEND_BC_OPT_CLOSE, pDevice->sDB.extAddr);
		return;
	}	
	else if (opt == ENUM_BC_OPT_LOCAL){// 正在播放
		CMD_UpdateDeviceConfig(pDevice->sDB.extAddr, filePath);
		pDevice->enStopFlag = ENUM_STOP_MSG_CHANGE;
		if (pDevice->playSrc == ENUM_MODEL_FILE ||
			pDevice->playSrc == ENUM_MODEL_FILE_LIST ||
			pDevice->playSrc == ENUM_MODEL_TTS){
				// 先停，发送停止消息
				if (pDevice->threadID){				
					g_Play.stop(pDevice->playParam);						
				}
		}		
		else if (pDevice->playSrc == ENUM_MODEL_MIC){
			//bMic = true;
			//m_pAPP->EndPlay(pDevice);
			::SendMessage(m_pAPP->m_hWnd, RECV_MSG_STOP_MIC, NULL, (LPARAM)pDevice);
		}
		else if (pDevice->playSrc == ENUM_MODEL_AUX){
			//bAux = true;
			//m_pAPP->EndPlay(pDevice);
			::SendMessage(m_pAPP->m_hWnd, RECV_MSG_STOP_AUX, NULL, (LPARAM)pDevice);
		}
	}
}
void CSIPCallDlg::CMD_UpdateDeviceConfig(int extAddr, char* file)
{
	SPlayConfig config;
	config.extAddr = extAddr;
	config.playSrc = ENUM_MODEL_FILE;	
	config.playMode = ENUM_PLAY_MODEL_ONE_CONTINUE;
	memcpy(config.playFile, file, strlen(file));
	g_Log.output(LOG_TYPE, "cmd update config, ext[%d], src[%d], mode[%d], idx[%d], file[%s].......\r\n", config.extAddr, (int)config.playSrc, (int)config.playMode, config.curPlayIdx, config.playFile);
	g_data.AddPlayConfig(config);
}
LRESULT CSIPCallDlg::OnRecvMsgCallReqFromMasterSystem(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	BYTE callOpt = (BYTE)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		switch(callOpt)
		{
		case ENUM_CALL_OPEN_REQ:  //open call req
			{
				if(pDevice->callStatus == ENUM_DEVICE_STATUS_OK||
					pDevice->callStatus == ENUM_DEVICE_STATUS_ERROR){
						SCallMsg callmsg;
						callmsg.type = 3;
						callmsg.status = ENUM_CALL_REQ;
						callmsg.extAddr = extAddr;
						callmsg.timestamp = GetTickCount();
						if(!IsExistCall(extAddr))
						{
							m_vecCallMsg.push_back(callmsg);
						}else
						{
							g_MSG.SendHangUp(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
							Sleep(30);
							g_MSG.SendOpenCall(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
						}
				}else if (pDevice->callStatus == ENUM_DEVICE_STATUS_UNKNOWN)
				{
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);
				}
				else  if(pDevice->callStatus == ENUM_DEVICE_STATUS_RING)//电话正在使用，返回using状态
				{
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_RING);
				}else if(pDevice->callStatus == ENUM_DEVICE_STATUS_RUN)
				{
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_USING);
				}
			}
			break;
		case ENUM_CALL_CLOSE_REQ:  //close call req
			{
				SetCallMsgStatus(extAddr,ENUM_CALL_WAIT_E2);
				g_MSG.SendHangUp(pDevice->sDB.extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
			}
			break;
		case ENUM_CALL_DIVERT_OPEN_REQ:
			{
				//pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_RUN;
				Status2Button(extAddr);

				//呼叫转移、告知分控打开分机
				//此时主控收到的是ring 事件，若分机不回应E1事件，通知主控分机掉线，
				//此时分控不应该等待ring事件
				/************************************************************************/
				/* 这里的逻辑需要梳理 ，怎么产生通话事件  */
				/************************************************************************/
				DeleteCallMsg(extAddr);
				DeleteCallAnswer(extAddr);
				g_MSG.SendHangUp(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
				Sleep(50);
				//打开分机
				g_MSG.SendOpenCall(pDevice->sDB.extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
			}
			break;
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgE1(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;		//分机号
	SDevice* pDevice = GetDevice(extAddr);
	SCallMsg* pMsg = GetCallMsg(extAddr);
	if(pDevice&&pMsg){
		//CString str1, str2;
		if(pMsg->status==ENUM_CALL_WAIT_E1&&IsDeviceLocalController(pDevice->dwIP))
		{
			SetCallMsgStatus(extAddr,ENUM_CALL_E1);
			char pSendBuf[1024] = {0};	
			if (pMsg->type == 2){
				char* pGroupCall = GetGroup(pDevice->sDB.extAddr);
				if (pGroupCall)
				{
					sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Transfer attribute=\"Connect\">\r\n  <ext id=\"%s\"/>\r\n<outer to=\"%s\"/>\r\n</Transfer>", pDevice->sDB.extNo, pGroupCall);
				}
				else{
					sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Transfer attribute=\"Connect\">\r\n  <ext id=\"%s\"/>\r\n<outer to=\"%s\"/>\r\n</Transfer>", pDevice->sDB.extNo, g_Config.m_SystemConfig.Outer);
				}
			}
			else{
				sprintf(pSendBuf, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n<Transfer attribute=\"Connect\">\r\n  <ext id=\"%s\"/>\r\n<outer to=\"%s\"/>\r\n</Transfer>", pDevice->sDB.extNo, g_Config.m_SystemConfig.Outer);
			}

			//g_Log.output(LOG_TYPE, pSendBuf);
			TCHAR ipserver[MAX_PATH] = {0};
			MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.IP_Server, -1, ipserver, MAX_PATH);
			CString IP_Server = _T("");
			IP_Server.Format(_T("http://%s"), ipserver/*g_Config.m_SystemConfig.IP_Server*/);
#if 0


			if(s_UsingNetwork!=-1)
			{
				if(g_hSipgwPacketEvent&&!s_OmSend)
				{
					SetEvent(g_hSipgwPacketEvent);
				}else{
					AfxBeginThread(CheckSipgwUdpPacketProc,this);
				}
			}
#endif
			if (g_Config.m_SystemConfig.bHttpConnect)
			{
				m_http.HTTP_POST_DATA(IP_Server, pSendBuf/*, str1, str2*/);
			}
			else{
				m_http.TCP_POST_DATA(/*g_Config.m_SystemConfig.IP_Server, g_Config.m_SystemConfig.portEvtSend, */pSendBuf);
			}
			SetCallMsgStatus(extAddr,ENUM_CALL_WAIT_RING);
		}else{
			if(pMsg->status== ENUM_CALL_OPEN_ACK||pMsg->status == ENUM_CALL_WAIT_E1){
				//呼叫转移 通知分控打开 隧道分机 指令
				DeleteCallMsg(extAddr);
				DeleteCallAnswer(extAddr);
			}
		}
	}else{
		DeleteCallMsg(extAddr);
		g_Log.output(LOG_TYPE,"on recv e1 delete call msg....\r\n");
		DeleteCallAnswer(extAddr);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgE2(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	SCallMsg* pMsg = GetCallMsg(extAddr);
	SDevice* pDevice = GetDevice(extAddr);
	if(pMsg&&ENUM_CALL_WAIT_E2==pMsg->status)
	{
		g_Log.output(LOG_TYPE,"on recv e2 delete call msg and not wait_e1....\r\n");
		DeleteCallMsg(extAddr);
		DeleteCallAnswer(extAddr);
		if(pDevice){
			if(g_Config.m_SystemConfig.nMainSystem)
			{
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
			}else
			{
				g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USABLE);
			}
		}
	}
	//SDevice* pDevice = GetDevice(extAddr);
	if (pDevice && (pDevice->callStatus == ENUM_DEVICE_STATUS_UNKNOWN))
	{
		UpdateStatus_CALL(wParam, ENUM_DEVICE_STATUS_OK);
		//CK注释，更新状态->电话待机状态
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgE3(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	SBCMsg* pBCMsg = GetBCMsg(extAddr);
	SDevice* pDevice = GetDevice(extAddr);
	if(pBCMsg&&pDevice){
		if(pBCMsg->status == ENUM_BC_WAIT_E3){
			pBCMsg->status =  ENUM_BC_E3;
			if(pBCMsg->type == 2)
			{
				g_MSG.Send2OuterACK(atoi(pDevice->sDB.extNo),0,1);
			}
		}
		/*else
			DeleteBCMSG(extAddr);*/
		
		if(pDevice->cmd_open_bc_recv)
			UpdateStatus_BC(wParam,ENUM_DEVICE_STATUS_OUTER_USING);
		else
			UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_RUN);
		
	}else{
		UpdateStatus_BC(wParam,ENUM_DEVICE_STATUS_REMOTE_USING);
	}
	/*CString bcmsg ;
	bcmsg.Format(L"%d 分机正在使用广播...",extAddr);
	ShowInfo(bcmsg);*/
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgE4(WPARAM wParam,LPARAM lParam)
{
	//删除广播事件
	SBCMsg* pBCMsg = GetBCMsg((int)wParam);
	if(pBCMsg&& ENUM_BC_WAIT_E4 == pBCMsg->status)
	{
		UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_OK);
		SDevice* pDevice = GetDevice((int)wParam);
		if(g_Config.m_SystemConfig.nMainSystem&&pDevice)
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_BC_CLOSE);
		if(/*g_Config.m_SystemConfig.nMainSystem&&*/pBCMsg->type == 2&&pDevice)
		{
			g_MSG.Send2OuterACK(atoi(pDevice->sDB.extNo),1,1);
		}
		DeleteBCMSG((int)wParam);
	}
	return 0;
}
UINT SendBeat2OfflineProc(LPVOID extAddr);
LRESULT CSIPCallDlg::OnRecvNoE1Answer(WPARAM wParam,LPARAM lParam)
{
	CString str;
	str.Format(L"分机%d异常，未收到E1",wParam);
	ShowInfo(str);
	int extAddr = (int)wParam;
	SCallMsg* pMsg = (SCallMsg*)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		g_Log.output(LOG_TYPE,"recv msg no e1..extAddr:%d,extNo:%s\r\n",extAddr,pDevice->sDB.extNo);
		if(pMsg->type == 4)
		{
			if(pMsg->nSend<2){
				g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_OPEN_REQ);
				pMsg->timestamp = GetTickCount();
			}
			else
			{
				g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_CLOSE_REQ);
				if(IsBCRun(extAddr))
				{
					DeviceStop(pDevice,false);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
				}
				pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				Status2Button(extAddr);
				//SetDlgItemText(IDC_STATIC_MSG,L"e1 no answer");
				//g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);
				DeleteBCMSG(extAddr);

				AfxBeginThread(SendBeat2OfflineProc,(LPVOID)&extAddr);
			}
		}else if(pMsg->type == 5)
		{
			if(pMsg->nSend<2)
			{
				g_MSG.SendCallReq2Branch(extAddr,pDevice->dwIP,ENUM_CALL_DIVERT_OPEN_REQ);
				pMsg->timestamp = GetTickCount();
			}else
			{

			}
		}
		else{
			if(pMsg->nSend>=2){
				if(IsBCRun(extAddr))
				{
					DeviceStop(pDevice,false);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
				}
				pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				Status2Button(extAddr);
				//SetDlgItemText(IDC_STATIC_MSG,L"e1 no answer");
				g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);
				DeleteBCMSG(extAddr);

				AfxBeginThread(SendBeat2OfflineProc,(LPVOID)&extAddr);
				//DeleteCallMsg(extAddr);
			}else
			{
				g_MSG.SendHangUp(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
				//g_MSG.SendCloseBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,false);
				//Call(pMsg);
				Sleep(50);
				g_MSG.SendOpenCall(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
				pMsg->timestamp = GetTickCount();
			}
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvNoE2Answer(WPARAM wParam,LPARAM lParam)
{
	CString str;
	str.Format(L"分机%d异常，未收到E2",wParam);
	ShowInfo(str);
	int extAddr = (int)wParam;
	int type = (int)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		g_Log.output(LOG_TYPE,"recv msg no e2..extAddr:%d,extNo:%s\r\n",extAddr,pDevice->sDB.extNo);
		if(IsBCRun(extAddr))
		{
			DeviceStop(pDevice,false);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
		}
		pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
		pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
		Status2Button(extAddr);
		//SetDlgItemText(IDC_STATIC_MSG,L"e1 no answer");
		g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);	
		DeleteBCMSG(extAddr);
		//DeleteCallMsg(extAddr);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvNoE3Answer(WPARAM wParam,LPARAM lParam)
{
	CString str;
	str.Format(_T("分机%d异常，未收到E3"),(int)wParam);
	ShowInfo(str);
	int extAddr = (int)wParam;
	SBCMsg* pBCMsg = (SBCMsg*)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		g_Log.output(LOG_TYPE,"recv msg no e3..extAddr:%d,extNo:%s\r\n",extAddr,pDevice->sDB.extNo);
		if(!IsDeviceLocalController(pDevice->dwIP)){
			if(pBCMsg->nSend<2){
				g_MSG.SendBroadcastReq2Branch(extAddr,pDevice->dwIP,ENUM_BC_OPEN_REQ);
				pBCMsg->timestamp = GetTickCount();
			}
			else{
				g_MSG.SendBroadcastReq2Branch(extAddr,pDevice->dwIP,ENUM_BC_CLOSE_REQ);
				if(IsBCRun(extAddr))
				{
					DeviceStop(pDevice,false);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
					//pDevice->enStopFlag = enStopMsg.ENUM_STOP_MSG_CHANGE;
				}
				pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				Status2Button(extAddr);

				AfxBeginThread(SendBeat2OfflineProc,(LPVOID)&extAddr);
				if(pBCMsg->type == 2)
				{
					g_MSG.Send2OuterACK(atoi(pDevice->sDB.extNo),0,0);
				}
			}
		}else
		{
			if(pBCMsg->nSend<2)
			{
				g_MSG.SendCloseBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,false);
				Sleep(50);
				g_MSG.SendOpenBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
				pBCMsg->timestamp = GetTickCount();
			}else
			{
				if(IsBCRun(extAddr))
				{
					DeviceStop(pDevice,false);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
					//pDevice->enStopFlag = enStopMsg.ENUM_STOP_MSG_CHANGE;
				}
				pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				Status2Button(extAddr);
				//SetDlgItemText(IDC_STATIC_MSG,L"e1 no answer");
				if(!g_Config.m_SystemConfig.nMainSystem)
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);


				AfxBeginThread(SendBeat2OfflineProc,(LPVOID)&extAddr);
				//DeleteBCMSG(extAddr);
				//DeleteCallMsg(extAddr);
				if(pBCMsg->type == 2)
				{
					g_MSG.Send2OuterACK(atoi(pDevice->sDB.extNo),0,0);
				}
			}
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvNoE4Answer(WPARAM wParam,LPARAM lParam)
{
	CString str;
	str.Format(L"分机%d异常，未收到E4",wParam);
	ShowInfo(str);
	int extAddr = (int)wParam;
	SBCMsg* pBCMsg = (SBCMsg*)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		g_Log.output(LOG_TYPE,"recv msg no e4..extAddr:%d,extNo:%s\r\n",extAddr,pDevice->sDB.extNo);
		if(IsBCRun(extAddr))
		{
			DeviceStop(pDevice,false);
			pDevice->threadID = 0;
			pDevice->playTime = 0;
			pDevice->playFlag = 0;
			pDevice->Check = 0;
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
		}
		if(pBCMsg->nSend<2)
		{
			if(!IsDeviceLocalController(pDevice->dwIP))
			{
				g_MSG.SendBroadcastReq2Branch(extAddr,pDevice->dwIP,ENUM_BC_CLOSE_REQ);
			}else
			{
				g_MSG.SendCloseBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,false);
			}
			pBCMsg->timestamp = GetTickCount();
		}else
		{
			pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
			pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
			Status2Button(extAddr);
			//SetDlgItemText(IDC_STATIC_MSG,L"e1 no answer");
			if(!g_Config.m_SystemConfig.nMainSystem)
				g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);
			if(g_Config.m_SystemConfig.nMainSystem&&pBCMsg->type == 2)
			{
				g_MSG.Send2OuterACK(atoi(pDevice->sDB.extNo),1,0);
			}
		}
		
		//DeleteBCMSG(extAddr);
		//DeleteCallMsg(extAddr);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvNoRingEvt(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	CString str;
	str.Format(L"OM%d异常，未收到RING EVENT",wParam);
	ShowInfo(str);
	SDevice* pDevice = GetDevice(extAddr);
	if (pDevice)
	{
		g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OM_DOWN);
		g_MSG.SendHangUp(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
		g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
		if(IsBCRun(extAddr))
		{
			DeviceResume(pDevice);
		}
		//DeviceStop(pDevice,false);
		//DeleteBCMSG(extAddr);
		//pDevice->callStatus = ENUM_DEVICE_STATUS_ERROR;
		//pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
		pDevice->callStatus = ENUM_DEVICE_OM_DOWN;
		Status2Button(extAddr);
		DeleteCallAnswer(extAddr);
		//DeleteCallMsg(extAddr);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallFromDevice(WPARAM wParam,LPARAM lParam)
{
	// 		if (g_Config.m_SystemConfig.nMainSystem)
	// 		{
	if (g_Logic.m_UpdateDB)
	{
		return 0;
	}		
	AnswerCall(wParam);
	g_Log.output(LOG_TYPE, "RECV_MSG_CALL: %d\r\n", wParam);
	//		}		
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallOnlineFromOM(WPARAM wParam,LPARAM lParam)
{
	if (wParam == atoi(g_Config.m_SystemConfig.CallA))
	{
		m_CallA.status = ENUM_DEVICE_STATUS_OK;
		m_CallA.clearCall(); //电话A初始化
	} 
	else if (wParam == atoi(g_Config.m_SystemConfig.CallB))		
	{				//Asc  --int 
		m_CallB.status = ENUM_DEVICE_STATUS_OK;
		m_CallB.clearCall();			//电话B初始化
	}	
	{
		SEventInfo* pEventInfo = (SEventInfo*)lParam;
		if (pEventInfo)
		{
			SDevice* pDevice = GetDevice(pEventInfo->extNo);
			if (pDevice)
			{
				g_Log.output(LOG_TYPE,"on recv call online delete call msg....\r\n");
				DeleteCallMsg(pDevice->sDB.extAddr);
				EventChangeBtn(pDevice, RECV_MSG_CALL_OK);  //分机呼叫A/B电话结束后，恢复分机待机状态
				if(!g_Config.m_SystemConfig.nMainSystem)
				{
					g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USABLE);
					g_Log.output(LOG_TYPE,"recv call online from om\r\n");
				}else
				{
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
				}
			}
			//m_vecCallEvent.push_back(*pEventInfo);
			SDevice* pDevice1 = GetDeviceFromEvent(pEventInfo);
			if (pDevice1)
			{
				g_Log.output(LOG_TYPE,"on recv call online delete call msg....\r\n");
				DeleteCallMsg(pDevice1->sDB.extAddr);
				EventChangeBtn(pDevice1, RECV_MSG_CALL_OK);  //分机呼叫A/B电话结束后，恢复分机待机状态
				if(!g_Config.m_SystemConfig.nMainSystem)
				{
					g_MSG.SendDeviceCallStatus2Master(pDevice1->sDB.extAddr,ENUM_CALL_IS_USABLE);
					g_Log.output(LOG_TYPE,"recv call online from om\r\n");
				}else
				{
					g_MSG.SendCmd(atoi(pDevice1->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
				}
			}
			RemoveRing(pEventInfo->id);   //删除电话A/B 的通话状态(from  pDevice)
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallOfflineFromOM(WPARAM wParam,LPARAM lParam)
{
	SEventInfo* pEventInfo = (SEventInfo*)lParam;
	if (pEventInfo == NULL)
	{
		return 0;
	}	
	//m_vecCallEvent.push_back(*pEventInfo);
	if (wParam == atoi(g_Config.m_SystemConfig.CallA))
	{	
		//电话A的状态 ：未知
		m_CallA.status = ENUM_DEVICE_STATUS_UNKNOWN;
		//电话A全部置0
		m_CallA.clearCall(); 
	} 
	if (wParam == atoi(g_Config.m_SystemConfig.CallB))	
	{
		m_CallB.status = ENUM_DEVICE_STATUS_UNKNOWN;
		m_CallB.clearCall();
	}
	g_Log.output(LOG_TYPE,"on recv call offline delete call msg....\r\n");
	
	SDevice* pDevice = GetDevice(atoi(pEventInfo->from));
	if (pDevice)
	{
		if(IsBCRun(pDevice->sDB.extAddr))
		{
			DeviceStop(pDevice,false);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
		}
		DeleteCallMsg(pDevice->sDB.extAddr);
		DeleteBCMSG(pDevice->sDB.extAddr);
		EventChangeBtn(pDevice,RECV_MSG_CALL_OFFLINE);
		if(!g_Config.m_SystemConfig.nMainSystem)
			g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_OFFLINIE);
	}
	
	SDevice* pDevice1 = GetDevice(pEventInfo->extNo);
	if (pDevice1)
	{
		if(IsBCRun(pDevice1->sDB.extAddr))
		{
			DeviceStop(pDevice1,false);
			g_MSG.SendCmd(atoi(pDevice1->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
		}
		DeleteCallMsg(pDevice1->sDB.extAddr);
		DeleteBCMSG(pDevice1->sDB.extAddr);
		EventChangeBtn(pDevice1,RECV_MSG_CALL_OFFLINE);
		if(!g_Config.m_SystemConfig.nMainSystem)
			g_MSG.SendDeviceCallStatus2Master(pDevice1->sDB.extAddr,ENUM_CALL_IS_OFFLINIE);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallIdleFromOM(WPARAM wParam,LPARAM lParam)
{
	if (wParam == atoi(g_Config.m_SystemConfig.CallA))
	{
		m_CallA.status = ENUM_DEVICE_STATUS_OK;
		m_CallA.clearCall();
		s_callEvtTime = GetTickCount();
	} 
	if (wParam == atoi(g_Config.m_SystemConfig.CallB))	
	{
		m_CallB.status = ENUM_DEVICE_STATUS_OK;
		m_CallB.clearCall();	
		s_callEvtTime = GetTickCount();
	}		
	{
		//g_Log.output(LOG_TYPE, "RECV_MSG_CALL_IDLE............\r\n");
		SEventInfo* pEventInfo = (SEventInfo*)lParam;
		if (pEventInfo == NULL)
		{
			return 0;
		}	
		//m_vecCallEvent.push_back(*pEventInfo);
		SEventInfo* bAnswer = IsEventTypeExist(pEventInfo->extNo, ENUM_EVENT_ANSWER);
		//g_Log.output(LOG_TYPE, "RECV_MSG_CALL_IDLE 1............\r\n");
		if (!bAnswer)//空闲线路
		{
			//g_Log.output(LOG_TYPE, "RECV_MSG_CALL_IDLE 2............\r\n");
			SDevice* pDevice = GetDevice(pEventInfo->extNo);
			if (pDevice)
			{
				/*pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
				Status2Button(pDevice->sDB.extAddr);*/
				EventChangeBtn(pDevice,RECV_MSG_CALL_IDLE);
				if (!g_Config.m_SystemConfig.nMainSystem)
				{
					g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USABLE);
					g_Log.output(LOG_TYPE,"recv call idle from om\r\n");
				}else
				{
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
				}
			}
			//g_Log.output(LOG_TYPE, "RECV_MSG_CALL_IDLE 3............\r\n");
			// 				g_Log.output(LOG_TYPE, "RECV_MSG_CALL_IDLE no answer............\r\n");
			// 				CloseCallWithoutAnswer(pEventInfo->extNo);
		}
		else{
			g_Log.output(LOG_TYPE, "RECV_MSG_CALL_IDLE answer............\r\n");
		}
		DeleteEvent(pEventInfo->extNo);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallRingFromOM(WPARAM wParam,LPARAM lParam)
{
	SEventInfo* pEventInfo = (SEventInfo*)lParam;
	if (pEventInfo == NULL)
	{
		return 0;
	}
	SDevice* pDevice = GetDeviceFromEvent(pEventInfo);
	
	EventChangeBtn(pDevice, RECV_MSG_CALL_RING);
	if (pDevice && (atoi(pEventInfo->extNo) != atoi(g_Config.m_SystemConfig.CallA) &&
		atoi(pEventInfo->extNo) != atoi(g_Config.m_SystemConfig.CallB))) //分机呼外部电话
	{
		g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_RING);
		SetCallMsgStatus(pDevice->sDB.extAddr,ENUM_CALL_RING);
	}
	else if (pDevice)
	{
		SetCallMsgStatus(pDevice->sDB.extAddr,ENUM_CALL_RING);
		m_vecCallEvent.push_back(*pEventInfo);
		if(!IsDeviceLocalController(pDevice->dwIP))
		{
			//呼叫转移振铃，告知分控打开分机（send open call cmd）
			SCallMsg callMsg;
			callMsg.extAddr = pDevice->sDB.extAddr;
			callMsg.status = ENUM_CALL_OPEN_ACK;
			callMsg.timestamp = GetTickCount();
			callMsg.type = 5;
			callMsg.nSend = 0;
			if(!IsExistCall(callMsg.extAddr))
			{
				m_vecCallMsg.push_back(callMsg);
			}
			g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_DIVERT_OPEN_REQ);
			if(wParam == atoi(g_Config.m_SystemConfig.CallA))
			{
				m_CallA.preStatus = m_CallA.status;
				m_CallA.status = ENUM_DEVICE_STATUS_RING;  //设备拨号状态
				m_CallA.timestamp = GetTickCount();
				m_CallA.id = pEventInfo->id;

				sprintf(m_CallA.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
				m_CallA.extAddr = pDevice->sDB.extAddr;	
			}else if(wParam == atoi(g_Config.m_SystemConfig.CallB))
			{
				m_CallB.preStatus = m_CallB.status;
				m_CallB.status = ENUM_DEVICE_STATUS_RING;
				m_CallB.timestamp = GetTickCount();
				m_CallB.id = pEventInfo->id;
				sprintf(m_CallB.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
				m_CallB.extAddr = pDevice->sDB.extAddr;
			}
		}else{
			//收到振铃事件，OM工作正常，删除通话消息
			//DeleteCallMsg(pDevice->sDB.extAddr);
			//告知其他分控、主控，分机正在使用
			if(!g_Config.m_SystemConfig.nMainSystem)
			{
				g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_RING);
			}
			else
			{
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_RING);
			}
		}
	}

	if (m_CallA.status == ENUM_DEVICE_STATUS_OK)
	{
		if (pDevice&&m_CallA.extAddr == pDevice->sDB.extAddr)
		{
			m_CallA.preStatus = m_CallA.status;
			m_CallA.status = ENUM_DEVICE_STATUS_RING;  //设备拨号状态
			m_CallA.timestamp = GetTickCount();
			m_CallA.id = pEventInfo->id;

			sprintf(m_CallA.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
			m_CallA.extAddr = pDevice->sDB.extAddr;	
		}				
	} 
	if (m_CallB.status == ENUM_DEVICE_STATUS_OK)		
	{				
		if (pDevice&&m_CallB.extAddr == pDevice->sDB.extAddr)
		{
			m_CallB.preStatus = m_CallB.status;
			m_CallB.status = ENUM_DEVICE_STATUS_RING;
			m_CallB.timestamp = GetTickCount();
			m_CallB.id = pEventInfo->id;
			sprintf(m_CallB.szName, "%s: %s", pDevice->sDB.Name, pDevice->sDB.ip);		
			m_CallB.extAddr = pDevice->sDB.extAddr;
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallRunningFromOM(WPARAM wParam,LPARAM lParam)
{
	SEventInfo* pEventInfo = (SEventInfo*)lParam;
	if (pEventInfo == NULL)
	{
		return 0;
	}		
	// del 2017/08/13 m_vecCallEvent.push_back(*pEventInfo);
	//
	SDevice* pDevice = GetDeviceFromEvent(pEventInfo);
	EventChangeBtn(pDevice, RECV_MSG_CALL_RUN);
	// 设置接听
	//if (pDevice)
	//{
	//	TRACE("on recv call running delete call msg....\r\n");
	//	DeleteCallAnswer(pDevice->sDB.extAddr);
	//	//DeleteCallMsg(pDevice->sDB.extAddr);
	//	if(!IsCallRun(pDevice->sDB.extAddr))
	//	{
	//		g_MSG.SendOpenCall(pDevice->sDB.extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
	//	}
	//}
	if (wParam == atoi(g_Config.m_SystemConfig.CallA))//配置电话A
	{
		//if (m_CallB.status == ENUM_DEVICE_STATUS_RING)
		//{
		//	m_CallB.status = ENUM_DEVICE_STATUS_OK;//m_CallB.preStatus;
		//	m_CallB.clearCall();
		//}
		//SDevice* pDevice = GetDevice(m_CallA.extAddr);
		if (pDevice != NULL)
		{			
			m_CallA.status = ENUM_DEVICE_STATUS_RUN;
			m_CallA.timestamp = GetTickCount();
			PopCallThread(pDevice);
			//g_MSG.SendOpenCall(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
			
			OnSendOpenCam(pDevice);
			SRecord stRecord;
			CharToTchar(pDevice->sDB.ip, stRecord.IP);
			CharToTchar(pDevice->sDB.tunnelName, stRecord.tunnel);
			CharToTchar(pDevice->sDB.kmID, stRecord.kmid);
			CharToTchar(pDevice->sDB.extNo, stRecord.sip);
			CharToTchar(pDevice->sDB.Name, stRecord.noid);
			// 接警电话？？？？？？？？？？？
			CharToTchar(g_Config.m_SystemConfig.CallA, stRecord.desc);
			stRecord.addr = pDevice->sDB.extAddr;
			g_data.InsertCall(stRecord);
			pDevice->callStatus = ENUM_DEVICE_STATUS_RUN;
			UpdateStatus();	
			SetCallMsgStatus(pDevice->sDB.extAddr,ENUM_CALL_ANSWER);
			if(g_Config.m_SystemConfig.nMainSystem)
			{
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_START);
			}else
			{
				g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USING);
			}
		}				
	} 
	else if (wParam == atoi(g_Config.m_SystemConfig.CallB))	//配置电话B
	{
		//if (m_CallA.status == ENUM_DEVICE_STATUS_RING)
		//{
		//	m_CallA.status = ENUM_DEVICE_STATUS_OK;//m_CallA.preStatus;
		//	m_CallA.clearCall();
		//}
		//SDevice* pDevice = GetDevice(m_CallB.extAddr);
		if (pDevice != NULL)
		{		
			m_CallB.status = ENUM_DEVICE_STATUS_RUN;
			m_CallB.timestamp = GetTickCount();
			PopCallThread(pDevice);
			//g_MSG.SendOpenCall(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
			//g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_START);
			OnSendOpenCam(pDevice);
			SRecord stRecord;
			CharToTchar(pDevice->sDB.ip, stRecord.IP);
			CharToTchar(pDevice->sDB.tunnelName, stRecord.tunnel);
			CharToTchar(pDevice->sDB.kmID, stRecord.kmid);
			CharToTchar(pDevice->sDB.extNo, stRecord.sip);
			CharToTchar(pDevice->sDB.Name, stRecord.noid);
			// 接警电话？？？？？？？？？？？
			CharToTchar(g_Config.m_SystemConfig.CallB, stRecord.desc);
			stRecord.addr = pDevice->sDB.extAddr;
			g_data.InsertCall(stRecord);
			pDevice->callStatus = ENUM_DEVICE_STATUS_RUN;
			UpdateStatus();	
			SetCallMsgStatus(pDevice->sDB.extAddr,ENUM_CALL_ANSWER);
			if(g_Config.m_SystemConfig.nMainSystem)
			{
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_START);
			}else
			{
				g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USING);
			}
		}				
	}
	else {// 非本地接听
		RemoveRing(pEventInfo->id);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallHungUpFromOM(WPARAM wParam,LPARAM lParam)
{
	SEventInfo* pEventInfo = (SEventInfo*)lParam;
	if (pEventInfo && atoi(pEventInfo->extNo)) //ascii to integer
	{
		//m_vecCallEvent.push_back(*pEventInfo);
		SDevice* pDevice = GetDeviceFromEvent(pEventInfo);
		EventChangeBtn(pDevice, RECV_MSG_CALL_HANGUP);      //设置呼叫分机为挂机状态				
		if (wParam == atoi(g_Config.m_SystemConfig.CallA))	  //CALLA
		{					
			// 关闭声音	 //关闭分机解码板			
			if (pDevice)
			{
				if(IsBCRun(pDevice->sDB.extAddr)){
					DeviceResume(pDevice);
				}
				Sleep(300);
				if(IsDeviceLocalController(pDevice->dwIP)){
					g_MSG.SendHangUp(pDevice->sDB.extAddr/*m_CallA.extAddr*/, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
					
					OnSendCloseCam(pDevice);

					//不删除通话事件，为了判断是否收到挂机指令回复0xE2，
					//收到后删除通话事件，否则进入异常处理
					SetCallMsgStatus(pDevice->sDB.extAddr,ENUM_CALL_WAIT_E2);
					if(!g_Config.m_SystemConfig.nMainSystem){
						g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USABLE);
						//g_Log.output(LOG_TYPE,"recv call hung up from om");
					}else
					{
						g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);
					}
				}else{
					//呼叫转移情况下主控收到的挂机指令，删除通话事件，告知分控关闭分机
					g_Log.output(LOG_TYPE,"on recv hungup by divert delete call msg....\r\n");
					DeleteCallMsg(pDevice->sDB.extAddr);
					DeleteCallAnswer(pDevice->sDB.extAddr);
					g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_CLOSE_REQ);
				}
			}
			// 恢复电话A 的状态
			m_CallA.status = ENUM_DEVICE_STATUS_OK;//m_CallA.preStatus;					
			m_CallA.clearCall();
			s_callEvtTime = GetTickCount();
		}
		else if (wParam == atoi(g_Config.m_SystemConfig.CallB))	  //CALLB
		{				
			// 关闭声音				
			if (pDevice)
			{
				if(IsBCRun(pDevice->sDB.extAddr)){
					DeviceResume(pDevice);
				}
				Sleep(300);
				if(IsDeviceLocalController(pDevice->dwIP)){
					g_MSG.SendHangUp(pDevice->sDB.extAddr/*m_CallB.extAddr*/, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
					
					OnSendCloseCam(pDevice);
					SetCallMsgStatus(pDevice->sDB.extAddr,ENUM_CALL_WAIT_E2);
					if(!g_Config.m_SystemConfig.nMainSystem){
						g_MSG.SendDeviceCallStatus2Master(pDevice->sDB.extAddr,ENUM_CALL_IS_USABLE);
						//g_Log.output(LOG_TYPE,"recv call hung up from om");
					}else
					{
						g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);
					}
				}else
				{
					//呼叫转移情况下主控收到的挂机指令，删除通话事件，告知分控关闭分机
					g_Log.output(LOG_TYPE,"on recv hungup by divert delete call msg....\r\n");
					DeleteCallMsg(pDevice->sDB.extAddr);
					DeleteCallAnswer(pDevice->sDB.extAddr);
					g_MSG.SendCallReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_CALL_CLOSE_REQ);
				}
			}
			m_CallB.status = ENUM_DEVICE_STATUS_OK;//m_CallB.preStatus;					
			m_CallB.clearCall();
			s_callEvtTime = GetTickCount();
		}else{
			//分机呼分控 ，呼叫转移到主控 ，主控接听后挂断，分控收到的挂机指令
			SDevice* pDevice1 = GetDevice(pEventInfo->extNo);
			EventChangeBtn(pDevice1, RECV_MSG_CALL_HANGUP);
			if(pDevice1&&IsDeviceLocalController(pDevice1->dwIP))
			{
				if(IsBCRun(pDevice1->sDB.extAddr)){
					DeviceResume(pDevice1);
				}
				Sleep(300);
				g_MSG.SendHangUp(pDevice1->sDB.extAddr, pDevice1->sDB.ip, g_Config.m_SystemConfig.portDevice);
				//g_MSG.SendCmd(atoi(pDevice1->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);
				SetCallMsgStatus(pDevice1->sDB.extAddr,ENUM_CALL_WAIT_E2);
				OnSendCloseCam(pDevice1);
				if(!g_Config.m_SystemConfig.nMainSystem){
					g_MSG.SendDeviceCallStatus2Master(pDevice1->sDB.extAddr,ENUM_CALL_IS_USABLE);
					//g_Log.output(LOG_TYPE,"recv call hung up from om");
				}else
				{
					g_MSG.SendCmd(atoi(pDevice1->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);
				}
			}
		}	
		RemoveRing(pEventInfo->id);    //删除电话A/B的 通话事件
		// 删除事件信息
		//DeleteEvent(pEventInfo->extNo);
	}	
	else if (pEventInfo)
	{
		RemoveRing(pEventInfo->id);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallNoAnswerFromOM(WPARAM wParam,LPARAM lParam)
{
	SEventInfo* pEventInfo = (SEventInfo*)lParam;
	if (pEventInfo == NULL)
	{
		return 0;
	}

	SDevice* pDevice1 = GetDevice(pEventInfo->extNo);
	if(pDevice1){
		if (IsBCRun(pDevice1->sDB.extAddr))
		{
			DeviceResume(pDevice1);
		}
		//CloseCallWithoutAnswer(pDevice1);
		SetCallMsgStatus(pDevice1->sDB.extAddr,ENUM_CALL_WAIT_E2);
		EventChangeBtn(pDevice1,RECV_MSG_CALL_IDLE);
		if(g_Config.m_SystemConfig.nMainSystem)
		{
			if(IsDeviceLocalController(pDevice1->dwIP)){
				g_MSG.SendHangUp(pDevice1->sDB.extAddr,pDevice1->sDB.ip,g_Config.m_SystemConfig.portDevice);
				g_MSG.SendCmd(atoi(pDevice1->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
			}else{
				g_MSG.SendCallReq2Branch(pDevice1->sDB.extAddr,pDevice1->dwIP,ENUM_CALL_CLOSE_REQ);
			}	
			DeleteCallAnswer(pDevice1->sDB.extAddr);
		}else{
			g_MSG.SendHangUp(pDevice1->sDB.extAddr,pDevice1->sDB.ip,g_Config.m_SystemConfig.portDevice);
			g_MSG.SendDeviceCallStatus2Master(pDevice1->sDB.extAddr,ENUM_CALL_IS_USABLE);
		}
		g_Log.output(LOG_TYPE,"recv call no answer from om\r\n");
	}
	SDevice* pDevice2 = GetDevice(pEventInfo->from);
	if(pDevice2){
		if (IsBCRun(pDevice2->sDB.extAddr))
		{
			DeviceResume(pDevice2);
		}
		//CloseCallWithoutAnswer(pDevice2);
		SetCallMsgStatus(pDevice2->sDB.extAddr,ENUM_CALL_WAIT_E2);
		EventChangeBtn(pDevice2,RECV_MSG_CALL_IDLE);
		if(g_Config.m_SystemConfig.nMainSystem)
		{
			if(IsDeviceLocalController(pDevice2->dwIP)){
				g_MSG.SendHangUp(pDevice2->sDB.extAddr,pDevice2->sDB.ip,g_Config.m_SystemConfig.portDevice);
				g_MSG.SendCmd(atoi(pDevice2->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
			}else{
				g_MSG.SendCallReq2Branch(pDevice2->sDB.extAddr,pDevice2->dwIP,ENUM_CALL_CLOSE_REQ);
			}	
			DeleteCallAnswer(pDevice2->sDB.extAddr);
		}else{
			g_MSG.SendHangUp(pDevice2->sDB.extAddr,pDevice2->sDB.ip,g_Config.m_SystemConfig.portDevice);
			g_MSG.SendDeviceCallStatus2Master(pDevice2->sDB.extAddr,ENUM_CALL_IS_USABLE);
		}
		g_Log.output(LOG_TYPE,"recv call no answer from om\r\n");
	}
	// 清空关联的坐席电话
	
	RemoveRing(pEventInfo->id);
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgDeviceStatusReqFromMasterSystem(WPARAM wParam,LPARAM lParam)
{
	BYTE* pRecvBuff = (BYTE*)lParam;
	SendStatus2Subsystem(pRecvBuff, USER_BUFFER_SZ);
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgDeviceStatusResponseFromBranchSystem(WPARAM wParam,LPARAM lParam)
{
	BYTE* pRecvBuff = (BYTE*)lParam;
	// 发送反馈
	g_MSG.SendDeviceStatusAck(pRecvBuff[2], pRecvBuff[4], pRecvBuff[5], pRecvBuff[6], pRecvBuff[7]);
	// 更新状态
	BYTE extAddr = pRecvBuff[2];
	SDevice* pDevice = GetDevice(extAddr);
	if (pDevice)
	{
		if (pDevice->bcStatus == ENUM_DEVICE_STATUS_RUN ||
			pDevice->callStatus == ENUM_DEVICE_STATUS_RUN ||
			pDevice->callStatus == ENUM_DEVICE_STATUS_RING)
		{
		}
		else{
			if((enDeviceStatus)pRecvBuff[8] == ENUM_DEVICE_STATUS_RUN)
				pDevice->bcStatus = ENUM_DEVICE_STATUS_REMOTE_USING;
			else
				pDevice->bcStatus = (enDeviceStatus)(pRecvBuff[8]);
			pDevice->callStatus = (enDeviceStatus)(pRecvBuff[9]);
		}
		Status2Button(extAddr);
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgDeviceStatusAckFromMasterSystem(WPARAM wParam,LPARAM lParam)
{
	BYTE* pRecvBuff = (BYTE*)lParam;
	BYTE extAddr = pRecvBuff[2];
	m_StausDataLock.Lock();
	for (int i = 0; i < m_vecStausData.size(); i++)
	{
		if (m_vecStausData[i].extAddr == extAddr)
		{					
			m_vecStausData[i].bReply = true;					
		}
	}
	m_StausDataLock.Unlock();
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgBCOption(WPARAM wParam,LPARAM lParam)
{
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgHeartBeatReplyFromDevice(WPARAM wParam,LPARAM lParam)
{
	if (g_Logic.m_UpdateDB )
	{
		return 0;
	}
	if (!IsBCRun(wParam))	//确定broadcast运行状态
	{
		UpdateStatus_BC(wParam, ENUM_DEVICE_STATUS_OK);	 //更新广播状态
	}				
	if (!IsCallRun(wParam))	  //确定电话运行状态
	{
		UpdateStatus_CALL(wParam, ENUM_DEVICE_STATUS_OK);//更新电话状态
	}
	UpdateErrorCode(wParam, ENUM_DEVICE_STATUS_OK, ENUM_ERROR_STATUS_NONE);
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgCallStatusFromBranch(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	BYTE callStatus = (BYTE)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if (pDevice)
	{
		switch(callStatus)
		{
		case ENUM_CALL_IS_RING:
			{
				if(IsBCRun(extAddr))
				{
					DevicePause(pDevice);
				}
				//pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_RING;
				Status2Button(extAddr);
				//上报 ring状态
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_RING);
				//若之前点击了分机请求通话，删除事件（可以根据是否有事件给出提示）
				DeleteCallMsg(extAddr);
				g_Log.output(LOG_TYPE,"on recv call is ring delete call msg....\r\n");
				DeleteCallAnswer(extAddr);
				if(IsBCMsgExist(extAddr)&&GetBCMsg(extAddr)->status == ENUM_BC_WAIT_E3)
				{
					DeleteBCMSG(extAddr);
				}
			}
			break;
		case ENUM_CALL_IS_USING:		//call is using
			{
				if(IsBCRun(extAddr))
				{
					DevicePause(pDevice);
				}
				//pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_RUN;
				Status2Button(extAddr);
				//上报 running状态
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_CALL_START);
				//若之前点击了分机请求通话，删除事件（可以根据是否有事件给出提示）
				DeleteCallMsg(extAddr);
				g_Log.output(LOG_TYPE,"on recv call is using delete call msg....\r\n");
				DeleteCallAnswer(extAddr);
				if(IsBCMsgExist(extAddr)&&GetBCMsg(extAddr)->status == ENUM_BC_WAIT_E3)
				{
					DeleteBCMSG(extAddr);
				}
			}
			break;
		case ENUM_CALL_IS_USABLE:		//call is usable
			{
				//pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
				if(pDevice->callStatus!=ENUM_DEVICE_STATUS_RING){
					if (IsBCRun(extAddr))
					{
						DeviceResume(pDevice);
					}
					if(pDevice->errorCode!=ENUM_ERROR_STATUS_NONE)
						pDevice->callStatus = ENUM_DEVICE_STATUS_ERROR;
					else
						pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
					//上报 ok状态
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_CALL_STOP);
				}
				Status2Button(extAddr);
			}
			break;
		case ENUM_CALL_IS_OFFLINIE:		//call is offline
			{
				if (IsBCRun(extAddr))
				{
					DeviceStop(pDevice,false);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);
				}
				pDevice->bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				pDevice->callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
				Status2Button(extAddr);
				g_Log.output(LOG_TYPE,"on recv call is offline delete call msg....\r\n");
				DeleteCallMsg(extAddr);
				DeleteBCMSG(extAddr);
				DeleteCallAnswer(extAddr);
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_CALL_STOP);
			}
			break;
		case ENUM_CALL_IS_OM_DOWN:    //OM down
			{
				if (IsBCRun(extAddr))
				{
					DeviceResume(pDevice);
				}
				//pDevice->callStatus = ENUM_DEVICE_STATUS_ERROR;
				pDevice->callStatus = ENUM_DEVICE_OM_DOWN;
				Status2Button(extAddr);
				DeleteCallMsg(extAddr);
				g_Log.output(LOG_TYPE,"on recv call om is down delete call msg....\r\n");
				DeleteCallAnswer(extAddr);
			}
			break;
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgBroadcastStatusFromBranch(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	BYTE broadcastStatus = (BYTE)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if (pDevice)
	{
		switch(broadcastStatus)
		{
		case ENUM_BC_IS_USING:    //bc is using
			{
				if(IsBCMsgExist(extAddr)){
					//pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
					pDevice->bcStatus = ENUM_DEVICE_STATUS_REMOTE_USING;
					//SetBCStatus(extAddr,ENUM_BC_E3);
					DeleteBCMSG(extAddr);
				}
				else
				{
					pDevice->bcStatus = ENUM_DEVICE_STATUS_REMOTE_USING;
				}	
				Status2Button(extAddr);
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_BC_OPEN);
			}
			break;
		case ENUM_BC_IS_USABLE:   //bc is usable
			{
				pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
				Status2Button(extAddr);
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_BC_CLOSE);
			}
			break;
		case ENUM_BC_IS_REMOTE_USING:
			{
				if(IsBCMsgExist(extAddr))
				{
					SetBCStatus(extAddr,ENUM_BC_E3);
					pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;
					Status2Button(extAddr);
					g_MSG.SendCmd(atoi(pDevice->sDB.extNo),ENUM_CMD_CODE_BC_OPEN);
				}
			}
			break;
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvMsgBroadcastReqFromMaster(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	BYTE broadcastOpt = (BYTE)lParam;
	SDevice* pDevice = GetDevice(extAddr);
	if (pDevice)
	{
		switch(broadcastOpt)
		{
		case ENUM_BC_OPEN_REQ:		//open bc
			{
				if(pDevice->callStatus !=ENUM_DEVICE_STATUS_RUN&&
					pDevice->callStatus!= ENUM_DEVICE_STATUS_UNKNOWN&&
					pDevice->callStatus!=ENUM_DEVICE_STATUS_RING)
				{
					if(pDevice->bcStatus ==ENUM_DEVICE_STATUS_RUN/*&&
						pDevice->bcStatus!=ENUM_DEVICE_STATUS_UNKNOWN*/){
							g_MSG.SendDeviceBroadcastStatus2Master(extAddr,pDevice->dwIP,ENUM_BC_IS_USING);
					}
					else if(pDevice->bcStatus == ENUM_DEVICE_STATUS_REMOTE_USING)
					{
						g_MSG.SendDeviceBroadcastStatus2Master(extAddr,pDevice->dwIP,ENUM_BC_IS_REMOTE_USING);
					}
					else
					{
						//UpdateStatus_BC(extAddr,ENUM_DEVICE_STATUS_REMOTE_USING);
						g_MSG.SendCloseBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,false);
						Sleep(50);
						g_MSG.SendOpenBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice);
					}
				}else if(pDevice->callStatus == ENUM_DEVICE_STATUS_RUN)
				{
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_USING);
				}else if(pDevice->callStatus == ENUM_DEVICE_STATUS_RING)
				{
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_RING);
				}
				else if(pDevice->callStatus == ENUM_DEVICE_STATUS_UNKNOWN)
				{
					g_MSG.SendDeviceCallStatus2Master(extAddr,ENUM_CALL_IS_OFFLINIE);
				}
			}
			break;
		case ENUM_BC_CLOSE_REQ:		// close  bc
			{
				if(IsBCRun(extAddr))
				{
					g_MSG.SendDeviceBroadcastStatus2Master(extAddr,pDevice->dwIP,ENUM_BC_IS_USING);
				}
				else{
					UpdateStatus_BC(extAddr,ENUM_DEVICE_STATUS_OK);
					g_MSG.SendCloseBroadcast(extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,true);
				}
			}
			break;
		case 0x03:       //pause play bc
			{
				DevicePause(pDevice);
			}
			break;
		case 0x04:		//continue play bc
			{
				DeviceResume(pDevice);
			}
			break;
		}
	}
	return 0;
}
void CSIPCallDlg::AddBCMsg(SBCMsg bcMSG)
{
	m_BCMsgLock.Lock();
	if(!IsBCMsgExist(bcMSG.extAddr))
		m_vecBCMsg.push_back(bcMSG);
	m_BCMsgLock.Unlock();
}
void CSIPCallDlg::DeleteBCMSG(int extAddr)
{
	m_BCMsgLock.Lock();
	vector<SBCMsg>::iterator bcIter = m_vecBCMsg.begin();
	for (;bcIter!=m_vecBCMsg.end();)
	{
		if(bcIter->extAddr == extAddr)
		{
			m_vecBCMsg.erase(bcIter);
			break;
		}else
		{
			bcIter++;
		}
	}
	m_BCMsgLock.Unlock();
}
SBCMsg* CSIPCallDlg::GetBCMsg(int extAddr)
{
	m_BCMsgLock.Lock();
	SBCMsg* sbcMsg = NULL;
	vector<SBCMsg>::iterator sbcIter = m_vecBCMsg.begin();
	for (;sbcIter!=m_vecBCMsg.end();)
	{
		if (sbcIter->extAddr == extAddr)
		{
			sbcMsg = &(*sbcIter);
			break;
		}else{
			sbcIter++;
		}
	}
	m_BCMsgLock.Unlock();
	return sbcMsg;
}
bool CSIPCallDlg::IsBCMsgExist(int extAddr)
{
	m_BCMsgLock.Lock();
	bool flag = false;
	for (unsigned int i=0;i<m_vecBCMsg.size();i++)
	{
		if (m_vecBCMsg[i].extAddr == extAddr)
		{
			flag = true;
			break;
		}
	}
	m_BCMsgLock.Unlock();
	return flag;
}
void CSIPCallDlg::SetBCStatus(int extAddr,enBCStatus bcstatus)
{
	m_BCMsgLock.Lock();
	for (unsigned int i=0;i<m_vecBCMsg.size();i++)
	{
		if (m_vecBCMsg[i].extAddr == extAddr)
		{
			m_vecBCMsg[i].timestamp = GetTickCount();
			m_vecBCMsg[i].status = bcstatus;
			break;
		}
	}
	m_BCMsgLock.Unlock();
}
void CSIPCallDlg::UpdateBCMsg()
{
	m_BCMsgLock.Lock();
	for (unsigned int i =0; i<m_vecBCMsg.size();)
	{
		DWORD dwTime = GetTickCount();
		if (dwTime - m_vecBCMsg[i].timestamp > 20*1000&&
			m_vecBCMsg[i].status == ENUM_BC_WAIT_E4)
		{
			g_Log.output(LOG_TYPE,"!!!bc msg status:%d\r\n",m_vecBCMsg[i].status);
			SendMessage(NO_E4_ANSWER,m_vecBCMsg[i].extAddr,(LPARAM)&m_vecBCMsg[i]);
			if(m_vecBCMsg[i].nSend>=2){
				m_vecBCMsg.erase(m_vecBCMsg.begin()+i);
			}else
			{
				m_vecBCMsg[i].nSend++;
				//间隔2S发两次关广播指令
			}
		}else if (dwTime - m_vecBCMsg[i].timestamp >10*1000 &&
			m_vecBCMsg[i].status == ENUM_BC_WAIT_E3)
		{
			g_Log.output(LOG_TYPE,"!!!bc msg status:%d\r\n",m_vecBCMsg[i].status);

			SendMessage(NO_E3_ANSWER,m_vecBCMsg[i].extAddr,(LPARAM)&m_vecBCMsg[i]);
			if(m_vecBCMsg[i].nSend>=2)
				m_vecBCMsg.erase(m_vecBCMsg.begin()+i);
			else
				m_vecBCMsg[i].nSend++;
		}else
		{
			i++;
		}
	}
	m_BCMsgLock.Unlock();
}
LRESULT CSIPCallDlg::OnRecvBranchOMDown(WPARAM wParam,LPARAM lParam)
{
	CString *ip = (CString*)lParam;
	BYTE status = (BYTE)wParam;
	int len =WideCharToMultiByte(CP_ACP,0,*ip,-1,NULL,0,NULL,NULL);  
	char *ptxtTemp =new char[len +1];  
	WideCharToMultiByte(CP_ACP,0,*ip,-1,ptxtTemp,len,NULL,NULL );  
	TakeDeviceByOMStatus(ptxtTemp,status);
	delete[] ptxtTemp;
	return 0;
}
HANDLE g_hAudioHandle = NULL,g_hCallHandle = NULL;
LRESULT CSIPCallDlg::OnRecvCallRecordAgentExit(WPARAM wParam,LPARAM lParam)
{
	int callgwFlag = (int)wParam;
	if(callgwFlag == 0)
	{
		if(!g_bExitCallgw){
			MessageBox(L"call record agent has been exit...");
			g_bExitCallgw = true;
		}
	}
	else
	{
		if(g_bExitCallgw){
			g_bExitCallgw = false;
			MessageBox(L"call record agent has been brought online...");
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvAudioRecordAgentExit(WPARAM wParam,LPARAM lParam)
{
	int audiogwFlag = (int)wParam;
	if(audiogwFlag == 0)
	{
		if(!g_bExitAudiogw){
			MessageBox(L"audio record agent has been exit...");
			g_bExitAudiogw = true;
		}
	}
	else
	{
		if(g_bExitAudiogw){
			g_bExitAudiogw = false;
			MessageBox(L"audio record agent has been brought online...");
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvSipgwExit(WPARAM wParam,LPARAM lParam)
{
	int sipgwFlag = (int)wParam;
	if(sipgwFlag == 0)
	{
		if(!g_bExitSipgw){
			MessageBox(L"sipgw has been exit...");
			EnableCall(false);
			if(!g_Config.m_SystemConfig.nMainSystem)
			{
				g_MSG.SendBranchOMDown2Master(2);

			}
			g_bExitSipgw = true;
		}
	}
	else
	{
		if(g_bExitSipgw){
			EnableCall(true);
			g_bExitSipgw = false;
			g_MSG.SendBranchOMDown2Master(1);
		}
	}
	return 0;
}
void CSIPCallDlg::OnOpenaudiorecord()
{
	// TODO: 在此添加命令处理程序代码
	SHELLEXECUTEINFO ShExecInfo;  
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);  
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS ;  
	ShExecInfo.hwnd = NULL;  
	ShExecInfo.lpVerb = NULL;  
	ShExecInfo.lpFile = L"AudioServer.exe"; //can be a file as well  
	ShExecInfo.lpParameters = L"";   
	ShExecInfo.lpDirectory = L"D:\\Program Files\\SRAudio\\AudioServer";  
	ShExecInfo.nShow = SW_MINIMIZE;  
	ShExecInfo.hInstApp = NULL;   
	BOOL ret = ShellExecuteEx(&ShExecInfo); 
	if (ret&&ShExecInfo.hProcess)
	{
		g_hAudioHandle = ShExecInfo.hProcess;
		AfxBeginThread(CheckAudioServerEXE,this);
	}
	/*
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	CreateProcess(NULL,"D:\\Program Files\\SRAudio\\AudioServer",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	GUITHREADINFO gui;
	gui.cbSize = sizeof(gui);
	GetGUIThreadInfo(pi.dwThreadId,&gui);*/

	//::SendMessage(FromHandle(g_hAudioHandle)->m_hWnd,WM_SYSCOMMAND,SC_MINIMIZE,0);
	GetMenu()->EnableMenuItem(ID_OpenAudioRecord,MF_GRAYED|MF_DISABLED);
	GetMenu()->EnableMenuItem(ID_CloseAudioRecord,MF_ENABLED);
}
void CSIPCallDlg::OnOpencallrecord()
{
	// TODO: 在此添加命令处理程序代码
	SHELLEXECUTEINFO ShExecInfo;  
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);  
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS ;  
	ShExecInfo.hwnd = NULL;  
	ShExecInfo.lpVerb = NULL;  
	ShExecInfo.lpFile = L"C:\\Program Files (x86)\\pbxrecord\\pbxrecord.exe"; //can be a file as well  
	ShExecInfo.lpParameters = L"";   
	ShExecInfo.lpDirectory = NULL;  
	ShExecInfo.nShow = SW_HIDE;  
	ShExecInfo.hInstApp = NULL;   
	BOOL ret = ShellExecuteEx(&ShExecInfo); 
	
	if (ret&&ShExecInfo.hProcess)
	{
		g_hCallHandle = ShExecInfo.hProcess;
		AfxBeginThread(CheckCallRecordEXE,this);
	}
	
	GetMenu()->EnableMenuItem(ID_OpenCallRecord,MF_GRAYED|MF_DISABLED);
	GetMenu()->EnableMenuItem(ID_CloseCallRecord,MF_ENABLED);
}
void CSIPCallDlg::OnCloseaudiorecord()
{
	// TODO: 在此添加命令处理程序代码
	if(g_hAudioHandle)
	{
		TerminateProcess(g_hAudioHandle,0);
		g_hAudioHandle = NULL;
	}
}
void CSIPCallDlg::OnClosecallrecord()
{
	// TODO: 在此添加命令处理程序代码
	if(g_hCallHandle)
	{
		TerminateProcess(g_hCallHandle,0);
		g_hCallHandle = NULL;
	}
}
void CSIPCallDlg::SendBeat2Offlines()
{
	//如果是主控巡检，下发给分控进行巡检
	bool bSend2BranchController = false;
	if(g_Config.m_SystemConfig.nMainSystem)
	{
		bSend2BranchController = true;
	}
	for (int i = 0; i < m_nTunnelNum; i++)
	{
		int upSize = m_pTunnelInfo[i].vecDeviceUp.size();
		for (int j = 0; j < upSize; j++)
		{
			if (g_bExitApp || s_bBeatStop)
			{				
				return;
			}
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceUp.begin() + j;
			SDevice* pDevice = &(*vecIter);
			
			if(!IsNeedSendBeat2Offline(pDevice)){
				continue;
			}
			if(bSend2BranchController&&!IsDeviceLocalController(pDevice->dwIP))
			{
				//分控设备，发到分控进行巡检
				g_MSG.SendDeviceStatusReq(*pDevice,0);
				continue;
			}

			g_MSG.SendCloseBroadcast(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice, true);
			Sleep(20);		
			g_MSG.SendHangUp(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
			Sleep(20);
			// 发送心跳
			g_MSG.SendHeartBeat(m_pTunnelInfo[i].vecDeviceUp[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceUp[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
			
			Sleep(20);
		}
		int downSz = m_pTunnelInfo[i].vecDeviceDown.size();
		for (int j = 0; j < downSz; j++)
		{
			if (g_bExitApp ||  s_bBeatStop)
			{
				return;
			}
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[i].vecDeviceDown.begin() + j;
			SDevice* pDevice = &(*vecIter);
			
			if(!IsNeedSendBeat2Offline(pDevice)){
				continue;
			}
			if(bSend2BranchController&&!IsDeviceLocalController(pDevice->dwIP))
			{
				//分控设备，发到分控进行巡检
				g_MSG.SendDeviceStatusReq(*pDevice,0);
				continue;
			}
			g_MSG.SendCloseBroadcast(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice, true);
			Sleep(20);
			
			g_MSG.SendHangUp(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
			Sleep(20);
			// 发送心跳
			g_MSG.SendHeartBeat(m_pTunnelInfo[i].vecDeviceDown[j].sDB.extAddr, m_pTunnelInfo[i].vecDeviceDown[j].sDB.ip, g_Config.m_SystemConfig.portDevice);
			Sleep(20);
		}			
	}
}
void CSIPCallDlg::SendBeat2Offline(int extAddr)
{
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		if(!IsDeviceLocalController(pDevice->dwIP))
		{
			//分控设备，发到分控进行巡检
			g_MSG.SendDeviceBeatReq(extAddr,pDevice->dwIP);
		}else
		{
			g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice, true);
			Sleep(20);		
			g_MSG.SendHangUp(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
			Sleep(20);
			// 发送心跳
			g_MSG.SendHeartBeat(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);

			Sleep(20);
		}
	}
}
bool CSIPCallDlg::IsNeedSendBeat2Offline(SDevice* pDevice)
{
	if (pDevice)
	{
		if ((pDevice->bcStatus == ENUM_DEVICE_STATUS_UNKNOWN ||
			pDevice->callStatus == ENUM_DEVICE_STATUS_UNKNOWN ||
			pDevice->callStatus == ENUM_DEVICE_STATUS_ERROR ||
			pDevice->bcStatus == ENUM_DEVICE_STATUS_ERROR)&&
			(pDevice->sDB.IsVisible()))
		{
			return true;
		}
		else{
			return false;
		}
	}
	return false;
}
LRESULT CSIPCallDlg::OnRecvMsgDeviceBeatReq(WPARAM wParam,LPARAM lParam)
{
	int extAddr = (int)wParam;
	SDevice* pDevice = GetDevice(extAddr);
	if(pDevice)
	{
		g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice, true);
		Sleep(20);		
		g_MSG.SendHangUp(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
		Sleep(20);
		// 发送心跳
		g_MSG.SendHeartBeat(pDevice->sDB.extAddr,  pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);

		Sleep(20);
	}
	return 0;
}
UINT SendBeat2OfflineProc(LPVOID extAddr)
{
	Sleep(30*1000);
	m_pAPP->SendBeat2Offline((int)extAddr);
	return 0;
}
#if 0
UINT CheckUsingNetworkProc(LPVOID lp)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	pcap_t *adhandle;
	int res;
	int i = 0;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	int nNetworkId = -1;
	/* Retrieve the device list */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return -1;
	}
	/* Jump to the selected adapter */
	for(d=alldevs, i=0; d;){
		/* Open the adapter */
		if ((adhandle= pcap_open_live(d->name,1024,1,1000,errbuf)) == NULL)
		{
			pcap_freealldevs(alldevs);
			return -1;
		}
		/* Retrieve the packets */
		int nTimeOut = 0;
		while((res = pcap_next_ex( adhandle, &header, &pkt_data)) >= 0){
			if(res == 0){
				/* Timeout elapsed */
				nTimeOut++;
				if(nTimeOut>10)
					break;
				continue;
			}else
			{
				nNetworkId = i;
				break;
			}
		}
		if(nNetworkId!=-1)
		{
			break;
		}else
		{
			if(d->next)
			{
				d=d->next;
				i++;
			}else{
				d = alldevs;
				i = 0;
			}
		}
	}
	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);
	s_UsingNetwork = nNetworkId;
	return 0;
}
UINT CheckSipgwUdpPacketProc(LPVOID lp)
{
	g_hSipgwPacketEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;
	pcap_t *adhandle;
	int res;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	ip_header *ih;
	udp_header *uh;
	u_int ip_len;
	u_short sport,dport;
	u_int netmask;
	char packet_filter[] = "port 5060 or 52892";
	struct bpf_program fcode;
	
	/* Retrieve the device list */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return -1;
	}
	/* Jump to the selected adapter */
	for(d=alldevs, i=0; i< s_UsingNetwork ;d=d->next, i++);

	/* Open the adapter */
	if ((adhandle= pcap_open_live(d->name,	// name of the device
		1024,1,1000,errbuf)) == NULL)
	{
		pcap_freealldevs(alldevs);
		return -1;
	}

	if(d->addresses != NULL)
		/* Retrieve the mask of the first address of the interface */
		netmask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* If the interface is without addresses we suppose to be in a C class network */
		netmask=0xffffff; 

	//compile the filter
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) <0 )
	{
		pcap_freealldevs(alldevs);
		return -1;
	}
	//set the filter
	if (pcap_setfilter(adhandle, &fcode)<0)
	{
		pcap_freealldevs(alldevs);
		return -1;
	}
	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);

	int nTimeOut = 0;
	int nSipPortTime = 0;
	int nOmPortTime = 0;
	/* Retrieve the packets */
	while((WAIT_OBJECT_0==WaitForSingleObject(g_hSipgwPacketEvent,INFINITE))&&
		((res = pcap_next_ex( adhandle, &header, &pkt_data)) >= 0)){
		
		s_OmSend = true;		//信号量已有信号，不用setevent
		if(res == 0){
			/* Timeout elapsed */
			nTimeOut++;
			if(nTimeOut<10)
			{
				continue;
			}
		}else{
			ih = (ip_header *) (pkt_data +
				14); //length of ethernet header
			/* retireve the position of the udp header */
			ip_len = (ih->ver_ihl & 0xf) * 4;
			uh = (udp_header *) ((u_char*)ih + ip_len);
			/* convert from network byte order to host byte order */
			sport = ntohs( uh->sport );
			dport = ntohs( uh->dport );
			if(sport == 52892)
				nSipPortTime++;
			if(dport == 5060)
				nOmPortTime++;
		}
		
		if(nSipPortTime>0&&nOmPortTime>0)
		{
			ResetEvent(g_hSipgwPacketEvent);
			nTimeOut = 0;
			nSipPortTime = 0;
			nOmPortTime = 0;
			s_OmSend = false;		//信号量无信号，可以 setevent
		}else if(nTimeOut>10)
		{
			ResetEvent(g_hSipgwPacketEvent);
			SendMessage(m_pAPP->m_hWnd,NO_PACKETS,nSipPortTime,nOmPortTime);
			nTimeOut = 0;
			nSipPortTime = 0;
			nOmPortTime = 0;
			s_OmSend = false;		//信号量无信号，可以 setevent
		}
	}
	return 0;
}
#endif
LRESULT CSIPCallDlg::OnRecvMsgNoPackets(WPARAM wParam,LPARAM lParam)
{
	if((int)wParam == 0)
	{
		ShowInfo(L"sipgw not send some msg");
	}
	if((int)lParam == 0)
	{
		ShowInfo(L"not recv some msg from om");
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvOuterBCStart(WPARAM wParam,LPARAM lParam)
{
	CString path = *(CString*)wParam;
	vector<int> ets = *(vector<int>*)lParam;
	ShowInfo(L"path:"+path);
	char c_path[256] = {0};
	WideCharToMultiByte(CP_ACP,0,path,-1,c_path,256,NULL,FALSE);
	for(int i=0;i<ets.size();i++){
		TRACE("open ets:%d\r\n",ets[i]);
		char extNo[125]={0};
		_itoa(ets[i],extNo,10);
		SDevice* pDevice = GetDevice(extNo);
		if(pDevice)
		{
			CMD_PlayOneDevice(pDevice,c_path);
		}
	}
	return 0;
}
LRESULT CSIPCallDlg::OnRecvOuterBCClose(WPARAM wParam,LPARAM lParam)
{
	vector<int> ets = *(vector<int>*)lParam;
	for(int i=0;i<ets.size();i++){
		TRACE("close ets:%d\r\n",ets[i]);
		char extNo[125]={0};
		_itoa(ets[i],extNo,10);
		SDevice* pDevice = GetDevice(extNo);
		if(pDevice)
		{
			if(IsDeviceLocalController(pDevice->dwIP))
			{
				if (pDevice->bcStatus == ENUM_DEVICE_STATUS_OUTER_USING)
				{
					DeviceStop(pDevice,false);
				}
				//pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
				SetBCStatus(pDevice->sDB.extAddr,ENUM_BC_WAIT_E4);
				g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr,pDevice->sDB.ip,g_Config.m_SystemConfig.portDevice,false);
				//Status2Button(pDevice->sDB.extAddr);
			}else
			{
				if (pDevice->bcStatus == ENUM_DEVICE_STATUS_OUTER_USING)
				{
					DeviceStop(pDevice,false);
				}
				g_Log.output(LOG_TYPE,"device bc status.%d,,,msg size:%d,\r\n",pDevice->bcStatus,m_vecBCMsg.size());
				SetBCStatus(pDevice->sDB.extAddr,ENUM_BC_WAIT_E4);
				g_MSG.SendBroadcastReq2Branch(pDevice->sDB.extAddr,pDevice->dwIP,ENUM_BC_CLOSE_REQ);
			}
		}
	}
	return 0;
}