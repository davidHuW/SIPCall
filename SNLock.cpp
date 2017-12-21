#include "StdAfx.h"
#include "SNLock.h"


CSNLock::CSNLock()
{
	m_ppmt = new CRITICAL_SECTION;
	if(m_ppmt != NULL){
		::InitializeCriticalSection( (CRITICAL_SECTION*)m_ppmt );
	}
	m_bLock = false;
}
CSNLock::~CSNLock()
{
	if(m_bLock){
		Unlock();
	}

	if(m_ppmt != NULL){
		::DeleteCriticalSection( (CRITICAL_SECTION*)m_ppmt );
		delete m_ppmt;
		m_ppmt = NULL;
	}
}
void CSNLock::Lock()
{
	if(m_ppmt != NULL){
		m_bLock = true;
		::EnterCriticalSection( (CRITICAL_SECTION*)m_ppmt );
	}
}
void CSNLock::Unlock()
{
	if(m_ppmt != NULL){
		m_bLock = false;
		::LeaveCriticalSection( (CRITICAL_SECTION*)m_ppmt );
	}
}
/*ÿ���߳��з����ٽ���Դ���Ƕδ����Ϊ�ٽ�����Critical Section����
EnterCriticalSection�����ٽ�,����
LeaveCriticalSection������
 ���߳�������ȷ��ͬһʱ��ֻ��һ���̲߳��������������ݵĲ�������*/