#include "stdafx.h"
#include "SocketBase.h"
#include "SOCKET.H"
#include "CommonFacility.h"

/////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSocketDatagram::CSocketDatagram(char* buffer,__int16 PortNo)
	: m_connection(PortNo)
{
	memset( m_multicastIP,0x00,16 );
	if( (m_buffer = buffer) != NULL )
		memset( m_buffer,0x00,MAX_BUFF_SIZE );
}

CSocketDatagram::~CSocketDatagram()
{
}

char* CSocketDatagram::GetBuffer()
{
	return m_buffer;
}

BOOL CSocketDatagram::Bind(LPSTR multicastIP,__int16 portNo, DWORD localIP)
{
	if( multicastIP ) memcpy( m_multicastIP,multicastIP,15 );
	if( portNo ) m_connection.m_SetPortNo( portNo );
	return m_connection.m_Bind(multicastIP, localIP);
}

int CSocketDatagram::Multicast(WORD port,LPSTR buffer,DWORD size)
{
	if( strlen( m_multicastIP ) > 0 )
		return m_connection.m_Send( MultiByteStr_WideStr(m_multicastIP),port,buffer,size);
	return 0;
}

int CSocketDatagram::Send(DWORD ip,WORD port,LPSTR buffer,DWORD size)
{
	return m_connection.m_Send(ip,port,buffer,size);
}

int CSocketDatagram::Send(LPSTR ip,WORD port,LPSTR buffer,DWORD size)
{
	return m_connection.m_Send( CString(ip),port,buffer,size);
}

int	CSocketDatagram::Recv(CString& ip,WORD& port)
{
	DWORD	size;
	return m_connection.m_Recv(ip,port,m_buffer,size=MAX_BUFF_SIZE);
}

int	CSocketDatagram::Recv(DWORD& ip,WORD& port)
{
	DWORD	size;
	return m_connection.m_Recv(ip,port,m_buffer,size=MAX_BUFF_SIZE);
}
