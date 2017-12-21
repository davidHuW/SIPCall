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
	BOOL bFound; //�ж��Ƿ�ɹ��ҵ��ļ�
	bFound = tempFind.FindFile(strFoldername + L"\\*.bmp");   //�޸�" "�����ݸ��޶������ļ�����
	while(bFound)      //���������ļ�
	{
		bFound=tempFind.FindNextFile(); //��һ��ִ��FindNextFile��ѡ�񵽵�һ���ļ����Ժ�ִ��Ϊѡ��
		//����һ���ļ�
		if(tempFind.IsDots()){
			continue; //����ҵ����Ƿ����ϲ��Ŀ¼ ��������β���
		}
		else if(tempFind.IsDirectory())   //�ҵ������ļ��У���������ļ����µ��ļ�
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
	BOOL bFound; //�ж��Ƿ�ɹ��ҵ��ļ�
	bFound = tempFind.FindFile(strFoldername + L"\\*.png");   //�޸�" "�����ݸ��޶������ļ�����
	while(bFound)      //���������ļ�
	{
		bFound=tempFind.FindNextFile(); //��һ��ִ��FindNextFile��ѡ�񵽵�һ���ļ����Ժ�ִ��Ϊѡ��
		//����һ���ļ�
		if(tempFind.IsDots()){
			continue; //����ҵ����Ƿ����ϲ��Ŀ¼ ��������β���
		}
		else if(tempFind.IsDirectory())   //�ҵ������ļ��У���������ļ����µ��ļ�
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