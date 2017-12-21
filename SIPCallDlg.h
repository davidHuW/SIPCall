// SIPCallDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "resource.h"
#include "Config.h"
#include "DataM.h"
#include "DecodeMsg.h"
#include "HttpRecv.h"
#include "AudioPlay.h"
#include "LogicApp.h"
#include "MgrUser.h"
#include "MgrPic.h"
#include "BtnST.h"
#include "MgrPlay.h"
#include "Label.h"
#include "LayoutConfig.h"

#define BTN_NUM_TUNNEL	(10)
#define BTN_NUM_CALL	(20)
#define BTN_NUM_MSG		BTN_NUM_CALL

#define BTN_NUM_WAIT	(5)

enum {
	BTN_CREATE_START = 10000,


	BTN_TUNNEL_BASEID = 10100,

	BTN_IMAGE_BASEID_1_0 = 10200,// 上电话	
	BTN_IMAGE_BASEID_2_0 = 10300,// 下电话

	BTN_STATIC_BASEID_1 = 10400,// 上标签(设备名称)
	BTN_STATIC_BASEID_2 = 10500,// 下标签(设备名称)
	

	CHECK_BOX_BASEID_1 = 10600,	 // 上广播
	CHECK_BOX_BASEID_2 = 10700,  // 下广播

	BTN_IMAGE_PRE = 10801,		//device 翻页键
	BTN_IMAGE_NEXT = 10802,		//device 翻页键

	BTN_STATIC_BASEID_11 = 11101,// 上标签（Error状态）
	BTN_STATIC_BASEID_21 = 11251,// 下标签（Error状态）

	BTN_STATIC_BASEID_KM1 = 11401,// 上标签（km）
	BTN_STATIC_BASEID_KM2 = 11501,// 下标签（km）


	BTN_CONFIG_BASEID = 12000,

	BTN_ARROW_LEFT = 13001,		// 左箭头
	BTN_ARROW_RIGHT = 13002,	// 右箭头
	BTN_SIGN_LEFT = 13003,		// 左方向
	BTN_SIGN_RIGHT = 13004,		// 右方向
	
	BTN_STATIC_WAIT_BASEID = 13100,// 等待区域

	BTN_CREATE_END = 15000,
};

struct NTP_Packet 
{ 
	int Control_Word; 
	int root_delay; 
	int root_dispersion; 
	int reference_identifier; 
	__int64	 reference_timestamp; 
	__int64 originate_timestamp; 
	__int64 receive_timestamp; 
	int transmit_timestamp_seconds; 
	int transmit_timestamp_fractions; 
}; 

struct SCallInfo{
	enDeviceStatus status;
	enDeviceStatus preStatus;
	char szIP[64];
	char szName[64];
	
	int id;
	DWORD timestamp;// 电话
	DWORD timeError;// 消息
	int extAddr;
	SCallInfo(){
		init();
	}
	void clearCall(){
		memset(szIP, 0, 64 * sizeof(char));
		memset(szName, 0, 64 * sizeof(char));
		//startTime = 0;
		extAddr = 0;
		timestamp = 0;	
		id = 0;
	}
	void init(){
		preStatus = status = ENUM_DEVICE_STATUS_UNKNOWN;
		memset(szIP, 0, 64 * sizeof(char));
		memset(szName, 0, 64 * sizeof(char));
		
		//startTime = 0;
		extAddr = 0;
		timestamp = 0;
		timestamp = 0;	
		id = 0;
	}
};
struct SCallMsg{
	int extAddr;
	DWORD timestamp;
	int type;	//1:主叫 2:被叫 
				//3:主控 主叫 分控分机（分控产生） 4:主控 主叫 分控分机(分控产生)  
				//5:呼叫转移 ，主控让分控打开分机(主控产生)
	enCallStatus status;
	short nSend ;
	SCallMsg(){
		extAddr = 0;
		timestamp = 0;
		type = 0;
		status = ENUM_CALL_NONE;
		nSend = 0;
	}
};
struct SBCMsg{
	int extAddr;
	DWORD timestamp;
	int type;
	enBCStatus status;
	short nSend ;
	SBCMsg(){
		extAddr = 0;
		timestamp = 0;
		type = 0;
		status = ENUM_BC_NONE;
		nSend = 0;
	}
};
struct SCallEvt{
	int extAddr;
	DWORD dwTime;
	char extNo[16];
	SCallEvt(){
		extAddr = 0;
		memset(extNo, 0, 16 * sizeof(char));
		dwTime = GetTickCount();
	}
};
struct SStatus4Send
{
	int extAddr;
	DWORD timestamp;
	SStatus4Send()
	{
		extAddr = 0;
		timestamp = 0;
	}
};
struct SMonitorDevice{
//	int thread;
//	_PlayParam param;
	SDevice* pDevice;
//	bool bMonitor;
	SMonitorDevice(){
//		thread = 0;
		pDevice = NULL;
//		bMonitor = false;
	}	
// 	bool IsPlay(){
// 		if (thread == 0)
// 		{
// 			return false;
// 		}
// 		return true;
// 	}
};

struct SGroupCall{
	int extAddr;
	char group[8];
	SGroupCall(){
		extAddr = 0;
		memset(group, 0, sizeof(char) * 8);
	}
};

struct SBCYing{
	int extAddr;
	int ying;
	SBCYing(){
		extAddr = 0;
		ying = 100;
	}
};

/* 4 bytes IP address */
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header
{
	u_char	ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	u_char	tos;			// Type of service 
	u_short tlen;			// Total length 
	u_short identification; // Identification
	u_short flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	u_char	ttl;			// Time to live
	u_char	proto;			// Protocol
	u_short crc;			// Header checksum
	ip_address	saddr;		// Source address
	ip_address	daddr;		// Destination address
	u_int	op_pad;			// Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header
{
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
}udp_header;
// CSIPCallDlg dialog
class CSIPCallDlg : public CDialog
{
// Construction
public:
	CSIPCallDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SIPCALL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	void SetLayout();
private:
	void SetTitleLayout();
	void SetTunnelNameLayout();
	void SetTunnelBtnLayout();
	void SetDeviceLayout();
	void SetCallLayout();
	void SetWaitLayout();
private:
	CButton* NewMyButton(int nID,CRect rect,int nStyle, CString Caption = L"");
	CButton* NewCheckBox(int nID,CRect rect,int nStyle, CString Caption = L"");
	CStatic* NewStatic(int nID,CRect rect,int nStyle, CString Caption = L"");
	CButton* NewBMPStatic(int nID,CRect rect,int nStyle, CString Caption = L"");
	void ReleaseButton(CButton*);
	void BtnInit();
	void BtnUninit();
	void  SetButtonBMP(CButton* pButton, CString strSrc);
private:
	void OnCreateButtonClick(UINT nID);
	void OnErrorMsgDBClick(UINT nID);
	bool IsExistCall(int extAddr);
private:
	
	CButton* m_pbtnCall1[BTN_NUM_CALL];	// 上电话
	CButton* m_pbtnMsg1[BTN_NUM_MSG];	// 上广播
	CStatic* m_pStatic1[BTN_NUM_MSG];   // 上标签
	CStatic* m_pStatic3[BTN_NUM_MSG];   // 上标签（状态）

	CButton* m_pbtnCall2[BTN_NUM_CALL]; // 下电话
	CButton* m_pbtnMsg2[BTN_NUM_MSG];	// 下广播
	CStatic* m_pStatic2[BTN_NUM_MSG];   // 下标签
	CStatic* m_pStatic4[BTN_NUM_MSG];   // 上标签（状态）
	CStatic* m_pStatic5[BTN_NUM_WAIT];   // 上标签（状态）

	CStatic* m_pStaticKM1[BTN_NUM_CALL];   // 上标签（KM）
	CStatic* m_pStaticKM2[BTN_NUM_CALL];   // 上标签（KM）


	CStatic m_pIMG_PE_U[BTN_NUM_MSG];	// 上人行道
	CStatic m_pIMG_PE_D[BTN_NUM_MSG]; // 下人行道


	CButtonST m_pBtnTunnel[BTN_NUM_TUNNEL];
	CButton* m_pbtnImgPre;	// device 上翻页
	CButton* m_pbtnImgNext;	// device 下翻页

	CFont m_editFont1; 
	CFont m_editFont_title; 
private:
	int m_width;
	int m_height;
	CMenu m_Menu;

	
private:
	void CheckTunnelPreNext();
	void CheckDevicePreNext();

	// Logic
	int GetDeviceCol(int idx, int up, int down);
	void SetDevicePos(int yoffset, int id, int idx, int col, int ctrlW = CTRL_BTN_W);
public:
	

	
	STunnel* m_pTunnelInfo;	 //ck注释，声明隧道结构体指针
	int m_nTunnelNum;	   //隧道数目
	int m_nCurTunnelIdx;	

	int m_nPageTunnel;
	int m_nPageDevice;
	
	
	CHttpRecv m_http;
	bool m_bCall;
	CSNLock m_DBLock; 
private:
	void UpdateErrorMsg();
	void UpdateCallMsg();
	void UpdateCallInfo();
	void CallFromMsg();
	bool Call(SCallMsg* pCallMsg);
	SCallMsg* GetCallMsg(int extAddr);
	void SetCallMsgStatus(int extAddr,enCallStatus callstatus);
	void DeleteCallMsg(int extAddr);

	void DisplayCallMsg();
private:
	vector<SCallMsg> m_vecCallMsg; // 电话消息队列
	CSNLock m_MsgLock; 
	CSNLock m_HeatLock; 
public:
	void InitSocket();
private:
	void  getCurDeviceNum(int curTunnelIdx, int& upNum, int& downNum);		
	void InitSel(int curTunnelIdx, int check = 0);
	bool IsDeviceOK(int curTunnelIdx);
private:
	void ReflashCallThread();
	void PopCallThread(SDevice* pDevice);

public:
	void SendBeatThead(int bFirst);
	void SendBeat(int bFirst);
	void Status2Button();
private:
	bool IsNeedSendBeat(SDevice* pDevice);
public:
	SDevice* GetDevice(int extAddr);
	SDevice* GetDevice(char* extNo);
private:
	// 更新设备状态(广播+电话)
	void UpdateStatus();
	//void UpdateOffLine();
	// update 0:all,1:call,2:msg
	
	// 0: 更新广播 1： 更新电话
	//void UpdateStatus(int extAddr, enDeviceStatus status, int update = 0);
	void UpdateStatus_BC(int extAddr, enDeviceStatus status);
	void UpdateStatus_CALL(int extAddr, enDeviceStatus status);
	void UpdateDoorRT(int extAddr, int flag, DWORD dwTime);
	void UpdateErrorCode(int extAddr, enDeviceStatus status, enErrorCode error);
	void Status2ButtonBc(enDeviceStatus status, CButton* pButton, bool bZoom = false);
	void Status2ButtonCall(enDeviceStatus status, CButton* pButton, bool bOut = false);
	void Status2ErrorMsg(int flag, UINT ID);

	void AddLogDB();
	void AddLogDB(int ext);
	void AddLogRecord(SDevice& stDevice);
	void AddLogRecords(SDevice& stDevice);
	bool Device2LogRecord(SDevice& stDevice, SRecord& record);
	// 广播是否在运行
	bool IsBCRun(int ext);
	// 电话是否在运行
	bool IsCallRun(int ext);
public:
	void UpdateData();
private:
	// 读取数据库
	void ReadDB();
	
	void DB2TunnelBtn();
	void DB2DeviceBtn(int nCurTunnelIdx);

#if 0
int ClickCall(SDevice* pDevice, int type);
#endif
	void AnswerCall(int extAddr);
private:
	int IsCallReady();
	void Call2Btn();
public:
	SCallInfo m_CallA;
	SCallInfo m_CallB;
	bool m_bPause;
private:
	void InitAPPPath();


public:
	void OnResetApp();
	BOOL UpdateSysTime();
	void CheckVersion();	
	enDeviceStatus GetCallStatus(char* id);
//add 3.7
private:
	// 发送广播
	void PlayBC();	
	void PlayOneDevice(SDevice* pDevice, bool& bMic, bool& bAux, bool bCmd);
	
	// 停止广播
	void StopBC(bool bExit = false);
	void StopOneDevice(SDevice* pDevice, bool& bMIC, bool& bAux, bool bCmd, bool bExit);
	
private:
	// 0:未选择 1：选择+配置 2：选择+未配置
	int GetDeviceConfig();
public:
	void AddMsgRecord(SDevice& stDevice, enPlaySrc model);
	void Text2Speach(char* textFile);
private:
	// 时间停止，发送结束消息
	bool TimeStopBC();
	bool TimeStopDevice(SDevice* pDevice);
	// 收到结束消息
	void CompltePlayBC(int threadID);
	bool MsgStopPlayDevice(SDevice* pDevice);
	void AutoSelAllCheckBox();	
public:
	// 结束并关闭播放
	void EndPlay(SDevice* pDevice);

	// 开始并开启广播
	bool StartPlay(SDevice* pDevice);
public:
	bool DevicePlay(SDevice* pDevice, bool bOpenAudio);
	void DeviceStop(SDevice* pDevice, bool bCloseAudio);
	void DevicePause(SDevice* pDevice);
	void DeviceResume(SDevice* pDevice);
	int GetDeviceBCStatus(SDevice* pDevice);
private:
	void OneKeyClose();
	bool GetDeviceIndex(int extAddr, int& nPageDevice, int& nTunnelIndex);
	void AutoChangePage(int extAddr);
public:
	void Device2PlayDeviceAttr(SDevice* pDevice, SPlayDeviceAttr* pPlayDeviceAttr);
	SDevice* GetDevice(DWORD dwIP);
private:
	CToolTipCtrl m_Mytip;
private:
	CMgrPic m_MgrPic;
public:
	void AutoCloseMicAux(bool bCloseMic, bool bCloseAux);
private:
	void RepaintTunnelBtn();
	bool IsTunnelRun(int nTunnelIdx);
	bool IsTTSRun();
	void GetTunnelPlayStatus(bool& bmic, bool& baux);
	
	bool IsCurBcRun(int nTunnelIdx);

	void Status2Button(int extAddr);
	SDevice* GetDeviceFromEvent(SEventInfo* pEvent);
	void EventChangeBtn(SDevice* pDevice, enRecvMsg msg);

	void RemoveRing(int id);
private:
	
	void DeleteCallAnswer(int extAddr);

	void CloseCallWithoutAnswer(SDevice* pDevice);
	void UpdateStatusFromPCBeat(SBeatReply* pBeat);
private:
	SEventInfo* IsEventTypeExist(char* pExtNo, enEventStatus type);	
	void DeleteEvent(char* pExtNo);
	void DeleteEvent(int id);
private:
	vector<SCallEvt> m_vecCallEvt;
	vector<SEventInfo> m_vecCallEvent;
private:
	char m_szLoginName[MAX_PATH];

private:
	// 重启判断
	void SetReboot();
	bool IsReboot();
	void ClearReboot();

private:
	void ReadNoBCFile();
	bool IsNoBC(char* ip);
private:
	vector<SNoIPFileData> m_vecNoBCData;
	vector<SNoIPFileData> m_vecNoCallData;
	vector<SNoIPFileData> m_vecOutData;
	vector<SNoIPFileData> m_vecLineData;
private:
	void SeatBeatReply(int extAddr);
	void UpdateDeviceLog();
private:
	vector<SBeatData> m_vecBeatData; // 巡检数据
	vector<SBeatData> m_vecStausData; // 主控发送给分控数据
	CSNLock m_BeatDataLock;
	CSNLock m_StausDataLock;
public:
	bool SleepThread(DWORD dwTime, bool bAutoExit = false);
	void ReleaseExit();
	void OnSendOpenCam(SDevice* pDevice);
	void OnSendCloseCam(SDevice* pDevice);
private:
	void SendBeatContinue();
	char* PlaySrc2String(enPlaySrc src);
	char* PlaySrc2String(SDevice& stDevice);
private:
	char m_szTip[MAX_PATH];
private:
	void ReadNoCallFile();
	bool IsNoCall(char* ip);
	void AddTipBC(SDevice* pDevice, int nCtrlID);

	void ReadDeviceLineFile();
	bool IsExistPE(char* ip);
public:
	void ReadGroupFile();
	char* GetGroup(int extAddr);
private:
	SGroupCall* m_pGroupCall;
	int m_nGroupCall;

public:
	// 读声音文件
	void ReadYingFile();
	int GetYing(int extAddr);
private:
	SBCYing* m_pBCYing;
	int m_nBCYing;

private:
	void ReadIPGroupFile();
private:
	SKeyValue* m_pIPGroupFile;
	int m_nIPGroupFile;

public:
	void ReadDeviceOutFile();
	bool IsDeviceOut(char* ip);
public:
	void SendSatus2Device();
	void SendStatus2Subsystem(BYTE* pBuffer, int size);
	void SendSatus2DeviceThread();

	// monitor
public:
	//void SetMonitor(SDevice* pDevice);
	// 开始监听
	int StartMonitor();
	//bool IsMonitor();	
	// 停止监听
	void EndMonitor();
	//bool StopMonitorFromMsg(int thread);
	bool IsMonitorDevice(char* ip);
private:
	SMonitorDevice m_MonitorDevice;

public:
	CString Txt2TTSPath(char* czTextPath);
private:
	CString m_strTTSFile;

public:
	// 读指令文件
	void ReadDevCmdFile();
	SDeviceCmd* GetDevCmd(int groupID);
private:
	vector<SDeviceCmd> m_vecDeciveCMD; // 存放指令设备
	
	void CMD_PlayOneDevice(SDevice* pDevice, char* file);
	void CMD_UpdateDeviceConfig(int extAddr, char* file);
private:

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnSet();
	afx_msg void OnRecordCall();
	afx_msg void OnRecordMsg();
	afx_msg void OnRecordLog();
	afx_msg void OnRecordDB();
	CStatic m_PictureB;
	CStatic m_PictureA;
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButtonPre();
	afx_msg void OnBnClickedButtonDeviceNext();
	afx_msg void OnBnClickedButtonDevicePre();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedBtnSendBc();
	afx_msg void OnBnClickedCheckSendBc();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);	

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMenuAboutInfo();
	afx_msg void OnMenuHelp();
	afx_msg void OnBnClickedBtnConfig();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnUpdateMenuUser(CCmdUI *pCmdUI);
	afx_msg void OnMenuOnekeyclose();

	//HBRUSH m_hbrush; 
	CStatic m_PicArrowL;
	CStatic m_PicArrowR;
	afx_msg void OnClose();
	afx_msg void OnMenuManBeat();
	CStatic m_pictureBK;
	
	CStatic m_ctrlStaticTunnelTitle;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMenuVideo();
	afx_msg void OnBnClickedBtnTts();
	afx_msg void OnMenuMonitor();
	afx_msg void OnMenuMonitorClose();
	afx_msg void OnBnClickedCheckUpAll();
	afx_msg void OnBnClickedCheckDownAll();

private:
	void EnableCall(bool bCanCall);
	bool IsDeviceLocalController(char* dwIp);
	afx_msg LRESULT OnRecvMsgE1(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgE2(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgE3(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgE4(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgHeartBeatReplyFromDevice(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgBCOption(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgDoorRTOption(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallFromDevice(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallHungUpFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallOnlineFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallRingFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallRunningFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallOfflineFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallIdleFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallNoAnswerFromOM(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgHeartBeatFromDevice(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgDeviceStatusReqFromMasterSystem(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgDeviceStatusResponseFromBranchSystem(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgDeviceStatusAckFromMasterSystem(WPARAM,LPARAM);
	afx_msg LRESULT OnRecvMsgCallReqFromMasterSystem(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvMsgCallStatusFromBranch(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvMsgBroadcastStatusFromBranch(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvMsgBroadcastReqFromMaster(WPARAM wParam,LPARAM lParam);


	afx_msg LRESULT OnRecvNoE1Answer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvNoE2Answer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvNoE3Answer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvNoE4Answer(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvNoRingEvt(WPARAM wParam,LPARAM lParam);

	afx_msg LRESULT OnRecvBranchOMDown(WPARAM wParam,LPARAM lParam);

	afx_msg LRESULT OnRecvSipgwExit(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvAudioRecordAgentExit(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvCallRecordAgentExit(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvOuterBCStart(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRecvOuterBCClose(WPARAM wParam,LPARAM lParam);
private:
	vector<SBCMsg> m_vecBCMsg;
	CSNLock m_BCMsgLock;
public:
	void AddBCMsg(SBCMsg bcMSG);
	void DeleteBCMSG(int extAddr);
	SBCMsg* GetBCMsg(int extAddr);
	bool IsBCMsgExist(int extAddr);
	void SetBCStatus(int extAddr,enBCStatus bcstatus);
	void UpdateBCMsg();

private:
	void TakeDeviceByOMStatus(char* szIp,BYTE status);
public:
	void ShowInfo(CString str);
	afx_msg void OnOpenaudiorecord();
	afx_msg void OnOpencallrecord();
	afx_msg void OnCloseaudiorecord();
	afx_msg void OnClosecallrecord();
	afx_msg LRESULT OnRecvMsgDeviceBeatReq(WPARAM wParam,LPARAM lParam); 
	afx_msg LRESULT OnRecvMsgNoPackets(WPARAM wParam,LPARAM lParam);
public:
	void SendBeat2Offlines();
	void SendBeat2Offline(int extAddr);
	bool IsNeedSendBeat2Offline(SDevice* pDevice);

public:
	vector<SStatus4Send> m_vecStatus4Send;
	CSNLock m_StatusLock;
};

extern CSIPCallDlg* m_pAPP;
extern CLog2File g_Log;
extern CAudioPlay g_Play;
extern CConfig g_Config;
extern CDataM g_data;
extern CDecodeMsg g_MSG;
extern CMgrUser g_User;
extern CMgrPlay g_MgrPlay;

extern CString g_szAPPPath;
extern CString g_ttsFilePath;

extern bool g_bOpenMic;
extern bool g_bOpenAux;
extern bool g_bOpenAPP;

extern bool g_bExitApp;