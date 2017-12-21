#pragma once
#include "ConfigEnumDef.h"
#include "HttpTCP.h"



struct SEventInfo{
	enEventStatus type;
	int id;
	char extNo[12];	
	char from[12];
// 	char extNo[16];	
// 	char from[16];
	SEventInfo(){
		type = ENUM_EVENT_NONE;
		id = 0;
		memset(extNo, 0, 12 * sizeof(char));
		memset(from, 0, 12 * sizeof(char));
// 		memset(extNo, 0, 16 * sizeof(char));
// 		memset(from, 0, 16 * sizeof(char));
	};
};

struct SSocketInfo{
	SOCKET sfd;
	char ip[32];
	USHORT port;
	DWORD time;
	SSocketInfo(){
		init();
	}
	void init(){
		sfd = 0;
		port = 0;
		memset(ip, 0, 32 * sizeof(char));
		time = 0;
	}
};

class CHttpRecv
{
public:
	CHttpRecv(void);
	~CHttpRecv(void);
	bool HTTP_GET_DATA(CString strURL, CString fileaddr);
	bool HTTP_POST_DATA(CString strUrl, const char*strPara/*, CString &strContent, CString &strDescript*/);
	
	void CreateTCPRecvThread();

	int TCP_POST_DATA(/*char* pIP, USHORT port, */char* pSendBuf);

	void DecodeEvent(char* pRecvData);
	void DecodeEvent(char* pRecvData, SEventInfo& evt);
	void DecodeCdr(char* pRecvData, SEventInfo& evt);
private:
	int DecodeExt(char* pRecvData);
	enEventStatus GetEvtType(const char* pType);
};
