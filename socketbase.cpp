#include "stdafx.h"
#include "SocketBase.h"
#include "CommonFacility.h"

///////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSocketBase::CSocketBase()
{
}

CSocketBase::~CSocketBase()
{
}

BOOL CSocketBase::CheckSocketVersion()
{
	WORD		wVersionRequested;
	WSADATA		wsaData;
	DWORD		err = 0;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if(err == SOCKET_ERROR)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧WSAStartup Failed\n" );
		SOCKET_TRACE(szError);
		return FALSE;
	}
    /* Tell the user that we couldn't find a useable */ 
    /* winsock.dll.     */ 
	/* Confirm that the Windows Sockets DLL supports 1.1.*/ 
	/* Note that if the DLL supports versions greater */ 
	/* than 1.1 in addition to 1.1, it will still return */ 
	/* 1.1 in wVersion since that is the version we */ 
	/* requested. */ 
	if(	LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) { WSACleanup(); return FALSE; }
	return TRUE;
}

DWORD CSocketBase::Accept(SOCKET listener,SOCKET& sock)
{
	sockaddr_in	addr;
	int			addrlen = sizeof(addr);
	sock = accept(listener,(SOCKADDR*)(&addr),&addrlen);
	//addr.sa_family
	if( sock == INVALID_SOCKET ) return 0;
	return addr.sin_addr.S_un.S_addr;
}

BOOL CSocketBase::Bind(SOCKET& opt_socket,int type,__int16 port, DWORD localIP, LPSTR multicastIP)
{
	INT				err = 0;
	INT				ReceiveBufferSize = MAX_BUFF_SIZE;
	INT				SendBufferSize = MAX_BUFF_SIZE;
	unsigned long	mode = SOCKET_ASYNCHRONOUS;

	if( !CheckSocketVersion() ) return FALSE;
	/* The Windows Sockets DLL is acceptable. Proceed. */
	// Open a socket to listen for incoming connections.
	Disconnect(opt_socket);
	opt_socket = socket(AF_INET, type, 0); //type = SOCK_STREAM or SOCK_DGRAM
	if(opt_socket == INVALID_SOCKET)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧Socket Create Failed\n" );
		SOCKET_TRACE(szError);
		return FALSE;
	}
	// Set the receive buffer size...
	err = setsockopt(opt_socket, SOL_SOCKET, SO_RCVBUF,
		(char*) &ReceiveBufferSize, sizeof (ReceiveBufferSize));
	if(err == SOCKET_ERROR)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧setsockopt( SO_RCVBUF ) failed: %ld\n",GetLastError() );
		SOCKET_TRACE(szError);
		closesocket(opt_socket);
		WSACleanup();
		return FALSE;
	}
	// ...and the send buffer size for our new socket
	err = setsockopt(opt_socket, SOL_SOCKET, SO_SNDBUF,
		(char*) &SendBufferSize, sizeof (SendBufferSize));
	if(err == SOCKET_ERROR)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧setsockopt( SO_SNDBUF ) failed: %ld\n",GetLastError() );
		SOCKET_TRACE(szError);
		closesocket(opt_socket);
		WSACleanup();
		return FALSE;
	}
	// Set the blocking and nonblocking mode
	err = ioctlsocket( opt_socket,FIONBIO,&mode );
	if(err == SOCKET_ERROR)
	{
		DWORD	exterr = WSAGetLastError();
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧Socket Bind Failed,error=%d\n",exterr);
		SOCKET_TRACE(szError);
		if(exterr == WSAEADDRINUSE)
		{
			sprintf_s( szError,sizeof(szError),"⑧Can not set blocking/nonblocking mode.\n" );
			SOCKET_TRACE(szError);
		}
		closesocket(opt_socket);
		WSACleanup();
		return FALSE;
	}
	char				hostname[256];
	struct hostent FAR*	localhost = NULL;
	IN_ADDR				IpAddress;
	SOCKADDR_IN			localAddr;
	char*				uchIp = NULL;
	unsigned int*		dwIp = NULL;
	unsigned int		dwLoc = ((0x000000ff & localIP) << 24) |
								((0x0000ff00 & localIP) << 8) |
								((0x00ff0000 & localIP) >> 8) |
								((0xff000000 & localIP) >> 24);
	bool				bAddrSpecified = false;

	memset( hostname,0,256 );
	gethostname( hostname,255 );
	localhost = gethostbyname ( hostname );
	dwIp = (unsigned int*)localhost->h_addr_list[0];
	uchIp = localhost->h_addr_list[0];

	for (int i = 0; i < localhost->h_length; i ++) {
		if (dwIp[i] == dwLoc) {
			uchIp = (char*)(&dwIp[i]);
			bAddrSpecified = true;
			break;
		}
	}
	IpAddress.S_un.S_un_b.s_b1 = (unsigned char)uchIp[0];
	IpAddress.S_un.S_un_b.s_b2 = (unsigned char)uchIp[1];
	IpAddress.S_un.S_un_b.s_b3 = (unsigned char)uchIp[2];
	IpAddress.S_un.S_un_b.s_b4 = (unsigned char)uchIp[3];
	memcpy( m_host_ip,uchIp,4 );
	ZeroMemory (&localAddr, sizeof (localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);

	switch(type)
	{
	case SOCK_STREAM:
		localAddr.sin_addr = IpAddress;
		break;
	case SOCK_DGRAM:
		if( port != 0 )
		{
			const int		routenum = 1;
			const int		loop = 0;
			err = setsockopt(	opt_socket,IPPROTO_IP,
								IP_MULTICAST_TTL, (char*)&routenum, sizeof(routenum));
			if(err == SOCKET_ERROR)
			{
				char	szError[256];

				memset(	szError,0x00,256 );
				sprintf_s( szError,sizeof(szError),"⑧setsockopt( Multicast ) failed: %ld\n",GetLastError() );
				SOCKET_TRACE(szError);
				closesocket(opt_socket);
				WSACleanup();
				return FALSE;
			}
			err = setsockopt(	opt_socket,IPPROTO_IP,
								IP_MULTICAST_LOOP, (char*)&loop, sizeof(loop));
			if(err == SOCKET_ERROR)
			{
				char	szError[256];

				memset(	szError,0x00,256 );
				sprintf_s( szError,sizeof(szError),"⑧setsockopt( Multicast ) failed: %ld\n",GetLastError() );
				SOCKET_TRACE(szError);
				closesocket(opt_socket);
				WSACleanup();
				return FALSE;
			}
			if (bAddrSpecified) localAddr.sin_addr = IpAddress;
			else localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		break;
	default:;
	}
	// Bind our server to the agreed upon port number.  See
	// commdef.h for the actual port number.
	if( type != SOCK_DGRAM || port != 0 )
		err = bind(opt_socket, (PSOCKADDR) & localAddr, sizeof (localAddr));
	if(err == SOCKET_ERROR)
	{
		DWORD	exterr = WSAGetLastError();
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧Socket Bind Failed,error=%d\n",exterr);
		SOCKET_TRACE(szError);
		if(exterr == WSAEADDRINUSE)
		{
			sprintf_s( szError,sizeof(szError),"⑦The port number may already be in use.\n" );
			SOCKET_TRACE(szError);
		}
		closesocket(opt_socket);
		WSACleanup();
		return FALSE;
	}
	// Prepare to accept client connections.  Allow up to 5 pending connections.
	switch( type )
	{
	case SOCK_STREAM:
		{
			err = listen(opt_socket, 5);
			if(err == SOCKET_ERROR)
			{
				char	szError[256];

				memset(	szError,0x00,256 );
				sprintf_s( szError,sizeof(szError),"⑧Socket Listen Failed,error=%d\n",GetLastError() );
				SOCKET_TRACE(szError);
				closesocket(opt_socket);
				WSACleanup();
				return FALSE;
			}
		}
		break;
	case SOCK_DGRAM:
		if( multicastIP && port != 0 )
		{
			struct ip_mreq	mreq;
			memset(&mreq, 0x00, sizeof(mreq));
			mreq.imr_interface.s_addr = bAddrSpecified ? dwLoc : htonl(INADDR_ANY);
			mreq.imr_multiaddr.s_addr = inet_addr(multicastIP);
			err = setsockopt(
						opt_socket, 
						IPPROTO_IP,
						IP_ADD_MEMBERSHIP,
						(char*)&mreq, 
						sizeof(mreq));
			if (err == SOCKET_ERROR)
			{
				DWORD	exterr = WSAGetLastError();
				char	szError[256];

				memset(	szError,0x00,256 );
				sprintf_s( szError,sizeof(szError),"⑦Try to join Multicast group failed: %ld\n",exterr);
				SOCKET_TRACE(szError);
			}
		}
		break;
	default:;
	}
	// Only Handle a single Queue
	return TRUE;
}

BOOL CSocketBase::Connect(SOCKET& clientsocket,int byte1,int byte2,int byte3,int byte4,int port)
{
	IN_ADDR		RemoteIpAddress;
	SOCKADDR_IN	RemoteAddr;
	INT			ReceiveBufferSize = MAX_BUFF_SIZE;
	INT			SendBufferSize = MAX_BUFF_SIZE;
	INT			err;
	char		b1 = (char)byte1;
	char		b2 = (char)byte2;
	char		b3 = (char)byte3;
	char		b4 = (char)byte4;
	unsigned long	mode = SOCKET_ASYNCHRONOUS;

	if( !CheckSocketVersion() ) return FALSE;
	// Open a socket using the Internet Address family and TCP
	clientsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientsocket == INVALID_SOCKET)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧DoEcho: socket failed: %ld\n",GetLastError() );
		SOCKET_TRACE(szError);
		return FALSE;
	}
	// Set the receive buffer size...
	err = setsockopt(clientsocket, SOL_SOCKET, SO_RCVBUF,
		(char*) &ReceiveBufferSize, sizeof (ReceiveBufferSize));
	if(err == SOCKET_ERROR)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧setsockopt( SO_RCVBUF ) failed: %ld\n",GetLastError() );
		SOCKET_TRACE(szError);
		closesocket(clientsocket);		return FALSE;
	}
	// ...and the send buffer size for our new socket
	err = setsockopt(clientsocket, SOL_SOCKET, SO_SNDBUF,
		(char*) &SendBufferSize, sizeof (SendBufferSize));
	if(err == SOCKET_ERROR)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧setsockopt( SO_SNDBUF ) failed: %ld\n",GetLastError() );
		SOCKET_TRACE(szError);
		closesocket(clientsocket);
		return FALSE;
	}
	// Connect to an agreed upon port on the host.  See the
	// commdef.h file for the actual port number
	ZeroMemory (&RemoteAddr, sizeof (RemoteAddr));

	RemoteIpAddress.S_un.S_un_b.s_b1 = b1;
	RemoteIpAddress.S_un.S_un_b.s_b2 = b2;
	RemoteIpAddress.S_un.S_un_b.s_b3 = b3;
	RemoteIpAddress.S_un.S_un_b.s_b4 = b4;

	RemoteAddr.sin_family = AF_INET;
	RemoteAddr.sin_port = htons (port);
	RemoteAddr.sin_addr = RemoteIpAddress;

	err = connect(clientsocket, (PSOCKADDR) & RemoteAddr, sizeof (RemoteAddr));
	if (err == SOCKET_ERROR)
	{
		int		exterr = WSAGetLastError();
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧DoEcho: connect failed: %ld\n",exterr );
		SOCKET_TRACE(szError);
		switch( exterr )
		{
		case WSAEWOULDBLOCK:		break;
		default:;
		}
		closesocket(clientsocket);
		clientsocket = INVALID_SOCKET;
		return FALSE;
	}
	// Set the blocking and nonblocking mode
	err = ioctlsocket( clientsocket,FIONBIO,&mode );
	if(err == SOCKET_ERROR)
	{
		DWORD	exterr = WSAGetLastError();
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧Socket Connect Failed,error=%d\n",exterr);
		SOCKET_TRACE(szError);
		if(exterr == WSAEADDRINUSE)
		{
			sprintf_s( szError,sizeof(szError),"⑧Can not set blocking/nonblocking mode.\n" );
			SOCKET_TRACE(szError);
		}
		return FALSE;
	}
	return TRUE;
}

void CSocketBase::Disconnect(SOCKET& socket)
{
	DWORD	err = 0;
	// Close connection to remote host
	//
	if( socket != INVALID_SOCKET )
	{
		err = closesocket (socket);
		socket = INVALID_SOCKET;
	}
	if (err == SOCKET_ERROR)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧closesocket failed: %ld\n",GetLastError() );
		SOCKET_TRACE(szError);
	}
}

int CSocketBase::SendTo(SOCKET& socket,CString ip,int port,LPSTR buffer,DWORD size)
{
	int				b1,b2,b3,b4;
	IN_ADDR			ip_add;
	SOCKADDR_IN		address;
	int				tolen;

	ZeroMemory (&address, sizeof (address));

#ifdef	UNICODE
	swscanf_s( ip,_T("%d.%d.%d.%d"),&b1,&b2,&b3,&b4 );
#else	//UNICODE
	sscanf_s( ip,_T("%d.%d.%d.%d"),&b1,&b2,&b3,&b4 );
#endif	//UNICODE
	ip_add.S_un.S_un_b.s_b1 = (char)b1;
	ip_add.S_un.S_un_b.s_b2 = (char)b2;
	ip_add.S_un.S_un_b.s_b3 = (char)b3;
	ip_add.S_un.S_un_b.s_b4 = (char)b4;

	address.sin_family = AF_INET;
	address.sin_port = htons (port);
	address.sin_addr = ip_add;
	tolen = sizeof(address);
LABEL_SEND:
	int	nSend = sendto( socket,buffer,size,0,(PSOCKADDR) & address,tolen );
	if (nSend == SOCKET_ERROR) {
		int		exterr = WSAGetLastError();
		if (exterr == WSAEWOULDBLOCK) {
			WSAEVENT events[1];
			events[0] = WSACreateEvent();
			WSAEventSelect(socket,events[0],FD_WRITE);
			int nIndex = WSAWaitForMultipleEvents(1,events,false,WSA_INFINITE,false);
			if( nIndex == WSA_WAIT_EVENT_0){
				WSANETWORKEVENTS net_event;
				WSAEnumNetworkEvents(socket,events[0],&net_event);
				if(net_event.lNetworkEvents & FD_WRITE) {
					goto LABEL_SEND;
				}
			}
		} else {
			char	szError[256];

			memset(	szError,0x00,256 );
			sprintf_s( szError,sizeof(szError),"\r\n⑥Error on UDP sendto, error code = %d\n",exterr);
			SOCKET_TRACE(szError);
		}
		return SOCKET_ERROR;
	}
	return nSend;
}

int CSocketBase::RecvFrom(SOCKET& socket,CString& ip,int port,LPSTR buffer,DWORD size)
{
	DWORD	dwIP = IPconvertA2N( ip );
	int	nRecv = RecvFrom(socket,dwIP,port,buffer,size);
	ip = IPconvertN2A( dwIP );
	return nRecv;
}

int CSocketBase::RecvFrom(SOCKET& socket,DWORD& ip,int port,LPSTR buffer,DWORD size)
{
	int				b1 = 0x000000ff & (ip >> 24);
	int				b2 = 0x000000ff & (ip >> 16);
	int				b3 = 0x000000ff & (ip >>  8);
	int				b4 = 0x000000ff & ip;
	IN_ADDR			ip_add;
	SOCKADDR_IN		address;
	int				fromlen;

	ZeroMemory (&ip_add, sizeof (ip_add));
	ZeroMemory (&address, sizeof (address));
	ip_add.S_un.S_un_b.s_b1 = (char)b1;
	ip_add.S_un.S_un_b.s_b2 = (char)b2;
	ip_add.S_un.S_un_b.s_b3 = (char)b3;
	ip_add.S_un.S_un_b.s_b4 = (char)b4;
	address.sin_family = AF_INET;
	address.sin_port = htons (port);
	address.sin_addr = ip_add;
	fromlen = sizeof(address);

	int recvSize = recvfrom( socket,buffer,size,0,(PSOCKADDR) & address,&fromlen );
	if (recvSize <= 0)
	{
		int	exterr = WSAGetLastError();
		char	szError[256];

		memset(	szError,0x00,256 );
		switch( exterr )
		{
		case WSAEWOULDBLOCK:		return 0;	//无数据
		case WSAENOTSOCK:			SOCKET_TRACE("\r\n②没有建立连接");
									return SOCKET_ERROR;	//
		case WSAECONNRESET:			sprintf_s( szError,sizeof(szError),"\r\n③%d.%d.%d.%d 下线",address.sin_addr.S_un.S_un_b.s_b1,
																							address.sin_addr.S_un.S_un_b.s_b2,
																							address.sin_addr.S_un.S_un_b.s_b3,
																							address.sin_addr.S_un.S_un_b.s_b4);
									SOCKET_TRACE(szError);
									return SOCKET_ERROR;	//
		default:;
		}
		sprintf_s( szError,sizeof(szError),"⑥Socket Error: %ld\n",exterr);
		SOCKET_TRACE(szError);
		closesocket(socket);		return SOCKET_ERROR;
	}

	b1 = 0xff000000 & (address.sin_addr.S_un.S_un_b.s_b1<<24);
	b2 = 0x00ff0000 & (address.sin_addr.S_un.S_un_b.s_b2<<16);
	b3 = 0x0000ff00 & (address.sin_addr.S_un.S_un_b.s_b3<< 8);
	b4 = 0x000000ff & address.sin_addr.S_un.S_un_b.s_b4;
	ip = b1 | b2 | b3 | b4;

	return recvSize;
}

int CSocketBase::SendData(SOCKET& socket,LPSTR buffer,DWORD size)
{
	int	result = 0;	

	for( int nRetry = 5000; nRetry > 0; Sleep(50),nRetry -- )
	{
		if( (result=send(socket, buffer, size, 0)) != SOCKET_ERROR ) return result;
		DWORD	exterr = WSAGetLastError();
		if( exterr != WSAEWOULDBLOCK )
		{
			char	szError[256];

			memset(	szError,0x00,256 );
			sprintf_s( szError,sizeof(szError),"⑥Socket Send Failed,error=%d\n",exterr);
			SOCKET_TRACE(szError);
			return 0;
		}
	}
	return 0;
}

int CSocketBase::RecvData(SOCKET& socket,LPSTR buffer,DWORD size)
{
	int		result=0;
	char	szError[256];

	memset(	szError,0x00,256 );	
	result = recv(socket, (LPSTR)buffer, size, 0);
	switch( result )
	{
	case 0:
		sprintf_s( szError,sizeof(szError),"①The remote side has shut down the connection gracefully.\n");
		SOCKET_TRACE(szError);
		Disconnect(socket);
		return -1;
	case SOCKET_ERROR:
		{
			DWORD	exterr = WSAGetLastError();

			sprintf_s( szError,sizeof(szError),"⑤Socket Recv Failed,error=%d\n",exterr );
			switch( exterr )
			{
			case WSAEWOULDBLOCK:	return 0;	//有连接，无数据
			case WSAENOTSOCK:		SOCKET_TRACE("\r\n⑤没有建立连接");
									return 0;	//
//			case WSAECONNRESET:		printf("\n对端下线");		return 0;	//
//			case WSAECONNRESET:		TRACE_LOG(error_log,"The connection has been reset by remote side.\n");
//			case WSAENETDOWN:		TRACE_LOG(error_log,"The Windows Sockets implementation has detected that the network subsystem has failed.\n");
//			case WSAECONNABORTED:	TRACE_LOG(error_log,"The virtual circuit was aborted due to timeout or other failure.\n");
//			case WSAECONNRESET:		TRACE_LOG(error_log,"The virtual circuit was reset by the remote side.\n");
			default:				Disconnect(socket);
									SOCKET_TRACE(szError);
									return -1;
			}
		}
		break;
	default:;
	}
	return result;
}