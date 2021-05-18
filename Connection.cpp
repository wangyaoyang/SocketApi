#include "stdafx.h"
#include "SocketBase.h"
#include "SocketThread.h"
#include "CommonFacility.h"

/////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CConnection::CConnection(__int16 PortNo)
{
	m_PortNo = PortNo;
	m_socket = INVALID_SOCKET;
}

CConnection::CConnection()
{
	m_dwRemoteAdd = 0;
	m_PortNo = 0;
	m_socket = INVALID_SOCKET;
}

CConnection::~CConnection(void)
{
	m_DisConnect();
}

DWORD CConnection::m_GetRemoteAdd()
{
	return m_dwRemoteAdd;
}

__int16 CConnection::m_GetPortNo()
{
	return m_PortNo;
}

void CConnection::m_SetPortNo(__int16 portNo)
{
	m_PortNo = portNo;
}

BOOL CConnection::m_Connect(int b1,int b2,int b3,int b4,__int16 port)
{
	if( Connect(m_socket,b1,b2,b3,b4,m_PortNo = port) )
	{
		m_dwRemoteAdd = (0xff000000 & (b1 << 24)) |
						(0x00ff0000 & (b2 << 16)) |
						(0x0000ff00 & (b3 <<  8)) |
						(0x000000ff & b4);
		return true;
	}
	else
	{
		m_dwRemoteAdd = 0;
		return false;
	}
}

void CConnection::m_DisConnect()
{
	Disconnect(m_socket);
}

BOOL CConnection::m_IsConnected()
{
	if( m_socket != INVALID_SOCKET ) return TRUE;
		else return FALSE;
}

BOOL CConnection::m_Bind(SOCKET& listener, DWORD localIP)
{
	return Bind( listener,SOCK_STREAM,m_PortNo,localIP);
}

BOOL CConnection::m_Bind(LPSTR multicastIP, DWORD localIP)
{
	return Bind( m_socket,SOCK_DGRAM,m_PortNo,localIP, multicastIP );
}

DWORD CConnection::m_Accept(SOCKET listener)
{
	m_dwRemoteAdd = Accept(listener,m_socket);
	if( m_socket == INVALID_SOCKET )
	{
		DWORD	exterr = WSAGetLastError();
		char	szError[256];
		memset(	szError,0x00,256 );
		m_dwRemoteAdd = 0;
		if(exterr == WSAEWOULDBLOCK)
		{
			sprintf_s( szError,sizeof(szError),"\r\n¢ÛThe socket is marked as nonblocking and no connections are present to be accepted.\n" );
			//SOCKET_TRACE(szError);
		}
		else
		{
			sprintf_s( szError,sizeof(szError),"\r\n¢ßSocket Accept Failed,error=%d\n",exterr);
			SOCKET_TRACE(szError);
		}
		return exterr;
	}
	return 0;
}

int CConnection::m_Send(DWORD ip,WORD port,LPSTR buffer,DWORD size)
{
	char	sIP[16];
	DWORD	b1 = 0x000000ff & (ip>>24);
	DWORD	b2 = 0x000000ff & (ip>>16);
	DWORD	b3 = 0x000000ff & (ip>> 8);
	DWORD	b4 = 0x000000ff & ip;
	memset( sIP,0x00,16 );
	sprintf_s( sIP,sizeof(sIP),"%d.%d.%d.%d",b1,b2,b3,b4 );
	return SendTo( m_socket,CString(sIP),port,buffer,size );
}

int CConnection::m_Send(CString ip,WORD port,LPSTR buffer,DWORD size)
{
	return SendTo( m_socket,ip,port,buffer,size );
}

int CConnection::m_Recv(CString& ip,WORD& port,LPSTR buffer,DWORD& size)
{
	return RecvFrom( m_socket,ip,port,buffer,size );
}

int CConnection::m_Recv(DWORD& ip,WORD& port,LPSTR buffer,DWORD& size)
{
	return RecvFrom( m_socket,ip,port,buffer,size );
}

int CConnection::m_Send(LPSTR buffer,DWORD size)
{
	return SendData( m_socket,buffer,size );
}

int CConnection::m_Recv(LPSTR buffer,DWORD size)
{
	return RecvData( m_socket,buffer,size );
}
