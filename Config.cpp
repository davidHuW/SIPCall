#include "StdAfx.h"
#include "Config.h"
#include "SIPCallDlg.h"

CConfig::CConfig(void)
{
}


CConfig::~CConfig(void)
{
}

void CConfig::ReadConfig()
{
	//m_SystemConfig.init();
	memset(&m_SystemConfig, 0, sizeof(SSystemConfig));
	//wsprintf(m_SystemConfig.path, m_ini.GetPath());
	CString strIP = m_ini.GetPath();
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetPath(), -1, m_SystemConfig.path, MAX_PATH, NULL, NULL);
	// path record
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetPathRecord(), -1, m_SystemConfig.pathRecord, MAX_PATH, NULL, NULL);
	m_SystemConfig.portDevice = m_ini.GetPort();	
	m_SystemConfig.portAudio = m_ini.GetAudioPort();
	m_SystemConfig.portPC = m_ini.GetPCPort();
	m_SystemConfig.portOut = m_ini.GetOutPort();
	m_SystemConfig.portOut2 = m_ini.GetOutPort2();
	m_SystemConfig.portEvtListen = m_ini.GetEvtPort();
	m_SystemConfig.portEvtSend = m_ini.GetEvtSendPort();
	m_SystemConfig.nJT = m_ini.GetJT();
	m_SystemConfig.nPop = m_ini.GetPop();
	m_SystemConfig.portOuterBCControl = m_ini.GetOuterBCControlPort();
	// mic ip
	//wsprintf(m_SystemConfig.IP_MIC , _T("%s"), m_pAPP->m_ini.GetIP_MIC());
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_MIC(), -1, m_SystemConfig.IP_MIC, MAX_PATH, NULL, NULL);
	//SERVER IP
	//wsprintf(m_SystemConfig.IP_Server ,_T("%s"), m_pAPP->m_ini.GetIP_Server());
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_Server(), -1, m_SystemConfig.IP_Server, MAX_PATH, NULL, NULL);
	//wsprintf(m_SystemConfig.CallA ,_T("%s"), m_pAPP->m_ini.GetCallA());
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetCallA(), -1, m_SystemConfig.CallA, MAX_PATH, NULL, NULL);
	//wsprintf(m_SystemConfig.CallB ,_T("%s"), m_pAPP->m_ini.GetCallB());
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetCallB(), -1, m_SystemConfig.CallB, MAX_PATH, NULL, NULL);
	//wsprintf(m_SystemConfig.Outer,_T("%s"), m_pAPP->m_ini.GetOuter());
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetOuter(), -1, m_SystemConfig.Outer, MAX_PATH, NULL, NULL);
	
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_Out(), -1, m_SystemConfig.IP_Out, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_Out2(), -1, m_SystemConfig.IP_Out2, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_MIC_Reocrd(), -1, m_SystemConfig.IP_MIC_RECORD, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_AUX(), -1, m_SystemConfig.IP_AUX, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_Monitor(), -1, m_SystemConfig.IP_Monitor, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.GetIP_OUT_BC_CONTROL(), -1, m_SystemConfig.IP_OUTER_BC_CONTROL, MAX_PATH, NULL, NULL);

	m_SystemConfig.nPlayTime = min(max(1, m_ini.GetPlayTime()), MAX_PLAY_TIME);

	WideCharToMultiByte(CP_ACP, 0, m_ini.Get_APP_Title(), -1, m_SystemConfig.title, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.Get_Sign_Left(), -1, m_SystemConfig.signL, MAX_PATH, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_ini.Get_Sign_Right(), -1, m_SystemConfig.signR, MAX_PATH, NULL, NULL);

	m_SystemConfig.nMainSystem = m_ini.GetMainFlag();
	m_SystemConfig.bHttpConnect = m_ini.GetSocketConnect();
	m_SystemConfig.nNetOF = m_ini.GetNetOF();
	//m_SystemConfig.nVolume = m_ini.GetVolume();
	m_SystemConfig.nRecord = m_ini.GetRecord();
}
void CConfig::SaveConfig()
{
	TCHAR path[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.path, -1, path, MAX_PATH);
	m_ini.SetPath(path/*m_SystemConfig.path*/);
	TCHAR pathRecord[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.pathRecord, -1, pathRecord, MAX_PATH);
	m_ini.SetPathRecord(pathRecord);
	m_ini.SetPort(m_SystemConfig.portDevice);	
	m_ini.SetAudioPort(m_SystemConfig.portAudio);
	m_ini.SetPCPort(m_SystemConfig.portPC);
	m_ini.SetOutPort(m_SystemConfig.portOut);
	m_ini.SetOutPort2(m_SystemConfig.portOut2);
	m_ini.SetEvtPort(m_SystemConfig.portEvtListen);
	m_ini.SetEvtSendPort(m_SystemConfig.portEvtSend);
	m_ini.SetOuterBCControlPort(m_SystemConfig.portOuterBCControl);
	m_ini.SetSocketConnect(m_SystemConfig.bHttpConnect);
	m_ini.SetNetOF(m_SystemConfig.nNetOF);
	//m_ini.SetVolume(m_SystemConfig.nVolume);
	m_ini.SetRecord(m_SystemConfig.nRecord);
	m_ini.SetJT(m_SystemConfig.nJT);
	m_ini.SetPop(m_SystemConfig.nPop);
	// mic ip
	TCHAR IP_MIC[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_MIC, -1, IP_MIC, MAX_PATH);
	m_ini.SetIP_MIC(IP_MIC/*m_SystemConfig.IP_MIC*/);
	//SERVER IP
	TCHAR IP_Server[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_Server, -1, IP_Server, MAX_PATH);
	m_ini.SetIP_Server(IP_Server/*m_SystemConfig.IP_Server*/);
	TCHAR CallA[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.CallA, -1, CallA, MAX_PATH);
	m_ini.SetCallA(CallA);

	TCHAR CallB[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.CallB, -1, CallB, MAX_PATH);	
	m_ini.SetCallB(CallB/*m_SystemConfig.CallB*/);
	TCHAR Outer[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.Outer, -1, Outer, MAX_PATH);	
	m_ini.SetOuter(Outer/*m_SystemConfig.Outer*/);

	

	TCHAR OutIP[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_Out, -1, OutIP, MAX_PATH);	
	m_ini.SetIP_Out(OutIP/*m_SystemConfig.Outer*/);

	TCHAR OutIP2[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_Out2, -1, OutIP2, MAX_PATH);	
	m_ini.SetIP_Out2(OutIP2);


	TCHAR IP_MIC_RECORD[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_MIC_RECORD, -1, IP_MIC_RECORD, MAX_PATH);
	m_ini.SetIP_MIC_Reocrd(IP_MIC_RECORD);

	TCHAR IP_AUX[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_AUX, -1, IP_AUX, MAX_PATH);
	m_ini.SetIP_AUX(IP_AUX);

	TCHAR IP_Monitor[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_Monitor, -1, IP_Monitor, MAX_PATH);
	m_ini.SetIP_Monitor(IP_Monitor);

	TCHAR IP_Outer_BC_Control[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.IP_OUTER_BC_CONTROL, -1, IP_Outer_BC_Control, MAX_PATH);
	m_ini.SetIP_OUT_BC_CONTROL(IP_Outer_BC_Control);

	TCHAR APP_TITLE[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.title, -1, APP_TITLE, MAX_PATH);
	m_ini.Set_APP_Title(APP_TITLE);


	TCHAR SignL[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.signL, -1, SignL, MAX_PATH);
	m_ini.Set_Sign_Left(SignL);

	TCHAR SignR[MAX_PATH] = {0};
	MultiByteToWideChar(CP_ACP, 0, m_SystemConfig.signR, -1, SignR, MAX_PATH);
	m_ini.Set_Sign_Right(SignR);

	m_ini.SetPlayTime(m_SystemConfig.nPlayTime);

	m_ini.SetMainFlag(m_SystemConfig.nMainSystem);
}