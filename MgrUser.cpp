#include "StdAfx.h"
#include "MgrUser.h"
#include "DataM.h"

CMgrUser::CMgrUser(void)
{
	
}


CMgrUser::~CMgrUser(void)
{
	if (m_vecUser.empty())
	{
		m_vecUser.clear();
	}
}
bool CMgrUser::ReadUser(char* path)
{
	if (m_vecUser.empty())
	{
		m_vecUser.clear();
	}
	m_vecUser.reserve(50);
	HANDLE hFile = CreateFileA(path, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		InitUser();
		return false;
	}
	DWORD dwSz = GetFileSize(hFile, NULL);
	int recNum = dwSz / sizeof(SUsrInfo);
	if (recNum <= 0)
	{
		InitUser();
		CloseHandle(hFile);
		return false;
	}
	else{
		ZBase64 bs64;
		for (int i = 0; i < recNum; i++)
		{
			SUsrInfo info;
			DWORD dwRead;
			ReadFile(hFile, &info, sizeof(SUsrInfo), &dwRead, NULL);
			int len = 0;
			sprintf(info.name, "%s", bs64.Decode(info.name, strlen(info.name), len).c_str());
			sprintf(info.pwd, "%s", bs64.Decode(info.pwd, strlen(info.pwd), len).c_str());
			m_vecUser.push_back(info);
		}
		CloseHandle(hFile);
		return true;
	}
	
}
void  CMgrUser::SaveUser(char* path)
{
	HANDLE hFile = CreateFileA(path, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		InitUser();
		return;
	}
	ZBase64 bs64;
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		SUsrInfo info;
		info.type = m_vecUser[i].type;
		sprintf(info.name, "%s", bs64.Encode((unsigned char*)(m_vecUser[i].name), strlen(m_vecUser[i].name)).c_str());
		sprintf(info.pwd, "%s", bs64.Encode((unsigned char*)(m_vecUser[i].pwd), strlen(m_vecUser[i].pwd)).c_str());
		DWORD dwRead;
		WriteFile(hFile, &info, sizeof(SUsrInfo), &dwRead, NULL);		
	}
	CloseHandle(hFile);
}
void  CMgrUser::ModifySuperPWD(char* pwd)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (m_vecUser[i].type == ENUM_TYPE_SUPER)
		{
			sprintf(m_vecUser[i].pwd, pwd);
			break;
		}
	}
}
void  CMgrUser::ModifyAdminPWD(char* pwd)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (m_vecUser[i].type == ENUM_TYPE_ADMIN)
		{
			sprintf(m_vecUser[i].pwd, pwd);
			break;
		}
	}
}
void  CMgrUser::ModifyUserPWD(char* name, char* pwd)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (m_vecUser[i].type == ENUM_TYPE_USER)
		{
			if (strcmp(m_vecUser[i].name, name) == 0)
			{
				sprintf(m_vecUser[i].pwd, pwd);
				break;
			}
		}
	}
}
bool  CMgrUser::AddUser(char* name, char* pwd)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (true/*m_vecUser[i].type == ENUM_TYPE_USER*/)
		{
			if (strcmp(m_vecUser[i].name, name) == 0)
			{
				return false;
			}
		}
	}
	SUsrInfo infoUser;
	infoUser.type = ENUM_TYPE_USER;
	sprintf(infoUser.name, "%s", name);
	sprintf(infoUser.pwd, "%s", pwd);
	m_vecUser.push_back(infoUser);
	return true;
}
bool  CMgrUser::RemoveUser(char* name)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (m_vecUser[i].type == ENUM_TYPE_USER)
		{
			if (strcmp(m_vecUser[i].name, name) == 0)
			{
				m_vecUser.erase(m_vecUser.begin() + i);
				return true;
			}
		}
	}
	return false;
}
void  CMgrUser::InitUser()
{
	// super
	SUsrInfo infoSuper;
	infoSuper.type = ENUM_TYPE_SUPER;
	sprintf(infoSuper.name, "%s", "root");
	sprintf(infoSuper.pwd, "%s", "root");
	m_vecUser.push_back(infoSuper);
	// admin
	SUsrInfo infoAdmin;
	infoAdmin.type = ENUM_TYPE_ADMIN;
	sprintf(infoAdmin.name, "%s", "admin");
	sprintf(infoAdmin.pwd, "%s", "admin");
	m_vecUser.push_back(infoAdmin);
	// user
	SUsrInfo infoUser;
	infoUser.type = ENUM_TYPE_USER;
	sprintf(infoUser.name, "%s", "user");
	sprintf(infoUser.pwd, "%s", "user");
	m_vecUser.push_back(infoUser);
}
SUsrInfo* CMgrUser::GetUser(int& nNum)
{
	nNum = 0;
	if (m_vecUser.empty())
	{
		return NULL;
	}
	else{
		nNum = m_vecUser.size();
		vector<SUsrInfo>::iterator vecIter = m_vecUser.begin();
		return &(*vecIter);
	}
}
void CMgrUser::ModifyPWD(char* name, char* pwd)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (strcmp(m_vecUser[i].name, name) == 0)
		{
			sprintf(m_vecUser[i].pwd, pwd);
			break;
		}
	}
}
int CMgrUser::GetUserType(char* name)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (strcmp(m_vecUser[i].name, name) == 0)
		{
			return m_vecUser[i].type;
		}
	}
	return -1;
}
void CMgrUser::GetUser(char* name, SUsrInfo& user)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (strcmp(m_vecUser[i].name, name) == 0)
		{
			memcpy(&user, &(m_vecUser[i]), sizeof(SUsrInfo));
			break;
		}
	}
	
}
void CMgrUser::ResetAllPwd()
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		memset(m_vecUser[i].pwd, 0, 100 * sizeof(char));
		sprintf(m_vecUser[i].pwd, m_vecUser[i].name);	
	}
}
void CMgrUser::ResetPwd(char* name)
{
	for (int i = 0; i < m_vecUser.size(); i++)
	{
		if (strcmp(m_vecUser[i].name, name) == 0)
		{
			memset(m_vecUser[i].pwd, 0, 100 * sizeof(char));
			sprintf(m_vecUser[i].pwd, m_vecUser[i].name);
		}
			
	}
}