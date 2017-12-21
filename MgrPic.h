#pragma once
#include <vector>
using namespace std;

struct SPicInfo{
	CString strFile;
	HBITMAP hbitmap;
};
class CMgrPic
{
public:
	CMgrPic(void);
	~CMgrPic(void);
	void LoadPic();
	HBITMAP GetBitmap(CString strFile);
private:	
	void Release();
	void FindFile(CString strFoldername);
	void FindPNGFile(CString strFoldername);
private:
	vector<SPicInfo> m_vecPic;
};

