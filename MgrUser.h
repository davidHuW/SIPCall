#pragma once

#include <vector>
#include <algorithm>
using namespace std;

#include "ZBase64.h"
#include "ConfigEnumDef.h"

struct SUsrInfo{
	enAdminType type;
	char name[100];
	char pwd[100];
	SUsrInfo(){
		type = ENUM_TYPE_USER;
		memset(name, 0, 100 * sizeof(char));
		memset(pwd, 0, 100 * sizeof(char));
	}
};

class CMgrUser
{
public:
	CMgrUser(void);
	~CMgrUser(void);
	bool ReadUser(char* path);
	void SaveUser(char* path);
	void ModifySuperPWD(char* pwd);
	void ModifyAdminPWD(char* pwd);
	void ModifyUserPWD(char* name, char* pwd);
	void ModifyPWD(char* name, char* pwd);
	int GetUserType(char* name);
	void GetUser(char* name, SUsrInfo& user);
	bool AddUser(char* name, char* pwd);
	bool RemoveUser(char* name);
	SUsrInfo* GetUser(int& nNum);
	void ResetAllPwd();
	void ResetPwd(char* name);
private:
	void InitUser();
private:
	vector<SUsrInfo> m_vecUser;
};

