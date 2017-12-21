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
/*每个线程中访问临界资源的那段代码称为临界区（Critical Section）。
EnterCriticalSection进入临界,加锁
LeaveCriticalSection，解锁
 多线程中用来确保同一时刻只有一个线程操作被保护的数据的操作函数*/