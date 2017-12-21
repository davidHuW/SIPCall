#include "StdAfx.h"
#include "DecodeMsg.h"
#include "SIPCallDlg.h"

static bool s_bExit = false;
static bool s_bRun = false;


#define RECV_BUFFER_SZ (4096)

#define MSG_QUE (0)

static BYTE s_RecvBuf[RECV_BUFFER_SZ] = {0};
static BYTE s_OuterRecvBuf[RECV_BUFFER_SZ] = {0};
static BYTE s_PCRecvBuf[RECV_BUFFER_SZ] = {0};

static BYTE s_PCSendBuf[RECV_BUFFER_SZ] = {0};

typedef bool (*DLLFUN)(HWND hWnd);
DLLFUN dll_Fun = NULL;


#define MSG_OUTER_PORT	(54321)
#define MSG_PC_PORT		(54320)
#define MSG_SEND_COUNT_NORMAL	(1)
#define MSG_SEND_COUNT_RESEND2	(2)
#define BC_SEND_MSG	(1)
extern char* g_dwLocalIp;

void Byte2Hex(BYTE *pbSrc, char *szDst, UINT uLen)	 //?? to ê?áù????
{
	char strtemp[256];
	//UCHAR b[512] = {0};
	for(UINT k = 0; k < uLen; k++)//×a???aBYTEDíêy×é
	{
		BYTE bt=*(char*)(pbSrc + k); //×?·?Dí

		memset(strtemp, 0, 256);
		//??×?·?ò?ê?áù????·?ê??íè?áùê±±?á?strtemp′?·?￡?×￠òa?aà??óè?ò???????
		sprintf(strtemp, "%02x ", bt);	//2?1?á???μ??°??210
		//strtemp.Format("%02x ", bt);

		strcat(szDst, strtemp); //?óè??óê?±à?-?ò??ó|×?·?′?
	}
}
CDecodeMsg::CDecodeMsg(void)
{
	WORD VersionRequested = MAKEWORD(1, 1);
	WSADATA WsaData; 
	int Error = WSAStartup(VersionRequested, &WsaData);
    //g_dwLocalIp = GetHostAddr();
	g_dwLocalIp = GetHostIP();
	m_vecIPData.reserve(10);
}
CDecodeMsg::~CDecodeMsg(void)
{
	s_bExit = true;
	s_bRun = false;

	if (m_socket != 0xFFFFFFFF)
	{
		if(SOCKET_ERROR != closesocket(m_socket)){
			m_socket = 0xFFFFFFFF;			
		}	
	}
	if (m_socketPC != 0xFFFFFFFF)
	{
		if(SOCKET_ERROR != closesocket(m_socketPC)){
			m_socketPC = 0xFFFFFFFF;			
		}	
	}
	s_bExit = false;
	if (!m_vecIPData.empty())
	{
		m_vecIPData.clear();
	}
	WSACleanup();
}
int CDecodeMsg::CreateSocket(/*char* ip, unsigned short port*/)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
	if(m_socket == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	USHORT pcport = g_Config.m_SystemConfig.portPC;
	serAddr.sin_port = htons(pcport);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if(bind(m_socket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind error !");
		closesocket(m_socket);
		return 0;
	}
	CreateRecvThread();
	return 1;
}
int CDecodeMsg::CreatePCSocket()
{
	m_socketPC = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
	if(m_socketPC == INVALID_SOCKET)
	{
		printf("socket pc error !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	USHORT pcport = MSG_PC_PORT;
	serAddr.sin_port = htons(pcport);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if(bind(m_socketPC, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind pc error !");
		closesocket(m_socketPC);
		return 0;
	}
	CreatePCRecvThread();
	return 1;
}
int CDecodeMsg::CreateOuterSocket()
{
	m_socketOuter = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
	if(m_socketOuter == INVALID_SOCKET)
	{
		printf("socket outer error !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	USHORT outerport = MSG_OUTER_PORT;
	serAddr.sin_port = htons(outerport);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if(bind(m_socketOuter, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind outer error !");
		closesocket(m_socketOuter);
		return 0;
	}
	CreateOuterRecvThread();
	return 1;
}
UINT RecvMsgProc(LPVOID pParam)  
{  
	g_Log.output(LOG_TYPE, "recv msg init....\r\n");
	CDecodeMsg* pDecodeMsg = (CDecodeMsg*)pParam;
	if (pDecodeMsg == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
	g_Log.output(LOG_TYPE, "recv msg start....\r\n");

// 	fd_set  fds;
// 	struct  timeval  timeout={0, 10}; //selectμè′y3??￡?3?????ˉ￡?òa·?×èè??í??0
	while(!g_bExitApp/*!s_bExit*/){	
		//sockaddr_in sin;
		//int sinlen;
		//int recvLen = recvfrom(pDecodeMsg->m_socket, (char*)s_RecvBuf, RECV_BUFFER_SZ, 0, (sockaddr *)&sin, &sinlen);
		Sleep(1);
		int timeout = 1000;
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(pDecodeMsg->m_socket, &rdset);
		timeval tm;
		tm.tv_sec = timeout / 1000;
		tm.tv_usec = timeout % 1000 * 1000;
		int ts = select(pDecodeMsg->m_socket + 1, &rdset, NULL,  NULL, &tm);
		if(ts <= -1){
			int iError = WSAGetLastError();
			return 0;
		}
		else if (ts == 0)
		{
			// 3?ê±
		}
		else{
#if 0
			int recvLen = recv(pDecodeMsg->m_socket, (char*)s_RecvBuf, RECV_BUFFER_SZ, 0);
#else
			sockaddr_in sin;
			int sinlen = sizeof(struct sockaddr_in);
			int recvLen = recvfrom(pDecodeMsg->m_socket, (char*)s_RecvBuf, RECV_BUFFER_SZ, 0, (sockaddr *)&sin, &sinlen);
#endif
			if (recvLen > 0)
			{
				char* recvIP = inet_ntoa(sin.sin_addr);
				USHORT recvPort = ntohs(sin.sin_port);
				//g_Log.output(LOG_TYPE_8, "recv msg len: %d....ip: %s\r\n", recvLen, recvIP);
				if (recvLen == SEND_BUFFER_SZ)
				{
					pDecodeMsg->DecodeMsg(s_RecvBuf, recvLen, recvIP, recvPort);
					
				}
				else if (recvLen == 7)
				{
					pDecodeMsg->DecodeMsgError7(s_RecvBuf, recvLen, recvIP, recvPort);
				}
				else if (recvLen == 9)
				{
					pDecodeMsg->DecodeMsgError9(s_RecvBuf, recvLen, recvIP, recvPort);
				}
			}
			else{
				int iError = WSAGetLastError();
				g_Log.output(LOG_TYPE_8, "recv msg len[%d], error[%d], socket[%d]!!!!!\r\n", recvLen, iError, pDecodeMsg->m_socket);
			}
		}
	}
	return 1;	
} 
void CDecodeMsg::CreateRecvThread()	 //	  ′′?¨ Thread??3ì
{
	AfxBeginThread(RecvMsgProc, this);  
}
UINT RecvPCMsgProc(LPVOID pParam)  
{  
	g_Log.output(LOG_TYPE, "recv pc msg init....\r\n");
	CDecodeMsg* pDecodeMsg = (CDecodeMsg*)pParam;
	if (pDecodeMsg == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
	g_Log.output(LOG_TYPE, "recv pc msg start....\r\n");


	while(!g_bExitApp){	

		Sleep(1);
		int timeout = 1000;
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(pDecodeMsg->m_socketPC, &rdset);
		timeval tm;
		tm.tv_sec = timeout / 1000;
		tm.tv_usec = timeout % 1000 * 1000;
		int ts = select(pDecodeMsg->m_socketPC + 1, &rdset, NULL,  NULL, &tm);
		if(ts <= -1){
			int iError = WSAGetLastError();
			return 0;
		}
		else if (ts == 0)
		{
			// 超时
		}
		else{

			sockaddr_in sin;
			int sinlen = sizeof(struct sockaddr_in);
			int recvLen = recvfrom(pDecodeMsg->m_socketPC, (char*)s_PCRecvBuf, RECV_BUFFER_SZ, 0, (sockaddr *)&sin, &sinlen);

			if (recvLen > 0)
			{
				if (recvLen == USER_BUFFER_SZ)
				{
					pDecodeMsg->DecodeUserMsg(s_PCRecvBuf, USER_BUFFER_SZ);
					//g_Log.output(LOG_TYPE_8, "@@recv pc--ip: %s  data:%s\r\n", inet_ntoa(sin.sin_addr),s_PCRecvBuf);
				}
			}
			else{
				int iError = WSAGetLastError();
				g_Log.output(LOG_TYPE_8, "recv pc msg len[%d], error[%d], socket[%d]!!!!!\r\n", recvLen, iError, pDecodeMsg->m_socketPC);
			}
		}

	}

	return 1;	
} 
UINT RecvOuterMsgProc(LPVOID pParam)  
{  
	g_Log.output(LOG_TYPE, "recv outer msg init....\r\n");
	CDecodeMsg* pDecodeMsg = (CDecodeMsg*)pParam;
	if (pDecodeMsg == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
	g_Log.output(LOG_TYPE, "recv outer msg start....\r\n");


	while(!g_bExitApp){	

		Sleep(1);
		int timeout = 1000;
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(pDecodeMsg->m_socketOuter, &rdset);
		timeval tm;
		tm.tv_sec = timeout / 1000;
		tm.tv_usec = timeout % 1000 * 1000;
		int ts = select(pDecodeMsg->m_socketOuter + 1, &rdset, NULL,  NULL, &tm);
		if(ts <= -1){
			int iError = WSAGetLastError();
			return 0;
		}
		else if (ts == 0)
		{
			// 超时
		}
		else{

			sockaddr_in sin;
			int sinlen = sizeof(struct sockaddr_in);
			int recvLen = recvfrom(pDecodeMsg->m_socketOuter, (char*)s_OuterRecvBuf, RECV_BUFFER_SZ, 0, (sockaddr *)&sin, &sinlen);

			if (recvLen > 0)
			{
				pDecodeMsg->DecodeOuterMsg(s_OuterRecvBuf,recvLen);
			}
			else{
				int iError = WSAGetLastError();
				g_Log.output(LOG_TYPE_8, "recv outer msg len[%d], error[%d], socket[%d]!!!!!\r\n", recvLen, iError, pDecodeMsg->m_socketPC);
			}
		}

	}

	return 1;	
} 
void CDecodeMsg::CreatePCRecvThread()
{
	AfxBeginThread(RecvPCMsgProc, this);
}
void CDecodeMsg::CreateOuterRecvThread()
{
	AfxBeginThread(RecvOuterMsgProc, this);
}
//----------------------分割线-------------------------
void CDecodeMsg::SendHeartBeat(BYTE subTelNo, char* ip, int port)//巡检
{
	m_Lock.Lock();
	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0x33;   //巡检
	m_pBuffer[4] = 0x00;
	m_pBuffer[5] = 0x01;
	m_pBuffer[6] = 0x01;
	m_pBuffer[7] = GetCheckNum();  //巡检     A09F023300010176
	
	
	Send8Byte2Device(m_pBuffer, ip, port);
	m_Lock.Unlock();
}
void CDecodeMsg::SendCallReq2Branch(BYTE subTelNo,char* szIp,BYTE callOpt)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xA0;		//对分控分机进行操作请求
	s_PCSendBuf[4] = callOpt;		//0x01 打开分机，0x02 关闭分机，0x03 呼叫转移打开分机
	s_PCSendBuf[8] = 0xff;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,szIp,port);
	}	
	m_LockPC.Unlock();
}
//************************************
// Method:    SendBroadcastReq
// FullName:  CDecodeMsg::SendBroadcastReq
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: BYTE subTelNo
// Parameter: char * szIp
// Parameter: BYTE broadcastOpt 1:open 2:close
//************************************
void CDecodeMsg::SendBroadcastReq2Branch(BYTE subTelNo,char* szIp,BYTE broadcastOpt)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xA3;		//发送打开或关闭广播 指令
	s_PCSendBuf[4] = broadcastOpt;   //1：open   2：close   3:pause   4:resume
	s_PCSendBuf[8] = 0xff;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,szIp,port);
	}
	else{

	}	
	m_LockPC.Unlock();
}
void CDecodeMsg::DecodeBroadcastReqFromMaster(BYTE* pBuffer,int size)
{
	SendMessage(m_pAPP->m_hWnd,RECV_MSG_BC_REQ,pBuffer[2],pBuffer[4]);
}
void CDecodeMsg::SendOpenCall(BYTE subTelNo, char* ip, int port) //通话
{
	m_Lock.Lock();

	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0x01;
	m_pBuffer[4] = 0x00;
	m_pBuffer[5] = 0x01;
	m_pBuffer[6] = 0x01;
	m_pBuffer[7] = GetCheckNum();	 //通话     A09F 02 0100010144 
	
	Send8Byte2Device(m_pBuffer, ip, port);
	m_Lock.Unlock();
}
void CDecodeMsg::SendDeviceCallStatus2Master(BYTE subTelNo,BYTE status)
{
	//branch send device call status 2 master
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xA1;		//send call status 
	s_PCSendBuf[4] = status;    //01: is using 02: is usable 03: is offline 04:om down  ,05:sipgw  down
	s_PCSendBuf[8] = 0xff;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if(!g_Config.m_SystemConfig.nMainSystem)
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,data[0].ip,port);
		
	m_LockPC.Unlock();
}
void CDecodeMsg::SendBranchOMDown2Master(BYTE status)
{
	//·??úμ??°?y?úê1ó?￡?í¨?a·????￠?÷??
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	s_PCSendBuf[2] = 0;
	s_PCSendBuf[3] = 0xA5;		//·??úμ??°×′ì?
	s_PCSendBuf[4] = status;    //01￡oonline￡?02￡odown
	s_PCSendBuf[8] = 0xff;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	IP2BYTE(g_dwLocalIp,s_PCSendBuf[9],s_PCSendBuf[10],s_PCSendBuf[11],s_PCSendBuf[12]);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if(!g_Config.m_SystemConfig.nMainSystem)
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,data[0].ip,port);

	m_LockPC.Unlock();
}
void CDecodeMsg::DecodeBrachOMDown(BYTE* pBuffer,int size)
{
	CString ip = BYTE2IP(pBuffer[9],pBuffer[10],pBuffer[11],pBuffer[12]);
	SendMessage(m_pAPP->m_hWnd,RECV_BRANCH_OM_STATUS,pBuffer[2],(LPARAM)&ip);
}
void CDecodeMsg::DecodeDeviceCallStatusFromBranch(BYTE* pBuffer,int size)
{
	SendMessage(m_pAPP->m_hWnd,RECV_MSG_CALL_STATUS,pBuffer[2],pBuffer[4]);
}
void CDecodeMsg::SendDeviceBroadcastStatus2Master(BYTE subTelNo,char* szIp,BYTE status)
{
	//分机广播正在使用，通知分控、主控
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xA2;	  //分机广播状态
	s_PCSendBuf[4] = status;    //01：在使用，02：可用，03：离线
	s_PCSendBuf[8] = 0xff;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		//sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,szIp,port);
	}
	else{
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,data[0].ip,port);
	}	
	m_LockPC.Unlock();
}
void CDecodeMsg::DecodeDeviceBroadcastStatusFromBranch(BYTE* pBuffer,int size)
{
	//2 分机号，4广播状态
	SendMessage(m_pAPP->m_hWnd,RECV_MSG_BROADCAST_STATUS,pBuffer[2],pBuffer[4]);
}
void CDecodeMsg::SendHangUp(BYTE subTelNo, char* ip, int port)	 //挂机
{
	m_Lock.Lock();
	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0x02;
	m_pBuffer[4] = 0x00;
	m_pBuffer[5] = 0x01;
	m_pBuffer[6] = 0x01;
	m_pBuffer[7] = GetCheckNum();
	
	Send8Byte2Device(m_pBuffer, ip, port);	//挂机     A09F020200010145
	m_Lock.Unlock();
}
void CDecodeMsg::SendOpenBroadcast(BYTE subTelNo, char* ip, int port)//开广播
{
#if BC_SEND_MSG
	//g_Log.output(LOG_TYPE, "SendOpenBroadcast...., %d\r\n", port);
	m_Lock.Lock();
	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0x05;   //广播开   A09F020501010149
	m_pBuffer[4] = 0x01;
	m_pBuffer[5] = 0x01;
	m_pBuffer[6] = 0x01;
	m_pBuffer[7] = GetCheckNum();
	
	Send8Byte2Device(m_pBuffer, ip, port);
	m_Lock.Unlock();
#endif
}
void CDecodeMsg::SendCloseBroadcast(BYTE subTelNo, char* ip, int port, bool beat)
{
#if BC_SEND_MSG
	m_Lock.Lock();
	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0x05;
	m_pBuffer[4] = 0x00;
	m_pBuffer[5] = 0x01;
	m_pBuffer[6] = 0x01;
	m_pBuffer[7] = GetCheckNum();
	
	Send8Byte2Device(m_pBuffer, ip, port);
	m_Lock.Unlock();
#endif
}
//------------ ??ó| ------------
void CDecodeMsg::SendCallAnswer(BYTE subTelNo, char* ip, int port)
{
	// 回应
	// 00 00 02 E1 00 00 00 E3
	m_Lock.Lock();
	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0xE5;	  //呼叫回应 A09F02E500000026
	m_pBuffer[4] = 0x00;
	m_pBuffer[5] = 0x00;
	m_pBuffer[6] = 0x00;
	m_pBuffer[7] = GetCheckNum();
	
	Send8Byte2Device(m_pBuffer, ip, port);
	m_Lock.Unlock();
}
BYTE CDecodeMsg::GetCheckNum(int len) //第八位校验和
{
	BYTE checkNum = 0;
	for (int i = 0; i < len - 1; i++)
	{
		checkNum += m_pBuffer[i];
	}
	checkNum &= 0xFF;
	return checkNum;
}
//----------------------分割线-------------------------
void CDecodeMsg::DecodeMsgError7(BYTE* pBuffer, int size, char* szIP, USHORT port)
{
	BYTE pErrorBuf[8] = {0};	  //错误数据初始化0
	if (pBuffer[0] == 0x9F)
	{
		pErrorBuf[0] = 0xA0;	 //错误位左移？
		memcpy(pErrorBuf + 1, pBuffer, 7 * sizeof(BYTE));

		// check
		BYTE checkNum = 0;		  //校验位置0
		for (int i = 0; i < SEND_BUFFER_SZ - 1; i++)	 //(8)
		{
			checkNum += pErrorBuf[i];
		}
		checkNum &= 0xFF;
		if (pErrorBuf[7] == checkNum)
		{
			DecodeMsg(pErrorBuf, 8, szIP, port);
		}	
		else{ //错误数据校验位不对，则报错
			g_Log.output(LOG_TYPE_8, "recv msg len size(7) check error....\r\n");
		}
	}
	else{  //正常MSG第一位不对，直接报错！！！
		g_Log.output(LOG_TYPE_8, "recv msg len size(7) error....\r\n");
	}
}
void CDecodeMsg::DecodeMsgError9(BYTE* pBuffer, int size, char* szIP, USHORT port)
{
	BYTE pErrorBuf[8] = {0};
	if ((pBuffer[0] == 0xA0) && (pBuffer[1] == 0x9F))
	{		
		memcpy(pErrorBuf, pBuffer, 8 * sizeof(BYTE)); //复制到ERRORbuf
		// check
		BYTE checkNum = 0;
		for (int i = 0; i < SEND_BUFFER_SZ - 1; i++)
		{
			checkNum += pErrorBuf[i];
		}
		checkNum &= 0xFF;
		if (pErrorBuf[7] == checkNum)
		{
			DecodeMsg(pErrorBuf, 8, szIP, port);
		}	
		else{
			g_Log.output(LOG_TYPE_8, "recv msg len size(9) check error....\r\n");
		}
	}
	else{
		g_Log.output(LOG_TYPE_8, "recv msg len size(9) error....\r\n");
	}
}
//-------------消息处理构造函数-------beginning------------------
void StringSplit(CString source, CStringArray& dest, char division)     
{     
	if(source.IsEmpty())  
	{  

	}  
	else  
	{  
		int pos = source.Find(division);  
		if(pos == -1)  
		{  
			dest.Add(source);  
		}  
		else  
		{  
			dest.Add(source.Left(pos));  
			source = source.Mid(pos+1);  
			StringSplit(source,dest,division);  
		}  
	}  
}  
void CDecodeMsg::DecodeOuterMsg(BYTE* pBuffer,int size)
{
	CStringArray msgs;
	CString msg;
	msg.Format(L"%s",pBuffer);		//unicode编码 发送
	/*wchar_t outStr[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, (char*)pBuffer, -1, outStr, MAX_PATH);
	msg = outStr;*/
	StringSplit(msg,msgs,';');
	if(msgs.GetSize()>0)
	{
		int compareRtn = msgs[0].CompareNoCase(L"GetMusicList");
		if(compareRtn == 0)
		{
			DecodeOuterBCMusicPath();
		}else{
			CString cmd,path;
			vector<int> ets;
			cmd = msgs[0];
			path = msgs[1];
			CStringArray etsStr;
			StringSplit(msgs[2],etsStr,',');
			for(int i=0;i<etsStr.GetSize();i++)
			{
				int et = _ttoi(etsStr[i]);
				ets.push_back(et);
			}
			DecodeOuterBCControl(cmd,path,ets);
		}
	}
}
BOOL IsFileExist(const CString& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
void CDecodeMsg::DecodeOuterBCMusicPath()
{
	char* ip = g_Config.m_SystemConfig.IP_OUTER_BC_CONTROL;
	int port = g_Config.m_SystemConfig.portOuterBCControl;
	TCHAR pBuf[MAX_PATH] = {0};
	int nNum = 0;
	SListSong* pListSong = g_data.GetSongList(nNum);
	for(int i=0;i<nNum;i++)
	{
		//sprintf(pBuf, "music:%s;",pListSong[i].szName);
		MultiByteToWideChar(CP_ACP,0,pListSong[i].szName,-1,pBuf,MAX_PATH);
		sendtomsg(m_socketOuter, (char*)pBuf, wcslen(pBuf)*sizeof(TCHAR), MSG_SEND_COUNT_RESEND2, ip, port);

		Sleep(30);
	}
}
void CDecodeMsg::DecodeOuterBCControl(const CString& cmd,const CString& path,const vector<int>& ets)
{
	if(cmd.CompareNoCase(_T("start")) == 0)
	{
		if(path.IsEmpty()||!IsFileExist(g_szAPPPath+"music\\"+path))
		{
			return ;
		}
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_OUTER_BC_OPT_START,(WPARAM)&(g_szAPPPath+"music\\"+path),(LPARAM)&ets);
	}else
	{
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_OUTER_BC_OPT_CLOSE,NULL,(LPARAM)&ets);
	}
}
void CDecodeMsg::DecodeMsg(BYTE* pBuffer, int size, char* szIP, USHORT port)
{
	if (size != SEND_BUFFER_SZ)	 //	 MSG长度不为8，报错
	{
		g_Log.output(LOG_TYPE_8, "recv device msg len size error....\r\n");
		return;
	}
	else{
		char szHex[256] = {0};
		Byte2Hex(pBuffer, szHex, SEND_BUFFER_SZ);
		g_Log.output(LOG_TYPE_8, "recv msg: IP:%s,,data:%s..\r\n", szIP,szHex);
		//正确则输出MSG!
	}
	// check  校验BEGIN
	BYTE checkNum = 0;
	for (int i = 0; i < SEND_BUFFER_SZ - 1; i++)
	{
		checkNum += pBuffer[i];
	}
	checkNum &= 0xFF;  
// 	if (pBuffer[7] != checkNum)
// 	{
// 		return;
// 	}
	// end
	BYTE msgFlag = pBuffer[3];	// 第四位命令标志位（开门关门巡检等~）
	switch(msgFlag){
	case 0x58:	//开门
		DecodeDoor(pBuffer, size);	//解析门开，调用DecodeDoor（调用窗口句柄回到WINpro触发消息）!!!!!
		Send8Byte2PC(pBuffer, szIP, port);
		break ;		
	case 0x57:	//关门
		DecodeDoorClose(pBuffer, size);
		Send8Byte2PC(pBuffer, szIP, port);
		break;
	case 0x03:
		DecodeCall(pBuffer, size);
		//Send8Byte2PC(pBuffer, szIP, port);
		break ;
	case 0x43:	  //巡检回应
		DecodeHeartBeat(pBuffer, size);
		Send8Byte2PC(pBuffer, szIP, port);
		break ;
	case 0xE1:		//·??ú???′′ò?aí¨?°￡??÷??ê?μ?oó?üD?×′ì??￠????·???￡?·???ê?μ?oó?üD?×′ì?
		//DecodeE1(pBuffer, size);
		//if(g_Config.m_SystemConfig.nMainSystem)
		PostMessage(m_pAPP->m_hWnd, RECV_MSG_E1,  pBuffer[2], NULL);
		Send8Byte2PC(pBuffer, szIP, port);
		break ;
	case 0xE2:
		//DecodeE2(pBuffer, size);
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_E2,  pBuffer[2], NULL);
		Send8Byte2PC(pBuffer, szIP, port);
		break; 
	case 0xE3:
		DecodeE3(pBuffer, size);
		//×a·￠1?2￥?a???′￡?·￠?í??Dèòaò????a1?2￥??á????￠?óáD￡??¨ê±?ì2é
		Send8Byte2PC(pBuffer, szIP, port);
		break ;
	case 0xE4:
		DecodeE4(pBuffer, size);
		////×a·￠1?2￥1????′￡?·￠?í??Dèòaò???1?1?2￥??á????￠?óáD￡??¨ê±?ì2é
		Send8Byte2PC(pBuffer, szIP, port);
		break ;
	case 0xE9:
		Decode_Auto_BC_Recv(pBuffer, size);
		break;	
	case 0x19:
		Decode_Auto_BC_ReRecv(pBuffer, size);
		break;
	}	
}
//-------------消息处理构造函数--ending-----------------------
//--------------解析用户MSG-------------------------------
void CDecodeMsg::DecodeUserMsg(BYTE* pBuffer, int size)
{	
	if (size != USER_BUFFER_SZ)		 //用户MSG 16
	{
		g_Log.output(LOG_TYPE, "recv pc msg len size error....\r\n");
		return;
	}
	else{
		char szHex[256] = {0};
		Byte2Hex(pBuffer, szHex, USER_BUFFER_SZ);
		g_Log.output(LOG_TYPE_8, "!!!!recv pc msg：data:%s..\r\n",szHex);
	}
	// check
	BYTE checkNum = 0;
	for (int i = 0; i < USER_BUFFER_SZ - 1; i++)
	{
		checkNum += pBuffer[i];
	}
	checkNum &= 0xFF;

	BYTE msgFlag = pBuffer[3];	  //消息标志位
	switch(msgFlag){
	case 0xBB:	//PC心跳
		break;
	case 0xDB:	//手动巡检？	
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_DEVICE_BEAT,  pBuffer[2], NULL);
		g_Log.output(LOG_TYPE, "RECV_MSG_DEVICE_BEAT.........\r\n");
		break;
	case 0xDA:	//手动巡检回复？	
		{
			SBeatReply beat;
			beat.extAddr = pBuffer[2];
			beat.bc =  (enDeviceStatus)(pBuffer[8]);
			beat.call =  (enDeviceStatus)(pBuffer[9]);
			SendMessage(m_pAPP->m_hWnd, RECV_MSG_DEVICE_BEAT_REPLY,  pBuffer[2], (LPARAM)(&beat));	
			g_Log.output(LOG_TYPE, "RECV_MSG_DEVICE_BEAT_REPLY[%d].........\r\n", pBuffer[2]);
		}
		break;
	case 0xDC:	//更新状态
		{
			SBeatReply beat;
			beat.extAddr = pBuffer[2];
			beat.bc =  (enDeviceStatus)(pBuffer[8]);
			beat.call =  (enDeviceStatus)(pBuffer[9]);
			SendMessage(m_pAPP->m_hWnd, RECV_MSG_DEVICE_UPDATE_STATUS,  pBuffer[2], (LPARAM)(&beat));	
			g_Log.output(LOG_TYPE, "RECV_MSG_DEVICE_UPDATE_STATUS[%d].........\r\n", pBuffer[2]);
		}
		break;
	case 0x90:		//由分控请求改为主控请求
		DecodeDeviceStatusReq(pBuffer, size);
		break;
	case 0x91:		//由主控推分控改为分控推给主控
		DecodeDeviceStatusResponse(pBuffer, size);
		break;
	case 0x92:		//由分控推主控改为主控推给分控
		DecodeDeviceStatusAck(pBuffer, size);
		break;
	case 0xA0:
		DecodeCallReqFromMaster(pBuffer,size);
		break;
	case 0xA1:
		DecodeDeviceCallStatusFromBranch(pBuffer,size);
		break;
	case 0xA2:
		DecodeDeviceBroadcastStatusFromBranch(pBuffer,size);
		break;
	case 0xA3:
		DecodeBroadcastReqFromMaster(pBuffer,size);
		break;
	case 0xE1:
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_E1,pBuffer[2],pBuffer[4]);
		break;
	case 0xE2:
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_E2,pBuffer[2],pBuffer[4]);
		break;
	case 0xE3:
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_E3,pBuffer[2],pBuffer[4]);
		break;
	case 0xE4:
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_E4,pBuffer[2],pBuffer[4]);
		break;
	case 0x43:
		DecodeHeartBeat(pBuffer,size);
		break;
	case 0xA5:
		DecodeBrachOMDown(pBuffer,size);
		break;
	case 0xB0:
		DecodeDeviceBeatReq(pBuffer,size);
		break;
	}	
}
void CDecodeMsg::SendCallResponse2Master(BYTE subTelNo)
{

}
void CDecodeMsg::DecodeCallResponseFromMaster(BYTE* pBuffer,int size)
{}
void CDecodeMsg::DecodeCallReqFromMaster(BYTE* pBuffer,int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_REQ,  pBuffer[2], pBuffer[4]);
}
void CDecodeMsg::SendPCBeat()
{	
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;	
	s_PCSendBuf[2] = 0x00;
	s_PCSendBuf[3] = 0xBB;
	s_PCSendBuf[4] = 0x00;
	s_PCSendBuf[5] = 0x00;
	s_PCSendBuf[6] = 0x00;
	s_PCSendBuf[7] = 0x00;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		}
	}
	else{
		if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}
	}	
	m_LockPC.Unlock();	
}
void CDecodeMsg::SendPCBeatReply(char* pIP)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;	
	s_PCSendBuf[2] = 0x00;
	s_PCSendBuf[3] = 0xBA;
	s_PCSendBuf[4] = 0x00;
	s_PCSendBuf[5] = 0x00;
	s_PCSendBuf[6] = 0x00;
	s_PCSendBuf[7] = 0x00;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	
	USHORT port = MSG_PC_PORT;
	sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, pIP, port);
	m_LockPC.Unlock();	
}
void CDecodeMsg::SendDeviceBeat(BYTE subTelNo)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;	
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xDB;
	s_PCSendBuf[4] = 0x00;
	s_PCSendBuf[5] = 0x00;
	s_PCSendBuf[6] = 0x00;
	s_PCSendBuf[7] = 0x00;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		}
	}
	else{
		if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}
	}	
	m_LockPC.Unlock();	
}
void CDecodeMsg::SendDeviceBeatReply(BYTE subTelNo, enDeviceStatus bc,  enDeviceStatus call)
{
	m_LockPC.Lock();   //线程，上锁
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xDA;
	s_PCSendBuf[4] = 0x00;
	s_PCSendBuf[5] = 0x00;
	s_PCSendBuf[6] = 0x00;
	s_PCSendBuf[7] = 0x00;
	s_PCSendBuf[8] = bc;
	s_PCSendBuf[9] = call;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);


	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		}
	}
	else{
		if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}
	}	
	m_LockPC.Unlock();
}
void CDecodeMsg::SendDeviceUpdateStatus(BYTE subTelNo, enDeviceStatus bc)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
	
	s_PCSendBuf[2] = subTelNo;
	s_PCSendBuf[3] = 0xDC;
	s_PCSendBuf[4] = 0x00;
	s_PCSendBuf[5] = 0x00;
	s_PCSendBuf[6] = 0x00;
	s_PCSendBuf[7] = 0x00;
	s_PCSendBuf[8] = bc;
	s_PCSendBuf[9] = 0x00;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);

	
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		}
	}
	else{
		if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}
	}	
	m_LockPC.Unlock();
}
void CDecodeMsg::DecodeHeartBeat(BYTE* pBuffer, int size)
{
	BYTE flag = pBuffer[5];
	enDeviceStatus status = ENUM_DEVICE_STATUS_UNKNOWN;
	switch(flag){
	case 0x00:		//00表示有回应
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_OK,  pBuffer[2], NULL);
		break ;
	case 0x06:
		//status = ENUM_DEVICE_STATUS_MIC;
#if 0
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_MIC);
#else
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_OK,  pBuffer[2], NULL);
#endif
		break ;
		//------心跳包测试-------各种ERR--------
	case 0x08:
		//status = ENUM_DEVICE_STATUS_DOOR;
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_DOOR);	
		break ;
	case 0x10:
		//status = ENUM_DEVICE_STATUS_BUTTON;
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_BUTTON);
		break ;
	case (0x06 + 0x08):
		//status = ENUM_DEVICE_STATUS_MIC_DOOR;
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_MIC_DOOR);
		break;
	case (0x06 + 0x10):
		//status = ENUM_DEVICE_STATUS_MIC_BTN;
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_MIC_BTN);
		break;
	case (0x08 + 0x10):
		//status = ENUM_DEVICE_STATUS_DOOR_BTN;
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_DOOR_BTN);
		break;
	case (0x06 + 0x08 + 0x10):
		//status = ENUM_DEVICE_STATUS_MIC_DOOR_BTN;
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_BEAT_ERROR,  pBuffer[2], ENUM_ERROR_STATUS_MIC_DOOR_BTN);
		break;
	}	
}
//-----给构造函数调用------------
void CDecodeMsg::DecodeCall(BYTE* pBuffer, int size)
{	
	// 回应
	// 00 00 02 E1 00 00 00 E3
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL,  pBuffer[2], NULL);
}
void CDecodeMsg::DecodeDoor(BYTE* pBuffer, int size)//700行左右跳转解析门开命令
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_DOOR_RT_OPEN,  pBuffer[2], NULL);
	//触发WINproc消息机制
}
void CDecodeMsg::DecodeDoorClose(BYTE* pBuffer, int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_DOOR_RT_CLOSE,  pBuffer[2], NULL);
}
// 1?2￥?a??ó|
void  CDecodeMsg::DecodeE3(BYTE* pBuffer, int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_E3,  pBuffer[2], NULL);
	//SendBCStatus(ENUM_SEND_BC_STATUS_OPEN, pBuffer[2]);	
}
// 广播关回应
void  CDecodeMsg::DecodeE4(BYTE* pBuffer, int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_E4,  pBuffer[2], NULL);
	//SendBCStatus(ENUM_SEND_BC_STATUS_CLOSE, pBuffer[2]);
}
// 分控请求状态
void CDecodeMsg::DecodeDeviceStatusReq(BYTE* pBuffer, int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_DEVICESTATUS_MASTERSYSTEM_REQ,  pBuffer[2], (LPARAM)pBuffer);
}
// 分控回应状态接受
void CDecodeMsg::DecodeDeviceStatusAck(BYTE* pBuffer, int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_DEVICESTATUS_MASTERSYSTEM_ACK,  pBuffer[2], (LPARAM)pBuffer);
}
void CDecodeMsg::DecodeDeviceStatusResponse(BYTE* pBuffer, int size)
{
	SendMessage(m_pAPP->m_hWnd, RECV_MSG_DEVICESTATUS_BRANCHSYSTEM_RESPONSE,  pBuffer[2], (LPARAM)pBuffer);
}
void CDecodeMsg::SendOpenMic(char* ip, int port, bool bAux)
{
	g_Log.output(LOG_TYPE, "SendOpenMic....port, %d\r\n", port);
	m_Lock.Lock();
	// 先关闭
	BYTE pBuf1[] = {0x43,0x21,0x00,0x04,0x00,0x00,0x00,0x00,0x68};
	
	sendtomsg(m_socket, (const char*)pBuf1, 9, MSG_SEND_COUNT_RESEND2, ip, port);
	Sleep(10);
	// 再开启	
	//
	if (bAux)
	{
		g_bOpenAux = true;
		BYTE pBuf[] = {0x43,0x20,0x00,0x10,0x00,0x00,0x00,0x00,0xe0,0x64,0x01,0x07,0x64,0x01,0xff,0xff,0x00,0x00,0x01,0x00,0x23};					  
		sendtomsg(m_socket, (const char*)pBuf, 21, MSG_SEND_COUNT_RESEND2, ip, port);
	}
	else{
		g_bOpenMic = true;
		//BYTE pBuf[] = {0x43,0x20,0x00,0x10,0x00,0x00,0x00,0x00,0xe0,0x64,0x01,0x09,0x5a,0x01,0xff,0xff,0x00,0x00,0x01,0x00,0x1b};					   
		BYTE pBuf[] = {0x43,0x20,0x00,0x10,0x00,0x00,0x00,0x00,0xe0,0x64,0x01,0x09,0x64,0x01,0xff,0xff,0x00,0x00,0x01,0x00,0x25};
		sendtomsg(m_socket, (const char*)pBuf, 21, MSG_SEND_COUNT_RESEND2, ip, port);
	}
	m_Lock.Unlock();	
}
void CDecodeMsg::SendCloseMic(char* ip, int port)
{	
	m_Lock.Lock();
	g_bOpenMic = false;
	BYTE pBuf[] = {0x43,0x21,0x00,0x04,0x00,0x00,0x00,0x00,0x68};
	sendtomsg(m_socket, (const char*)pBuf, 9, MSG_SEND_COUNT_RESEND2, ip, port);
	m_Lock.Unlock();
}
void CDecodeMsg::SendCloseAux(char* ip, int port)
{
	m_Lock.Lock();
	g_bOpenAux = false;
	BYTE pBuf[] = {0x43,0x21,0x00,0x04,0x00,0x00,0x00,0x00,0x68};
	sendtomsg(m_socket, (const char*)pBuf, 9, MSG_SEND_COUNT_RESEND2, ip, port);
	m_Lock.Unlock();
}
void CDecodeMsg::SendTempGroup(char* ip, int port, enTempGroupID enGID)
{
	m_Lock.Lock();
	// 53:02:00:01:00:56
	BYTE pBuf[] = {0x53,0x02,0x00,0x01,0x00,0x56};
	pBuf[4] = enGID;
	pBuf[5] = GetCheckSum(pBuf, 5);
	for(int i = 0; i < MSG_SEND_COUNT_RESEND2; i++){
		sendtomsg(m_socket, (const char*)pBuf, 6, MSG_SEND_COUNT_NORMAL, ip, port);
		Sleep(10);
	}
	
	m_Lock.Unlock();
}
void CDecodeMsg::ReSend2Device(const char* pbuf, int len, int sendCount, char* ip, int port)
{
	m_Lock.Lock();
	sendtomsg(m_socket, pbuf, len, sendCount, ip, port);
	m_Lock.Unlock();
}
void CDecodeMsg::sendtomsg(SOCKET s, const char* pbuf, int len, int sendCount, char* ip, int port)
{	
	if (sendCount < 1)
	{
		sendCount = 1;
	}
	Sleep(10);	//挂起10MS
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = inet_addr(ip);
	int sinlen = sizeof(sin);

	char pDest[260] = {0};
	BYTE* pTemp = (BYTE*)pbuf;
	Byte2Hex(pTemp, pDest, len);

	for (int i = 0; i < sendCount; i++)
	{
		if (i > 0)
		{
			Sleep(100);
		}
		int ret = sendto(s, pbuf, len, 0, (sockaddr *)&sin, sinlen);	
		if (ret > 0)
		{			
			g_Log.output(LOG_TYPE_8, "send msg: %s....., ip: %s\r\n",pDest, ip); 		
		}
		else{
			g_Log.output(LOG_TYPE_8, "MSG Send fail: %s....., ip: %s￡?ErrorCode = %d\r\n", pDest, ip, WSAGetLastError()); 
		}
	}
	
}
void CDecodeMsg::SendCmd(int extAddr, enCmdCode cmd)  //发送命令
{
	if (extAddr == atoi(g_Config.m_SystemConfig.CallA) ||
		extAddr == atoi(g_Config.m_SystemConfig.CallB))
	{
		return;	  //有电话进直接跳出？！
	}
	char* ip = g_Config.m_SystemConfig.IP_Out;
	int port = g_Config.m_SystemConfig.portOut;
	char pBuf[32] = {0};
	switch (cmd)   //匹配命令
	{
	case ENUM_CMD_CODE_RING:
		sprintf(pBuf, "@_%d_03;", extAddr);
		break;
	case ENUM_CMD_CODE_CALL_START:
		sprintf(pBuf, "@_%d_01;", extAddr);
		break;
	case ENUM_CMD_CODE_CALL_STOP:
		sprintf(pBuf, "@_%d_02;", extAddr);
		break;
	case ENUM_CMD_CODE_BC_OPEN:
		sprintf(pBuf, "@_%d_05;", extAddr);
		break;
	case ENUM_CMD_CODE_BC_CLOSE:
		sprintf(pBuf, "@_%d_06;", extAddr);
		break;
	case ENUM_CMD_CODE_ERROR:
		sprintf(pBuf,	 "@_%d_04;", extAddr);
		break;
	}

	sendtomsg(m_socket, pBuf, 32, MSG_SEND_COUNT_RESEND2, ip, port);
	if (g_Config.m_SystemConfig.portOut2 > 0)
	{
		char* ip2 = g_Config.m_SystemConfig.IP_Out2;
		int port2 = g_Config.m_SystemConfig.portOut2;
		BYTE btIP1, btIP2, btIP3, btIP4;
		IP2BYTE(ip2, btIP1, btIP2, btIP3, btIP4);
		if((btIP1 >= 0 && btIP1 <= 255) && 
			(btIP2 >= 0 && btIP2 <= 255) && 
			(btIP3 >= 0 && btIP3 <= 255) && 
			(btIP4 >= 0 && btIP4 <= 255))
		{
			sendtomsg(m_socket, pBuf, 32, MSG_SEND_COUNT_NORMAL, ip2, port2);
		}		
	}
}
void CDecodeMsg::Send2OuterACK(int extNo,BYTE opt,BYTE bcAck)
{
	char* ip = g_Config.m_SystemConfig.IP_OUTER_BC_CONTROL;
	int port = g_Config.m_SystemConfig.portOuterBCControl;
	char pBufAscii[32] = {0};
	switch (opt)   //匹配命令
	{
	case 0:
		sprintf(pBufAscii, "start;%d;%s;",extNo,bcAck?"Ok":"No");
		break;
	case 1:
		sprintf(pBufAscii, "stop;%d;%s;",extNo,bcAck?"Ok":"No");
		break;
	}
	TCHAR pBufUnicode[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP,0,pBufAscii,-1,pBufUnicode,MAX_PATH);
	sendtomsg(m_socketOuter, (char*)pBufUnicode, wcslen(pBufUnicode)*sizeof(TCHAR), MSG_SEND_COUNT_RESEND2, ip, port);
}
BYTE CDecodeMsg::GetCheckSum(BYTE pBuf[], int nNum)	 //0获取校验位函数0
{
	BYTE checkSum = 0;
	for (int i = 0; i < nNum; i++)
	{
		checkSum += pBuf[i];
	}
	return checkSum;
}
char* CDecodeMsg::GetHostIP()  //获取主机IP
{
	char    HostName[100] = {0};
	gethostname(HostName, sizeof(HostName));// 获得本机主机名.

	hostent* hn;
	hn = gethostbyname(HostName);//根据本机主机名得到本机ip

	return inet_ntoa(*(struct in_addr *)hn->h_addr_list[0]);//把ip换成字符串形式
}
DWORD CDecodeMsg::GetHostAddr()	//获取主机地址（双字节型）
{
	char* pIP = GetHostIP();
	return inet_addr(pIP);
}
void CDecodeMsg::ReadIPData()
{	
	if (!m_vecIPData.empty())
	{
		m_vecIPData.clear();  // erase all
	}
	HANDLE hFile = CreateFile(g_szAPPPath + L"addr.ip", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		InitIPData();
		return;
	}
	DWORD dwSz = GetFileSize(hFile, NULL);
	int recNum = dwSz / sizeof(SIPData);	
	for (int i = 0; i < recNum; i ++)
	{
		DWORD dwRead = 0;
		SIPData data;		
		ReadFile(hFile, &data, sizeof(SIPData), &dwRead, NULL);		
		m_vecIPData.push_back(data);
	}
	if (m_vecIPData.empty())
	{
		InitIPData();
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
}
void CDecodeMsg::InitIPData()	//初始化IP数据
{
	if (!m_vecIPData.empty())  //非空则清零
	{
		m_vecIPData.clear();
	}
	for (int i = 0; i < 1; i++)
	{
		SIPData data;
		data.type = 1;	
		
		sprintf(data.ip, "192.168.100.%d", i + 1); //IP数据格式化
		m_vecIPData.push_back(data);  //----向量传入数据---
	}
}
void CDecodeMsg::SaveIPData()	//保存IP数据
{	
	if (m_vecIPData.empty())
	{
		return;
	}
	HANDLE hFile = CreateFile(g_szAPPPath + L"addr.ip", GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{		
		return;
	}		
	for (int i = 0; i < m_vecIPData.size(); i ++)
	{
		DWORD dwWrite = 0;				
		WriteFile(hFile, &(m_vecIPData[i]), sizeof(SIPData), &dwWrite, NULL);			
	}

	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}

}
SIPData* CDecodeMsg::GetIPData(int& nNum)	//获取IP nNum
{
	if (m_vecIPData.empty())
	{
		nNum = 0;
		return NULL;
	}
	nNum = m_vecIPData.size();
	vector<SIPData>::iterator vecIter = m_vecIPData.begin();//遍历
	return &(*vecIter);
}
//增删IP数据
void CDecodeMsg::AddIPData(SIPData& data)
{
	m_vecIPData.push_back(data);
}
void CDecodeMsg::DelIPData(char* ip)
{
	for (int i = 0; i < m_vecIPData.size(); i++)
	{
		if (strcmp(m_vecIPData[i].ip, ip) == 0)
		{
			m_vecIPData.erase(m_vecIPData.begin() + i);
			return;
		}
	}
}
bool CDecodeMsg::IsMainSystem()	  //CK注释，判断是否主控
{
	if (m_vecIPData.empty())	//sipcalldata数据为空
	{
		return false;
	}
	if (strcmp(GetHostIP(), m_vecIPData[0].ip) == 0)   //IP和主机相同
	{
		return true;
	}
	return false;
}
void CDecodeMsg::ModifyHead()  //修改Head
{
	if (g_Config.m_SystemConfig.nNetOF)
	{
		m_pBuffer[0] = 0x00;
		m_pBuffer[1] = 0x00;
	}
	else{
		m_pBuffer[0] = 0xA0;
		m_pBuffer[1] = 0x9F;
	}	
}
void CDecodeMsg::SendDeviceBeatReq(BYTE subTelNo,char* szIp)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
		
	s_PCSendBuf[3] = 0xB0;
	
	s_PCSendBuf[2] = subTelNo;
		
	s_PCSendBuf[8] = 0;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,szIp,port);
	}
	m_LockPC.Unlock();
}
void CDecodeMsg::DecodeDeviceBeatReq(BYTE* pBuffer,int size)
{
	SendMessage(m_pAPP->m_hWnd,RECV_MSG_DEVICE_BEAT_REQ,pBuffer[2],NULL);
}
//发送请求状态
void CDecodeMsg::SendDeviceStatusReq(SDevice& stDevice, BYTE all)
{
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, 16 *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
		
	s_PCSendBuf[3] = 0x90;
	if (all == 1)
	{
	}
	else{
		s_PCSendBuf[2] = stDevice.sDB.extAddr;
		IP2BYTE(stDevice.sDB.ip, s_PCSendBuf[4], s_PCSendBuf[5], s_PCSendBuf[6], s_PCSendBuf[7]);		
	}	
	s_PCSendBuf[8] = all;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		sendtomsg(m_socketPC,(const char*)s_PCSendBuf,USER_BUFFER_SZ,MSG_SEND_COUNT_NORMAL,stDevice.dwIP,port);
		//for (int i = 1; i < nNum; i++)
		//{
			//sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		//}
	}
	else{
		/*if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}*/
	}	
	m_LockPC.Unlock();	
}
void CDecodeMsg::SendDeviceStatus(SDevice* pDevice)
{
	if (pDevice == NULL)
	{
		return;
	}
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
		
	s_PCSendBuf[2] = pDevice->sDB.extAddr;
	s_PCSendBuf[3] = 0x91;
	IP2BYTE( pDevice->sDB.ip, s_PCSendBuf[4], s_PCSendBuf[5], s_PCSendBuf[6], s_PCSendBuf[7]);			
	s_PCSendBuf[8] = pDevice->bcStatus;
	s_PCSendBuf[9] = pDevice->callStatus;
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		/*for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		}*/
	}
	else{
		if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}
	}	
	m_LockPC.Unlock();
}
void CDecodeMsg::SendDeviceStatusAck(BYTE extAddr, BYTE IP1, BYTE IP2, BYTE IP3, BYTE IP4)
{
	
	m_LockPC.Lock();
	memset(s_PCSendBuf, 0, USER_BUFFER_SZ *sizeof(BYTE));
	s_PCSendBuf[0] = 0xA0;
	s_PCSendBuf[1] = 0x9F;
		
	s_PCSendBuf[2] = extAddr;
	s_PCSendBuf[3] = 0x92;
	s_PCSendBuf[4] = IP1, 
	s_PCSendBuf[5] = IP2, 
	s_PCSendBuf[6] = IP3, 
	s_PCSendBuf[7] = IP4;			
	
	s_PCSendBuf[15] = GetCheckNum(USER_BUFFER_SZ);
	int nNum = 0;
	SIPData* data = GetIPData(nNum);
	USHORT port = MSG_PC_PORT;
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, port);
		}
	}
	else{
		/*if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)s_PCSendBuf, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port);
		}*/
	}	
	m_LockPC.Unlock();
}
void CDecodeMsg::IP2BYTE(char* ip, BYTE& ip1, BYTE& ip2, BYTE& ip3, BYTE& ip4)
{
	DWORD dwIP = ntohl( inet_addr( ip ) );
	ip1 = ip2 = ip3 = ip4 = 0;
	ip1 = (dwIP&0xff000000)>>24;
	ip2 = (dwIP&0x00ff0000)>>16;
	ip3 = (dwIP&0x0000ff00)>>8;
	ip4 = dwIP&0x000000ff;	
}
CString CDecodeMsg::BYTE2IP(BYTE ip1,BYTE ip2,BYTE ip3,BYTE ip4)
{
	CString str;
	str.Format(L"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
	return str;
}
void CDecodeMsg::Send8Byte2Device(BYTE* pBytes, char* szIP, USHORT port)
{
	sendtomsg(m_socket, (const char*)pBytes, SEND_BUFFER_SZ, MSG_SEND_COUNT_RESEND2, szIP, port);	
}
void CDecodeMsg::Send8Byte2PC(BYTE* pBytes, char* szIP, USHORT port)
{		
	if (g_Config.m_SystemConfig.nMainSystem) // 主系统->各个分系统
	{		
		/*int nNum = 0;
		SIPData* data = GetIPData(nNum);
		BYTE pTrans[TRANS_BUFFER_SZ] = {0};
		Byte8ToTransMsg(pBytes, szIP, port, pTrans);
		for (int i = 1; i < nNum; i++)
		{
			sendtomsg(m_socketTrans, (const char*)pTrans, TRANS_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[i].ip, MSG_TRANS_PORT);
		}*/
	}else{
		int nNum = 0;
		SIPData* data = GetIPData(nNum);
		USHORT port1 = MSG_PC_PORT;
		if (nNum > 0)
		{
			sendtomsg(m_socketPC, (const char*)pBytes, USER_BUFFER_SZ, MSG_SEND_COUNT_NORMAL, data[0].ip, port1);
		}
	}
}
void CDecodeMsg::Decode_Auto_BC_Recv(BYTE* pBuffer, int size)
{
	if (pBuffer[4] == 0x01)
	{
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_AUTO_BC_OPEN_RECV,  pBuffer[2], NULL);
		SDeviceCmd* pDeviceCmd = m_pAPP->GetDevCmd(pBuffer[2]);
		if (pDeviceCmd)
		{
			Send_Auto_BC_Recv(pBuffer[2], pDeviceCmd->groupIP, g_Config.m_SystemConfig.portDevice, true);
		}
	}
	else if (pBuffer[4] == 0x00)
	{
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_AUTO_BC_CLOSE_RECV,  pBuffer[2], NULL);
		SDeviceCmd* pDeviceCmd = m_pAPP->GetDevCmd(pBuffer[2]);
		if (pDeviceCmd)
		{
			Send_Auto_BC_Recv(pBuffer[2], pDeviceCmd->groupIP, g_Config.m_SystemConfig.portDevice, false);
		}
	}
}
void CDecodeMsg::Send_Auto_BC_Recv(BYTE subTelNo, char* ip, int port, bool bOpen)
{
	m_Lock.Lock();
	m_pBuffer[0] = 0xA0;
	m_pBuffer[1] = 0x9F;
	ModifyHead();
	m_pBuffer[2] = subTelNo;
	m_pBuffer[3] = 0x09;
	if (bOpen)
	{
		m_pBuffer[4] = 0x01;
	}
	else{
		m_pBuffer[4] = 0x00;
	}
	m_pBuffer[5] = 0x00;
	m_pBuffer[6] = 0x00;
	m_pBuffer[7] = GetCheckNum();

	Send8Byte2Device(m_pBuffer, ip, port);
	m_Lock.Unlock();
}
void CDecodeMsg::Decode_Auto_BC_ReRecv(BYTE* pBuffer, int size)
{
	if (pBuffer[4] == 0x01)
	{
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_AUTO_BC_OPEN_RERECV,  pBuffer[2], NULL);
	}
	else if (pBuffer[4] == 0x00)
	{
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_AUTO_BC_CLOSE_RERECV,  pBuffer[2], NULL);
	}
}