#pragma once

#include "SNLock.h"
#include "MsgDef.h"
#include "Log2File.h"
#include "configenumdef.h"
#include "DataM.h"
//#include "MsgQue.h"



struct SIPData{
	int type;
	char ip[32];
	SIPData(){
		type = 0;
		memset(ip, 0, 32 * sizeof(char));
	}
};


class CDecodeMsg
{
public:
	CDecodeMsg(void);
	~CDecodeMsg(void);
public:
	// udp send
	int CreateSocket(/*char* ip, unsigned short port*/);
	// PC之间通讯
	int CreatePCSocket();
	int CreateOuterSocket();
private:
	void CreateRecvThread();
	void CreatePCRecvThread();
	void CreateOuterRecvThread();
public:
	void SendHeartBeat(BYTE subTelNo, char* ip, int port);
	void SendOpenCall(BYTE subTelNo, char* ip, int port);
	void SendHangUp(BYTE subTelNo, char* ip, int port);
	void SendOpenBroadcast(BYTE subTelNo, char* ip, int port);
	void SendCloseBroadcast(BYTE subTelNo, char* ip, int port, bool beat);
	void SendOpenMic(char* ip, int port, bool bAux);
	void SendCloseMic(char* ip, int port);
	void SendCloseAux(char* ip, int port);
	void SendTempGroup(char* ip, int port, enTempGroupID enGID);
	void SendCallAnswer(BYTE subTelNo, char* ip, int port);
private:
	BYTE GetCheckNum(int len = SEND_BUFFER_SZ);
public:
	void DecodeMsg(BYTE* pBuffer, int size, char* szIP, USHORT port);
	void DecodeMsgError7(BYTE* pBuffer, int size, char* szIP, USHORT port);
	void DecodeMsgError9(BYTE* pBuffer, int size, char* szIP, USHORT port);
	void DecodeCall(BYTE* pBuffer, int size);
	void DecodeDoor(BYTE* pBuffer, int size);
	void DecodeDoorClose(BYTE* pBuffer, int size);
	// 通话回应
	void DecodeE1(BYTE* pBuffer, int size);
	// 挂机回应
	void DecodeE2(BYTE* pBuffer, int size);
	// 广播开回应
	void DecodeE3(BYTE* pBuffer, int size);
	// 广播关回应
	void DecodeE4(BYTE* pBuffer, int size);
	// 分控请求状态
	void DecodeDeviceStatusReq(BYTE* pBuffer, int size);
	// 分控回应状态接受
	void DecodeDeviceStatusAck(BYTE* pBuffer, int size);
	// 主控推送状态
	void DecodeDeviceStatusResponse(BYTE* pBuffer, int size);
private:
	void DecodeHeartBeat(BYTE* pBuffer, int size);
	void DecodeBC(BYTE* pBuffer, int size);
private:
	BYTE m_pBuffer[16];
public:
	void ReSend2Device(const char* pbuf, int len, int sendCount, char* ip, int port);
private:
	void sendtomsg(SOCKET s, const char* pbuf, int len, int sendCount, char* ip, int port);
public:
	void SendCmd(int extAddr, enCmdCode cmd);
	BYTE GetCheckSum(BYTE pBuf[], int nNum);
public:
	int m_socket;
	int m_socketPC;
	int m_socketOuter;
private:
	CSNLock m_Lock;
	CSNLock m_LockPC;
	CSNLock m_LockOuter;
public:
	char* GetHostIP();
	DWORD GetHostAddr();

	void ReadIPData();
	void SaveIPData();
	void AddIPData(SIPData& data);
	void DelIPData(char* ip);
	SIPData* GetIPData(int& nNum);
	bool IsMainSystem();
public:
	void DecodeUserMsg(BYTE* pBuffer, int size);
	// 发送PC心跳
	void SendPCBeat();
	// PC心跳回应
	void SendPCBeatReply(char* pIP);
public:
	// 发送
	void SendDeviceBeat(BYTE subTelNo);
	void SendDeviceBeatReply(BYTE subTelNo, enDeviceStatus bc,  enDeviceStatus call);
	void SendDeviceUpdateStatus(BYTE subTelNo, enDeviceStatus bc);
private:
	void InitIPData();
private:
	vector<SIPData> m_vecIPData; //定义SIPData结构类型的向量！！！！
private:
	void ModifyHead();

public:
	void SendDeviceStatusReq(SDevice& stDevice, BYTE all = 0);
	void SendDeviceStatus(SDevice* pDevice);
	void SendDeviceStatusAck(BYTE extAddr, BYTE IP1, BYTE IP2, BYTE IP3, BYTE IP4);
	void IP2BYTE(char* ip, BYTE& ip1, BYTE& ip2, BYTE& ip3, BYTE& ip4);
	CString BYTE2IP(BYTE ip1,BYTE ip2,BYTE ip3,BYTE ip4);
public:
	void Send8Byte2Device(BYTE* pBytes, char* szIP, USHORT port);
	void Send8Byte2PC(BYTE* pBytes, char* szIP, USHORT port);
private:
	//上位机软件接收指令后自动开/关广播
public:
	void Decode_Auto_BC_Recv(BYTE* pBuffer, int size);
	void Send_Auto_BC_Recv(BYTE subTelNo, char* ip, int port, bool bOpen);
	void Decode_Auto_BC_ReRecv(BYTE* pBuffer, int size);
public:
	void SendCallReq2Branch(BYTE subTelNo,char* szIp,BYTE callOpt);
	void DecodeCallReqFromMaster(BYTE* pBuffer,int size);
	void SendCallResponse2Master(BYTE subTelNo);
	void DecodeCallResponseFromMaster(BYTE* pBuffer,int size);
	void SendBroadcastReq2Branch(BYTE subTelNo,char* szIp,BYTE broadcastOpt);
	void DecodeBroadcastReqFromMaster(BYTE* pBuffer,int size);

	void SendDeviceCallStatus2Master(BYTE subTelNo,BYTE status);
	void DecodeDeviceCallStatusFromBranch(BYTE* pBuffer,int size);
	
	void SendDeviceBroadcastStatus2Master(BYTE subTelNo,char* szIp,BYTE status);
	void DecodeDeviceBroadcastStatusFromBranch(BYTE* pBuffer,int size);

	void SendBranchOMDown2Master(BYTE status);
	void DecodeBrachOMDown(BYTE* pBuffer,int size);


	void SendDeviceBeatReq(BYTE subTelNo,char* szIp);
	void DecodeDeviceBeatReq(BYTE* pBuffer,int size);

public:
	void DecodeOuterMsg(BYTE* pBuffer,int size);
	void DecodeOuterBCControl(const CString& cmd,const CString& path,const vector<int>& ets);
	void DecodeOuterBCMusicPath();
	void Send2OuterACK(int extNo,BYTE opt,BYTE bcAck);
};


