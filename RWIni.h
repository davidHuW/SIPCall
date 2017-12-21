#pragma once
class CRWIni
{
public:
	CRWIni(void);
	~CRWIni(void);	

	void SetPort(int value); 
	int GetPort();	

	void SetAudioPort(int value); 
	int GetAudioPort();	

	void SetPCPort(int value); 
	int GetPCPort();	

	void SetOutPort(int value); 
	int GetOutPort();

	void SetOutPort2(int value); 
	int GetOutPort2();

	void SetEvtPort(int value); 
	int GetEvtPort();

	void SetEvtSendPort(int value); 
	int GetEvtSendPort();

	void SetOuterBCControlPort(int value);
	int GetOuterBCControlPort();

	void SetRecord(int value); 
	int GetRecord();

	void SetJT(int value); 
	int GetJT();

	void SetPop(int value); 
	int GetPop();


	void SetSocketConnect(int value); 
	int GetSocketConnect();

	void SetPlayTime(int value); 
	int GetPlayTime();

	void SetPath(CString strPath);
	CString GetPath();

	void SetPathRecord(CString strPath);
	CString GetPathRecord();

	void SetIP_Server(CString strValue);
	CString GetIP_Server();

	void SetIP_MIC(CString strValue);
	CString GetIP_MIC();

	void SetIP_Out(CString strValue);
	CString GetIP_Out();

	void SetIP_Out2(CString strValue);
	CString GetIP_Out2();

	void SetCallA(CString strValue);
	CString GetCallA();

	void SetCallB(CString strValue);
	CString GetCallB();

	void SetOuter(CString strValue);
	CString GetOuter();

	void SetIP_OUT_BC_CONTROL(CString strVal);
	CString GetIP_OUT_BC_CONTROL();

	void SetIP_MIC_Reocrd(CString strValue);
	CString GetIP_MIC_Reocrd();

	void SetIP_AUX(CString strValue);
	CString GetIP_AUX();

	void SetIP_Monitor(CString strValue);
	CString GetIP_Monitor();

	void Set_APP_Title(CString strValue);
	CString Get_APP_Title();

	void Set_Sign_Left(CString strValue);
	CString Get_Sign_Left();

	void Set_Sign_Right(CString strValue);
	CString Get_Sign_Right();

	void SetMainFlag(int nMain); 
	int GetMainFlag();

	void SetNetOF(int nNetOF); 
	int GetNetOF();

	void SetVolume(int nVolume); 
	int GetVolume();
public:
	CString ReadStr(CString appName,CString keyName);
private:
	CString m_rtnStr;
private:
	void WriteInter(CString appName, CString keyName, int value); 
	int ReadInter(CString appName, CString keyName);
private:
	CString m_strIniPath;
private:
	CString m_strCallA;
	CString m_strCallB;
	CString m_strOuter;	
	CString m_strIP_MIC;
	CString m_strIP_SERVER;
	CString m_strPath;
	CString m_strPathRecord;
	CString m_strIP_Out;
	CString m_strIP_Out2;
	CString m_strIP_Outer_BC_Control;
	CString m_strIP_MICRecord;
	CString m_strIP_AUX;
	CString m_strIP_Monitor;
	CString m_strAPP_Title;
	CString m_strAPP_SignL;
	CString m_strAPP_SignR;
};

