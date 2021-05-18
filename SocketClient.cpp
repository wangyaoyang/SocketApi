#include "stdafx.h"
#include "SocketBase.h"
#include "SOCKET.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSocketClient::CSocketClient(char* buffer)
	: m_connection(0x0000)
{
	if( (m_buffer = buffer) != NULL )
		memset( m_buffer,0x00,MAX_BUFF_SIZE );
}

CSocketClient::~CSocketClient()
{
}

char* CSocketClient::GetBuffer()
{
	return m_buffer;
}

int CSocketClient::Recv()
{
	return m_connection.m_Recv(m_buffer,MAX_BUFF_SIZE);
}

int CSocketClient::Send(char* buffer,DWORD size)
{
	return m_connection.m_Send(buffer,size);
}

int CSocketClient::Connect(int b1,int b2,int b3,int b4,__int16 port)
{
	return m_connection.m_Connect( b1,b2,b3,b4,port );
}

void CSocketClient::DisConnect()
{
	m_connection.m_DisConnect();
}

int CSocketClient::IsConnected()
{
	return m_connection.m_IsConnected();
}