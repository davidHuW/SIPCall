#pragma once

#include "SNLock.h"

class CHttpTCP
{
public:
	CHttpTCP(void);
	~CHttpTCP(void);

	int CreateSocket();
	
	bool POST(char* pSendData);

	void CreateRecvThread();

private:
	void http_parse_request_url(const char *buf, char *host, 
		unsigned short *port, char *file_name);
	void http_parse_request_cmd(char *buf, int buflen, char *file_name, char *suffix);
	int http_send_response(SOCKET soc, char *buf, int buf_len);
public:
	SOCKET m_Socket; 
	CSNLock m_Lock; 
};

