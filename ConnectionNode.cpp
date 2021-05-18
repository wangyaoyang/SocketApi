#include "stdafx.h"
#include "SocketBase.h"
#include "SocketThread.h"

/////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CConnectionSet::CConnectionSet(__int16 port)
{
	for( int i=0;i<MAX_CONNECTIONS;i++ )
		m_connection[i].m_SetPortNo(port);
}

CConnectionSet::~CConnectionSet()
{
	cleanUp();
}

void CConnectionSet::moveFirst(int& position)
{
	position = 0;
}

void CConnectionSet::moveLast(int& position)
{
	position = MAX_CONNECTIONS-1;
}

void CConnectionSet::moveNext(int& position)
{
	if( position < MAX_CONNECTIONS-1 ) position ++;
		else position = -1;
}

void CConnectionSet::movePrev(int& position)
{
	if( position > 0 ) position --;
		else position = -1;
}

void CConnectionSet::UnLock(int position)
{
	if( position < 0 || position > MAX_CONNECTIONS-1 ) return;
	CString		szEventName;
	HANDLE		hEvent = NULL;
	szEventName.Format( _T("EVENT_SOCKET_CONNECTION_SET_%d"),position );
	if( hEvent = CreateEvent( NULL,TRUE,TRUE,szEventName ) )
	{
		SetEvent( hEvent );
		CloseHandle( hEvent );
	}
}

CConnection* CConnectionSet::Lock(int position)
{
	if( position < 0 || position > MAX_CONNECTIONS-1 ) return NULL;
	CString		szEventName;
	HANDLE		hEvent = NULL;
	szEventName.Format( _T("EVENT_SOCKET_CONNECTION_SET_%d"),position );
	if( hEvent = CreateEvent( NULL,TRUE,TRUE,szEventName ) )
	{
		DWORD	dwWait = WaitForSingleObject( hEvent,100 );
		if( dwWait == WAIT_OBJECT_0 )
		{
			ResetEvent( hEvent );
			CloseHandle( hEvent );
			return &m_connection[position];
		}
		CloseHandle( hEvent );
	}
	return NULL;
}

void CConnectionSet::cleanUp()
{
	for( int pos = 0; pos < MAX_CONNECTIONS; pos ++ )
		if( m_connection[pos].m_IsConnected() )
			m_connection[pos].m_DisConnect();
}
