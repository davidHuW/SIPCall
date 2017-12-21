#include "StdAfx.h"
#include "DataM.h"
#include "SIPCallDlg.h"
#if _DEBUG
#define DEBUG_DATA (0)
#endif

CDataM::CDataM(void)
{
	m_Songid = 0;
	m_vecDeviceBeat.reserve(300);
	m_vecPlayConfig.reserve(300);
	m_vecListSong.reserve(200);
	m_vecPlayList.reserve(100);
	m_vecRecCall.reserve(1000);
	m_vecRecMsg.reserve(1000);
	m_vecRecLog.reserve(1000);
}


CDataM::~CDataM(void)
{
	clear();
	m_sqlite.Close();
	ClearPlayList();
	ClearSongList();
	ClearPlayConfig();
}
void CDataM::clear()
{
	if (!m_vecTunnel.empty())
	{
		for (int i = 0; i <m_vecTunnel.size(); i ++)
		{
			if (!m_vecTunnel[i].vecDeviceUp.empty())
			{				
				m_vecTunnel[i].vecDeviceUp.clear();
			}
			if (!m_vecTunnel[i].vecDeviceDown.empty())
			{
				m_vecTunnel[i].vecDeviceDown.clear();
			}
		}
		m_vecTunnel.clear();
	}
	if(!m_vecDevice.empty()){
		m_vecDevice.clear();
	}
	if (!m_vecDB.empty())
	{
		m_vecDB.clear();
	}
	if (!m_vecTempT.empty())
	{
		m_vecTempT.clear();
	}
}
bool less_name_id(const SDBInfo& item1, const SDBInfo& item2)
{
// 	if (strcmp(item1.tunnelName, item2.tunnelName) == 0)
// 	{
// 		return item1.id < item2.id;
// 	}
	if (item1.tempTID == item2.tempTID)
	{
		return item1.id < item2.id;
	}
	else{
		return item1.tempTID < item2.tempTID;
	}
}

CString CDataM::getDBPath()
{
	//m_strDBPath = L"";

	// 得到exe执行路径.  
	TCHAR tcExePath[MAX_PATH] = {0};  
	::GetModuleFileName(NULL, tcExePath, MAX_PATH);  
	// 设置ini路径到exe同一目录下  
#ifndef CONFIG_FILE  
#define CONFIG_FILE     (TEXT("db.db3"))  
#endif  
	//_tcsrchr() 反向搜索获得最后一个'\\'的位置，并返回该位置的指针  
	TCHAR *pFind = _tcsrchr(tcExePath, '\\');  
	if (pFind == NULL)  
	{  
		return L"";  
	}  
	*pFind = '\0';  

	CString szIniPath = tcExePath;  
	szIniPath += "\\";  
	szIniPath += CONFIG_FILE;  

	//m_strDBPath = szIniPath;
	return szIniPath;
}
bool CDataM::ReadDB()
{	
	CString strDBPath = getDBPath();
	if(!m_sqlite.Open(strDBPath)){
		return false;
	}

#if DEBUG_DATA
	int maxTunnelSz = 35/*rand() % 10 + 30*/;
	
	for (int i = 0; i < maxTunnelSz; i ++)
	{
		STunnel sTunel;
		sTunel.TunnelID = 10000 + i;
		memset(sTunel.Name, 0, 256);
		sprintf(sTunel.Name, "隧道%d", i + 1);

		int maxUpDeviceSz = (i + 1)* 10 + 5;
		int maxDownDeviceSz = (i + 1)* 10 + 6;

		for (int j = 0; j < maxUpDeviceSz; j ++)
		{
			SDevice sDevice;
			//memset(&sDevice, 0, sizeof(SDevice));
			sDevice.sDB.id = 10000 + j;
		
			sprintf(sDevice.sDB.Name, "ET1%02d", j);
			sprintf(sDevice.sDB.ip, "192.168.1.%d", j);
			
			sTunel.vecDeviceUp.push_back(sDevice);

		}
		for (int j = 0; j < maxDownDeviceSz; j++)
		{
			SDevice sDevice;
			//memset(&sDevice, 0, sizeof(SDevice));
			sDevice.sDB.id = 10100 + j;
			
			sprintf(sDevice.sDB.Name, "ET2%02d", j);
			sprintf(sDevice.sDB.ip, "192.168.1.%d", j + 100);
		
			sTunel.vecDeviceDown.push_back(sDevice);
		}
		m_vecTunnel.push_back(sTunel);
	}
#else
		
	// tunnel 
	TCHAR sql[512] = {0};
	_stprintf(sql,_T("%s"),_T("select * from info"));
	SQLiteDataReader Reader0 = m_sqlite.ExcuteQuery(sql);
	bool colExist = Reader0.IsExistColumn(L"ipcam");
	if (!colExist)
	{
		TCHAR sql0[512] = {0};
		_stprintf(sql0,_T("%s"),_T("alter table info add ipcam VARCHAR"));

		if (m_sqlite.ExcuteNonQuery(sql0))
		{
			int a = 0;
			a++;
		}
		else{
			LPCTSTR error = m_sqlite.GetLastErrorMsg();
			int a = 0;
			a++;
		}
	}
	//LPCTSTR temp =  m_sqlite.GetLastErrorMsg();
	int index = 0;
	while(Reader0.Read())  
	{  
		SDBInfo info;
		//memset(&info, 0, sizeof(SDBInfo));

		int idx = 0;
		info.id = Reader0.GetIntValue(idx++);
		LPCTSTR strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.Name, 256, NULL, NULL);
		// ip
		strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.ip, 256, NULL, NULL);
		
		// 
		info.extAddr = Reader0.GetIntValue(idx++);
		// extNo
		strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.extNo, 256, NULL, NULL);
		// tunnel name
		strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.tunnelName, 256, NULL, NULL);
		// kmid 
		strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.kmID, 256, NULL, NULL);
		// desc 
		strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.desc, 128, NULL, NULL);

		// flag
		info.flag = Reader0.GetIntValue(idx++);

		// ip cam
		strValue = Reader0.GetStringValue(idx++);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.ip_cam, 256, NULL, NULL);

	    strValue = Reader0.GetStringValue(idx++);
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, info.controllerip, 256, NULL, NULL);

		m_vecDB.push_back(info);
		index++;
	  
 
	}  	
	Reader0.Close();
	
	DB2Data();
	
#if 0

	// device
	_stprintf(sql,_T("%s"),_T("select * from device"));
	SQLiteDataReader Reader1 = m_sqlite.ExcuteQuery(sql);	
	index = 0;
	
	while(Reader1.Read())  
	{  
		SDevice sDevice;
		//memset(&sDevice, 0, sizeof(SDevice));
		sDevice.DeviceID = Reader1.GetIntValue(0);
		sDevice.tid =  Reader1.GetIntValue(1);
		sDevice.type = Reader1.GetIntValue(2);
		LPCTSTR strValue = Reader1.GetStringValue(3);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, sDevice.Name, 256, NULL, NULL);

		strValue = Reader1.GetStringValue(4);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, sDevice.IP, 256, NULL, NULL);

		strValue = Reader1.GetStringValue(5);		
		WideCharToMultiByte(CP_ACP, 0, strValue, -1, sDevice.MAC, 256, NULL, NULL);

		m_vecDevice.push_back(sDevice);
		index++;
	}  	
	Reader1.Close();
	// convert
	for (int i = 0; i < m_vecTunnel.size(); i++)
	{
		int tid = m_vecTunnel[i].TunnelID;
		for (int j = 0; j < m_vecDevice.size(); j++)
		{
			if (m_vecDevice[j].tid != tid)
			{
				continue;
			}
			if (m_vecDevice[i].type == 0)
			{
				m_vecTunnel[i].vecDeviceUp.push_back(m_vecDevice[j]);
			}
			else{
				m_vecTunnel[i].vecDeviceDown.push_back(m_vecDevice[j]);
			}
		}
	}
#endif


#endif
	
	return true;
}
void CDataM::AddTunnelOutFlag()
{
	m_pAPP->ReadDeviceOutFile();
	for (int i = 0; i < m_vecTunnel.size(); i ++)
	{
		int upSz = m_vecTunnel[i].vecDeviceUp.size();
		for (int j = 0; j < upSz; j++)
		{
			if (m_pAPP->IsDeviceOut(m_vecTunnel[i].vecDeviceUp[j].sDB.ip)/*(j == 0) || (j == upSz - 1)*/)
			{
				 m_vecTunnel[i].vecDeviceUp[j].bOutTunnel = 1;
			}
		}
		int downSz = m_vecTunnel[i].vecDeviceDown.size();
		for (int j = 0; j < downSz; j++)
		{
			if (m_pAPP->IsDeviceOut(m_vecTunnel[i].vecDeviceDown[j].sDB.ip)/*(j == 0) || (j == downSz - 1)*/)
			{
				m_vecTunnel[i].vecDeviceDown[j].bOutTunnel = 1;
			}
		}
	}
}
void CDataM::AddYing()
{
	m_pAPP->ReadYingFile();
	for (int i = 0; i < m_vecTunnel.size(); i ++)
	{
		int upSz = m_vecTunnel[i].vecDeviceUp.size();
		for (int j = 0; j < upSz; j++)
		{
			m_vecTunnel[i].vecDeviceUp[j].playParam.Volume = m_pAPP->GetYing(m_vecTunnel[i].vecDeviceUp[j].sDB.extAddr);
		}
		int downSz = m_vecTunnel[i].vecDeviceDown.size();
		for (int j = 0; j < downSz; j++)
		{
			m_vecTunnel[i].vecDeviceDown[j].playParam.Volume = m_pAPP->GetYing(m_vecTunnel[i].vecDeviceDown[j].sDB.extAddr);
		}
	}
}
bool less_id(const SDBInfo& item1, const SDBInfo& item2)
{
	return item1.id < item2.id;	
}

SDBInfo* CDataM::GetDB(int& nNum)
{
	nNum = 0;
	if (m_vecDB.empty())
	{
		return NULL;
	}
	sort(m_vecDB.begin(), m_vecDB.end(), less_id);
	nNum = m_vecDB.size();
	vector<SDBInfo>::iterator vecIter = m_vecDB.begin();
	return &(*vecIter);
}
void CDataM::AddNullDB(int id)
{
	SDBInfo info;
	info.id = id;
	m_vecDB.push_back(info);

	m_sqlite.BeginTransaction();
	// 插入数据   
	
	char strSQL[512] = {0};
	sprintf(strSQL,"insert into info(id,name,ip,extaddr,extNo,tname,kmid,desc,flag,ipcam,ipcontroller) \
	values('%d','%s','%s','%d','%s','%s','%s','%s','%d','%s','%s')", \
	info.id, info.Name, info.ip,info.extAddr, info.extNo, info.tunnelName, info.kmID, info.desc, info.flag, info.ip_cam,info.controllerip);  

	wchar_t wsql[512] = {0};
	MultiByteToWideChar(CP_ACP, 0, strSQL, -1, wsql, 512);
	if(!m_sqlite.ExcuteNonQuery(wsql))  
	{   
		m_sqlite.RollbackTransaction();
	}else
		m_sqlite.CommitTransaction();
}
void CDataM::AddOneDB(SDBInfo& info)
{
	
	m_vecDB.push_back(info);
}
void CDataM::DeleteOneDB(int id)
{
	if(m_vecDB.empty()){
		return;
	}
	for(int i = 0; i < m_vecDB.size(); i ++){
		if (id == m_vecDB[i].id&&m_vecDB[i].deleteFlag!=1)
		{
			m_vecDB[i].deleteFlag = 1;
			m_sqlite.BeginTransaction();
			// 插入数据   

			char strSQL[512] = {0};
			sprintf(strSQL,"delete from info where id = '%d'",id);  

			wchar_t wsql[512] = {0};
			MultiByteToWideChar(CP_ACP, 0, strSQL, -1, wsql, 512);
			if(!m_sqlite.ExcuteNonQuery(wsql))  
			{   
				m_sqlite.RollbackTransaction();
			}else
				m_sqlite.CommitTransaction();
		}
	}
}
void CDataM::SaveDB()
{
	//TCHAR sql[512] = {0};
	//_stprintf(sql,_T("%s"),_T("delete from info"));
	
	//if (m_sqlite.ExcuteNonQuery(sql))
	//{
		m_sqlite.BeginTransaction();
		// 批量插入数据   
		for(int i=0;i<m_vecDB.size();i++)  
		{  
			char strSQL[512] = {0};
			sprintf(strSQL,"update info set name = '%s',ip = '%s',extaddr='%d',\
						   extNo='%s',tname='%s',kmid='%s',desc='%s',flag='%d',ipcam='%s',ipcontroller='%s' where id = '%d'",\
						   m_vecDB[i].Name,m_vecDB[i].ip,m_vecDB[i].extAddr,m_vecDB[i].extNo,\
						   m_vecDB[i].tunnelName,m_vecDB[i].kmID,m_vecDB[i].desc,m_vecDB[i].flag,m_vecDB[i].ip_cam,\
						   m_vecDB[i].controllerip,m_vecDB[i].id);
			//char strVal[256] = {0};
			//itoa(m_vecDB[i].controllerip,strVal,10);
			//sprintf(strSQL,"insert into info(id,name,ip,extaddr,extNo,tname,kmid,desc,flag,ipcam,ipcontroller) \
				values('%d','%s','%s','%d','%s','%s','%s','%s','%d','%s','%s')", \
				m_vecDB[i].id, m_vecDB[i].Name, m_vecDB[i].ip, m_vecDB[i].extAddr, m_vecDB[i].extNo, m_vecDB[i].tunnelName, m_vecDB[i].kmID, m_vecDB[i].desc, m_vecDB[i].flag, m_vecDB[i].ip_cam,m_vecDB[i].controllerip);  
			wchar_t wsql[512] = {0};
			MultiByteToWideChar(CP_ACP, 0, strSQL, -1, wsql, 512);
			if(!m_sqlite.ExcuteNonQuery(wsql))  
			{  
				_tprintf(_T("%s\n"),m_sqlite.GetLastErrorMsg());  
				break;  
			}  
		}  
		// 提交事务   
		m_sqlite.CommitTransaction(); 

	/*}
	else{
		LPCTSTR error = m_sqlite.GetLastErrorMsg();
		int a = 0;
		a++;
	}*/
	
}
bool CDataM::deleteDB()
{
	TCHAR sql[512] = {0};
	_stprintf(sql,_T("%s"),_T("delete from info"));

	if (m_sqlite.ExcuteNonQuery(sql))
	{
		return true;
	}
	else{
		LPCTSTR error = m_sqlite.GetLastErrorMsg();
		return false;
	}
}
bool CDataM::deleteCall()
{
	TCHAR sql[512] = {0};
	_stprintf(sql,_T("%s"),_T("delete from rectel"));

	if (m_sqlite.ExcuteNonQuery(sql))
	{
		return true;
	}
	else{
		LPCTSTR error = m_sqlite.GetLastErrorMsg();
		return false;
	}

}
bool CDataM::deleteMsg()
{
	TCHAR sql[512] = {0};
	_stprintf(sql,_T("%s"),_T("delete from recmsg"));

	if (m_sqlite.ExcuteNonQuery(sql))
	{
		return true;
	}
	else{
		LPCTSTR error = m_sqlite.GetLastErrorMsg();
		return false;
	}
}
bool CDataM::deleteLog()
{
	TCHAR sql[512] = {0};
	_stprintf(sql,_T("%s"),_T("delete from reclog"));

	if (m_sqlite.ExcuteNonQuery(sql))
	{
		return true;
	}
	else{
		LPCTSTR error = m_sqlite.GetLastErrorMsg();
		return false;
	}
}
void CDataM::ClearDB()
{
	if (!m_vecDB.empty())
	{
		m_vecDB.clear();
	}
}
void CDataM::DB2Data()
{
	if (!m_vecDB.empty())
	{
		SetTempTID();
		sort(m_vecDB.begin(), m_vecDB.end(), less_name_id);

		if (!m_vecTunnel.empty())
		{
			for (int i = 0; i <m_vecTunnel.size(); i ++)
			{
				if (!m_vecTunnel[i].vecDeviceUp.empty())
				{
					m_vecTunnel[i].vecDeviceUp.clear();
				}
				if (!m_vecTunnel[i].vecDeviceDown.empty())
				{
					m_vecTunnel[i].vecDeviceDown.clear();
				}
			}
			m_vecTunnel.clear();
		}

		// convert
		int tid = 0;
		char* pTName = "未知";
		for (int i = 0; i < m_vecDB.size(); i ++)
		{
			if (m_vecDB[i].id == 0xFFFFFF)
			{
				continue;
			}
			if (strcmp(m_vecDB[i].tunnelName, pTName) == 0)
			{
				SDevice sDevice;
				memcpy(&(sDevice.sDB), &(m_vecDB[i]), sizeof(SDBInfo));
				//sDevice.dwIP = inet_addr(sDevice.sDB.ip);
				memcpy(sDevice.dwIP,m_vecDB[i].controllerip,256);
				if(!m_vecDB[i].flag){
					m_vecTunnel[m_vecTunnel.size() - 1].vecDeviceUp.push_back(sDevice);
				}
				else{
					m_vecTunnel[m_vecTunnel.size() - 1].vecDeviceDown.push_back(sDevice);
				}

			}
			else{
				STunnel sTunnel;
				sTunnel.TunnelID = tid++;
				memcpy(sTunnel.Name, m_vecDB[i].tunnelName, 256);
				/************************************************************************/
				/*    根据隧道名获得隧道方向    */
				/************************************************************************/
				TCHAR wTunnel[MAX_PATH] = {0};
				MultiByteToWideChar(CP_ACP, 0, sTunnel.Name, -1, wTunnel, MAX_PATH);
				sTunnel.SignL = g_Config.m_ini.ReadStr(wTunnel,L"signl");
				sTunnel.SignR = g_Config.m_ini.ReadStr(wTunnel,L"signr");

				SDevice sDevice;
				memcpy(&(sDevice.sDB), &(m_vecDB[i]), sizeof(SDBInfo));
				memcpy(sDevice.dwIP,m_vecDB[i].controllerip,256);
				//sDevice.dwIP = inet_addr(sDevice.sDB.ip);
				if (!m_vecDB[i].flag)
				{
					sTunnel.vecDeviceUp.push_back(sDevice);
				}
				else{
					sTunnel.vecDeviceDown.push_back(sDevice);
				}				
				m_vecTunnel.push_back(sTunnel);
				pTName = m_vecDB[i].tunnelName;
			}
		}
	}
	else{
		if (!m_vecTunnel.empty())
		{
			for (int i = 0; i <m_vecTunnel.size(); i ++)
			{
				if (!m_vecTunnel[i].vecDeviceUp.empty())
				{
					m_vecTunnel[i].vecDeviceUp.clear();
				}
				if (!m_vecTunnel[i].vecDeviceDown.empty())
				{
					m_vecTunnel[i].vecDeviceDown.clear();
				}
			}
			m_vecTunnel.clear();
		}
	}
	AddTunnelOutFlag();
	AddYing();
}
STunnel* CDataM::GetData(int& nNum)
{
	nNum = 0;
	if (m_vecTunnel.empty())
	{
		return NULL;
	}
	nNum = m_vecTunnel.size();
	vector<STunnel>::iterator vecIter = m_vecTunnel.begin();
	return &(*vecIter);
}
bool CDataM::ReadCall(enDataSortType enType)
{
	if (!m_vecRecCall.empty())
	{
		m_vecRecCall.clear();
	}
	// tunnel 
	TCHAR sql[512] = {0};
	if (enType == ENUM_SORT_ALL)
	{
		wsprintf(sql,_T("%s"),_T("select * from rectel order by 日期 desc"));
	}
	else if (enType == ENUM_SORT_WEEK)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		if (tm.wDay < 7)
		{
			if(tm.wMonth - 1 < 0){
				tm.wMonth = 12;
				tm.wYear -= 1;
			}
			else{
				tm.wMonth -= 1;				
			}
			tm.wDay = 30 - (7 - tm.wDay);
		}
		else{
			tm.wDay -= 7;
		}
		wsprintf(sql,_T("select * from rectel  where 日期 > '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
		
	}
	else if (enType == ENUM_SORT_DAY)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);		
		wsprintf(sql,_T("select * from rectel  where 日期 >= '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
	}
	else if (enType == ENUM_SORT_HOUR)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		tm.wHour -= 1;
		if (tm.wHour < 0)
		{
			tm.wHour += 24;
			tm.wDay -= 1;
		}
		wsprintf(sql,_T("select * from rectel  where 日期 > '%04d-%02d-%02d %02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour);
	}
	else{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		if(tm.wMonth - 1 < 0){
			tm.wMonth = 12;
			tm.wYear -= 1;
		}
		else{
			tm.wMonth -= 1;			
		}
		wsprintf(sql,_T("select * from rectel  where 日期 > '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
	}
	SQLiteDataReader Reader0 = m_sqlite.ExcuteQuery(sql);
	LPCTSTR temp =  m_sqlite.GetLastErrorMsg();
	int index = 0;
	while(Reader0.Read())  
	{  
		SRecord sRec;
		//memset(&sRec, 0, sizeof(SRecord));
		wsprintf(sRec.sip, _T("%s"), Reader0.GetStringValue(0));
		sRec.addr = Reader0.GetIntValue(1);
		wsprintf(sRec.IP, _T("%s"), Reader0.GetStringValue(2));
		wsprintf(sRec.desc, _T("%s"), Reader0.GetStringValue(3));
		wsprintf(sRec.tunnel, _T("%s"), Reader0.GetStringValue(4));
		wsprintf(sRec.noid, _T("%s"), Reader0.GetStringValue(5));
		wsprintf(sRec.kmid, _T("%s"), Reader0.GetStringValue(6));
		wsprintf(sRec.data, _T("%s"), Reader0.GetStringValue(7));
		//LPCTSTR strValue = Reader0.GetStringValue(2);
		
		
		m_vecRecCall.push_back(sRec);
		index++;
	 

	}  	
	Reader0.Close();
	return true;
}


bool CDataM::ReadMsg(enDataSortType enType)
{
	if (!m_vecRecMsg.empty())
	{
		m_vecRecMsg.clear();
	}
	// tunnel 
	//TCHAR sql[512] = {0};
	//wsprintf(sql,_T("%s"),_T("select * from recmsg order by 日期 desc"));
	TCHAR sql[512] = {0};
	if (enType == ENUM_SORT_ALL)
	{
		wsprintf(sql,_T("%s"),_T("select * from recmsg order by 日期 desc"));
	}
	else if (enType == ENUM_SORT_WEEK)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		if (tm.wDay < 7)
		{
			if(tm.wMonth - 1 < 0){
				tm.wMonth = 12;
				tm.wYear -= 1;
			}
			else{
				tm.wMonth -= 1;				
			}
			tm.wDay = 30 - (7 - tm.wDay);
		}
		else{
			tm.wDay -= 7;
		}
		wsprintf(sql,_T("select * from recmsg  where 日期 > '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);

	}
	else if (enType == ENUM_SORT_DAY)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);		
		wsprintf(sql,_T("select * from recmsg  where 日期 >= '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
	}
	else if (enType == ENUM_SORT_HOUR)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		tm.wHour -= 1;
		if (tm.wHour < 0)
		{
			tm.wHour += 24;
			tm.wDay -= 1;
		}
		wsprintf(sql,_T("select * from recmsg  where 日期 > '%04d-%02d-%02d %02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour);
	}
	else{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		if(tm.wMonth - 1 < 0){
			tm.wMonth = 12;
			tm.wYear -= 1;
		}
		else{
			tm.wMonth -= 1;			
		}
		wsprintf(sql,_T("select * from recmsg  where 日期 > '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
	}

	SQLiteDataReader Reader0 = m_sqlite.ExcuteQuery(sql);
	//LPCTSTR temp =  m_sqlite.GetLastErrorMsg();
	int index = 0;
	while(Reader0.Read())  
	{  
		SRecord sRec;
		//memset(&sRec, 0, sizeof(SRecord));
		wsprintf(sRec.sip, _T("%s"), Reader0.GetStringValue(0));
		sRec.addr = Reader0.GetIntValue(1);
		wsprintf(sRec.IP, _T("%s"), Reader0.GetStringValue(2));
		wsprintf(sRec.desc, _T("%s"), Reader0.GetStringValue(3));
		wsprintf(sRec.tunnel, _T("%s"), Reader0.GetStringValue(4));
		wsprintf(sRec.noid, _T("%s"), Reader0.GetStringValue(5));
		wsprintf(sRec.kmid, _T("%s"), Reader0.GetStringValue(6));
		wsprintf(sRec.data, _T("%s"), Reader0.GetStringValue(7));
		//LPCTSTR strValue = Reader0.GetStringValue(2);

		m_vecRecMsg.push_back(sRec);
		index++;


	}  	
	Reader0.Close();
	return true;
}
SRecord* CDataM::GetRecordCall(int& nNum)
{
	nNum = 0;
	if (m_vecRecCall.empty())
	{
		return NULL;
	}
	nNum = m_vecRecCall.size();
	vector<SRecord>::iterator vecIter = m_vecRecCall.begin();
	return &(*vecIter);
}
SRecord* CDataM::GetRecordMsg(int& nNum)
{
	nNum = 0;
	if (m_vecRecMsg.empty())
	{
		return NULL;
	}
	nNum = m_vecRecMsg.size();
	vector<SRecord>::iterator vecIter = m_vecRecMsg.begin();
	return &(*vecIter);
}

SRecord* CDataM::GetRecordLog(int& nNum)
{
	nNum = 0;
	if (m_vecRecLog.empty())
	{
		return NULL;
	}
	nNum = m_vecRecLog.size();
	vector<SRecord>::iterator vecIter = m_vecRecLog.begin();
	return &(*vecIter);
}
bool CDataM::ReadLog(enDataSortType enType)
{
	if (!m_vecRecLog.empty())
	{
		m_vecRecLog.clear();
	}
	// tunnel 
	//TCHAR sql[512] = {0};
	//wsprintf(sql,_T("%s"),_T("select * from reclog order by 日期 desc"));
	TCHAR sql[512] = {0};
	if (enType == ENUM_SORT_ALL)
	{
		wsprintf(sql,_T("%s"),_T("select * from reclog order by 日期 desc"));
	}
	else if (enType == ENUM_SORT_WEEK)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		if (tm.wDay < 7)
		{
			if(tm.wMonth - 1 < 0){
				tm.wMonth = 12;
				tm.wYear -= 1;
			}
			else{
				tm.wMonth -= 1;				
			}
			tm.wDay = 30 - (7 - tm.wDay);
		}
		else{
			tm.wDay -= 7;
		}
		wsprintf(sql,_T("select * from reclog  where 日期 > '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);

	}
	else if (enType == ENUM_SORT_DAY)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);		
		wsprintf(sql,_T("select * from reclog  where 日期 >= '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
	}
	else if (enType == ENUM_SORT_HOUR)
	{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		tm.wHour -= 1;
		if (tm.wHour < 0)
		{
			tm.wHour += 24;
			tm.wDay -= 1;
		}
		wsprintf(sql,_T("select * from reclog  where 日期 > '%04d-%02d-%02d %02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour);
	}
	else{
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		if(tm.wMonth - 1 < 0){
			tm.wMonth = 12;
			tm.wYear -= 1;
		}
		else{
			tm.wMonth -= 1;			
		}
		wsprintf(sql,_T("select * from reclog  where 日期 > '%04d-%02d-%02d' order by 日期 desc"), tm.wYear, tm.wMonth, tm.wDay);
	}
	SQLiteDataReader Reader0 = m_sqlite.ExcuteQuery(sql);
	//LPCTSTR temp =  m_sqlite.GetLastErrorMsg();
	int index = 0;
	while(Reader0.Read())  
	{  
		SRecord sRec;
		//memset(&sRec, 0, sizeof(SRecord));
		wsprintf(sRec.sip, _T("%s"), Reader0.GetStringValue(0));
		sRec.addr = Reader0.GetIntValue(1);
		wsprintf(sRec.IP, _T("%s"), Reader0.GetStringValue(2));
		wsprintf(sRec.desc, _T("%s"), Reader0.GetStringValue(3));
		wsprintf(sRec.tunnel, _T("%s"), Reader0.GetStringValue(4));
		wsprintf(sRec.noid, _T("%s"), Reader0.GetStringValue(5));
		wsprintf(sRec.kmid, _T("%s"), Reader0.GetStringValue(6));
		wsprintf(sRec.data, _T("%s"), Reader0.GetStringValue(7));
		//LPCTSTR strValue = Reader0.GetStringValue(2);
		m_vecRecLog.push_back(sRec);
		index++;


	}  	
	Reader0.Close();
	return true;
}
bool CDataM::InsertCall(SRecord& rec)
{
	TCHAR sql[512] = {0};
	memset(sql,0,sizeof(sql));  
	wsprintf(sql,_T("insert into rectel(分机SIP号,分机地址,分机IP地址,接警电话,\
		隧道,桩号,公里号,日期) values('%s','%d','%s','%s','%s','%s','%s','%s')"),\
		rec.sip, rec.addr, rec.IP, rec.desc, rec.tunnel, rec.noid, rec.kmid, rec.data);  
	if(!m_sqlite.ExcuteNonQuery(sql))  
	{  
		CString strError = m_sqlite.GetLastErrorMsg();  
		return false;
	}  
	return true;
}
bool CDataM::InsertMsg(SRecord& rec)
{
	TCHAR sql[512] = {0};
	memset(sql,0,sizeof(sql));  
	wsprintf(sql,_T("insert into recmsg(分机SIP号,分机地址,分机IP地址,广播类型,\
					隧道,桩号,公里号,日期) values('%s','%d','%s','%s','%s','%s','%s','%s')"),\
					rec.sip, rec.addr, rec.IP, rec.desc, rec.tunnel, rec.noid, rec.kmid, rec.data);  
	if(!m_sqlite.ExcuteNonQuery(sql))  
	{  
		CString strError = m_sqlite.GetLastErrorMsg();  
		return false;
	}  
	return true;
}
bool CDataM::InsertLog(SRecord& rec)
{
	TCHAR sql[512] = {0};
	memset(sql,0,sizeof(sql));  
	wsprintf(sql,_T("insert into reclog(分机SIP号,分机地址,分机IP地址,状态,\
					隧道,桩号,公里号,日期) values('%s','%d','%s','%s','%s','%s','%s','%s')"),\
					rec.sip, rec.addr, rec.IP, rec.desc, rec.tunnel, rec.noid, rec.kmid, rec.data);  
	if(!m_sqlite.ExcuteNonQuery(sql))  
	{  
		CString strError = m_sqlite.GetLastErrorMsg();  
		return false;
	}  
	return true;
}
bool CDataM::InsertLogs(vector<SRecord>& vecRecord)
{

	m_sqlite.BeginTransaction();
	// 批量插入数据   
	for(int i=0; i < vecRecord.size(); i++)  
	{  
		TCHAR sql[512] = {0};
		memset(sql,0,sizeof(sql));  
		wsprintf(sql,_T("insert into reclog(分机SIP号,分机地址,分机IP地址,状态,\
						隧道,桩号,公里号,日期) values('%s','%d','%s','%s','%s','%s','%s','%s')"),\
						vecRecord[i].sip, vecRecord[i].addr, vecRecord[i].IP, vecRecord[i].desc, vecRecord[i].tunnel, vecRecord[i].noid, vecRecord[i].kmid, vecRecord[i].data);  
		if(!m_sqlite.ExcuteNonQuery(sql))  
		{  
			_tprintf(_T("%s\n"),m_sqlite.GetLastErrorMsg());  
			return false;  
		}  
	}  
	// 提交事务   
	m_sqlite.CommitTransaction(); 
	return true;
}

void TcharToChar (const TCHAR * tchar, char * _char)  
{  
	int iLength ;  
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);  
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);   
} 
//同上   
void CharToTchar (const char * _char, TCHAR * tchar)  
{  
	int iLength ;  

	iLength = MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, NULL, 0) ;  
	MultiByteToWideChar (CP_ACP, 0, _char, strlen (_char) + 1, tchar, iLength) ;  
} 

int CDataM::AddSong(char* szName, char* szPath)
{
	SListSong song;
	song.id = m_Songid++;
	memcpy(song.szName,szName,strlen(szName) * sizeof(char));
	memcpy(song.szPath,szPath,strlen(szPath) * sizeof(char));
	m_vecListSong.push_back(song);
	return m_vecListSong.size();
}
SListSong* CDataM::GetSongList(int& nNum)
{
	nNum = 0;
	if (m_vecListSong.empty())
	{		
		return NULL;
	}
	vector<SListSong>::iterator vecIter = m_vecListSong.begin();
	nNum = m_vecListSong.size();
	return &(*vecIter);
}
void CDataM::ClearSongList()
{
	if (!m_vecListSong.empty()){

		m_vecListSong.clear();
	}
	m_Songid = 1;
	
}
bool CDataM::IsTNameExist(char* TName)
{
	for (int i = 0; i < m_vecTempT.size(); i++)
	{
		if (strcmp(TName, m_vecTempT[i].tunnelName) == 0)
		{
			return true;
		}
	}
	return false;
}
int CDataM::GetTempID(char* TName)
{
	int tid = -1;
	for(int i = 0; i < m_vecTempT.size(); i++){
		if (strcmp(TName, m_vecTempT[i].tunnelName) == 0)
		{
			return m_vecTempT[i].tid;
		}
	}
	return tid;
}
void CDataM::SetTempTID()
{
	if (m_vecDB.empty())
	{
		return;
	}
	if (!m_vecTempT.empty())
	{
		m_vecTempT.clear();
	}
	m_nTempTID = 1000;
	for(int i = 0; i < m_vecDB.size(); i++){
		if (!IsTNameExist(m_vecDB[i].tunnelName))
		{
			STempTID temp;
			memset(&temp, 0, sizeof(STempTID));
			temp.tid = m_nTempTID++;
			strcpy(temp.tunnelName, m_vecDB[i].tunnelName);
			m_vecTempT.push_back(temp);
		}
	}
	// set
	for(int i = 0; i < m_vecDB.size(); i++){
		m_vecDB[i].tempTID = GetTempID(m_vecDB[i].tunnelName);
	}
}
int CDataM::GetPlayListSz()
{
	return m_vecPlayList.size();
}
char* CDataM::GetPlayFile(int idx)
{
	if (m_vecPlayList.empty())
	{
		return NULL;
	}
	if (idx < 0 || idx >= m_vecPlayList.size())
	{
		return NULL;
	}
	return m_vecPlayList[idx].szPath;
}
void CDataM::ClearPlayList()
{
	if (!m_vecPlayList.empty())
	{
		m_vecPlayList.clear();
	}
}
void CDataM::AddPlayList(SListSong& song)
{
	m_vecPlayList.push_back(song);
}
void CDataM::DelPlayList(int idx)
{
	if (!m_vecPlayList.empty()){	
		m_vecPlayList.erase(m_vecPlayList.begin() + idx);
	}
}
SListSong* CDataM::GetPlayList(int& nNum)
{	
	if (m_vecPlayList.empty())
	{
		nNum = 0;
		return NULL;
	}
	else{
		nNum = m_vecPlayList.size();
		vector<SListSong>::iterator vecIter = m_vecPlayList.begin();
		return &(*vecIter);
	}	
}
SPlayConfig* CDataM::GetAllPlayConfig(int& nNum)
{
	if (m_vecPlayConfig.empty())
	{
		nNum = 0;
		return NULL;
	}
	else{
		nNum = m_vecPlayConfig.size();
		vector<SPlayConfig>::iterator vecIter = m_vecPlayConfig.begin();
		return &(*vecIter);
	}	
}
void CDataM::DeletePlayConfig(int extAddr)
{
	for (int i = 0; i < m_vecPlayConfig.size(); )
	{
		if (m_vecPlayConfig[i].extAddr == extAddr)
		{
			m_vecPlayConfig.erase(m_vecPlayConfig.begin() + i);
		}
		else{
			i++;
		}
	}
	
}
void CDataM::ClearPlayConfig()
{
	if (!m_vecPlayConfig.empty())
	{
		m_vecPlayConfig.clear();
	}
}
void CDataM::AddPlayConfig(SPlayConfig& config)
{
	SPlayConfig* pConfig = GetPlayConfig(config.extAddr);
	if (pConfig)
	{
		memcpy(pConfig, &config, sizeof(SPlayConfig));
	}
	else{
		m_vecPlayConfig.push_back(config);
	}
	
	
}
SPlayConfig* CDataM::GetPlayConfig(int extAddr)
{
	for (int i = 0; i < m_vecPlayConfig.size(); i++)
	{
		if (m_vecPlayConfig[i].extAddr == extAddr)
		{
			vector<SPlayConfig>::iterator vecIter = m_vecPlayConfig.begin() + i;
			return &(*vecIter);
		}
	}
	return NULL;
}
void CDataM::Config2Device(SDevice*& pDevice)
{
	if (pDevice == NULL)
	{
		return;
	}
	SPlayConfig* pConfig = GetPlayConfig(pDevice->sDB.extAddr);
	if (pConfig == NULL)
	{
		return;
	}
	pDevice->playSrc = pConfig->playSrc;
	pDevice->playMode = pConfig->playMode;
	memcpy(pDevice->playFile, pConfig->playFile, MAX_PATH * sizeof(char));
	pDevice->curPlayIdx = pConfig->curPlayIdx;

}
void CDataM::SelectDeviceBeat()
{
	m_LockBeat.Lock();
	if (!m_vecDeviceBeat.empty())
	{
		m_vecDeviceBeat.clear();
	}
	for (int i = 0; i < m_vecTunnel.size(); i++)
	{
		for (int j = 0; j < m_vecTunnel[i].vecDeviceUp.size(); j++ )
		{
			enBCOptStatus opt = m_vecTunnel[i].vecDeviceUp[j].IsBcPlay();
			if (opt == ENUM_BC_OPT_REMOTE || 
				opt == ENUM_BC_OPT_LOCAL)
			{
				SDeviceBeatInfo info;
				info.extAddr = m_vecTunnel[i].vecDeviceUp[j].sDB.extAddr;
				info.opt = opt;
				m_vecDeviceBeat.push_back(info);
			}
		}
		for (int j = 0; j < m_vecTunnel[i].vecDeviceDown.size(); j++ )
		{
			enBCOptStatus opt = m_vecTunnel[i].vecDeviceDown[j].IsBcPlay();
			if (opt == ENUM_BC_OPT_REMOTE ||
				opt == ENUM_BC_OPT_LOCAL)
			{
				SDeviceBeatInfo info;
				info.extAddr = m_vecTunnel[i].vecDeviceDown[j].sDB.extAddr;
				info.opt = m_vecTunnel[i].vecDeviceDown[j].IsBcPlay();
				m_vecDeviceBeat.push_back(info);
			}
		}
	}
	m_LockBeat.Unlock();
}
void CDataM::UpdateReplay(int extAddr, SBeatReply* pBeatReply)
{
	if (pBeatReply == NULL)
	{
		return;
	}
	m_LockBeat.Lock();
	if (!m_vecDeviceBeat.empty())
	{
		for (int i = 0; i < m_vecDeviceBeat.size(); i++)
		{
			if (m_vecDeviceBeat[i].extAddr == extAddr)
			{
				m_vecDeviceBeat[i].bReply = true;
				m_vecDeviceBeat[i].bc = pBeatReply->bc;
				break;
			}
		}
	}
	m_LockBeat.Unlock();
}
SDeviceBeatInfo* CDataM::GetDeviceBeatInfo(int& nNum)
{
	nNum = 0;
	if (m_vecDeviceBeat.empty())
	{
		return NULL;
	}
	nNum = m_vecDeviceBeat.size();
	vector<SDeviceBeatInfo>::iterator vecIter = m_vecDeviceBeat.begin();
	return &(*vecIter);
}
UINT SendDeviceBeatProc(LPVOID pParam)  
{  
	CDataM* pAPP = (CDataM*)pParam;
	if (pAPP == NULL)
	{
		return 0;
	}
	CWinThread* pThread = AfxGetThread();  
	if (pThread == NULL)  
	{  
		return 0;
	} 
	int nNum = 0;
	SDeviceBeatInfo* pDeviceBeat = pAPP->GetDeviceBeatInfo(nNum);
	for (int m = 0; m < 3; m++)
	{
		for (int i = 0; i < nNum; i++)
		{
			if (g_bExitApp)
			{
				return 0;
			}
			if (!pDeviceBeat[i].bReply)
			{
				//if (pDeviceBeat[i].opt == ENUM_BC_OPT_REMOTE){ // 远程的检查有心跳吗
				g_MSG.SendDeviceBeat(pDeviceBeat[i].extAddr);
				//}
				//else if(pDeviceBeat[i].opt == ENUM_BC_OPT_LOCAL){// 本地不发给本地
				//	pDeviceBeat[i].bReply = true;
				//}				
				m_pAPP->SleepThread(10);
			}		
		}
	}
	if (!m_pAPP->SleepThread(10*1000, true))
	{
		return 0;
	}
	bool bNoReplay = false;
	for (int i = 0; i < nNum; i++)
	{
		if (!pDeviceBeat[i].bReply)
		{
			bNoReplay = true;
			break;
		}		
	}
	if (bNoReplay)// 等待更新
	{
		if (!m_pAPP->SleepThread(20*1000, true))
		{
			return 0;
		}
	}
	// 关闭
	for (int i = 0; i < nNum; i++)
	{
		if (g_bExitApp)
		{
			return 0;
		}

		if (!pDeviceBeat[i].bReply)
		{
			SDevice* pDevice = m_pAPP->GetDevice(pDeviceBeat[i].extAddr);
			if (pDevice)
			{
				g_Log.output(LOG_TYPE, "SendCloseBroadcast,等待更新，关闭:%d\r\n",pDevice->sDB.extAddr);
				g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice, false);
				Sleep(300);
				g_MSG.SendHangUp(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
				pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
				pDevice->callStatus = ENUM_DEVICE_STATUS_OK;		
			}
		}
		else{
			if (pDeviceBeat[i].opt == ENUM_BC_OPT_LOCAL){
				// 直接推送状态
				g_MSG.SendDeviceUpdateStatus(pDeviceBeat[i].extAddr, ENUM_DEVICE_STATUS_RUN);
			}
			else if (pDeviceBeat[i].opt == ENUM_BC_OPT_REMOTE){
				// 不是广播状态，关闭
				if (pDeviceBeat[i].bc != ENUM_DEVICE_STATUS_RUN)
				{
					SDevice* pDevice = m_pAPP->GetDevice(pDeviceBeat[i].extAddr);
					if (pDevice)
					{
						g_Log.output(LOG_TYPE, "SendCloseBroadcast,不是广播状态，关闭:%d\r\n", pDevice->sDB.extAddr);
						g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice, false);
						m_pAPP->SleepThread(300);
						g_MSG.SendHangUp(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice);
						pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
						pDevice->callStatus = ENUM_DEVICE_STATUS_OK;
					}
				}
			}
		}
	}
	m_pAPP->Status2Button();
	return 1;
}
void CDataM::SendDeviceBeat()
{
	AfxBeginThread(SendDeviceBeatProc, this);
}
