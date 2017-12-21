#include "StdAfx.h"
#include "MgrPic.h"
#include "SIPCallDlg.h"

CMgrPic::CMgrPic(void)
{
}


CMgrPic::~CMgrPic(void)
{
	Release();
}
void  CMgrPic::FindFile(CString strFoldername)
{
	CString m_cstrFileList=L"";
	CFileFind tempFind;
	BOOL bFound; //判断是否成功找到文件
	bFound = tempFind.FindFile(strFoldername + L"\\*.bmp");   //修改" "内内容给限定查找文件类型
	while(bFound)      //遍历所有文件
	{
		bFound=tempFind.FindNextFile(); //第一次执行FindNextFile是选择到第一个文件，以后执行为选择
		//到下一个文件
		if(tempFind.IsDots()){
			continue; //如果找到的是返回上层的目录 则结束本次查找
		}
		else if(tempFind.IsDirectory())   //找到的是文件夹，则遍历该文件夹下的文件
		{
// 			strTmp=L"";
// 			strTmp=tempFind.GetFilePath();
// 			FindFile(strTmp);
			continue;
		}
		else
		{
			SPicInfo pic;
			pic.strFile = tempFind.GetFilePath(); 
			pic.hbitmap = (HBITMAP)LoadImage(AfxGetInstanceHandle(), 
				pic.strFile, 
				IMAGE_BITMAP, 
				0, 
				0, 
				LR_LOADFROMFILE|LR_CREATEDIBSECTION);  
			if (pic.hbitmap != NULL)
			{
				m_vecPic.push_back(pic);
			}
			
		}
	}
	tempFind.Close();
	return;
}
void CMgrPic::FindPNGFile(CString strFoldername)
{
	CString m_cstrFileList=L"";
	CFileFind tempFind;
	BOOL bFound; //判断是否成功找到文件
	bFound = tempFind.FindFile(strFoldername + L"\\*.png");   //修改" "内内容给限定查找文件类型
	while(bFound)      //遍历所有文件
	{
		bFound=tempFind.FindNextFile(); //第一次执行FindNextFile是选择到第一个文件，以后执行为选择
		//到下一个文件
		if(tempFind.IsDots()){
			continue; //如果找到的是返回上层的目录 则结束本次查找
		}
		else if(tempFind.IsDirectory())   //找到的是文件夹，则遍历该文件夹下的文件
		{
			// 			strTmp=L"";
			// 			strTmp=tempFind.GetFilePath();
			// 			FindFile(strTmp);
			continue;
		}
		else
		{
			SPicInfo pic;
			pic.strFile = tempFind.GetFilePath(); 
			CImage image;  
			HRESULT ret = image.Load(pic.strFile);  
			pic.hbitmap = image.Detach();  			
			if (pic.hbitmap != NULL)
			{
				m_vecPic.push_back(pic);
			}

		}
	}
	tempFind.Close();
	return;
}
void CMgrPic::LoadPic()
{
	Release();
	FindFile(g_szAPPPath + L"texture");
	FindPNGFile(g_szAPPPath + L"texture");
}
void CMgrPic::Release()
{
	if (!m_vecPic.empty())
	{
		for (int i = 0; i < m_vecPic.size(); i++)
		{
			if (m_vecPic[i].hbitmap != NULL)
			{
				DeleteObject(m_vecPic[i].hbitmap);
				m_vecPic[i].hbitmap = NULL;
			}
		}
		m_vecPic.clear();
	}
}
HBITMAP CMgrPic::GetBitmap(CString strFile)
{
	
	CString strFindFile = g_szAPPPath + strFile;
	for (int i = 0; i < m_vecPic.size(); i ++)
	{
		if (strFindFile == m_vecPic[i].strFile)
		{
			return  m_vecPic[i].hbitmap;

		}
	}
	return NULL;
}