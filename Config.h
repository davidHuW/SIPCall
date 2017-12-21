#pragma once

#include "RWIni.h"


//系统配置
struct SSystemConfig{
	// port
	int portPC;
	int portDevice;
	int portAudio;
	int portOut;
	int portOut2;
	int portEvtListen;
	int portEvtSend;
	int portOuterBCControl;
	// IP
	char IP_Server[MAX_PATH];  // MAX_PATH==260指的是包含目录到文件名的全路径字符长度。
	char IP_MIC[MAX_PATH];
	char IP_MIC_RECORD[MAX_PATH];
	char IP_AUX[MAX_PATH];
	char IP_Monitor[MAX_PATH];
	char path[MAX_PATH];
	char pathRecord[MAX_PATH];
	char IP_Out[MAX_PATH];
	char IP_Out2[MAX_PATH];
	char CallA[MAX_PATH];
	char CallB[MAX_PATH];
	char Outer[MAX_PATH];	
	char title[MAX_PATH];	
	char signL[MAX_PATH];
	char signR[MAX_PATH];
	char IP_OUTER_BC_CONTROL[MAX_PATH];
	int nPlayTime;
	int nMainSystem;	  //主控系统权限大
	int bHttpConnect;
	int nNetOF;
	//int nVolume;
	int nRecord;//是否录音
	int nJT;	// 监听是否开启
	int nPop;	// 呼叫处理弹出框
};

#define  MAX_PLAY_TIME (5)

class CConfig
{
public:
	CConfig(void);
	~CConfig(void);
	void ReadConfig();
	void SaveConfig();
public:
	SSystemConfig m_SystemConfig;
public:
	CRWIni m_ini;
};


