#pragma once

#include "ConfigEnumDef.h"
#include "SNLock.h"
#include <vector>
#include <algorithm>
using namespace std;

struct SMsgInfo{
	DWORD dwTime;
	DWORD dwUpdateTime;
	DWORD dwIP;
	DWORD dwPort:16;	
	DWORD dwExt:8;
	DWORD dwMsgNo:8;	
	BYTE msg[SEND_BUFFER_SZ];
	SMsgInfo(){
		dwTime = 0;
		dwUpdateTime = 0;
		dwIP = 0;
		dwPort = 0;		
		dwExt = 0;
		dwMsgNo = 0;		
		memset(msg, 0, SEND_BUFFER_SZ * sizeof(BYTE));		
	}
};

class CMsgQue
{
public:
	CMsgQue(void);
	~CMsgQue(void);
	void SendMsgIn(BYTE* pMsg, char* ip, USHORT port);
	void RecvMsgOut(BYTE* pMsg);
	void RemoveMsgTimeout();
	void CreateMsgThread();
	void SendMsg();
private:
	vector<SMsgInfo> m_vecMsg;
	CSNLock m_LockMsg;
};

