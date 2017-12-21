#include "StdAfx.h"
#include "MgrPlay.h"
#include "SIPCallDlg.h"

CMgrPlay::CMgrPlay(void)
{
	m_vecPlayGroup.reserve(200);
	m_nGroupID = 100;
}


CMgrPlay::~CMgrPlay(void)
{
	Release();
}
SPlayGroup*  CMgrPlay::AddNew(SPlayGroup* pPlay, int nNum)
{
#if 0
	SPlayGroup* pPlayGroup = FindPlayGroup(play.vecDevice);
	if (pPlayGroup)
	{
		// 更新
		UpdateAttr(&(pPlayGroup->attr), &(play.attr));
	}
	else{
		// 删除存在的设备
		Delete(play);
		// 添加
		m_vecPlayGroup.push_back(play);
	}	
#else
	int preSz = m_vecPlayGroup.size();
	for (int i = 0; i < nNum; i++)
	{
		pPlay[i].GroupID = m_nGroupID++;		
		m_vecPlayGroup.push_back(pPlay[i]);
	}
	vector<SPlayGroup>::iterator vecIter = m_vecPlayGroup.begin() + preSz;
	return &(*vecIter);
#endif
}
bool less_attr(const SPlayDeviceAttr& item1, const SPlayDeviceAttr& item2)
{
	if (item1.attr.enSrc == item2.attr.enSrc)
	{
		if (item1.attr.enModel == item2.attr.enModel)
		{
			if (strcmp(item1.attr.pFile, item2.attr.pFile) == 0)
			{
				return item1.attr.curPlayIndex < item2.attr.curPlayIndex;
			} 
			else
			{
				return strcmp(item1.attr.pFile, item2.attr.pFile) < 0;
			}
		}
		else{
			return item1.attr.enModel < item2.attr.enModel;
		}
	}
	else{
		return item1.attr.enSrc < item2.attr.enSrc;
	}
}
int CMgrPlay::ReDividePlayGroup(vector<SPlayDeviceAttr>& vecDevice, SPlayGroup*& pPlayGroup)
{
	SPlayGroup* pDivideGroup = NULL;
	int groupNum = DividePlayGroup(vecDevice, pDivideGroup);
	// 分组后，删除存在的设备
	DeleteDevice(pDivideGroup, groupNum);
	// 再放入分组
	pPlayGroup = AddNew(pDivideGroup, groupNum);
	// 释放资源
	if (pDivideGroup != NULL)
	{
		FreePlayGroup(pDivideGroup);
		pDivideGroup = NULL;
	}
	return groupNum;
}
void CMgrPlay::Clear(SPlayGroup* pPlayGroup)
{
	if (pPlayGroup == NULL)
	{
		return;
	}
	if (!pPlayGroup->vecDevice.empty())
	{
		pPlayGroup->vecDevice.clear();
	}
}
int CMgrPlay::DividePlayGroup(vector<SPlayDeviceAttr>& vecDevice, SPlayGroup*& pPlayGroup)
{	
	// 排序
	if (!vecDevice.empty())
	{
		sort(vecDevice.begin(), vecDevice.end(), less_attr);

		pPlayGroup = new SPlayGroup[vecDevice.size()];
		// 把0先放入	
		int nGidx = 0;
		UpdateAttr(&(pPlayGroup[nGidx].attr), &(vecDevice[0].attr));
		SPlayDevice device;
		device.extAddr = vecDevice[0].extAddr;
		device.ip = vecDevice[0].ip;
		pPlayGroup[nGidx].vecDevice.push_back(device);	
		nGidx++;
		// 划分
		for (int i = 1; i < vecDevice.size(); i++)
		{
			if (IsAttrEqual(pPlayGroup[nGidx].attr, vecDevice[i].attr))
			{
				// 属性相同就加入device
				SPlayDevice device;
				device.extAddr = vecDevice[i].extAddr;
				device.ip = vecDevice[i].ip;
				pPlayGroup[nGidx].vecDevice.push_back(device);					
			}
			else{ // 不同就创建组
				
				UpdateAttr(&(pPlayGroup[nGidx].attr), &(vecDevice[i].attr));
				SPlayDevice device;
				device.extAddr = vecDevice[i].extAddr;
				device.ip = vecDevice[i].ip;
				pPlayGroup[nGidx].vecDevice.push_back(device);
				nGidx++;
			
			}
		}
		// 		
		return nGidx;
	}
	else{
		pPlayGroup = NULL;
		return 0;
	}
}
void CMgrPlay::FreePlayGroup(SPlayGroup* pPlayGroup)
{
	if (pPlayGroup != NULL)
	{
		Clear(pPlayGroup);
		delete[] pPlayGroup;
		pPlayGroup = NULL;
	}
}
int CMgrPlay::ReDividePlayGroup(vector<SPlayDevice>& vecDevice, SPlayGroup*& pPlayGroup)
{
	SPlayGroup* pPlay = FindPlayGroup(vecDevice);
	if (pPlay)
	{
		pPlayGroup = pPlay;
		return 1;
	}
	else{
		// 拆分成group
		vector<SPlayDeviceAttr> vecTempDevice;
		for (int i = 0; i < vecDevice.size(); i++)
		{			
			SPlayGroup* pPlayGroup = FindPlayGroup(vecDevice[i].ip);
			if (pPlayGroup)
			{
				SPlayDeviceAttr device;
				device.extAddr = vecDevice[i].extAddr;
				device.ip = vecDevice[i].ip;
				UpdateAttr(&(device.attr), &(pPlayGroup->attr));				
				vecTempDevice.push_back(device);
			}
			else{
				g_Log.output(LOG_TYPE, "FindPlayGroup fail: %d....\r\n", vecDevice[i].ip);
			}
		}
		// 排序
		if (!vecTempDevice.empty())
		{
			sort(vecTempDevice.begin(), vecTempDevice.end(), less_attr);

			vector<SPlayGroup> vecPlayGroup;
			// 把0先放入
			SPlayGroup group;
			UpdateAttr(&(group.attr), &(vecTempDevice[0].attr));
			SPlayDevice device;
			device.extAddr = vecTempDevice[0].extAddr;
			device.ip = vecTempDevice[0].ip;
			group.vecDevice.push_back(device);
			vecPlayGroup.push_back(group);
			// 划分
			for (int i = 1; i < vecTempDevice.size(); i++)
			{
				if (IsAttrEqual(vecPlayGroup[vecPlayGroup.size() - 1].attr, vecTempDevice[i].attr))
				{
					// 属性相同就加入device
					SPlayDevice device;
					device.extAddr = vecTempDevice[i].extAddr;
					device.ip = vecTempDevice[i].ip;
					vecPlayGroup[vecPlayGroup.size() - 1].vecDevice.push_back(device);					
				}
				else{ // 不同就创建组
					SPlayGroup group;
					UpdateAttr(&(group.attr), &(vecTempDevice[i].attr));
					SPlayDevice device;
					device.extAddr = vecTempDevice[i].extAddr;
					device.ip = vecTempDevice[i].ip;
					group.vecDevice.push_back(device);
					vecPlayGroup.push_back(group);
				}
			}
			// 
			vector<SPlayGroup>::iterator vecIter = vecPlayGroup.begin();
			pPlayGroup = &(*vecIter);
			return vecPlayGroup.size();
		}
		return 0;
	}	
	
}
void CMgrPlay::Delete(vector<DWORD>& vecIP)
{
	// 删除存在的设备
	for (int j = 0; j < vecIP.size(); j++)
	{
		DWORD preDeviceIP = vecIP[j];
		for (int m = 0; m < m_vecPlayGroup.size(); )
		{			
			for (int k = 0; k < m_vecPlayGroup[m].vecDevice.size(); )
			{
				if (preDeviceIP == m_vecPlayGroup[m].vecDevice[k].ip)
				{					
					m_vecPlayGroup[m].vecDevice.erase(m_vecPlayGroup[m].vecDevice.begin() + k);					
				}
				else{
					k++;
				}
			}	
			// 删除后可能出现空记录
			if (m_vecPlayGroup[m].vecDevice.empty())
			{
				m_vecPlayGroup.erase(m_vecPlayGroup.begin() + m);
			}
			else{
				m++;
			}
		}		
	}
}
void CMgrPlay::DeleteDevice(SPlayGroup* pPlayGroup, int nNum)
{
	for (int i = 0; i < nNum; i++)
	{
		DeleteDevice(pPlayGroup[i]);
	}
}
void CMgrPlay::DeleteDevice(SPlayGroup& play)
{
	// 删除存在的设备
	for (int j = 0; j < play.vecDevice.size(); j++)
	{
		DWORD newDeviceIP = play.vecDevice[j].ip;
		for (int m = 0; m < m_vecPlayGroup.size(); )
		{			
			for (int k = 0; k < m_vecPlayGroup[m].vecDevice.size(); )
			{
				if (newDeviceIP == m_vecPlayGroup[m].vecDevice[k].ip)
				{					
					m_vecPlayGroup[m].vecDevice.erase(m_vecPlayGroup[m].vecDevice.begin() + k);					
				}
				else{
					k++;
				}
			}	
			// 删除后可能出现空记录
			if (m_vecPlayGroup[m].vecDevice.empty())
			{
				Clear(&(m_vecPlayGroup[m]));
				m_vecPlayGroup.erase(m_vecPlayGroup.begin() + m);
			}
			else{
				m++;
			}
		}		
	}
}
SPlayGroup* CMgrPlay::FindPlayGroup(int threadID)
{	
	for (int i = 0; i < m_vecPlayGroup.size(); i++)
	{
		if (threadID == m_vecPlayGroup[i].threadID)
		{
			vector<SPlayGroup>::iterator vecIter = m_vecPlayGroup.begin() + i;
			return &(*vecIter);
		}
	}
	return NULL;
}
void CMgrPlay::Release()
{
	if (!m_vecPlayGroup.empty())
	{
		for (int i = 0; i < m_vecPlayGroup.size(); i++)
		{
			Clear(&(m_vecPlayGroup[i]));
		}
		m_vecPlayGroup.clear();
	}
}

bool CMgrPlay::IsDeviceEqual(vector<SPlayDevice>& vecNew, vector<SPlayDevice>& vecStore)
{
	if (vecNew.size() != vecStore.size())
	{
		return false;
	}
	else{
		for (int i = 0; i < vecStore.size(); i++)
		{
			DWORD dwIP = vecStore[i].ip;
			if (!IsDeviceFind(dwIP, vecNew))
			{
				return false;
			}
			
		}
		return true;

	}
}

bool CMgrPlay::IsDeviceFind(DWORD dwIP, vector<SPlayDevice>& vecPlayDevice)
{
	for (int i = 0; i < vecPlayDevice.size(); i++)
	{
		if (dwIP == vecPlayDevice[i].ip)
		{
			return true;
		}
	}
	return false;
}
SPlayGroup* CMgrPlay::FindPlayGroup(vector<SPlayDevice>& vecPlayDevice)
{
	for (int i = 0; i < m_vecPlayGroup.size(); i++)
	{
		if (IsDeviceEqual(vecPlayDevice, m_vecPlayGroup[i].vecDevice))
		{
			vector<SPlayGroup>::iterator vecIter = m_vecPlayGroup.begin() + i;
			return &(*vecIter);
		}
	}
	return NULL;
}
SPlayGroup* CMgrPlay::FindPlayGroup(DWORD dwIP)
{
	for (int i = 0; i < m_vecPlayGroup.size(); i++)
	{
		for (int j = 0; j < m_vecPlayGroup[i].vecDevice.size(); j++)
		{
			if (dwIP == m_vecPlayGroup[i].vecDevice[j].ip)
			{
				vector<SPlayGroup>::iterator vecIter = m_vecPlayGroup.begin() + i;
				return &(*vecIter);
			}
		}		
	}
	return NULL;
}
void CMgrPlay::UpdateAttr(SPlayAttr* pDest, SPlayAttr* pSrc)
{	
	pDest->enSrc = pSrc->enSrc;
	switch (pDest->enSrc)
	{
	case ENUM_MODEL_FILE:
		pDest->enModel = pSrc->enModel;
		memcpy(pDest->pFile, pSrc->pFile, MAX_PATH * sizeof(char));
		pDest->curPlayIndex = 0;
		break;
	case ENUM_MODEL_FILE_LIST:
		pDest->enModel = pSrc->enModel;
		memset(pDest->pFile, 0, MAX_PATH * sizeof(char));
		pDest->curPlayIndex = pSrc->curPlayIndex;
		break;
	case ENUM_MODEL_TTS:
		pDest->enModel = pSrc->enModel;
		memcpy(pDest->pFile, pSrc->pFile, MAX_PATH * sizeof(char));
		pDest->curPlayIndex = 0;
		break;
	case ENUM_MODEL_AUX:
		pDest->enModel = ENUM_PLAY_MODEL_NONE;
		memset(pDest->pFile, 0, MAX_PATH * sizeof(char));
		pDest->curPlayIndex = 0;
		break;
	case ENUM_MODEL_MIC:
		pDest->enModel = ENUM_PLAY_MODEL_NONE;
		memset(pDest->pFile, 0, MAX_PATH * sizeof(char));
		pDest->curPlayIndex = 0;
		break;
	default:
		pDest->enModel = ENUM_PLAY_MODEL_NONE;
		memset(pDest->pFile, 0, MAX_PATH * sizeof(char));
		pDest->curPlayIndex = 0;
	}
}
bool CMgrPlay::IsAttrEqual(const SPlayAttr& item1, const SPlayAttr& item2)
{
	if (item1.enSrc == item2.enSrc)
	{
		switch(item1.enSrc){
		case ENUM_MODEL_FILE:
			if (item1.enModel == item2.enModel &&
				strcmp(item1.pFile, item2.pFile) == 0)
			{
				return true;
			}
			else{
				return false;
			}
			break;
		case ENUM_MODEL_FILE_LIST:
			if (item1.enModel == item2.enModel &&
				item1.curPlayIndex == item2.curPlayIndex)
			{
				return true;
			}
			else{
				return false;
			}
			break;
		case ENUM_MODEL_TTS:
			if (item1.enModel == item2.enModel &&
				strcmp(item1.pFile, item2.pFile) == 0)
			{
				return true;
			}
			else{
				return false;
			}
			break;
		case ENUM_MODEL_MIC:
		case ENUM_MODEL_AUX:
			return true;
			break;
		default:
			return true;
			break;
		};
	}
	else{
		return false;
	}
}

void CMgrPlay::PlayGroups(SPlayGroup* pGroup, int nNum)
{
	for (int i = 0; i < nNum; i++)
	{
		PlayOneGroup(pGroup);
	}
	
}
void CMgrPlay::PlayFile(SPlayGroup* pGroup, bool RePlay)
{
	vector<DWORD> vecIP;
	for (int j = 0; j < pGroup->vecDevice.size(); j++)
	{
		vecIP.push_back(pGroup->vecDevice[j].ip);
	}
	pGroup->threadID = g_Play.play(pGroup->attr.pFile, pGroup->param, vecIP);		
	vecIP.clear();	

	if (RePlay)
	{
		return;
	}
	if (pGroup->vecDevice.size() > 0)
	{
		pGroup->playTime = GetTickCount();
	}
	if (pGroup->threadID)	{//成功
		for(int i = 0; i < pGroup->vecDevice.size(); i++){
			SDevice* pDevice = m_pAPP->GetDevice(pGroup->vecDevice[i].ip);
			if (pDevice)
			{
				USHORT port = g_Config.m_SystemConfig.portDevice;
				g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, port);	
				g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
				pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;				
				pDevice->playFlag = true;
				m_pAPP->AddMsgRecord(*pDevice, pGroup->attr.enSrc);
			}
		}
	}
	else{//失败

	}

}
void CMgrPlay::PlayAUX(SPlayGroup* pGroup)
{
	for(int i = 0; i < pGroup->vecDevice.size(); i++){
		SDevice* pDevice = m_pAPP->GetDevice(pGroup->vecDevice[i].ip);
		if (pDevice)
		{
			g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_AUX);
			
			USHORT port = g_Config.m_SystemConfig.portDevice;
			g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, port);	
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;			
			pDevice->playFlag = true;
			m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_AUX);
		}
	}		
	if (pGroup->vecDevice.size() > 0)
	{
		pGroup->playTime = GetTickCount();
	}
}
void CMgrPlay::PlayMIC(SPlayGroup* pGroup)
{
	for(int i = 0; i < pGroup->vecDevice.size(); i++){
		SDevice* pDevice = m_pAPP->GetDevice(pGroup->vecDevice[i].ip);
		if (pDevice)
		{
			
			g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);
			
			USHORT port = g_Config.m_SystemConfig.portDevice;
			g_MSG.SendOpenBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, port);	
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_OPEN);
			pDevice->bcStatus = ENUM_DEVICE_STATUS_RUN;		
			pDevice->playFlag = true;
			m_pAPP->AddMsgRecord(*pDevice, ENUM_MODEL_MIC);
			
		}
	}		
	if (pGroup->vecDevice.size() > 0)
	{
		pGroup->playTime = GetTickCount();
	}
}

void CMgrPlay::SendStopGroup(SPlayGroup* pGroup, int nNum, enStopMsg msg)
{
	for (int i = 0; i < nNum; i ++)
	{
		SendStopGroup(pGroup, msg);
	}
}
void CMgrPlay::PlayOneGroup(SPlayGroup* pGroup)
{
	enPlaySrc src = pGroup->attr.enSrc;
	switch(src){
	case ENUM_MODEL_FILE:
		PlayFile(pGroup, false);			
		break;
	case ENUM_MODEL_FILE_LIST:
		{
			char* pFile = g_data.GetPlayFile(pGroup->attr.curPlayIndex);
			if (pFile)
			{
				sprintf(pGroup->attr.pFile, pFile);
				PlayFile(pGroup, false);				
			}		

		}		
		break;
	case ENUM_MODEL_TTS:
		WideCharToMultiByte(CP_ACP, 0, g_ttsFilePath, -1, pGroup->attr.pFile, MAX_PATH, NULL, NULL);
		PlayFile(pGroup, false);		
		break;
	case ENUM_MODEL_MIC:
		PlayMIC(pGroup);		
		break;
	case ENUM_MODEL_AUX:
		PlayAUX(pGroup);			
		break;
	};
}
void CMgrPlay::EndGroup(SPlayGroup* pGroup)
{
	for(int i = 0; i < pGroup->vecDevice.size(); i++){
		SDevice* pDevice = m_pAPP->GetDevice(pGroup->vecDevice[i].ip);
		if (pDevice)
		{
			pDevice->threadID = 0;
			pDevice->playTime = 0;
			pDevice->playFlag = 0;
			pDevice->bcStatus = ENUM_DEVICE_STATUS_OK;
			pDevice->Check = 0;
			// 关广播				
			if (pGroup->attr.enSrc == ENUM_MODEL_AUX || pGroup->attr.enSrc == ENUM_MODEL_MIC)
			{
				g_MSG.SendTempGroup(pDevice->sDB.ip, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_NONE);
			}
			g_Log.output(LOG_TYPE, "SendCloseBroadcast,EndGroup:%d\r\n",pDevice->sDB.extAddr);
			g_MSG.SendCloseBroadcast(pDevice->sDB.extAddr, pDevice->sDB.ip, g_Config.m_SystemConfig.portDevice, false);
			g_MSG.SendCmd(atoi(pDevice->sDB.extNo), ENUM_CMD_CODE_BC_CLOSE);		
		}
	}	
	Remove(pGroup);
}
void CMgrPlay::SendStopGroup(SPlayGroup* pGroup, enStopMsg msg)
{
	enPlaySrc src = pGroup->attr.enSrc;
	pGroup->msg = msg;
	switch(src){
	case ENUM_MODEL_FILE:
	case ENUM_MODEL_FILE_LIST:	
	case ENUM_MODEL_TTS:		
		if (pGroup->threadID)
		{
			g_Play.stop(pGroup->param);
		}		
		break;
	case ENUM_MODEL_MIC:	
	case ENUM_MODEL_AUX:		
		EndGroup(pGroup);
		
		break;
	};
}
void CMgrPlay::MsgStop(int threadID)
{
	SPlayGroup* pGroup = FindPlayGroup(threadID);
	if (pGroup)
	{
		
	}
}


void CMgrPlay::MsgStopGroup(SPlayGroup* pGroup)
{
	switch(pGroup->msg){
	case ENUM_STOP_MSG_NONE:// 自动播放结束
		{
			if (pGroup->attr.enSrc == ENUM_MODEL_FILE){ // 单文件
				if (pGroup->attr.enModel == ENUM_PLAY_MODEL_ONE_ONCE){// 播放一次
					EndGroup(pGroup);	
				} 
				else{// 重复播放
					PlayFile(pGroup, true);					
				}
			}
			else if (pGroup->attr.enSrc == ENUM_MODEL_FILE_LIST){// 列表文件可以单曲播放、顺序播放
				if (pGroup->attr.enModel == ENUM_PLAY_MODEL_ONE_ONCE){
					EndGroup(pGroup);					
				}
				else if(pGroup->attr.enModel == ENUM_PLAY_MODEL_ONE_CONTINUE){//单曲循环					
					PlayFile(pGroup, true);				
				}			
				else if (pGroup->attr.enModel == ENUM_PLAY_MODEL_LIST){//列表				
					pGroup->attr.curPlayIndex++;
					if (pGroup->attr.curPlayIndex >= g_data.GetPlayListSz() || 
						pGroup->attr.curPlayIndex < 0)
					{
						pGroup->attr.curPlayIndex = 0;
					}
					char* pFile = g_data.GetPlayFile(pGroup->attr.curPlayIndex);
					if (pFile)
					{
						sprintf(pGroup->attr.pFile, pFile);
						PlayFile(pGroup, true);
					}
				}
			}
			else if (pGroup->attr.enSrc == ENUM_MODEL_TTS){ // TTS
				if (pGroup->attr.enModel == ENUM_PLAY_MODEL_ONE_ONCE){// 播放一次
					EndGroup(pGroup);					
				} 
				else{// 重复播放
					PlayFile(pGroup, true);					
				}
			}
		}
		break;
	case ENUM_STOP_MSG_CLICK:// 强制停止
		{
			EndGroup(pGroup);
		}
		break;
	case ENUM_STOP_MSG_TIME://  时间停止
		{
			EndGroup(pGroup);
		}
		break;
	case ENUM_STOP_MSG_CHANGE:// 播放切换
		{
			if (pGroup->attr.enSrc == ENUM_MODEL_FILE){ // 单文件

				PlayFile(pGroup, true);		
			}
			else if (pGroup->attr.enSrc == ENUM_MODEL_FILE_LIST){// 列表文件可以单曲播放、顺序播放
				if (pGroup->attr.curPlayIndex >= g_data.GetPlayListSz() || 
					pGroup->attr.curPlayIndex < 0)
				{
					pGroup->attr.curPlayIndex = 0;
				}
				char* pFile = g_data.GetPlayFile(pGroup->attr.curPlayIndex);
				if (pFile)
				{
					sprintf(pGroup->attr.pFile, pFile);
					PlayFile(pGroup, true);		
				}						
				else{
					EndGroup(pGroup);
				}					
			}
			else if (pGroup->attr.enSrc == ENUM_MODEL_TTS){ // TTS
				WideCharToMultiByte(CP_ACP, 0, g_ttsFilePath, -1, pGroup->attr.pFile, MAX_PATH, NULL, NULL);
				PlayFile(pGroup, true);						
			}
			else if (pGroup->attr.enSrc == ENUM_MODEL_MIC)
			{
				PlayMIC(pGroup);
			}
			else if (pGroup->attr.enSrc == ENUM_MODEL_AUX)
			{
				PlayAUX(pGroup);				
			}
			pGroup->msg = ENUM_STOP_MSG_NONE;
		}
		break;
	}

}
bool CMgrPlay::IsExistAUX(SPlayGroup* pGroup, int nNum)
{
	return IsExistSrc(pGroup, nNum, ENUM_MODEL_AUX);
}
bool CMgrPlay::IsExistMIC(SPlayGroup* pGroup, int nNum)
{
	return IsExistSrc(pGroup, nNum, ENUM_MODEL_MIC);
}
bool CMgrPlay::IsExistSrc(SPlayGroup* pGroup, int nNum, enPlaySrc src)
{
	for (int i = 0; i < nNum; i++)
	{
		if (pGroup[i].attr.enSrc == src)
		{
			return true;
		}
	}
	return false;
}
void CMgrPlay::Remove(SPlayGroup* pPlay)
{
	for (int i = 0; i < m_vecPlayGroup.size(); i++)
	{
		if (pPlay->GroupID == m_vecPlayGroup[i].GroupID)
		{
			if (!m_vecPlayGroup[i].vecDevice.empty())
			{
				m_vecPlayGroup[i].vecDevice.clear();
			}
			m_vecPlayGroup.erase(m_vecPlayGroup.begin() + i);
			return;
		}
	}
}
void CMgrPlay::PlayGroup(vector<SPlayDeviceAttr>& vecDeviceAttr)
{
	// 得到分组信息
	SPlayGroup* pPlayGroup;
	int nNum = ReDividePlayGroup(vecDeviceAttr, pPlayGroup);
	// 开始播放
	PlayGroups(pPlayGroup, nNum);
	if (IsExistMIC(pPlayGroup, nNum))
	{
		g_MSG.SendTempGroup(g_Config.m_SystemConfig.IP_MIC_RECORD, g_Config.m_SystemConfig.portAudio, ENUM_TEMP_GROUP_ID_MIC);
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_MIC, portAudio, false);
		g_Play.RecordStart();
	}
	if (IsExistAUX(pPlayGroup, nNum))
	{
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendOpenMic(g_Config.m_SystemConfig.IP_AUX, portAudio, true);	
	}		
}
void CMgrPlay::StopGroup(vector<SPlayDeviceAttr>& vecDeviceAttr, enStopMsg msg)
{
	// 得到分组信息
	SPlayGroup* pPlayGroup;
	int nNum = ReDividePlayGroup(vecDeviceAttr, pPlayGroup);
	// 开始停止
	SendStopGroup(pPlayGroup, nNum, msg);	

	if (IsExistMIC(pPlayGroup, nNum))
	{
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendCloseMic(g_Config.m_SystemConfig.IP_MIC, portAudio);	
		g_Play.RecordStop();
	}
	if (IsExistAUX(pPlayGroup, nNum))
	{
		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
		g_MSG.SendCloseAux(g_Config.m_SystemConfig.IP_AUX, portAudio);	
	}	
}