#include "StdAfx.h"
#include "AudioPlay.h"
#include "SIPCallDlg.h"

#define  RECORD_OPEN (1)

CAudioPlay::CAudioPlay(void)
{
	m_ipAddr = inet_addr( "192.168.1.106" );
	m_GroupID = 0;
// 	m_ParamsAllocNum = 500;
// 	m_pParams = NULL;
// 	m_pParams = new _PlayParam[m_ParamsAllocNum];
// 	m_ParamsNum = 0;
//	m_vecParams.reserve(500);
#if RECORD_OPEN
	m_pPlayer = NULL;
#endif
}


CAudioPlay::~CAudioPlay(void)
{
	m_ipAddr = 0;
// 	if (m_pParams != NULL)
// 	{
// 		delete[] m_pParams;
// 		m_pParams = NULL;
// 	}
// 	m_ParamsNum = 0;
// 	m_ParamsAllocNum = 0;
#if RECORD_OPEN
	if (m_pPlayer != NULL)
	{
		CSoundBase::Release();
		m_pPlayer = NULL;
	}
#endif
	
}
void CAudioPlay::initparm(_PlayParam& mParam)
{
	
	mParam.hWnd = m_pAPP->m_hWnd/*(UInt32)this.Handle*/;
	mParam.Priority = 10/*Convert.ToUInt32(priedit.Text)*/;
	mParam.Volume = max(1, min(mParam.Volume, 100));//g_Config.m_SystemConfig.nVolume/*Convert.ToUInt32(volm.Text)*/;
	mParam.Bass =100/*Convert.ToUInt32(LowAudioHz.Value)*/;
	mParam.Bass_En =0/*Convert.ToUInt32 (LowRatio.Value)*/;
	mParam.Treble = 100/*Convert.ToUInt32(HigAudioHz.Value)*/;
	mParam.Treble_En = 0/*Convert.ToUInt32(HigRatio.Value)*/;
	if (m_enModel == ENUM_MODEL_FILE/*autypecom.Text=="文件"*/)                                                     //声音源为文件
	{
		mParam.SourceType = 0;
	}
	else if (m_enModel == ENUM_MODEL_MIC/*autypecom.Text=="声卡"*/)                                                //声音源为声卡
	{
		mParam.SourceType = 1;
		mParam.DeviceID = 0;
		mParam.nChannels = 2;
		mParam.nSamplesPerSec = 44100;                                              //采样频率
		//      PlayParam->MuxName =System.Text.Encoding.Default.GetBytes(Parm.MuxName);
		if (false/*miccheck.Checked == true*/) {
			mParam.OptionByte = 1;
		}
		else {
			mParam.OptionByte = 0;
		}
	}
	else
	{

		mParam.SourceType= 3; 
		mParam.CastMode=2;
		mParam.IP=0;
		mParam.nChannels=2;
		mParam.nSamplesPerSec=44100;
// 		m_Param.filestream =File.Open(FILEBOX.Text, FileMode.Open, FileAccess.Read, FileShare.Read);
// 		m_Param.buflen = Convert.ToInt32(Parm.filestream.Length);
// 		m_Param.senddata = new byte[Parm.buflen];
// 		m_Param.filestream.Read(Parm.senddata, 0, Parm.buflen);
// 		m_Param.filestream.Close();
// 		m_Param.filestream.Dispose();
// 		m_Param.filestream = null;
		

	}
	
	if (m_enBroadcastType == ENUM_BROAD_ALL/*"广播"*/)
	{
		mParam.CastMode = 2;                                                        //广播模式
		mParam.IP = 0;                                                              //任意网卡
	}
	else if (m_enBroadcastType == ENUM_BROAD_GROUP/*typecom.Text == "组播"*/)
	{
		mParam.IP = m_ipAddr;/*GetIpAdress(aimedit.Text)*/;                                      //转化为IP地址参数
		mParam.MultiGroup = m_GroupID;/*Convert.ToUInt32(gplayid.Text)*/;                         //组号
		
		mParam.CastMode = 1;                                                        //组播模式

	}
	else
	{                        
		mParam.IP = m_ipAddr/*GetIpAdress(aimedit.Text)*/;                                       //转化为IP地址参数
		mParam.CastMode = 0;                                                        //单播模式 

	}
}
void CAudioPlay::SetModel(enPlaySrc enModel)
{
	m_enModel = enModel;
}
void CAudioPlay::SetBroadType(enBroadcastType enType)
{
	m_enBroadcastType = enType;
}
int CAudioPlay::play(char*  filePath, _PlayParam& mParam, char* ip)
{	
	SetIP(ip);
	SetBroadType(ENUM_BROAD_SINGLE);
	SetModel(ENUM_MODEL_FILE);

	initparm(mParam);	
	
	unsigned char pTempPath[MAX_PATH] = {0}; 
	memcpy(pTempPath, filePath, strlen(filePath));
	if(R_OK == lc_init(pTempPath, &mParam))
	{			

		int flag = lc_play(&mParam);
		if(flag > 0)
		{	
// 			if (time > 0)
// 			{
// 				int ret = lc_seek(&mParam, time);					
// 			}
			if (m_pAPP->IsMonitorDevice(ip))
			{
				lc_addip(&mParam, inet_addr( g_Config.m_SystemConfig.IP_Monitor));
			}
			return flag;
		}		
	}	
	return 0;
}

int CAudioPlay::play(char* filePath, _PlayParam& mParam, vector<DWORD>& vecIPS)
{
	if (vecIPS.empty())
	{
		return 0;
	}
	m_ipAddr = vecIPS[0];	
	SetBroadType(ENUM_BROAD_SINGLE);
	SetModel(ENUM_MODEL_FILE);
	
	initparm(mParam);

	unsigned char pTempPath[MAX_PATH] = {0}; 
	memcpy(pTempPath, filePath, strlen(filePath));
	if(R_OK == lc_init(pTempPath, &mParam))
	{		
		// add ip
		for (int i = 1; i < vecIPS.size(); i++)
		{
			if (lc_addip( &mParam, vecIPS[i]) == ERR_OPT)
			{				
				return 0;
			}		
		}
		return lc_play(&mParam);		
	}		
	return 0;
}

int CAudioPlay::stop(_PlayParam& mParam)
{	
	return lc_stop(&mParam);
}

int CAudioPlay::pausePlay(_PlayParam& mParam)
{
	if(status(mParam) == 1)
		return lc_pause(&mParam);
	return -2;
}

int CAudioPlay::continuePlay(_PlayParam& mParam)
{
	if (status(mParam) == 2)
		return lc_continue(&mParam);
	return -2;
}

int CAudioPlay::status(_PlayParam& mParam)// 0:运行1：暂停2：停止
{
	return lc_get_playstatus(&mParam);
}

int iptoint( const char *ip )
{
	return ntohl( inet_addr( ip ) );
}

void inttoip( int ip_num, char *ip )
{
//	strcpy( ip, (char*)inet_ntoa( htonl( ip_num ) ) );
}
void CAudioPlay::SetIP(char* IP)
{
	m_ipAddr = inet_addr( IP );
}
void CAudioPlay::SetGroupID(int groupID)
{
	m_GroupID = groupID;
}
void CAudioPlay::RecordInit()
{
	if (g_Config.m_SystemConfig.nRecord)
	{
		m_pPlayer = CSoundBase::GetInstance();
	}
}
void CAudioPlay::RecordStart() 
{
	// TODO: Add your control notification handler code here

	if (g_Config.m_SystemConfig.nRecord == 0)
	{
		return;
	}
	WAVEFORMAT_SETTING waveFormat;
	waveFormat.bits = BITS_16;
	waveFormat.channel = CHANNEL_SINGLE;
	waveFormat.samples = SAMPLES_22050;
	char szName[MAX_PATH] = {0};
	wchar_t wzName[MAX_PATH] = {0};
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	//wsprintf(wzName, L"%srecord\\%04d%02d%02d_%02d%02d%02d.wav", g_szAPPPath, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(szName, "%s\\%04d%02d%02d_%02d%02d%02d.wav", g_Config.m_SystemConfig.pathRecord, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, MAX_PATH);
	g_Log.output(LOG_TYPE, "Record path = %s\r\n", szName);
	if(m_pPlayer->Record(wzName, &waveFormat) == FALSE)
	{
		g_Log.output(LOG_TYPE, "Record failed\r\n");
		::MessageBox(NULL,TEXT("FAILED"),TEXT(""),MB_OK);
		
	}

}

void CAudioPlay::RecordStop() 
{
	// TODO: Add your control notification handler code here
	if (g_Config.m_SystemConfig.nRecord == 0)
	{
		return;
	}

	m_pPlayer->StopRecording();
	m_pPlayer->StopAll();	

}