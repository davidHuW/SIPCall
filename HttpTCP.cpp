#include "StdAfx.h"
#include "HttpTCP.h"

#include "SIPCallDlg.h"

#define HTTP_TCP_SRV_PORT (80)		//Զ�̶˿�
#define HTTP_TCP_LOC_PORT (8080)	//���ض˿�
/* ���峣�� */
#define HTTP_BUF_SIZE		(1024)	/* �������Ĵ�С*/
#define HTTP_HOST_LEN		(256)	/* ���������� */
#define HTTP_FILENAME_LEN   (256)   /* �ļ������� */
#define MAX_LISTEN   2				//����ͬʱ���ӵĿͻ�������
int g_fd_ArrayC[MAX_LISTEN] = {0}; //�������еĴ�������

static bool s_bExit = false;

CHttpTCP::CHttpTCP(void)
{

}


CHttpTCP::~CHttpTCP(void)
{
	s_bExit = true;
	closesocket(m_Socket);
}

// client





/**************************************************************************
 *
 * ��������: ���������в���, �ֱ�õ�������, �˿ںź��ļ���. �����и�ʽ:
 *           [http://www.baidu.com:8080/index.html]
 *
 * ����˵��: [IN]  buf, �ַ���ָ������;
 *           [OUT] host, ��������;
 *           [OUT] port, �˿�;
 *           [OUT] file_name, �ļ���;
 *
 * �� �� ֵ: void.
 *
 **************************************************************************/
void CHttpTCP::http_parse_request_url(const char *buf, char *host, 
                            unsigned short *port, char *file_name)
{
    int length = 0;
    char port_buf[8];
    char *buf_end = (char *)(buf + strlen(buf));
    char *begin, *host_end, *colon, *file;

    /* ���������Ŀ�ʼλ�� */
    begin = strstr((char*)buf, "//");
    begin = (begin ? begin + 2 : (char*)buf);
    
    colon = strchr(begin, ':');
    host_end = strchr(begin, '/');

    if (host_end == NULL)
    {
        host_end = buf_end;
    }
    else
    {   /* �õ��ļ��� */
        file = strrchr(host_end, '/');
        if (file && (file + 1) != buf_end)
            strcpy(file_name, file + 1);
    }

    if (colon) /* �õ��˿ں� */
    {
        colon++;

        length = host_end - colon;
        memcpy(port_buf, colon, length);
        port_buf[length] = 0;
        *port = atoi(port_buf);

        host_end = colon - 1;
    }

    /* �õ�������Ϣ */
    length = host_end - begin;
    memcpy(host, begin, length);
    host[length] = 0;
}




// server


/* �����ļ����Ͷ�Ӧ�� Content-Type */
struct doc_type
{
    char *suffix; /* �ļ���׺ */
    char *type;   /* Content-Type */
};

struct doc_type file_type[] = 
{
    {"html",    "text/html"  },
    {"gif",     "image/gif"  },
    {"jpeg",    "image/jpeg" },
    { NULL,      NULL        }
};

char *http_res_hdr_tmpl = "HTTP/1.1 200 OK\r\nServer: Huiyong's Server <0.1>\r\n"
    "Accept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: close\r\n"
    "Content-Type: %s\r\n\r\n";


/**************************************************************************
 *
 * ��������: �����ļ���׺���Ҷ�Ӧ�� Content-Type.
 *
 * ����˵��: [IN] suffix, �ļ�����׺;
 *
 * �� �� ֵ: �ɹ������ļ���Ӧ�� Content-Type, ʧ�ܷ��� NULL.
 *
 **************************************************************************/
char *http_get_type_by_suffix(const char *suffix)
{
    struct doc_type *type;

    for (type = file_type; type->suffix; type++)
    {
        if (strcmp(type->suffix, suffix) == 0)
            return type->type;
    }

    return NULL;
}

/**************************************************************************
 *
 * ��������: ����������, �õ��ļ��������׺. �����и�ʽ:
 *           [GET http://www.baidu.com:8080/index.html HTTP/1.1]
 *
 * ����˵��: [IN]  buf, �ַ���ָ������;
 *           [IN]  buflen, buf �ĳ���;
 *           [OUT] file_name, �ļ���;
 *           [OUT] suffix, �ļ�����׺;
 *
 * �� �� ֵ: void.
 *
 **************************************************************************/
void CHttpTCP::http_parse_request_cmd(char *buf, int buflen, char *file_name, char *suffix)
{
    int length = 0;
    char *begin, *end, *bias;

    /* ���� URL �Ŀ�ʼλ�� */
    begin = strchr(buf, ' ');
    begin += 1;
        
    /* ���� URL �Ľ���λ�� */
    end = strchr(begin, ' ');
    *end = 0;

    bias = strrchr(begin, '/');
    length = end - bias;

    /* �ҵ��ļ����Ŀ�ʼλ�� */
    if ((*bias == '/') || (*bias == '\\'))
    {
        bias++;
        length--;
    }

    /* �õ��ļ��� */
    if (length > 0)
    {
        memcpy(file_name, bias, length);
        file_name[length] = 0;

        begin = strchr(file_name, '.');
        if (begin)
            strcpy(suffix, begin + 1);
    }
}


/**************************************************************************
 *
 * ��������: ��ͻ��˷��� HTTP ��Ӧ.
 *
 * ����˵��: [IN]  buf, �ַ���ָ������;
 *           [IN]  buf_len, buf �ĳ���;
 *
 * �� �� ֵ: �ɹ����ط�0, ʧ�ܷ���0.
 *
 **************************************************************************/
int CHttpTCP::http_send_response(SOCKET soc, char *buf, int buf_len)
{
    int read_len, file_len, hdr_len, send_len;
    char *type;
    char read_buf[HTTP_BUF_SIZE];
    char http_header[HTTP_BUF_SIZE];
    char file_name[HTTP_FILENAME_LEN] = "index.html", suffix[16] = "html";
    FILE *res_file;

    /* �õ��ļ����ͺ�׺ */
    http_parse_request_cmd(buf, buf_len, file_name, suffix);

    res_file = fopen(file_name, "rb+"); /* �ö����Ƹ�ʽ���ļ� */
    if (res_file == NULL)
    {
        printf("[Web] The file [%s] is not existed\n", file_name);
        return 0;
    }

    fseek(res_file, 0, SEEK_END);
    file_len = ftell(res_file);
    fseek(res_file, 0, SEEK_SET);
    
    type = http_get_type_by_suffix(suffix); /* �ļ���Ӧ�� Content-Type */
    if (type == NULL)
    {
        printf("[Web] There is not the related content type\n");
        return 0;
    }

    /* ���� HTTP �ײ��������� */
    hdr_len = sprintf(http_header, http_res_hdr_tmpl, file_len, type);
    send_len = send(soc, http_header, hdr_len, 0);

    if (send_len == SOCKET_ERROR)
    {
        fclose(res_file);
        printf("[Web] Fail to send, error = %d\n", WSAGetLastError());
        return 0;
    }

    do /* �����ļ�, HTTP ����Ϣ�� */
    {
        read_len = fread(read_buf, sizeof(char), HTTP_BUF_SIZE, res_file);

        if (read_len > 0)
        {
            send_len = send(soc, read_buf, read_len, 0);
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));

    fclose(res_file);
    
    return 1;
}



int CHttpTCP::CreateSocket()
{
	m_Socket = socket(AF_INET, SOCK_STREAM, 0); /* ���� socket */
	if (m_Socket == INVALID_SOCKET)
	{
		printf("[Web] socket() Fails, error = %d\n", WSAGetLastError());
		return -1; 
	}
#if 0
	//���÷�������ʽ����
	unsigned long ul = 1;//1Ϊ������,0Ϊ����
	int result = ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);
	if (result == SOCKET_ERROR)
	{
		closesocket(m_Socket);
		printf("[Web] Fail to ioctlsocket, error = %d\n", WSAGetLastError());
		return -1; 
	}
#endif 

	struct sockaddr_in local_addr;   /* ���ص�ַ  */	
	
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(HTTP_TCP_LOC_PORT);
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int result = bind(m_Socket, (struct sockaddr *) &local_addr, sizeof(local_addr));
	if (result == SOCKET_ERROR) /* ��ʧ�� */
	{
		closesocket(m_Socket);
		printf("[Web] Fail to bind, error = %d\n", WSAGetLastError());
		return -1; 
	}



	result = listen(m_Socket, MAX_LISTEN);
	if (result == SOCKET_ERROR)
	{
		closesocket(m_Socket);
		printf("[Web] Fail to listen, error = %d\n", WSAGetLastError());
		return -1; 
	}
	printf("[Web] The server is running ... ...\n");

	DWORD nMode = 1;
	result = ioctlsocket( m_Socket, FIONBIO, &nMode );
	if( result == SOCKET_ERROR )
	{
		printf("!!! ioctlsocket failed: %d\n", WSAGetLastError());
		closesocket( result );
		WSACleanup();
		return -1;
	}

#if 0
	struct sockaddr_in serv_addr;   /* Զ�̵�ַ  */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(HTTP_TCP_SRV_PORT);
	//serv_addr.sin_addr.s_addr = addr;
	serv_addr.sin_addr.S_un.S_addr = inet_addr(g_Config.m_SystemConfig.IP_Server);	
	result = connect(m_Socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (result == SOCKET_ERROR)
	{
		closesocket(m_Socket);
		printf("[Web] Fail to connect, error = %d\n", WSAGetLastError());
		return -1; 
	}
#endif

	return 1;
}

bool CHttpTCP::POST(char* pSendData)
{	
#if 0
	char *http_req_hdr_tmpl = "GET %s HTTP/1.1\r\n"
    "Accept: image/gif, image/jpeg, */*\r\nAccept-Language: zh-cn\r\n"
    "Accept-Encoding: gzip, deflate\r\nHost: %s:%d\r\n"
    "User-Agent: Huiyong's Browser <0.1>\r\nConnection: Keep-Alive\r\n\r\n";
#endif
	/* ���� HTTP ���� */
	char data_buf[HTTP_BUF_SIZE] = {0};
	char *http_req_hdr_tmpl = "POST /xml HTTP/1.0\r\n"
		"Content-Type:text/html\r\n"
		"Content-Length:%d\r\n\r\n";

	int send_len = sprintf(data_buf, http_req_hdr_tmpl, strlen(pSendData));

	fd_set rdset;
	FD_ZERO(&rdset);
	FD_SET(m_Socket, &rdset);
	timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 0;
	int ts = select(m_Socket + 1, NULL, &rdset, NULL, &tm);
	if (ts <= 0)
	{
		return false;
	}

	int result = send(m_Socket, data_buf, send_len, 0);


	if(result != SOCKET_ERROR){
		return true;
	}
	else{
		return false;
	}

}
UINT HTTP_TCP_RECV(LPVOID pParam)
{
	CHttpTCP* pHttpTCP = (CHttpTCP*) pParam;
	if (pHttpTCP == NULL)
	{
		return 0;
	}
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[HTTP_BUF_SIZE] = {0}; 
#if 0
	//ѭ����������
	
	
	while (!s_bExit)
	{
		printf("�ȴ�����...\n");
		SOCKET sClient = accept(pHttpTCP->m_Socket, (SOCKADDR *)&remoteAddr, &nAddrlen);
		
		if(sClient == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("���ܵ�һ�����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));

		//��������
		int ret = recv(sClient, revData, HTTP_BUF_SIZE, 0);        
		if(ret > 0)
		{
			revData[ret] = 0x00;
			printf(revData);
		}

		//��������
		// 		char * sendData = "��ã�TCP�ͻ��ˣ�\n";
		// 		send(sClient, sendData, strlen(sendData), 0);
		closesocket(sClient);
	}
#else
	//printf("-���÷�������ģʽ: %s\n", nMode==0? "����ģʽ":"������ģʽ");	
	printf("-��ʼ׼����������\n");
	fd_set fdRead;
	//fd_set fdWrite;
	timeval tv={0,0};
	int     nLoopi = 0;
	int     nConnNum = 0;
	while(!s_bExit)
	{
		printf("-select ��ʼ\n");
		FD_ZERO(&fdRead/*, &fdWrite*/);
		FD_SET( pHttpTCP->m_Socket, &fdRead ); 
		//������������SOCKET����fdRead���н���select����
		for( nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi )
		{
			if( g_fd_ArrayC[nLoopi] !=0 )
			{
				printf("-LOOPI: ����SOCKET: %d\n",g_fd_ArrayC[nLoopi] );
				FD_SET( g_fd_ArrayC[nLoopi], &fdRead );
			}
		}
		//����selectģʽ���м���
		int nRes = select( 0, &fdRead, NULL, NULL, &tv );
		if( nRes == 0 )
		{
			printf("-!!! select timeout: %d sec\n",tv.tv_sec);
			continue; //��������
		}
		else if( nRes < 0 )
		{
			printf("!!! select failed: %d\n", WSAGetLastError());
			break;
		}

		//������еĿ���SOCKET
		printf("-���ҿ��õ�SOCKET\n");
		for( nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi )
		{
			if( FD_ISSET(g_fd_ArrayC[nLoopi], &fdRead) )
			{
				memset( revData, 0 ,sizeof(revData) );
				nRes = recv( g_fd_ArrayC[nLoopi], revData, sizeof(revData)-1, 0 );
				if( nRes <= 0 )
				{
					printf("-Client Has Closed.\n");
					closesocket( g_fd_ArrayC[nLoopi] );
					//���Ѿ��رյ�SOCKET��FD����ɾ��
					FD_CLR( g_fd_ArrayC[nLoopi], &fdRead );
					g_fd_ArrayC[nLoopi] = 0;
					--nConnNum;
				}
				else
				{
					revData[nRes] = '\0';
					printf("-Recvied: %s\n", revData);
					//send( g_fd_ArrayC[nLoopi], responseBuff, strlen(responseBuff), 0 );
				}
			}
		}//for( nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi )

		//����Ƿ�Ϊ�µ����ӽ���
		if( FD_ISSET( pHttpTCP->m_Socket, &fdRead) )
		{
			printf("-����һ���µĿͻ�����\n");
			SOCKET sClient = accept( pHttpTCP->m_Socket, (SOCKADDR *)&remoteAddr, &nAddrlen);
			if( sClient == WSAEWOULDBLOCK )
			{
				printf("!!! ������ģʽ�趨 accept���ò���\n");
				continue;
			}
			else if( sClient == INVALID_SOCKET  )
			{
				printf("!!! accept failed: %d\n", WSAGetLastError());
				continue;
			}
			//�µ����ӿ���ʹ��,�鿴�����������
			if( nConnNum<MAX_LISTEN )
			{
				for(nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi)
				{
					if( g_fd_ArrayC[nLoopi] == 0 )
					{//����µĿ�������
						g_fd_ArrayC[nLoopi] = sClient;
						break;
					}
				}
				++nConnNum;
				printf("-�µĿͻ�����Ϣ:[%d] %s:%d\n", sClient, inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
			}
			else
			{
				printf("-������������������: %d\n", sClient);
				//send( sClient, noresponseBuff, strlen(noresponseBuff), 0 );
				closesocket( sClient );
			}
		}//if( FD_ISSET( sListen, &fdRead) )
	}//while(true)
	printf("-�رշ�������SOCKET\n");
	closesocket( pHttpTCP->m_Socket );
#endif
	
	return 1;
}




void CHttpTCP::CreateRecvThread()
{
	AfxBeginThread(HTTP_TCP_RECV, this);  
}
