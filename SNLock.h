#pragma once
class CSNLock
{
public:
	CSNLock(void);
	~CSNLock(void);
	void Lock();
	void Unlock();
private:
	void * m_ppmt;
	bool m_bLock;
};

