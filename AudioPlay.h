#pragma once

#include <vector>
using namespace std;
#include "LCAudioThrDll.h"
#pragma comment(lib,"LCAudioThrDll.lib")   

#include "ConfigEnumDef.h"
#include "SoundBase.h"
#include "math.h"
#include "DataM.h"

enum enBroadcastType{
	ENUM_BROAD_SINGLE = 0,
	ENUM_BROAD_GROUP,
	ENUM_BROAD_ALL,
};



class CAudioPlay
{
public:
	CAudioPlay(void);
	~CAudioPlay(void);
	int play(char* filePath, _PlayParam& mParam, char* ip);
	int play(char* filePath, _PlayParam& mParam, vector<DWORD>& vecIPS);
	int stop(_PlayParam& mParam);
	int status(_PlayParam& mParam);
	int pausePlay(_PlayParam& mParam);
	int continuePlay(_PlayParam& mParam);
private:
	void SetModel(enPlaySrc enModel);
	void SetBroadType(enBroadcastType enType);
	void SetIP(char* IP);
	void SetGroupID(int groupID);
public:
	void initparm(_PlayParam& mParam);
	//void pushParms();

private:
	//_PlayParam m_Param;
	enPlaySrc m_enModel;
	enBroadcastType m_enBroadcastType;
	ULONG m_ipAddr;
	INT m_GroupID;//Ä¿±ê×éºÅ
// private:
// 	vector<_PlayParam> m_vecParams;
public:
	void RecordInit();
	void RecordStart();
	void RecordStop();
private:
	CSoundBase * m_pPlayer;


	
};

