#pragma once
#include <vector>
#include <algorithm>
using namespace std;
#include "AudioPlay.h"


struct SPlayDevice{
	int extAddr;
	DWORD ip;
	SPlayDevice(){
		extAddr = -1;
		ip = 0;
	}
};

struct SPlayAttr{
	enPlaySrc enSrc;
	enPlayModel enModel;
	int curPlayIndex;
	char pFile[MAX_PATH];	
	SPlayAttr(){
		enSrc = ENUM_MODEL_NONE;
		enModel = ENUM_PLAY_MODEL_NONE;
		curPlayIndex = 0;
		memset(pFile, 0, MAX_PATH * sizeof(char));		
	}
};

struct SPlayDeviceAttr{
	int extAddr;
	DWORD ip;
	SPlayAttr attr;
};



struct SPlayGroup{
	vector<SPlayDevice> vecDevice;
	SPlayAttr attr;	
	_PlayParam param;
	int threadID;
	DWORD GroupID;
	DWORD playTime;
	enStopMsg msg;
	SPlayGroup(){		
		threadID = 0;
		GroupID = 0;
		msg = ENUM_STOP_MSG_NONE;
		playTime = 0;
	}
};

class CMgrPlay
{
public:
	CMgrPlay(void);
	~CMgrPlay(void);
	SPlayGroup* AddNew(SPlayGroup* pPlay, int nNum);
	int ReDividePlayGroup(vector<SPlayDeviceAttr>& vecDevice, SPlayGroup*& pPlayGroup);
	SPlayGroup* FindPlayGroup(int threadID);
	void PlayGroups(SPlayGroup* pGroup, int nNum);
	void PlayOneGroup(SPlayGroup* pGroup);
	void MsgStop(int threadID);
	void MsgStopGroup(SPlayGroup* pGroup);

	void SendStopGroup(SPlayGroup* pGroup, int nNum, enStopMsg msg);
	void SendStopGroup(SPlayGroup* pGroup, enStopMsg msg);
	
	void EndGroup(SPlayGroup* pGroup);

	void PlayGroup(vector<SPlayDeviceAttr>& vecDevice);
	void StopGroup(vector<SPlayDeviceAttr>& vecDevice, enStopMsg msg);



	bool IsExistAUX(SPlayGroup* pGroup, int nNum);
	bool IsExistMIC(SPlayGroup* pGroup, int nNum);
	void Remove(SPlayGroup* pPlay);
private:
	void PlayFile(SPlayGroup* pGroup, bool RePlay);
	void PlayAUX(SPlayGroup* pGroup);
	void PlayMIC(SPlayGroup* pGroup);
private:
	int DividePlayGroup(vector<SPlayDeviceAttr>& vecDevice, SPlayGroup*& pPlayGroup);
	void FreePlayGroup(SPlayGroup* pPlayGroup);
	void Clear(SPlayGroup* pPlayGroup);
	bool IsExistSrc(SPlayGroup* pGroup, int nNum, enPlaySrc src);
private:
	int ReDividePlayGroup(vector<SPlayDevice>& vecDevice, SPlayGroup*& pPlayGroup);
	// É¾³ýÖ®Ç°¼ÇÂ¼
	void DeleteDevice(SPlayGroup* pPlayGroup, int nNum);
	void DeleteDevice(SPlayGroup& play);
	void Delete(vector<DWORD>& vecIP);
	void Release();	
	bool IsDeviceEqual(vector<SPlayDevice>& vecNew, vector<SPlayDevice>& vecStore);
	bool IsDeviceFind(DWORD dwIP, vector<SPlayDevice>& vecPlayDevice);
	SPlayGroup* FindPlayGroup(vector<SPlayDevice>& vecPlayDevice);
	SPlayGroup* FindPlayGroup(DWORD dwIP);
	void UpdateAttr(SPlayAttr* pDest, SPlayAttr* pSrc);
	bool IsAttrEqual(const SPlayAttr& item1, const SPlayAttr& item2);
private:
	vector<SPlayGroup> m_vecPlayGroup;
	DWORD m_nGroupID;
};

