#include "StdAfx.h"
#include "RWIni.h"


CRWIni::CRWIni(void)
{
	m_strIniPath = L"";

	// 得到exe执行路径.  
	TCHAR tcExePath[MAX_PATH] = {0};  
	::GetModuleFileName(NULL, tcExePath, MAX_PATH);  
	// 设置ini路径到exe同一目录下  
#ifndef CONFIG_FILE  
#define CONFIG_FILE     (TEXT("Config.ini"))  
#endif  
	//_tcsrchr() 反向搜索获得最后一个'\\'的位置，并返回该位置的指针  
	TCHAR *pFind = _tcsrchr(tcExePath, '\\');  
	if (pFind == NULL)  
	{  
		return;  
	}  
	*pFind = '\0';  

	CString szIniPath = tcExePath;  
	szIniPath += "\\";  
	szIniPath += CONFIG_FILE;  

	m_strIniPath = szIniPath;

}


CRWIni::~CRWIni(void)
{
	m_strIniPath = L"";
} 
void CRWIni::SetPort(int value)
{
	WriteInter(L"Port", L"com_port", value);
}
int CRWIni::GetPort()
{
	return ReadInter(L"Port", L"com_port");
}
void CRWIni::SetAudioPort(int value)
{
	WriteInter(L"Port", L"audio_port", value);
}
int CRWIni::GetAudioPort()
{
	return ReadInter(L"Port", L"audio_port");
}
void CRWIni::SetPCPort(int value)
{
	WriteInter(L"Port", L"pc_port", value);
}
int CRWIni::GetPCPort()
{
	return ReadInter(L"Port", L"pc_port");
}
void CRWIni::SetOutPort(int value)
{
	WriteInter(L"Port", L"out_port", value);
}
int CRWIni::GetOutPort()
{
	return ReadInter(L"Port", L"out_port");
}
void CRWIni::SetOutPort2(int value)
{
	WriteInter(L"Port", L"out_port2", value);
}
int CRWIni::GetOutPort2()
{
	return ReadInter(L"Port", L"out_port2");
}
void CRWIni::SetEvtPort(int value)
{
	WriteInter(L"Port", L"evt_port", value);
}
int CRWIni::GetEvtPort()
{
	return ReadInter(L"Port", L"evt_port");
}
void CRWIni::SetEvtSendPort(int value)
{
	WriteInter(L"Port", L"evt_port_send", value);
}
int CRWIni::GetEvtSendPort()
{
	return ReadInter(L"Port", L"evt_port_send");
}
void CRWIni::SetOuterBCControlPort(int value)
{
	WriteInter(L"Port",L"outer_bc_control_port",value);
}

int CRWIni::GetOuterBCControlPort()
{
	return ReadInter(L"Port",L"outer_bc_control_port");
}
void CRWIni::SetRecord(int value)
{
	WriteInter(L"Params", L"record_src", value);
}
int CRWIni::GetRecord()
{
	return ReadInter(L"Params", L"record_src");	
}
void CRWIni::SetJT(int value)
{
	WriteInter(L"Params", L"monitor_jt", value);
}
int CRWIni::GetJT()
{
	return ReadInter(L"Params", L"monitor_jt");
}

void CRWIni::SetPop(int value)
{
	WriteInter(L"Params", L"pop_call", value);
}
int CRWIni::GetPop()
{
	return ReadInter(L"Params", L"pop_call");
}
void CRWIni::SetSocketConnect(int value)
{
	WriteInter(L"Params", L"http_connect", value);
}
int CRWIni::GetSocketConnect()
{
	return ReadInter(L"Params", L"http_connect");
}
void CRWIni::SetPlayTime(int value)
{
	WriteInter(L"Time", L"play_time", value);
}
int CRWIni::GetPlayTime()
{
	return ReadInter(L"Time", L"play_time");
}

void CRWIni::SetPath(CString strPath)
{
	::WritePrivateProfileString(L"Path", L"path", strPath, m_strIniPath); 
}
CString CRWIni::GetPath()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"Path", L"path", NULL, szKeyValue, MAX_PATH, m_strIniPath);
// 	CString strPath = szKeyValue;
// 	return strPath;
	m_strPath = szKeyValue;
	return  m_strPath;
}


void CRWIni::SetPathRecord(CString strPath)
{
	::WritePrivateProfileString(L"Path", L"record", strPath, m_strIniPath); 
}
CString CRWIni::GetPathRecord()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"Path", L"record", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	// 	CString strPath = szKeyValue;
	// 	return strPath;
	m_strPathRecord = szKeyValue;
	return  m_strPathRecord;
}

void CRWIni::SetIP_Server(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"server_ip", strValue, m_strIniPath); 
}
CString CRWIni::GetIP_Server()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"server_ip", NULL, szKeyValue, MAX_PATH, m_strIniPath);
// 	CString strPath = szKeyValue;
// 	return strPath;
	m_strIP_SERVER = szKeyValue;
	return m_strIP_SERVER;
}

void CRWIni::SetIP_MIC_Reocrd(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"micreocrd_ip", strValue, m_strIniPath); 
}
CString CRWIni::GetIP_MIC_Reocrd()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"micreocrd_ip", L"192.168.1.7", szKeyValue, MAX_PATH, m_strIniPath);
	m_strIP_MICRecord = szKeyValue;
	return m_strIP_MICRecord;
}

void CRWIni::SetIP_AUX(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"aux_ip", strValue, m_strIniPath); 
}
CString CRWIni::GetIP_AUX()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"aux_ip", L"192.168.1.6", szKeyValue, MAX_PATH, m_strIniPath);
	m_strIP_AUX = szKeyValue;
	return m_strIP_AUX;
}
void CRWIni::SetIP_Monitor(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"monitor_ip", strValue, m_strIniPath); 
}
CString CRWIni::GetIP_Monitor()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"monitor_ip", L"192.168.1.8", szKeyValue, MAX_PATH, m_strIniPath);
	m_strIP_Monitor = szKeyValue;
	return m_strIP_Monitor;
}

void CRWIni::SetIP_Out(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"out_ip", strValue, m_strIniPath);
}
CString CRWIni::GetIP_Out()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"out_ip", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	// 	CString strPath = szKeyValue;
	// 	return strPath;
	m_strIP_Out = szKeyValue;
	return m_strIP_Out;
}


void CRWIni::SetIP_Out2(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"out_ip2", strValue, m_strIniPath);
}
CString CRWIni::GetIP_Out2()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"out_ip2", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	// 	CString strPath = szKeyValue;
	// 	return strPath;
	m_strIP_Out2 = szKeyValue;
	return m_strIP_Out2;
}
void CRWIni::SetIP_OUT_BC_CONTROL(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"outer_bc_control_ip", strValue, m_strIniPath);
}
CString CRWIni::GetIP_OUT_BC_CONTROL()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"outer_bc_control_ip", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	// 	CString strPath = szKeyValue;
	// 	return strPath;
	m_strIP_Outer_BC_Control = szKeyValue;
	return m_strIP_Outer_BC_Control;
}
void CRWIni::SetIP_MIC(CString strValue)
{
	::WritePrivateProfileString(L"IP", L"mic_ip", strValue, m_strIniPath);
}
CString CRWIni::GetIP_MIC()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"IP", L"mic_ip", NULL, szKeyValue, MAX_PATH, m_strIniPath);
// 	CString strPath = szKeyValue;
// 	return strPath;
	m_strIP_MIC = szKeyValue;
	return m_strIP_MIC;
}

void CRWIni::SetCallA(CString strValue)
{
	::WritePrivateProfileString(L"Call", L"call_a", strValue, m_strIniPath);
}
CString CRWIni::GetCallA()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"Call", L"call_a", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	m_strCallA = szKeyValue;
	return m_strCallA;
}

void CRWIni::SetCallB(CString strValue)
{
	::WritePrivateProfileString(L"Call", L"call_b", strValue, m_strIniPath);
}
CString CRWIni::GetCallB()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"Call", L"call_b", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	m_strCallB = szKeyValue;
	return m_strCallB;
}

void CRWIni::SetOuter(CString strValue)
{
	::WritePrivateProfileString(L"Call", L"outer", strValue, m_strIniPath);
}
CString CRWIni::GetOuter()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"Call", L"outer", NULL, szKeyValue, MAX_PATH, m_strIniPath);
	m_strOuter = szKeyValue;
	return m_strOuter;
}



void CRWIni::WriteInter(CString appName, CString keyName, int value)
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	_itow_s(value, szKeyValue, MAX_PATH, 10);
	::WritePrivateProfileString(appName, keyName, szKeyValue, m_strIniPath);  
}
int CRWIni::ReadInter(CString appName, CString keyName)
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(appName, keyName, NULL, szKeyValue, MAX_PATH, m_strIniPath);
	int value = _wtoi(szKeyValue);
	return value;
}
void CRWIni::Set_APP_Title(CString strValue)
{
	::WritePrivateProfileString(L"name", L"title", /*szTitle*/strValue, m_strIniPath);
}
CString CRWIni::Get_APP_Title()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"name", L"title", L"网络广播系统常州天涛通信有限公司", szKeyValue, MAX_PATH, m_strIniPath);
	
	m_strAPP_Title = szKeyValue;
	return m_strAPP_Title;
}

void CRWIni::Set_Sign_Left(CString strValue)
{
	::WritePrivateProfileString(L"name", L"signl", /*szTitle*/strValue, m_strIniPath);
}
CString CRWIni::Get_Sign_Left()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"name", L"signl", L"广州方向", szKeyValue, MAX_PATH, m_strIniPath);
	m_strAPP_SignL = szKeyValue;
	return m_strAPP_SignL;
}

void CRWIni::Set_Sign_Right(CString strValue)
{
	::WritePrivateProfileString(L"name", L"signr", /*szTitle*/strValue, m_strIniPath);
}
CString CRWIni::Get_Sign_Right()
{
	TCHAR szKeyValue[MAX_PATH] = {0};  
	::GetPrivateProfileString(L"name", L"signr", L"北京方向", szKeyValue, MAX_PATH, m_strIniPath);
	m_strAPP_SignR = szKeyValue;
	return m_strAPP_SignR;
}
CString CRWIni::ReadStr(CString appName,CString keyName)
{
	TCHAR szKeyVal[MAX_PATH] = {0};
	::GetPrivateProfileString(appName, keyName, L"XX方向", szKeyVal, MAX_PATH, m_strIniPath);
	m_rtnStr = szKeyVal;
	return m_rtnStr;
}

void CRWIni::SetMainFlag(int nMain)
{
	WriteInter(L"Params", L"main", nMain);
}
int CRWIni::GetMainFlag()
{
	return ReadInter(L"Params", L"main");
}

void CRWIni::SetNetOF(int nNetOF)
{
	WriteInter(L"Params", L"net_of", nNetOF);
}
int CRWIni::GetNetOF()
{
	return ReadInter(L"Params", L"net_of");
}
void CRWIni::SetVolume(int nVolume)
{
	WriteInter(L"Params", L"volume", nVolume);
}
int CRWIni::GetVolume()
{
	return max(80, ReadInter(L"Params", L"volume"));
}