#pragma once

#include "ConfigEnumDef.h"




struct SNoIPFileData{
	char ip[32];
	SNoIPFileData(){
		memset(ip, 0, 32 * sizeof(char));
	}
};

struct SKeyValue{
	int key;
	int value;
};

struct SBeatData{
	int extAddr;	
	DWORD dwTime;
	bool bReply;	
	char ip[32];
	SBeatData(){
		extAddr = 0;		
		dwTime = 0;
		bReply = false;	
		memset(ip, 0, 32 * sizeof(char));
	}
};

struct SBeatReply{
	int extAddr;
	enDeviceStatus bc;
	enDeviceStatus call;
	SBeatReply(){
		init();
	}
	void init(){
		extAddr = 0;
		bc = ENUM_DEVICE_STATUS_UNKNOWN;
		call = ENUM_DEVICE_STATUS_UNKNOWN;
	}
};

