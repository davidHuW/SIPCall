#include "StdAfx.h"
#include "RWXml.h"

#include <iostream>
#include "tinyxml.h"
#include "tinystr.h"



CRWXml::CRWXml(void)
{
}

CRWXml::~CRWXml(void)
{
}




CString CRWXml::GetAppPath()
{//��ȡӦ�ó����Ŀ¼
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL, modulePath, MAX_PATH);
	CString strModulePath(modulePath);
	strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
	return strModulePath;
}

bool CRWXml::CreateXmlFile(string& szFileName)
{//����xml�ļ�,szFilePathΪ�ļ������·��,�������ɹ�����true,����false
	try
	{
		//����һ��XML���ĵ�����
		TiXmlDocument *myDocument = new TiXmlDocument();
		//����һ����Ԫ�ز����ӡ�
		TiXmlElement *RootElement = new TiXmlElement("Persons");
		myDocument->LinkEndChild(RootElement);
		//����һ��PersonԪ�ز����ӡ�
		TiXmlElement *PersonElement = new TiXmlElement("Person");
		RootElement->LinkEndChild(PersonElement);
		//����PersonԪ�ص����ԡ�
		PersonElement->SetAttribute("ID", "1");
		//����nameԪ�ء�ageԪ�ز����ӡ�
		TiXmlElement *NameElement = new TiXmlElement("name");
		TiXmlElement *AgeElement = new TiXmlElement("age");
		PersonElement->LinkEndChild(NameElement);
		PersonElement->LinkEndChild(AgeElement);
		//����nameԪ�غ�ageԪ�ص����ݲ����ӡ�
		TiXmlText *NameContent = new TiXmlText("������");
		TiXmlText *AgeContent = new TiXmlText("22");
		NameElement->LinkEndChild(NameContent);
		AgeElement->LinkEndChild(AgeContent);
		CString appPath = GetAppPath();
		
		string seperator = "\\";
		char pAppPath[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, appPath, -1, pAppPath, MAX_PATH, NULL, NULL);
		string fullPath = pAppPath +seperator+szFileName;
		myDocument->SaveFile(fullPath.c_str());//���浽�ļ�
		
	}
	catch (string& e)
	{
		return false;
	}
	return true;
}

bool CRWXml::ReadXmlFile(CString  szFileName)
{//��ȡXml�ļ���������
	try
	{
		CString appPath = GetAppPath();
		CString seperator = _T("\\");
		CString fullPath = appPath +seperator+szFileName;
		char pFullPath[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, fullPath, -1, pFullPath, MAX_PATH, NULL, NULL);
		//����һ��XML���ĵ�����
		TiXmlDocument *myDocument = new TiXmlDocument(pFullPath);
		myDocument->LoadFile();
		//��ø�Ԫ�أ���Persons��
		TiXmlElement *RootElement = myDocument->RootElement();
		//�����Ԫ�����ƣ������Persons��
		cout << RootElement->Value() << endl;
		//��õ�һ��Person�ڵ㡣
		TiXmlElement *FirstPerson = RootElement->FirstChildElement();
		//��õ�һ��Person��name�ڵ��age�ڵ��ID���ԡ�
		TiXmlElement *NameElement = FirstPerson->FirstChildElement();
		TiXmlElement *AgeElement = NameElement->NextSiblingElement();
		TiXmlAttribute *IDAttribute = FirstPerson->FirstAttribute();
		//�����һ��Person��name���ݣ��������ǣ�age���ݣ�����ID���ԣ�����
		cout << NameElement->FirstChild()->Value() << endl;
		cout << AgeElement->FirstChild()->Value() << endl;
		cout << IDAttribute->Value()<< endl;
	}
	catch (string& e)
	{
		return false;
	}
	return true;
}