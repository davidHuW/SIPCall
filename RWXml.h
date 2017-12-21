#pragma once

#include <string>
#include <atlstr.h>
using namespace std;

class CRWXml
{
public:
	CRWXml(void);
	~CRWXml(void);
	bool CreateXmlFile(string& szFileName);
	bool ReadXmlFile(CString  szFileName);
private:
	CString GetAppPath();
};
