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
	int tempTID;			// 隧道ID，只在整理数据时候有用
	int id;
	char Name[256];			// 设备名称
	int flag;	
	char ip[256];			// 广播IP
	char ip_cam[256];		// 摄像头IP
	int extAddr;
	char extNo[256];
	char tunnelName[256];	// 隧道名称
	char kmID[256];
	char desc[128];
	char controllerip[256];     //对应控制器PC IP
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
//CK注释，定义Sdevice结构体
struct SDevice{
	SDBInfo sDB;
	enDeviceStatus bcStatus;		// 广播状态
	//enDeviceStatus status;		// display	
	//enDeviceStatus statusAfter;	// 后台状态, 用于检测心跳包
	int Check;						// 是否选中
	enDeviceStatus callStatus;	

	int doorRTFlag;
	DWORD doorRTTime;				// 实时门口故障	
	enErrorCode errorCode;			// 巡检错误
	
// add 3.7
	DWORD playTime;					// 播放启动时间
	enPlaySrc playSrc;				// 播放来源
	enPlayModel playMode;			// 播放方式
	int curPlayIdx;					// 当前播放（只针对列表）
	enStopMsg enStopFlag;			// 停止标志（0：自动，1：时间到期，2：手动结束，3：手动切换）
	//vector<SPlayList> vecPlayPath;// 播放列表
	char playFile[MAX_PATH];		// 播放单文件
	_PlayParam playParam;			// 播放参数
	int threadID;					// 播放线程ID
	bool playFlag;					// 是否播放
//
	char dwIP[256];						// IP地址
	
	BYTE bOutTunnel:1;					// 隧道外

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
	//CK注释，定义Sdevice结构体	---end

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
//CK注释，struct隧道 
struct STunnel{
	int TunnelID;	//隧道ID
	char Name[256];
	CString SignL;
	CString SignR;
	vector<SDevice> vecDeviceUp;   //上面的设备（上全选）,不确定长度的一维数组，用向量表示
	vector<SDevice> vecDeviceDown; //下面的设备（下全选）
	  //结构体构造函数用于初始化，类比于CLASS
	STunnel(){
		TunnelID = 0;
		memset(Name, 0, 256);
		SignL.Empty();
		SignR.Empty();
	}
};



struct SRecord{
	TCHAR sip[64];		// SIP号
	int addr;			// 分机地址
	TCHAR IP[32];		// IP
	TCHAR desc[32];		// 接警电话[呼叫记录]/广播类型[广播记录]/状态[巡检记录]
	TCHAR tunnel[64];	// 隧道
	TCHAR noid[64];		// 桩号
	TCHAR kmid[64];		// 公里号
	TCHAR data[64];		// 日期
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

	vector<SRecord> m_vecRecCall;//呼叫记录
	vector<SRecord> m_vecRecMsg; //广播记录
	vector<SRecord> m_vecRecLog; //巡检记录

// 增加播放列表
public:
	int AddSong(char* szName, char* szPath);
	SListSong* GetSongList(int& nNum);
	void ClearSongList();
private:
	vector<SListSong> m_vecListSong; // 歌曲列表
	int m_Songid;
	vector<SListSong> m_vecPlayList; // 播放列表
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
	// 增加声音设置
	void AddYing();
	
};


extern void TcharToChar (const TCHAR * tchar, char * _char);
extern void CharToTchar (const char * _char, TCHAR * tchar);  

extern enAdminType g_dataType;