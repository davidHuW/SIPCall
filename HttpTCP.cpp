#include "StdAfx.h"
#include "HttpTCP.h"

#include "SIPCallDlg.h"

#define HTTP_TCP_SRV_PORT (80)		//远程端口
#define HTTP_TCP_LOC_PORT (8080)	//本地端口
/* 定义常量 */
#define HTTP_BUF_SIZE		(1024)	/* 缓冲区的大小*/
#define HTTP_HOST_LEN		(256)	/* 主机名长度 */
#define HTTP_FILENAME_LEN   (256)   /* 文件名长度 */
#define MAX_LISTEN   2				//最多可同时连接的客户端数量
int g_fd_ArrayC[MAX_LISTEN] = {0}; //处理所有的待决连接

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
 * 函数功能: 解析命令行参数, 分别得到主机名, 端口号和文件名. 命令行格式:
 *           [http://www.baidu.com:8080/index.html]
 *
 * 参数说明: [IN]  buf, 字符串指针数组;
 *           [OUT] host, 保存主机;
 *           [OUT] port, 端口;
 *           [OUT] file_name, 文件名;
 *
 * 返 回 值: void.
 *
 **************************************************************************/
void CHttpTCP::http_parse_request_url(const char *buf, char *host, 
                            unsigned short *port, char *file_name)
{
    int length = 0;
    char port_buf[8];
    char *buf_end = (char *)(buf + strlen(buf));
    char *begin, *host_end, *colon, *file;

    /* 查找主机的开始位置 */
    begin = strstr((char*)buf, "//");
    begin = (begin ? begin + 2 : (char*)buf);
    
    colon = strchr(begin, ':');
    host_end = strchr(begin, '/');

    if (host_end == NULL)
    {
        host_end = buf_end;
    }
    else
    {   /* 得到文件名 */
        file = strrchr(host_end, '/');
        if (file && (file + 1) != buf_end)
            strcpy(file_name, file + 1);
    }

    if (colon) /* 得到端口号 */
    {
        colon++;

        length = host_end - colon;
        memcpy(port_buf, colon, length);
        port_buf[length] = 0;
        *port = atoi(port_buf);

        host_end = colon - 1;
    }

    /* 得到主机信息 */
    length = host_end - begin;
    memcpy(host, begin, length);
    host[length] = 0;
}




// server


/* 定义文件类型对应的 Content-Type */
struct doc_type
{
    char *suffix; /* 文件后缀 */
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
 * 函数功能: 根据文件后缀查找对应的 Content-Type.
 *
 * 参数说明: [IN] suffix, 文件名后缀;
 *
 * 返 回 值: 成功返回文件对应的 Content-Type, 失败返回 NULL.
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
 * 函数功能: 解析请求行, 得到文件名及其后缀. 请求行格式:
 *           [GET http://www.baidu.com:8080/index.html HTTP/1.1]
 *
 * 参数说明: [IN]  buf, 字符串指针数组;
 *           [IN]  buflen, buf 的长度;
 *           [OUT] file_name, 文件名;
 *           [OUT] suffix, 文件名后缀;
 *
 * 返 回 值: void.
 *
 **************************************************************************/
void CHttpTCP::http_parse_request_cmd(char *buf, int buflen, char *file_name, char *suffix)
{
    int length = 0;
    char *begin, *end, *bias;

    /* 查找 URL 的开始位置 */
    begin = strchr(buf, ' ');
    begin += 1;
        
    /* 查找 URL 的结束位置 */
    end = strchr(begin, ' ');
    *end = 0;

    bias = strrchr(begin, '/');
    length = end - bias;

    /* 找到文件名的开始位置 */
    if ((*bias == '/') || (*bias == '\\'))
    {
        bias++;
        length--;
    }

    /* 得到文件名 */
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
 * 函数功能: 向客户端发送 HTTP 响应.
 *
 * 参数说明: [IN]  buf, 字符串指针数组;
 *           [IN]  buf_len, buf 的长度;
 *
 * 返 回 值: 成功返回非0, 失败返回0.
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

    /* 得到文件名和后缀 */
    http_parse_request_cmd(buf, buf_len, file_name, suffix);

    res_file = fopen(file_name, "rb+"); /* 用二进制格式打开文件 */
    if (res_file == NULL)
    {
        printf("[Web] The file [%s] is not existed\n", file_name);
        return 0;
    }

    fseek(res_file, 0, SEEK_END);
    file_len = ftell(res_file);
    fseek(res_file, 0, SEEK_SET);
    
    type = http_get_type_by_suffix(suffix); /* 文件对应的 Content-Type */
    if (type == NULL)
    {
        printf("[Web] There is not the related content type\n");
        return 0;
    }

    /* 构造 HTTP 首部，并发送 */
    hdr_len = sprintf(http_header, http_res_hdr_tmpl, file_len, type);
    send_len = send(soc, http_header, hdr_len, 0);

    if (send_len == SOCKET_ERROR)
    {
        fclose(res_file);
        printf("[Web] Fail to send, error = %d\n", WSAGetLastError());
        return 0;
    }

    do /* 发送文件, HTTP 的消息体 */
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
	m_Socket = socket(AF_INET, SOCK_STREAM, 0); /* 创建 socket */
	if (m_Socket == INVALID_SOCKET)
	{
		printf("[Web] socket() Fails, error = %d\n", WSAGetLastError());
		return -1; 
	}
#if 0
	//设置非阻塞方式连接
	unsigned long ul = 1;//1为非阻塞,0为阻塞
	int result = ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);
	if (result == SOCKET_ERROR)
	{
		closesocket(m_Socket);
		printf("[Web] Fail to ioctlsocket, error = %d\n", WSAGetLastError());
		return -1; 
	}
#endif 

	struct sockaddr_in local_addr;   /* 本地地址  */	
	
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(HTTP_TCP_LOC_PORT);
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int result = bind(m_Socket, (struct sockaddr *) &local_addr, sizeof(local_addr));
	if (result == SOCKET_ERROR) /* 绑定失败 */
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
	struct sockaddr_in serv_addr;   /* 远程地址  */
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
	/* 发送 HTTP 请求 */
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
	//循环接收数据
	
	
	while (!s_bExit)
	{
		printf("等待连接...\n");
		SOCKET sClient = accept(pHttpTCP->m_Socket, (SOCKADDR *)&remoteAddr, &nAddrlen);
		
		if(sClient == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		printf("接受到一个连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr));

		//接收数据
		int ret = recv(sClient, revData, HTTP_BUF_SIZE, 0);        
		if(ret > 0)
		{
			revData[ret] = 0x00;
			printf(revData);
		}

		//发送数据
		// 		char * sendData = "你好，TCP客户端！\n";
		// 		send(sClient, sendData, strlen(sendData), 0);
		closesocket(sClient);
	}
#else
	//printf("-设置服务器端模式: %s\n", nMode==0? "阻塞模式":"非阻塞模式");	
	printf("-开始准备接受连接\n");
	fd_set fdRead;
	//fd_set fdWrite;
	timeval tv={0,0};
	int     nLoopi = 0;
	int     nConnNum = 0;
	while(!s_bExit)
	{
		printf("-select 开始\n");
		FD_ZERO(&fdRead/*, &fdWrite*/);
		FD_SET( pHttpTCP->m_Socket, &fdRead ); 
		//将待决的连接SOCKET放入fdRead集中进行select监听
		for( nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi )
		{
			if( g_fd_ArrayC[nLoopi] !=0 )
			{
				printf("-LOOPI: 待决SOCKET: %d\n",g_fd_ArrayC[nLoopi] );
				FD_SET( g_fd_ArrayC[nLoopi], &fdRead );
			}
		}
		//调用select模式进行监听
		int nRes = select( 0, &fdRead, NULL, NULL, &tv );
		if( nRes == 0 )
		{
			printf("-!!! select timeout: %d sec\n",tv.tv_sec);
			continue; //继续监听
		}
		else if( nRes < 0 )
		{
			printf("!!! select failed: %d\n", WSAGetLastError());
			break;
		}

		//检查所有的可用SOCKET
		printf("-查找可用的SOCKET\n");
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
					//将已经关闭的SOCKET从FD集中删除
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

		//检查是否为新的连接进入
		if( FD_ISSET( pHttpTCP->m_Socket, &fdRead) )
		{
			printf("-发现一个新的客户连接\n");
			SOCKET sClient = accept( pHttpTCP->m_Socket, (SOCKADDR *)&remoteAddr, &nAddrlen);
			if( sClient == WSAEWOULDBLOCK )
			{
				printf("!!! 非阻塞模式设定 accept调用不正\n");
				continue;
			}
			else if( sClient == INVALID_SOCKET  )
			{
				printf("!!! accept failed: %d\n", WSAGetLastError());
				continue;
			}
			//新的连接可以使用,查看待决处理队列
			if( nConnNum<MAX_LISTEN )
			{
				for(nLoopi=0; nLoopi<MAX_LISTEN; ++nLoopi)
				{
					if( g_fd_ArrayC[nLoopi] == 0 )
					{//添加新的可用连接
						g_fd_ArrayC[nLoopi] = sClient;
						break;
					}
				}
				++nConnNum;
				printf("-新的客户端信息:[%d] %s:%d\n", sClient, inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
			}
			else
			{
				printf("-服务器端连接数已满: %d\n", sClient);
				//send( sClient, noresponseBuff, strlen(noresponseBuff), 0 );
				closesocket( sClient );
			}
		}//if( FD_ISSET( sListen, &fdRead) )
	}//while(true)
	printf("-关闭服务器端SOCKET\n");
	closesocket( pHttpTCP->m_Socket );
#endif
	
	return 1;
}




void CHttpTCP::CreateRecvThread()
{
	AfxBeginThread(HTTP_TCP_RECV, this);  
}
