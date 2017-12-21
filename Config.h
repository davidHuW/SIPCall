#pragma once

#include "RWIni.h"


//ϵͳ����
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
	char IP_Server[MAX_PATH];  // MAX_PATH==260ָ���ǰ���Ŀ¼���ļ�����ȫ·���ַ����ȡ�
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
	int nMainSystem;	  //����ϵͳȨ�޴�
	int bHttpConnect;
	int nNetOF;
	//int nVolume;
	int nRecord;//�Ƿ�¼��
	int nJT;	// �����Ƿ���
	int nPop;	// ���д�������
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


