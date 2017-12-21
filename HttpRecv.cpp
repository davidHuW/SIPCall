#include "StdAfx.h"
#include "HttpRecv.h"
#include <afxinet.h>
#include "Log2File.h"
#include "SIPCallDlg.h"
#include <afxwin.h>
#include "tinyxml.h"
static bool s_bExit = false;
//#define TCP_RECV_PORT (8050)
#define  MAX_LISTEN_NUM (20)
#define  DEFAULT_BUFF	(1024)

CHttpRecv::CHttpRecv(void)
{
	s_bExit = false;
}

CHttpRecv::~CHttpRecv(void)
{
	s_bExit = true;
}

/********************************************************
 *��������MFC��ʵ��GET��ҳ��Ϣ�������浽�����ļ�
 *˵����
 *strURL��������ҳ��URL
 *fileaddr�����浽���ص��ļ���ַ
 *���أ����ɹ�����true�����򷵻�false
 *******************************************************/


bool 
CHttpRecv::HTTP_GET_DATA(CString strURL, CString fileaddr)
{
	CInternetSession session;
	CHttpConnection* pHttpConnection = NULL;
	CHttpFile *pHttpFile = NULL;
	CString strServer, strObject;
	INTERNET_PORT wPort;
	DWORD dwType;
 
	if(!AfxParseURL(strURL, dwType, strServer, strObject, wPort))
	{
		return false;//URL��������
	}
	pHttpConnection = session.GetHttpConnection(strServer, wPort);
	pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
	pHttpFile->SendRequest();
	DWORD dwRet;
	pHttpFile->QueryInfoStatusCode(dwRet);
	if(dwRet == HTTP_STATUS_OK)
	{
		CFile file;
		file.Open(fileaddr, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone, 0);

		char *pszBuffer = new char[1024];
		UINT nRead = pHttpFile->Read(pszBuffer, 1024); 
		while (nRead > 0)
		{
			file.Write(pszBuffer, nRead);
			nRead = pHttpFile->Read(pszBuffer, 1024);
		}
		file.Close();
		delete []pszBuffer;
	}
	if(pHttpFile != NULL)
	{
		pHttpFile->Close();
		delete pHttpFile;
		pHttpFile = 0;
	}
	if(pHttpConnection != NULL)
	{
		pHttpConnection->Close();
		delete pHttpConnection;
		pHttpConnection = 0;
	} 
	session.Close();
	return true;
}
UINT TcpSendProc(LPVOID lParam)
{
	char* pSendBuf = (char*)lParam;
	if (g_Config.m_SystemConfig.nNetOF)
	{
		if(pSendBuf)
			delete[] pSendBuf;
		return 1;
	}
	g_Log.output(LOG_TYPE, "tcp send proc!");
	
	g_Log.output(LOG_TYPE, "param:%s!",pSendBuf);
	//�����׽���
	SOCKET http_sock = socket(AF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
	if(http_sock == INVALID_SOCKET)
	{
		g_Log.output(LOG_TYPE, "event socket error !");
		if(pSendBuf)
			delete[] pSendBuf;
		return -1;
	}

	struct sockaddr_in serv_addr;  /* ��������ַ */
	/* ��������ַ */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(g_Config.m_SystemConfig.portEvtSend);
	serv_addr.sin_addr.s_addr = inet_addr(g_Config.m_SystemConfig.IP_Server);

	int result = connect(http_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (result == SOCKET_ERROR) /* ����ʧ�� */
	{
		closesocket(http_sock);
		printf("[Web] fail to connect, error = %d\n", WSAGetLastError());
		if(pSendBuf)
			delete[] pSendBuf;
		return -1; 
	}

	/* ���� HTTP ���� */	
	char pPostData[1024] = {0};
	//sprintf(pPostData, "POST / HTTP/1.1\r\nContent-Type: text/html\r\nContent-Length: %d\r\n%s", strlen(pSendBuf), pSendBuf);

	sprintf(pPostData,
		"POST /xml HTTP/1.0\r\n"
		"Content-Type:text/html\r\n"
		"Content-Length:%d\r\n"
		"\r\n"
		"%s"
		,strlen(pSendBuf),pSendBuf);

	result = send(http_sock, pPostData, strlen(pPostData), 0);
	if (result == SOCKET_ERROR) /* ����ʧ�� */
	{
		closesocket(http_sock);
		printf("[Web] fail to send, error = %d\n", WSAGetLastError());
		if(pSendBuf)
			delete[] pSendBuf;
		return -1; 
	}
	closesocket(http_sock);
	if(pSendBuf)
		delete[] pSendBuf;
	return 0;
}
int CHttpRecv::TCP_POST_DATA(/*char* pIP, USHORT port, */char* pSendBuf)
{
	char* psendbuf = new char[strlen(pSendBuf)+1];
	strcpy_s(psendbuf,strlen(pSendBuf)+1,pSendBuf);
	AfxBeginThread(TcpSendProc,psendbuf);
	return 1;
}
struct HTTP_STR{
	CString* Url;
	char* Param;
	/*CString& rtnContent;*/
};
UINT HttpSendProc(LPVOID lparam)
{
	//g_Log.output(LOG_TYPE,"sendproc  1111\r\n");
	HTTP_STR* httpParam = (HTTP_STR*)lparam;
	//g_Log.output(LOG_TYPE,"sendproc  222\r\n");
	if (g_Config.m_SystemConfig.nNetOF)
	{
		if(httpParam){
			if(httpParam->Param)
				delete[] httpParam->Param;
			if(httpParam->Url)
				delete httpParam->Url;
			delete httpParam;
		}
		return true;
	}
	
    try{
        bool bRet = false;
        CString strServer, strObject, strHeader, strRet;
        unsigned short nPort;
        DWORD dwServiceType;
		//int num = MultiByteToWideChar(0,0,httpParam->Url,-1,NULL,0);
		//��ó��ֽ�����Ŀռ�
		//wchar_t *wide = new wchar_t[num];
		//MultiByteToWideChar(0,0,httpParam->Url,-1,wide,num);
        if(!AfxParseURL(*httpParam->Url, dwServiceType, strServer, strObject, nPort))
        {
			g_Log.output(LOG_TYPE, "������Ч�������ַ\r\n");
			//delete[] wide;
			if(httpParam){
				if(httpParam->Param)
					delete[] httpParam->Param;
				if(httpParam->Url)
					delete httpParam->Url;
				delete httpParam;
			}
            return false;
        }
		//delete[] wide;
        CInternetSession sess;//Create session

		nPort = g_Config.m_SystemConfig.portEvtSend;
        CHttpFile* pFile;
        //////////////////////////////////////////////
        CHttpConnection *pServer = sess.GetHttpConnection(strServer, nPort); 
        if(pServer == NULL)
        {
			g_Log.output(LOG_TYPE, "�Բ������ӷ�����ʧ�ܣ�\r\n");
			if(httpParam){
				if(httpParam->Param)
					delete[] httpParam->Param;
				if(httpParam->Url)
					delete httpParam->Url;
				delete httpParam;
			}
            return false;
        }
        pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST,strObject,NULL,1,NULL,NULL,INTERNET_FLAG_EXISTING_CONNECT); 
        if(pFile == NULL)
        {
			g_Log.output(LOG_TYPE, "�Ҳ��������ַ\r\n");
			if(httpParam){
				if(httpParam->Param)
					delete[] httpParam->Param;
				if(httpParam->Url)
					delete httpParam->Url;
				delete httpParam;
			}
            return false;
        }
		
        pFile -> AddRequestHeaders(_T("Content-Type:text/html"));
		//pFile -> AddRequestHeaders(_T("Content-Type:text/html"));
        //pFile -> AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded")); 
        //pFile -> AddRequestHeaders(_T("Accept: */*"));
		CString strLen = _T("");
		strLen.Format(_T("Content-Length:%d"), strlen(httpParam->Param)/*strPara.GetLength()*/);
		pFile -> AddRequestHeaders(strLen);
		DWORD dw = strlen(httpParam->Param);
        pFile -> SendRequest(NULL, 0, (LPVOID)httpParam->Param, dw/*(LPTSTR)(LPCTSTR)strPara, strPara.GetLength()*/); 
		
		g_Log.output(LOG_TYPE, "SendRequest:\r\nlen:%d\r\n%s\r\n......end\r\n", dw, httpParam->Param);
        /*CString strSentence;
        DWORD dwStatus;
        DWORD dwBuffLen = sizeof(dwStatus);
        BOOL bSuccess = pFile->QueryInfo(
            HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,
            &dwStatus, &dwBuffLen);

        if( bSuccess && dwStatus>=  200 && dwStatus<300) 
        {
            char buffer[2049];
            memset(buffer, 0, 2049);
            int nReadCount = 0;
            while((nReadCount = pFile->Read(buffer, 2048)) > 0)
            {
                //httpParam->rtnContent += buffer;
                memset(buffer, 0, 2049);
            }
            bRet = true;
        }
        else
        {
			g_Log.output(LOG_TYPE, "��վ����������,bSuccess:%d,dwStatus:%d\r\n",bSuccess,dwStatus);
            bRet = false;
        }*/
        ////////////////////////////////////////
        
		//pServer->Close();
        sess.Close();
		pFile->Close();
		delete pFile;
		//g_Log.output(LOG_TYPE,"sendproc   3333\r\n");
		if(httpParam){
			if(httpParam->Param)
				delete[] httpParam->Param;
			if(httpParam->Url)
				delete httpParam->Url;
			delete httpParam;
		}
        return bRet;
    }
	catch(CInternetException* ex)
	{
		wchar_t errorMsg[1024] = {0};
		ex->GetErrorMessage(errorMsg,1024);
		char msg[1024] = {0};
		WideCharToMultiByte(CP_ACP, 0,errorMsg, -1, msg, 1024, NULL, NULL);
		g_Log.output(LOG_TYPE, "�������postʧ�ܣ�����ţ�%s\r\n", msg);
		if(httpParam){
			if(httpParam->Param)
				delete[] httpParam->Param;
			if(httpParam->Url)
				delete httpParam->Url;
			delete httpParam;
		}
		return false;
	}
    catch(...)
    {
        int nCode = GetLastError();
		g_Log.output(LOG_TYPE, "�������postʧ�ܣ�����ţ�%d\r\n", nCode);
		if(httpParam){
			if(httpParam->Param)
				delete[] httpParam->Param;
			if(httpParam->Url)
				delete httpParam->Url;
			delete httpParam;
		}
        return false;
    }
	return 0;
}
bool CHttpRecv::HTTP_POST_DATA(CString strUrl,			// url
							   const char* strPara)	// ����
							   //CString &strContent)	// ���ص���Ϣ
							   //CString &strDescript)	// log����
{
	HTTP_STR* httpstrx = new HTTP_STR;
	httpstrx->Url = new CString(strUrl);
	httpstrx->Param =new char[strlen(strPara)+1];
	strcpy_s(httpstrx->Param,strlen(strPara)+1,strPara);
	AfxBeginThread(HttpSendProc,httpstrx);
	return true;
//	if (g_Config.m_SystemConfig.nNetOF)
//	{
//		return true;
//	}
//	
//
//    try{
//
//        strDescript = _T("�ύ�ɹ���ɣ�");
//        bool bRet = false;
//        CString strServer, strObject, strHeader, strRet;
//        unsigned short nPort;
//        DWORD dwServiceType;
//        if(!AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort))
//        {
//            strDescript = strUrl + _T("������Ч�������ַ��");
//			g_Log.output(LOG_TYPE, "������Ч�������ַ\r\n");
//            return false;
//        }
//		
//        CInternetSession sess;//Create session
//// 		const int nTimeOut = 2000;
//// 		sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, nTimeOut); //���ӳ�ʱ����
//// 		CInternetSession cis;
//// 		const int nTimeOut = 5000;
//// 		cis.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, nTimeOut); //���ӳ�ʱ����
//// 		cis.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);  //����1��
//// 		CHttpConnection *pServer = cis.GetHttpConnection(strServer, nPort);  //ȡ��һ��Http����
//
//		nPort = g_Config.m_SystemConfig.portEvtSend;
//        CHttpFile* pFile;
//        //////////////////////////////////////////////
//        CHttpConnection *pServer = sess.GetHttpConnection(strServer, nPort); 
//        if(pServer == NULL)
//        {
//            strDescript = _T("�Բ������ӷ�����ʧ�ܣ�");
//			g_Log.output(LOG_TYPE, "�Բ������ӷ�����ʧ�ܣ�\r\n");
//            return false;
//        }
//        pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST,strObject,NULL,1,NULL,NULL,INTERNET_FLAG_EXISTING_CONNECT); 
//        if(pFile == NULL)
//        {
//            strDescript = _T("�Ҳ��������ַ") + strUrl;
//			g_Log.output(LOG_TYPE, "�Ҳ��������ַ\r\n");
//            return false;
//        }
//		
//		
//        pFile -> AddRequestHeaders(_T("Content-Type:text/html"));
//		//pFile -> AddRequestHeaders(_T("Content-Type:text/html"));
//        //pFile -> AddRequestHeaders(_T("Content-Type: application/x-www-form-urlencoded")); 
//        //pFile -> AddRequestHeaders(_T("Accept: */*"));
//		CString strLen = _T("");
//		strLen.Format(_T("Content-Length:%d"), strlen(strPara)/*strPara.GetLength()*/);
//		pFile -> AddRequestHeaders(strLen);
//		DWORD dw = strlen(strPara);
//        pFile -> SendRequest(NULL, 0, (LPVOID)strPara, dw/*(LPTSTR)(LPCTSTR)strPara, strPara.GetLength()*/); 
//		
//		g_Log.output(LOG_TYPE, "SendRequest:\r\nlen:%d\r\n%s\r\n......end\r\n", dw, strPara);
//        CString strSentence;
//        DWORD dwStatus;
//        DWORD dwBuffLen = sizeof(dwStatus);
//        BOOL bSuccess = pFile->QueryInfo(
//            HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,
//            &dwStatus, &dwBuffLen);
//
//        if( bSuccess && dwStatus>=  200 && dwStatus<300) 
//        {
//            char buffer[2049];
//            memset(buffer, 0, 2049);
//            int nReadCount = 0;
//            while((nReadCount = pFile->Read(buffer, 2048)) > 0)
//            {
//                strContent += buffer;
//                memset(buffer, 0, 2049);
//            }
//            bRet = true;
//        }
//        else
//        {
//            strDescript = _T("��վ����������") + strUrl;
//			g_Log.output(LOG_TYPE, "��վ����������\r\n");
//            bRet = false;
//        }
//        ////////////////////////////////////////
//        pFile->Close();
//        sess.Close();
//        return bRet;
//    }
//    catch(...)
//    {
//        int nCode = GetLastError();
//		
//        strDescript.Format(_T("�������postʧ�ܣ�����ţ�%d"), nCode);
//		g_Log.output(LOG_TYPE, "�������postʧ�ܣ�����ţ�%d\r\n", nCode);
//        return false;
//    }
}
UINT ListenProc(SOCKET slisten, CHttpRecv* pHttpRecv)  
{  	
	// start
	char recvBuff[DEFAULT_BUFF] = {0};
	char responseBuff[DEFAULT_BUFF] = {"Server Has Received"};
	char noresponseBuff[DEFAULT_BUFF] = {"������������������,�޷�����"};
	SSocketInfo g_fd_ArrayC[MAX_LISTEN_NUM]; //�������еĴ�������
	fd_set fdRead;
	//fd_set fdWrite;
	timeval tv = {0, 100*1000};
	int nLoopi = 0;
	int nConnNum = 0;
	int nRes = 0;
	SOCKET         sClient;
	//SOCKADDR_IN    addrListen;
	SOCKADDR_IN    addrClient;
	int            addrClientLen = sizeof(addrClient);
	g_Log.output(LOG_TYPE, "------��ʼ------\r\n");
	while(!g_bExitApp)
	{
		Sleep(1);

		FD_ZERO(&fdRead);
		FD_SET( slisten, &fdRead ); 
		//������������SOCKET����fdRead���н���select����
		for( nLoopi = 0; nLoopi < MAX_LISTEN_NUM; ++nLoopi )
		{
			if( g_fd_ArrayC[nLoopi].sfd !=0 )
			{
				//m_Log.output(LOG_TYPE, "-LOOPI: ����SOCKET: %d\r\n",g_fd_ArrayC[nLoopi] );
				FD_SET( g_fd_ArrayC[nLoopi].sfd, &fdRead );
			}
		}
		//����selectģʽ���м���
		nRes = select(0, &fdRead, NULL, NULL, &tv );
		if( nRes == 0 )
		{
			//m_Log.output(LOG_TYPE, "-!!! select timeout: %d sec\r\n",tv.tv_sec);
			continue; //��������
		}
		else if( nRes < 0 )
		{
			g_Log.output(LOG_TYPE, "!!! select failed: %d\r\n", WSAGetLastError());				
			break;
		}

		//������еĿ���SOCKET
		//printf("-���ҿ��õ�SOCKET\n");
		for( nLoopi = 0; nLoopi < MAX_LISTEN_NUM; ++nLoopi )
		{
			if( FD_ISSET(g_fd_ArrayC[nLoopi].sfd, &fdRead) )
			{
				memset( recvBuff, 0 ,sizeof(recvBuff) );
				nRes = recv( g_fd_ArrayC[nLoopi].sfd, recvBuff, sizeof(recvBuff) - 1, 0 );		

				if( nRes > 0 )
				{
					recvBuff[nRes] = '\0';
					g_fd_ArrayC[nLoopi].time = GetTickCount();
					//pAPP->TransSend(g_fd_ArrayC[nLoopi].ip, g_fd_ArrayC[nLoopi].port, recvBuff);
					g_Log.output(LOG_TYPE, "%s\r\n",recvBuff);
					pHttpRecv->DecodeEvent(recvBuff);
				}


				//�ر�SOCKET����FD����ɾ��
				//g_Log.output(LOG_TYPE, "-Client Closed. %s: %d.\r\n", g_fd_ArrayC[nLoopi].ip, g_fd_ArrayC[nLoopi].port);
				FD_CLR( g_fd_ArrayC[nLoopi].sfd, &fdRead );					
				closesocket( g_fd_ArrayC[nLoopi].sfd );
				g_fd_ArrayC[nLoopi].init();
				--nConnNum;
				//g_Log.output(LOG_TYPE, "-Client Closed.\r\n");
			}
		}//for( nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi )

		//����Ƿ�Ϊ�µ����ӽ���
		if( FD_ISSET( slisten, &fdRead) )
		{
			//g_Log.output(LOG_TYPE, "-����һ���µĿͻ�����\r\n");
			sClient = accept( slisten, (sockaddr*)&addrClient, &addrClientLen );
			if( sClient == WSAEWOULDBLOCK )
			{
				//m_Log.output(LOG_TYPE, "!!! ������ģʽ�趨 accept���ò���\r\n");
				continue;
			}
			else if( sClient == INVALID_SOCKET  )
			{
				//m_Log.output(LOG_TYPE, "!!! accept failed: %d\r\n", WSAGetLastError());
				continue;
			}
			//�µ����ӿ���ʹ��,�鿴�����������
			if( nConnNum < MAX_LISTEN_NUM )
			{
				for(nLoopi = 0; nLoopi < MAX_LISTEN_NUM; ++nLoopi)
				{
					if( g_fd_ArrayC[nLoopi].sfd == 0 )
					{//����µĿ�������
						g_fd_ArrayC[nLoopi].sfd = sClient;
						sprintf_s(g_fd_ArrayC[nLoopi].ip, inet_ntoa(addrClient.sin_addr));
						g_fd_ArrayC[nLoopi].port = ntohs(addrClient.sin_port);
						g_fd_ArrayC[nLoopi].time = GetTickCount();
						break;
					}
				}
				++nConnNum;
				//g_Log.output(LOG_TYPE, "-�µĿͻ�����Ϣ:[%d] %s:%d\r\n", sClient, inet_ntoa(addrClient.sin_addr), ntohs(addrClient.sin_port));
			}
			else
			{
				g_Log.output(LOG_TYPE, "-������������������: %d\r\n", sClient);
				//send( sClient, noresponseBuff, strlen(noresponseBuff), 0 );
				closesocket( sClient );

			}
		}//if( FD_ISSET( sListen, &fdRead) )
	}//while(true)	
	
	g_Log.output(LOG_TYPE, "------����------\r\n");
	return 1;
}

UINT TCP_RECV(LPVOID pParam)
{
	CHttpRecv* pHttpRecv = (CHttpRecv*)pParam;
	if (pHttpRecv == NULL)
	{
		return 0;
	}
	//�����׽���
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
	if(slisten == INVALID_SOCKET)
	{
		g_Log.output(LOG_TYPE, "event socket error !");
		return 0;
	}

	//��IP�Ͷ˿�
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_Config.m_SystemConfig.portEvtListen/*TCP_RECV_PORT*/);
	sin.sin_addr.S_un.S_addr = INADDR_ANY; 
	if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		g_Log.output(LOG_TYPE, "event bind error !");
		return 0;
	}

	//��ʼ����
	if(listen(slisten, MAX_LISTEN_NUM) == SOCKET_ERROR)
	{
		g_Log.output(LOG_TYPE, "event listen error !");
		return 0;
	}

	// ���÷�����ģʽ
	DWORD nMode = 1;
	if (ioctlsocket(slisten, FIONBIO, &nMode ) == SOCKET_ERROR)
	{		
		g_Log.output(LOG_TYPE, "socket set FIONBIO fail.....\r\n");
		return 0;
	}
#if 0
	//ѭ����������	
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[1024] = {0}; 
	while (!g_bExitApp/*!s_bExit*/)
	{
		//printf("�ȴ�����...\n");
		int timeout = 1000;
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(slisten, &rdset);
		timeval tm;
		tm.tv_sec = timeout / 1000;
		tm.tv_usec = timeout % 1000 * 1000;
		int ts = select(slisten + 1, &rdset, NULL,  NULL, &tm);
		if(ts <= -1){
			int iError = WSAGetLastError();
			return 0;
		}
		else if (ts == 0)
		{
			// ��ʱ
		}
		else{
			SOCKET sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
			if(sClient == INVALID_SOCKET)
			{
				g_Log.output(LOG_TYPE, "event accept error !");
				continue;
			}			
			else{//printf("���ܵ�һ�����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));
				while(!g_bExitApp){
					FD_ZERO(&rdset);
					FD_SET(sClient, &rdset);
					timeval tm;
					tm.tv_sec = timeout / 1000;
					tm.tv_usec = timeout % 1000 * 1000;
					int ts = select(sClient + 1, &rdset, NULL,  NULL, &tm);
					if(ts <= -1){
						int iError = WSAGetLastError();
						closesocket(sClient);
						break;
					}
					else if (ts == 0)
					{
						// ��ʱ
					}
					else{
						//��������
						int ret = recv(sClient, revData, 1024, 0);        
						if(ret > 0)
						{
							revData[ret] = 0x00;
							g_Log.output(LOG_TYPE, revData);
							pHttpRecv->DecodeEvent(revData);

						}
						closesocket(sClient);
						break;
					}					
				}				
			}			
		}		
	}
#else
	ListenProc(slisten, pHttpRecv);
#endif
	closesocket(slisten);
	return 1;
}

void CHttpRecv::CreateTCPRecvThread()
{
	AfxBeginThread(TCP_RECV, this);  
}
enEventStatus CHttpRecv::GetEvtType(const char* pType)
{
	if (pType == NULL)
	{
		return ENUM_EVENT_NONE;
	}
	// decode
	if(strncmp(pType, "RING", strlen("RING")) == 0){
		return ENUM_EVENT_RING;
	}
	else if(strncmp(pType, "ANSWER", strlen("ANSWER")) == 0){
		return ENUM_EVENT_ANSWER;
	}
	else if(strncmp(pType, "BYE", strlen("BYE")) == 0){
		return ENUM_EVENT_BYE;
	}
	else if(strncmp(pType, "ONLINE", strlen("ONLINE")) == 0){
		return ENUM_EVENT_ONLINE;
	}
	else if(strncmp(pType, "OFFLINE", strlen("OFFLINE")) == 0){
		return ENUM_EVENT_OFFLINE;
	}
	else if(strncmp(pType, "IDLE", strlen("IDLE")) == 0){
		return ENUM_EVENT_IDLE;
	}
	/*else if(strncmp(pType, "DIVERT", strlen("DIVERT")) == 0)
	{
		return ENUM_EVENT_DIVERT;
	}*/
	else{
		return ENUM_EVENT_NONE;
	}
}
void CHttpRecv::DecodeEvent(char* pRecvData, SEventInfo& evt)
{
	char* pXMLData = strstr(pRecvData, "<?xml version");
	if (pXMLData == NULL)
	{
		return;
	}
	// ���� 
	TiXmlDocument XmlParse;	
	XmlParse.Parse((char*)pXMLData, 0, TIXML_ENCODING_UTF8);

	
	//��ø�Ԫ��
	TiXmlElement *RootElement = XmlParse.RootElement();
	if(RootElement){
		TiXmlAttribute *EvtAttr = RootElement->FirstAttribute();
		// ����¼�����
		const char* pEvtType = EvtAttr->Value();
		//g_Log.output(LOG_TYPE,"!!!!!!%s!!!!!!\r\n",pEvtType);
		evt.type = GetEvtType(pEvtType);
		//g_Log.output(LOG_TYPE,"!!!!!!%d!!!!!!\r\n",evt.type);
		// ext
		TiXmlElement *extElement = RootElement->FirstChildElement("ext");	
#if 1
		if (extElement)
		{
			TiXmlAttribute *extAttr = extElement->FirstAttribute();
			for ( ; ; extAttr = extAttr->Next())
			{
				if (extAttr == NULL)
				{
					break;
				}
				const char* key = extAttr->Name();
				const char* value = extAttr->Value();		
				if (strncmp(key, "id", strlen("id")) == 0)
				{
					sprintf(evt.extNo, value);
				}			
			}		
		}
#else
		
#endif
		// outer  
		TiXmlElement *outElement = RootElement->FirstChildElement("outer");
		if (outElement)
		{
			TiXmlAttribute *outAttr = outElement->FirstAttribute();
			for ( ; ; outAttr = outAttr->Next())
			{
				if (outAttr == NULL)
				{
					break;
				}
				const char* key = outAttr->Name();
				const char* value = outAttr->Value();		
				if (strncmp(key, "id", strlen("id")) == 0)
				{
					evt.id = atoi(value);
				}
				else if (strncmp(key, "from", strlen("from")) == 0)
				{
					sprintf(evt.from, value);
				}

			}
		}

		// visitor  
		TiXmlElement *visitorElement = RootElement->FirstChildElement("visitor");
		if (visitorElement)
		{
			TiXmlAttribute *visitorAttr = visitorElement->FirstAttribute();
			for ( ; ; visitorAttr = visitorAttr->Next())
			{
				if (visitorAttr == NULL)
				{
					break;
				}
				const char* key = visitorAttr->Name();
				const char* value = visitorAttr->Value();		
				if (strncmp(key, "id", strlen("id")) == 0)
				{
					evt.id = atoi(value);
				}
				else if (strncmp(key, "from", strlen("from")) == 0)
				{
					sprintf(evt.from, value);
				}
			}
		}

		// divert  
		TiXmlElement *divertElement = RootElement->FirstChildElement("divert");
		if (divertElement)
		{
			TiXmlAttribute *visitorAttr = divertElement->FirstAttribute();
			for ( ; ; visitorAttr = visitorAttr->Next())
			{
				if (visitorAttr == NULL)
				{
					break;
				}
				const char* key = visitorAttr->Name();
				const char* value = visitorAttr->Value();		
				if (strncmp(key, "to", strlen("to")) == 0)
				{
					//evt.id = atoi(value);
					sprintf(evt.extNo,value);
				}
				else if (strncmp(key, "from", strlen("from")) == 0)
				{
					sprintf(evt.from, value);
				}
			}
		}
	}
}

void CHttpRecv::DecodeCdr(char* pRecvData, SEventInfo& evt)
{
	char* pXMLData = strstr(pRecvData, "<?xml version");
	if (pXMLData == NULL)
	{
		return;
	}
	// ���� 
	TiXmlDocument XmlParse;	
	XmlParse.Parse((char*)pXMLData, 0, TIXML_ENCODING_UTF8);


	//��ø�Ԫ��
	TiXmlElement *RootElement = XmlParse.RootElement();
	if(RootElement){
		TiXmlAttribute *EvtAttr = RootElement->FirstAttribute();
		// cdr  
		
		TiXmlElement * durElement = RootElement->FirstChildElement("Duration");
		const char* durValue = durElement->GetText();		

		TiXmlElement * typeElement = RootElement->FirstChildElement("Type");
		const char* typeValue = typeElement->GetText();

		if (durValue && (atoi(durValue) == 0)) //���˽���
		{	
			TiXmlElement * cpnElement = RootElement->FirstChildElement("CPN");		// ����
			const char* cpnValue = cpnElement->GetText();
			TiXmlElement * cpdnElement = RootElement->FirstChildElement("CDPN");	// ����
			const char* cpdnValue = cpdnElement->GetText();
			if (cpdnValue && cpnValue)
			{
				evt.type = ENUM_EVENT_NOANSWER;
				sprintf(evt.extNo, cpnValue);
				sprintf(evt.from, cpdnValue);
				// outer  
				TiXmlElement *outElement = RootElement->FirstChildElement("outer");
				if (outElement)
				{
					TiXmlAttribute *outAttr = outElement->FirstAttribute();
					for ( ; ; outAttr = outAttr->Next())
					{
						if (outAttr == NULL)
						{
							break;
						}
						const char* key = outAttr->Name();
						const char* value = outAttr->Value();		
						if (strncmp(key, "id", strlen("id")) == 0)
						{
							evt.id = atoi(value);
						}
					}
				}
			}
		}	
	}
}
void CHttpRecv::DecodeEvent(char* pRecvData)
{
	if (pRecvData == NULL)
	{
		return;
	}
	char* pDecodeData = strstr(pRecvData, "<Event attribute=");
	char* pCdrData = strstr(pRecvData, "<Cdr id");
	if ((pDecodeData == NULL) &&
		(pCdrData == NULL))
	{
		return;
	}

	SEventInfo evt;
	if (pDecodeData)
	{
		DecodeEvent(pRecvData, evt);
	}
	else if (pCdrData)
	{
		DecodeCdr(pRecvData, evt);
	}
#if 0
#else 
	switch(evt.type){
	case ENUM_EVENT_RING:			
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_RING, atoi(evt.extNo), (LPARAM)(&evt));		
		break;
	case ENUM_EVENT_ANSWER:		
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_RUN, atoi(evt.extNo), (LPARAM)(&evt));		
		break;
	case ENUM_EVENT_BYE:		
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_HANGUP, atoi(evt.extNo), (LPARAM)(&evt));
		break;
	case ENUM_EVENT_ONLINE:		
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_OK, atoi(evt.extNo), (LPARAM)(&evt));
		break;
	case ENUM_EVENT_OFFLINE:		
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_OFFLINE, atoi(evt.extNo), (LPARAM)(&evt));
		break;
	case ENUM_EVENT_IDLE:		
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_IDLE, atoi(evt.extNo), (LPARAM)(&evt));
		break;
	case ENUM_EVENT_NOANSWER:		
		SendMessage(m_pAPP->m_hWnd, RECV_MSG_CALL_NOANSWER, atoi(evt.extNo), (LPARAM)(&evt));
		break;
	case ENUM_EVENT_DIVERT:
		SendMessage(m_pAPP->m_hWnd,RECV_MSG_CALL_DIVERT,atoi(evt.extNo),(LPARAM)(&evt));
		break;
	}
#endif
}
int CHttpRecv::DecodeExt(char* pRecvData)
{
	char* pExtNo = NULL;
	if (pExtNo = strstr(pRecvData, "<ext id=\""))
	{
		char* pTemp = strstr(pExtNo, "\"");
		if (pTemp != NULL)
		{
			return atoi(pTemp + 1);
		}
		
	}
	return 0;
}