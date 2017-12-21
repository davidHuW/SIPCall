#pragma once


#include <vector>
#include <algorithm>
using namespace std;

#include "SQLite.h"
#include "CDataStruct.h"
#include "AudioPlay.h"
#include "SNLock.h"

#define NAME_NO_VISIBLE	("AB000")

struct SDBInfo{
	int deleteFlag;
	int tempTID;			// ���ID��ֻ����������ʱ������
	int id;
	char Name[256];			// �豸����
	int flag;	
	char ip[256];			// �㲥IP
	char ip_cam[256];		// ����ͷIP
	int extAddr;
	char extNo[256];
	char tunnelName[256];	// �������
	char kmID[256];
	char desc[128];
	char controllerip[256];     //��Ӧ������PC IP
	SDBInfo(){
		deleteFlag = 0;
		id = 0xFFFFFF;
		flag = 0;
		memset(Name, 0, 256);
		memset(ip, 0, 256);
		memset(ip_cam, 0, 256);
		extAddr = -1;
		memset(extNo, 0, 256);
		memset(tunnelName, 0, 256);
		memset(kmID, 0, 256);
		memset(desc, 0, 128);
		memset(controllerip,0,256);
	}
	bool IsVisible(){
		if (strncmp(Name, NAME_NO_VISIBLE, strlen(NAME_NO_VISIBLE)) == 0)
		{
			return false;
		}
		return true;
	}
};
struct STempTID{
	int tid;
	char tunnelName[256];
};

struct SPlayList{
	char song[MAX_PATH];
	SPlayList(){
		memset(song, 0, MAX_PATH * sizeof(char));
	}
};
struct SPlayConfig{
	int extAddr; 
	enPlaySrc playSrc;
	enPlayModel playMode;
	int curPlayIdx;
	char playFile[MAX_PATH];
	void init(){
		extAddr = 0;
		playSrc = ENUM_MODEL_NONE;
		playMode = ENUM_PLAY_MODEL_NONE;
		curPlayIdx = 0;
		memset(playFile, 0, MAX_PATH * sizeof(char));
	}
	SPlayConfig(){
		init();
	}
};

struct SFire{
	int groupID;
	int extAddr;
	char groupIP[32];
	char file[MAX_PATH];	
};

struct SDeviceCmd{
	int groupID;
	char groupIP[32];
	char file[MAX_PATH];
	vector<int> vecDevice;
};
//CKע�ͣ�����Sdevice�ṹ��
struct SDevice{
	SDBInfo sDB;
	enDeviceStatus bcStatus;		// �㲥״̬
	//enDeviceStatus status;		// display	
	//enDeviceStatus statusAfter;	// ��̨״̬, ���ڼ��������
	int Check;						// �Ƿ�ѡ��
	enDeviceStatus callStatus;	

	int doorRTFlag;
	DWORD doorRTTime;				// ʵʱ�ſڹ���	
	enErrorCode errorCode;			// Ѳ�����
	
// add 3.7
	DWORD playTime;					// ��������ʱ��
	enPlaySrc playSrc;				// ������Դ
	enPlayModel playMode;			// ���ŷ�ʽ
	int curPlayIdx;					// ��ǰ���ţ�ֻ����б�
	enStopMsg enStopFlag;			// ֹͣ��־��0���Զ���1��ʱ�䵽�ڣ�2���ֶ�������3���ֶ��л���
	//vector<SPlayList> vecPlayPath;// �����б�
	char playFile[MAX_PATH];		// ���ŵ��ļ�
	_PlayParam playParam;			// ���Ų���
	int threadID;					// �����߳�ID
	bool playFlag;					// �Ƿ񲥷�
//
	char dwIP[256];						// IP��ַ
	
	BYTE bOutTunnel:1;					// �����

	// add send
	BYTE cmd_open_bc_send:1;
	BYTE cmd_close_bc_send:1;
	BYTE cmd_open_call_send:1;
	BYTE cmd_close_call_send:1;

	// add recv
	BYTE cmd_open_bc_recv:1;
	BYTE cmd_close_bc_recv:1;
	BYTE cmd_open_call_recv:1;
	BYTE cmd_close_call_recv:1;

	SDevice(){
		init();
	}
	void init(){
		memset(&sDB, 0, sizeof(SDBInfo));
		//status = ENUM_DEVICE_STATUS_UNKNOWN;
		//statusAfter = ENUM_DEVICE_STATUS_UNKNOWN;
		callStatus = ENUM_DEVICE_STATUS_UNKNOWN;
		bcStatus = ENUM_DEVICE_STATUS_UNKNOWN;
		Check = 0;
		errorCode = ENUM_ERROR_STATUS_NONE;
		doorRTFlag = doorRTTime = 0;
		// add3.7
		playTime = 0;
		playSrc = ENUM_MODEL_NONE;
		playMode = ENUM_PLAY_MODEL_NONE;	
		memset(playFile, 0, MAX_PATH * sizeof(char));
		playFlag = false;
		curPlayIdx = 0;
		threadID = 0;
		enStopFlag = ENUM_STOP_MSG_NONE;
		/*dwIP = 0;*/
		memset(dwIP,0,256);
		cmd_open_bc_send = 0;
		cmd_close_bc_send = 0;
		cmd_open_call_send = 0;
		cmd_close_call_send = 0;

		
		cmd_open_bc_recv = 0;
		cmd_close_bc_recv = 0;
		cmd_open_call_recv = 0;
		cmd_close_call_recv = 0;
		bOutTunnel = 0;
	}
	//CKע�ͣ�����Sdevice�ṹ��	---end

	bool IsOffline(){
		if (bcStatus == ENUM_DEVICE_STATUS_UNKNOWN)
		{
			return true;
		}
		return false;
	}
	enBCOptStatus IsBcPlay(){
		if (bcStatus == ENUM_DEVICE_STATUS_RUN)
		{
			if (playFlag)
			{
				return ENUM_BC_OPT_LOCAL;
			}
			else{
				return ENUM_BC_OPT_REMOTE;
			}			
		}
		return ENUM_BC_OPT_NONE;
		//return playFlag;		
	}
	bool IsCheck(){
		if (Check)
		{
			return true;
		}
		return false;
	}
	bool IsConfig(){
		if (playSrc == ENUM_MODEL_NONE || 
			playMode == ENUM_PLAY_MODEL_NONE ||
			strlen(playFile) == 0)
		{
			return false;
		}
		return true;
	}
};
//CKע�ͣ�struct��� 
struct STunnel{
	int TunnelID;	//���ID
	char Name[256];
	CString SignL;
	CString SignR;
	vector<SDevice> vecDeviceUp;   //������豸����ȫѡ��,��ȷ�����ȵ�һά���飬��������ʾ
	vector<SDevice> vecDeviceDown; //������豸����ȫѡ��
	  //�ṹ�幹�캯�����ڳ�ʼ���������CLASS
	STunnel(){
		TunnelID = 0;
		memset(Name, 0, 256);
		SignL.Empty();
		SignR.Empty();
	}
};



struct SRecord{
	TCHAR sip[64];		// SIP��
	int addr;			// �ֻ���ַ
	TCHAR IP[32];		// IP
	TCHAR desc[32];		// �Ӿ��绰[���м�¼]/�㲥����[�㲥��¼]/״̬[Ѳ���¼]
	TCHAR tunnel[64];	// ���
	TCHAR noid[64];		// ׮��
	TCHAR kmid[64];		// �����
	TCHAR data[64];		// ����
	SRecord(){
		memset(sip, 0, 64);
		addr = 0;
		memset(IP, 0, 32);
		memset(desc, 0, 32);
		memset(tunnel, 0, 64);
		memset(noid, 0, 64);
		memset(kmid, 0, 64);
		memset(data, 0, 64);
		SYSTEMTIME tm;
		GetLocalTime(&tm);		
		wsprintf(data, _T("%04d-%02d-%02d %02d:%02d:%02d"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour,tm.wMinute,tm.wSecond);
		
	};
};

enum enDataSortType{
	ENUM_SORT_ALL,
	ENUM_SORT_HOUR,
	ENUM_SORT_DAY,
	ENUM_SORT_WEEK,
	ENUM_SORT_MONTH,
};

struct SListSong 
{
	int id;
	char szName[MAX_PATH];
	char szPath[MAX_PATH];
	SListSong(){
		id = -1;
		memset(szPath, 0, MAX_PATH * sizeof(char));
		memset(szName, 0, MAX_PATH * sizeof(char));
	}
	
};

struct SDeviceBeatInfo{
	int extAddr;	
	bool bReply;
	DWORD dwIP;
	enBCOptStatus opt;
	enDeviceStatus bc;	
	SDeviceBeatInfo(){
		init();
	}
	void init(){
		extAddr = 0;
		bReply = false;
		dwIP = 0;	
		opt = ENUM_BC_OPT_NONE;
		bc = ENUM_DEVICE_STATUS_UNKNOWN;
	}
};

class CDataM
{
public:
	CDataM(void);
	~CDataM(void);
	bool ReadDB();
	SDBInfo* GetDB(int& nNum);
	void AddNullDB(int id);
	void AddOneDB(SDBInfo& info);
	void DeleteOneDB(int id);
	void SaveDB();
	void ClearDB();
	void DB2Data();	
	STunnel* GetData(int& nNum);
public:
	bool ReadCall(enDataSortType);
	bool ReadMsg(enDataSortType);
	bool ReadLog(enDataSortType);

	SRecord* GetRecordCall(int& nNum);
	SRecord* GetRecordMsg(int& nNum);
	SRecord* GetRecordLog(int& nNum);
public:
	bool InsertCall(SRecord& rec);
	bool InsertMsg(SRecord& rec);
	bool InsertLog(SRecord& rec);
	bool InsertLogs(vector<SRecord>& vecRecord);
	bool deleteDB();
	bool deleteCall();
	bool deleteMsg();
	bool deleteLog();

private:
	void clear();
	CString getDBPath();
private:
	vector<STunnel> m_vecTunnel;
	vector<SDevice> m_vecDevice;
	vector<SDBInfo> m_vecDB;
	SQLite m_sqlite;
	//CString m_strDBPath;

	vector<SRecord> m_vecRecCall;//���м�¼
	vector<SRecord> m_vecRecMsg; //�㲥��¼
	vector<SRecord> m_vecRecLog; //Ѳ���¼

// ���Ӳ����б�
public:
	int AddSong(char* szName, char* szPath);
	SListSong* GetSongList(int& nNum);
	void ClearSongList();
private:
	vector<SListSong> m_vecListSong; // �����б�
	int m_Songid;
	vector<SListSong> m_vecPlayList; // �����б�
private:
	void SetTempTID();
	bool IsTNameExist(char* TName);
	int GetTempID(char* TName);
private:
	vector<STempTID> m_vecTempT;
	int m_nTempTID;

public: 
	int GetPlayListSz();
	char* GetPlayFile(int idx);
	void ClearPlayList();
	void AddPlayList(SListSong& song);
	void DelPlayList(int idx);
	SListSong* GetPlayList(int& nNum);
public:
	SPlayConfig* GetAllPlayConfig(int& nNum);
	void DeletePlayConfig(int extAddr);
	void ClearPlayConfig();
	void AddPlayConfig(SPlayConfig& config);
	SPlayConfig* GetPlayConfig(int extAddr);
	void Config2Device(SDevice*& pDevice);
private:
	vector<SPlayConfig> m_vecPlayConfig;
public:
	void SelectDeviceBeat();
	void UpdateReplay(int extAddr, SBeatReply* pBeatReply);
	SDeviceBeatInfo* GetDeviceBeatInfo(int& nNum);
	void SendDeviceBeat();
private:
	vector<SDeviceBeatInfo> m_vecDeviceBeat;
	CSNLock m_LockBeat;
private:
	void AddTunnelOutFlag();
	// ������������
	void AddYing();
	
};


extern void TcharToChar (const TCHAR * tchar, char * _char);
extern void CharToTchar (const char * _char, TCHAR * tchar);  

extern enAdminType g_dataType;