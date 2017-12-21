#pragma once

#include "DataM.h"
#include "DecodeMsg.h"

class CLogicApp
{
public:
	CLogicApp(void);
	~CLogicApp(void);
public:
	bool m_UpdateDB;	//修改DB数据
	bool m_SendBC;		//发送广播
	

};

extern CLogicApp g_Logic;
