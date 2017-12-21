#include "StdAfx.h"
#include "MsgQue.h"
#include "SIPCallDlg.h"

#define MSG_QUE_NUM			(1000000)
#define MSG_QUE_TIMEOUT		(10*1000)

#define MSG_NO_NONE			(0)
#define MSG_NO_CALL_OPEN	(1)
#define MSG_NO_CALL_CLOSE	(2)
#define MSG_NO_BC_OPEN		(3)
#define MSG_NO_BC_CLOSE		(4)
#define MSG_NO_BC_BEAT		(5)

static bool s_Exit = false;

CMsgQue::CMsgQue(void)
{
	m_vecMsg.reserve(MSG_QUE_NUM);
}


CMsgQue::~CMsgQue(void)
{
	s_Exit = true;
	Sleep(1000);
	if (!m_vecMsg.empty())
	{
		m_vecMsg.clear();
	}
}
void CMsgQue::SendMsgIn(BYTE* pMsg, char* ip, USHORT port)
{
	BYTE msgNo = MSG_NO_NONE;
	if (pMsg[3] == 0x01)
	{
		msgNo = MSG_NO_CALL_OPEN;
	}
	else if (pMsg[3] == 0x02)
	{
		msgNo = MSG_NO_CALL_CLOSE;
	}	
	else if ((pMsg[3] == 0x05) && (pMsg[4] == 0x01))
	{
		msgNo = MSG_NO_BC_OPEN;
	}
	else if ((pMsg[3] == 0x05) && (pMsg[4] == 0x00))
	{
		msgNo = MSG_NO_BC_CLOSE;
	}
	else if (pMsg[3] == 0x33)
	{
		msgNo = MSG_NO_BC_BEAT;
	}
	// 
	if (msgNo == MSG_NO_NONE)
	{
		return;
	}
	// 加入监管的消息队列
	m_LockMsg.Lock();
	if (m_vecMsg.empty())
	{
		SMsgInfo msg;
		msg.dwTime = msg.dwUpdateTime = GetTickCount();
		msg.dwIP = inet_addr(ip);
		msg.dwPort = port;
		memcpy(msg.msg, pMsg, SEND_BUFFER_SZ * sizeof(BYTE));
		msg.dwExt = pMsg[2];
		msg.dwMsgNo = msgNo;
		m_vecMsg.push_back(msg);
	}
	else{
		for (int i = 0; i < m_vecMsg.size(); i++){
			if ((m_vecMsg[i].dwMsgNo == msgNo) &&
				(m_vecMsg[i].dwExt == pMsg[2]))
			{
				// 已经存在，无须记录
				break;
			}
			else{
				SMsgInfo msg;
				msg.dwTime = msg.dwUpdateTime = GetTickCount();
				msg.dwIP = inet_addr(ip);
				msg.dwPort = port;
				memcpy(msg.msg, pMsg, SEND_BUFFER_SZ * sizeof(BYTE));
				msg.dwExt = pMsg[2];
				msg.dwMsgNo = msgNo;
				m_vecMsg.push_back(msg);
			}
		}
	}
	
	
	
	m_LockMsg.Unlock();
}
void CMsgQue::RecvMsgOut(BYTE* pMsg)
{
	BYTE msgNo = MSG_NO_NONE;
	if (pMsg[3] == 0xE1)
	{
		msgNo = MSG_NO_CALL_OPEN;
	}
	else if(pMsg[3] == 0xE2){
		msgNo = MSG_NO_CALL_CLOSE;
	}
	else if(pMsg[3] == 0xE3){
		msgNo = MSG_NO_BC_OPEN;
	}
	else if(pMsg[3] == 0xE4){
		msgNo = MSG_NO_BC_CLOSE;
	}
	else if (pMsg[3] == 0x43)
	{
		msgNo = MSG_NO_BC_BEAT;
	}
	// 过滤消息
	if (msgNo == MSG_NO_NONE)
	{
		return;
	}
	// 移除消息
	m_LockMsg.Lock();
	for (int i = 0; i < m_vecMsg.size(); )
	{
		if ((m_vecMsg[i].dwMsgNo == msgNo) &&
			(m_vecMsg[i].dwExt == pMsg[2]))
		{
			m_vecMsg.erase(m_vecMsg.begin() + i);
		}
		else{
			i++;
		}
	}
	m_LockMsg.Unlock();
}
void CMsgQue::RemoveMsgTimeout()
{
	// 移除过时消息
	m_LockMsg.Lock();
	for(int i = 0; i < m_vecMsg.size(); ){
		DWORD dwTime = GetTickCount();
		if ((dwTime - m_vecMsg[i].dwTime) > MSG_QUE_TIMEOUT)
		{
			m_vecMsg.erase(m_vecMsg.begin() + i);
		}
		else{
			i++;
		}
	}
	m_LockMsg.Unlock();
}

UINT RunMsgProc(LPVOID pParam)  
{  
	
	CMsgQue* pMsgQue = (CMsgQue*)pParam;
	if (pMsgQue == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
	while(!s_Exit){
		// 移除过时消息
		pMsgQue->RemoveMsgTimeout();
		// 发送数据
		pMsgQue->SendMsg();
		Sleep(1);
	}
	return 1;
}
void CMsgQue::CreateMsgThread()
{
	AfxBeginThread(RunMsgProc, this);
}
void CMsgQue::SendMsg()
{	
	m_LockMsg.Lock();
	vector<SMsgInfo> vecTemp;
	vecTemp.reserve(m_vecMsg.size() + 1);
	for (int i = 0; i < m_vecMsg.size(); i++)
	{
		vecTemp.push_back(m_vecMsg[i]);
	}
	m_LockMsg.Unlock();
	for (int i = 0; i < vecTemp.size(); i++)
	{
		if (s_Exit)
		{
			break;
		}
		
		DWORD dwTime = GetTickCount();
		if ((vecTemp[i].dwUpdateTime - dwTime) >= 1000) // 超过1s才能发送
		{
			in_addr inaddr;
			inaddr.s_addr = vecTemp[i].dwIP;
			char* szIP = inet_ntoa(inaddr);

			g_MSG.ReSend2Device((const char*)vecTemp[i].msg, SEND_BUFFER_SZ, 1, szIP, vecTemp[i].dwPort);
		}
		
	}
	if (!vecTemp.empty())
	{
		vecTemp.clear();
	}
}