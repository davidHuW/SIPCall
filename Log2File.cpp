#include "StdAfx.h"
#include "Log2File.h"
#include "SIPCallDlg.h"

#define  LOG_DIR (_T("LogFile"))
#define  LOG_FILE_SZ (10 * 1024* 1024)
CLog2File::CLog2File(void)
{
	CreateDirectory(LOG_DIR, NULL);
	m_nLogType = 0;
}

CLog2File::~CLog2File(void)
{
	m_nLogType = 0;
}
void CLog2File::setType(int type)
{
	m_nLogType = type;
}	
void CLog2File::output(const char* pszFormat, ...) 
{ 
	va_list args; 
	va_start(args, pszFormat); //一定要“...”之前的那个参数
	char pszDest[1024] = {0};
	_vsnprintf(pszDest, 1024, pszFormat, args); 
	va_end(args); 
	TRACE(pszDest);
	
	log2File(pszDest, strlen(pszDest));
	
}
void CLog2File::output(int type, const char* pszFormat, ...)
{
	if (type != LOG_TYPE_8)
	{
		//return;
	}
	m_Lock.Lock();
	
	m_nLogType = type;

	va_list args; 
	va_start(args, pszFormat); //一定要“...”之前的那个参数
	char pszDest[1024] = {0};
	_vsnprintf(pszDest, 1024, pszFormat, args); 
	va_end(args); 
	TRACE(pszDest);
	log2File(pszDest, strlen(pszDest));
	m_Lock.Unlock();
}
void CLog2File::log2File(char* pLog, int len)
{
	char* pFileName = getLogFileName(m_nLogType);
	//DWORD dwLogLimitSz = getLogFileLimitSz(m_nLogType);
	WCHAR pwFileName[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, pFileName, -1, pwFileName, MAX_PATH);
	WCHAR pFilePath[MAX_PATH] = {0};
	wsprintf(pFilePath, _T("%s%s\\%s"), g_szAPPPath, LOG_DIR, pwFileName);
	HANDLE hFile = CreateFile(pFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	DWORD dwFileSz = GetFileSize(hFile, NULL);
	if (dwFileSz > LOG_FILE_SZ)
	{
		CloseHandle(hFile);
		DeleteFile(pFilePath);
		hFile = CreateFile(pFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);
	// tm
	SYSTEMTIME tm;
	GetSystemTime(&tm);
	char pLogTime[64] = {0};
	sprintf(pLogTime, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ", tm.wYear, tm.wMonth, tm.wDay, tm.wHour + 8, tm.wMinute, tm.wSecond, tm.wMilliseconds);
	
	DWORD dwWrite = 0;
	WriteFile(hFile, pLogTime, strlen(pLogTime), &dwWrite, NULL);
	// log
	WriteFile(hFile, pLog, len, &dwWrite, NULL);
	CloseHandle(hFile);
}
void CLog2File::addConfig(int type, char* pFileName)
{
	SLogFileConfig config;
	memset(&config, 0, sizeof(SLogFileConfig));
	config.type = type;
	strncpy(config.FileName, pFileName, strlen(pFileName));
	m_vecConfig.push_back(config);
}
char* CLog2File::getLogFileName(int type)
{
	for (int i = 0; i < m_vecConfig.size(); i ++)
	{
		if (m_vecConfig[i].type == type)
		{
			return m_vecConfig[i].FileName;
		}
	}
	memset(m_pLogName, 0, MAX_PATH * sizeof(char));
	sprintf(m_pLogName, "log_%d.txt", type);
	return m_pLogName;
}
int CLog2File::getLogFileLimitSz(int type)
{
	for (int i = 0; i < m_vecConfig.size(); i ++)
	{
		if (m_vecConfig[i].type == type)
		{
			return m_vecConfig[i].limitSz;
		}
	}
	
	return LOG_FILE_SZ;
}
void  CLog2File::clearAll()
{
	// 查找当前路径下的所有文件夹和文件

	CString strDir = LOG_DIR;
	strDir += "\\*.txt";

	// 遍历得到所有子文件夹名
	CFileFind finder; 
	BOOL bWorking = finder.FindFile(strDir);

	while (bWorking)  
	{  
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory() && "." != finder.GetFileName() && ".." != finder.GetFileName())//注意该句需要排除“.”“..”
		{
			// 输出调试信息
			//CString strDB = finder.GetFileName();   

			//FindTxtFile(finder.GetFilePath());
		}
		else{
			CString strFile = finder.GetFilePath();   
			//char pFilePath[MAX_PATH] = {0};
			//WideCharToMultiByte(CP_ACP, 0, strFile, -1, pFilePath, MAX_PATH, NULL, NULL);

			DeleteFile(strFile);

		}
	} 

}