#pragma once

#include "DataM.h"
#include "DecodeMsg.h"

class CLogicApp
{
public:
	CLogicApp(void);
	~CLogicApp(void);
public:
	bool m_UpdateDB;	//�޸�DB����
	bool m_SendBC;		//���͹㲥
	

};

extern CLogicApp g_Logic;
