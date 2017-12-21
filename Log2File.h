#pragma once
#include <vector>
#include <algorithm>
using namespace std;

#include "SNLock.h"

struct SLogFileConfig{
	int type;
	char FileName[MAX_PATH];
	int limitSz;
	SLogFileConfig(){
		type = 0;
		memset(FileName, 0, MAX_PATH * sizeof(char));
		limitSz = 0;
	}
};

#define LOG_TYPE		(1000)
#define LOG_TYPE_8		(2000)
//#define LOG_TYPE_HTTP	(1001)

class CLog2File
{
public:
	CLog2File(void);
	~CLog2File(void);
	void addConfig(int type, char* pFileName);
	void setType(int type);
	void output(const char* pszFormat, ...) ;
	void output(int type, const char* pszFormat, ...);
	void clearAll();
private:
	char* getLogFileName(int type);
	int getLogFileLimitSz(int type);
	void log2File(char* pLog, int len);
private:
	int m_nLogType;
	vector<SLogFileConfig> m_vecConfig;
	char m_pLogName[MAX_PATH];
private:
	CSNLock m_Lock;
};
