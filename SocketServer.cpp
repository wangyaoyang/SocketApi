#include "stdafx.h"
#include "SocketBase.h"
#include "SocketThread.h"
#include "SOCKET.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSocketServer::CSocketServer(char* buffer,__int16 PortNo, DWORD localIP)
	: m_connectionSet(PortNo)
{
	if( (m_buffer = buffer) != NULL )
		memset( m_buffer,0x00,MAX_BUFF_SIZE );
	m_hListenEvent = NULL;
	m_listener = INVALID_SOCKET;
	CConnection*	_1stConnection = m_connectionSet.Lock(0);
	if( !_1stConnection ) return;
	if( !_1stConnection->m_Bind(m_listener, localIP) ) m_connectionSet.cleanUp();
	m_connectionSet.UnLock(0);
}

CSocketServer::~CSocketServer()
{
	if( m_hListenEvent ) CloseHandle( m_hListenEvent );
	m_connectionSet.cleanUp();
}

BOOL CSocketServer::StartListenThread()
{	//���������߳�
	if( m_hListenEvent != NULL ) return FALSE;
	CString			szEventName;
	CConnection*	conn = m_connectionSet.Lock(0);
	__int16			nPortNo = conn->m_GetPortNo();
	m_connectionSet.UnLock(0);
	szEventName.Format( _T("SOCKET_SERVER_LISTEN_EVENT_%d"),nPortNo );
	m_hListenEvent = CreateEvent(NULL, TRUE, FALSE, szEventName);
	if( (m_Thread = StartSocketThread((LPTHREAD_START_ROUTINE)
		ListenConnections,this )) == NULL ) return FALSE;
	return TRUE;
}

void CSocketServer::StopListenThread()
{
    if( m_hListenEvent ) SetEvent(m_hListenEvent);
	//Sleep(2000);
	//if( m_Thread ) TerminateThread( m_Thread,0x7212 );
}

void CSocketServer::Polling()
{
	int				pos = 0;
	DWORD			exterr = 0;
	CString			szEventName;
	CConnection*	conn = m_connectionSet.Lock(0);
	__int16			nPort = conn->m_GetPortNo();
	
	m_connectionSet.UnLock(0);
	szEventName.Format( _T("SOCKET_SERVER_LISTEN_EVENT_%d"),nPort );
	HANDLE			hEvent = OpenEvent( EVENT_ALL_ACCESS,TRUE,szEventName );
	while( !TestEventState( hEvent ) )
	{
		for( m_connectionSet.moveFirst(pos); pos != -1; m_connectionSet.moveNext(pos),Sleep(100) )
		{
			CConnection*	connection = m_connectionSet.Lock(pos);
			if( connection && connection->m_IsConnected() == FALSE )
			{	// �統ǰ�ڵ���У������õ�ǰ�ڵ����Ƿ��к�������
				exterr=connection->m_Accept(m_listener);
				switch( exterr )
				{
				case 0:					//�к��룬Accept�ɹ���������һ�ڵ����̽ѯ��
					TRACE("\nChannel [%d] : Accpet a connection.\n",pos);
					break;
				case WSAEWOULDBLOCK:	//�޺��룬Acceptʧ�ܣ��пսڵ㵫�޺���,��ͷ��ѯ��
					m_connectionSet.UnLock(pos);
					goto START_POLLING;
				default:;
				}
				m_connectionSet.UnLock(pos);
			} // �統ǰ�ڵ��ѱ�ռ�ã���ʹ����һ�ڵ㡣
			if( connection ) m_connectionSet.UnLock(pos);
		} //ѭ�����������������ͨ������ռ�á�
		START_POLLING: Sleep( 200 );
	}
	CloseHandle(hEvent);
	TRACE("\nSocket Stop Polling");
}

char* CSocketServer::GetBuffer()
{
	return m_buffer;
}

//	���
int CSocketServer::Browse(int& index)
{
	int		position = 0;
	int		length = 0;
	for( m_connectionSet.moveFirst(position); position != -1; m_connectionSet.moveNext(position) )
	{
		CConnection*	connection = m_connectionSet.Lock(position);
		if( !connection ) continue;
		if( !connection->m_IsConnected() ) { m_connectionSet.UnLock(position);	continue; }
		length = connection->m_Recv( m_buffer,MAX_BUFF_SIZE );
		switch( length )
		{
		case EOF:	//�������Ѿ��ж�
			connection->m_DisConnect();
			index = EOF;
			break;
		case 0:		//�����ݶ���
			index = EOF;
			break;
		default:	//��������
			m_connectionSet.UnLock(position);
			index = position;
			return length;
		}
		m_connectionSet.UnLock(position);
	}
	return length;
}

int CSocketServer::Recv(int index/*input*/)
{
	if( index < 0 || index > MAX_CONNECTIONS-1 ) return 0;
	CConnection*	connection = m_connectionSet.Lock(index);
	if( connection && connection->m_IsConnected() )
	{
		int	nRecv = connection->m_Recv( m_buffer,MAX_BUFF_SIZE );
		m_connectionSet.UnLock(index);
		return nRecv;
	}
	if( connection ) m_connectionSet.UnLock(index);
	return 0;
}

int CSocketServer::Send(int index,char* buffer,DWORD size)
{
	if( index < 0 || index > MAX_CONNECTIONS-1 ) return 0;
	CConnection*	connection = m_connectionSet.Lock(index);
	if( connection && connection->m_IsConnected() )
	{
		int	nSend = connection->m_Send(buffer,size);
		if( nSend <= 0 ) connection->m_DisConnect();
		m_connectionSet.UnLock(index);
		return nSend;
	}
	if( connection ) m_connectionSet.UnLock(index);
	return 0;	
}

int CSocketServer::Scan(int& index/*input*/)		//Ϊ0������ʾ������
{
	int		pos = 0,counter = 0;

	for( m_connectionSet.moveFirst(pos); pos != -1;
		 m_connectionSet.moveNext(pos) )
	{
		if( (counter = Recv(pos)) > 0 ) return counter;
	}
	return counter;
}

int	CSocketServer::Broadcast(char* buffer,DWORD size)
{
	int		pos = 0,counter = 0;

	for( m_connectionSet.moveFirst(pos); pos != -1;
		 m_connectionSet.moveNext(pos) )
	{
		if( Send( pos,buffer,size ) > 0 ) counter ++;
	}
	return counter;
}

BOOL CSocketServer::DisConnect(int position)
{
	if( position < 0 || position > MAX_CONNECTIONS-1 ) return FALSE;
	CConnection*	connection = m_connectionSet.Lock( position );
	if( connection && connection->m_IsConnected() )
	{
		connection->m_DisConnect();
		m_connectionSet.UnLock( position );
		return TRUE;
	}
	if( connection ) m_connectionSet.UnLock( position );
	return FALSE;
}
void CSocketServer::UnLock(int index)
{
	if( index < 0 || index > MAX_CONNECTIONS-1 ) return;
	m_connectionSet.UnLock(index);
}


CConnection* CSocketServer::Lock(int index)
{
	if( index < 0 || index > MAX_CONNECTIONS-1 ) return NULL;
	return m_connectionSet.Lock(index);
}

int CSocketServer::GetConnectionCount()
{
	int		position = 0,count = 0;
	for( m_connectionSet.moveFirst(position); position != -1;
		 m_connectionSet.moveNext(position) )
	{
		CConnection*	connection = m_connectionSet.Lock(position);
		if( connection && connection->m_IsConnected() ) count ++;
		if( connection ) m_connectionSet.UnLock(position);
	}
	return count;
}

DWORD CSocketServer::ClientAdd(int index)
{
	DWORD ip = 0;
	if( index < 0 || index > MAX_CONNECTIONS-1 ) return 0;
	CConnection* connection = m_connectionSet.Lock(index);
	if( connection && connection->m_IsConnected() )
		ip = connection->m_GetRemoteAdd();
	m_connectionSet.UnLock(index);
	return ip;
}

