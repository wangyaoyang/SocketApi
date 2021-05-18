#include "stdafx.h"
#include "SocketThread.h"
#include "SOCKET.H"
#include "CommonFacility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char	error_log[1024];

BOOL TestEventState(HANDLE& hEvent)
{
	DWORD	dwWait = 0;

	if( hEvent == NULL ) return TRUE;
	dwWait = WaitForSingleObject( hEvent,0 );
	switch( dwWait )
	{
	case WAIT_OBJECT_0:		break;
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
	default:				return FALSE;
	}
	//CloseHandle( hEvent );
	//hEvent = NULL;
	return TRUE;
}

HANDLE StartSocketThread(LPTHREAD_START_ROUTINE pThread,LPVOID pParam)
{	//启动一个线程，第一个参数是线程函数名，第二个参数是线程参数指针
	HANDLE	hThread=NULL;
	DWORD	dwThreadId;
	
	hThread = CreateThread(
		NULL,			// use default security attributes
		0,				// use default thread stack size
		pThread,		// pointer to thread function
		pParam,			// argument for new thread
		0,				// creation flags 
		&dwThreadId);	// pointer to returned thread identifier
	if(hThread == NULL)
	{
		char	szError[256];

		memset(	szError,0x00,256 );
		sprintf_s( szError,sizeof(szError),"⑧Failed create thread,error = %d\n",GetLastError() );
		SOCKET_TRACE(szError);
		return NULL;
	}
	return hThread;
}

UINT ListenConnections(LPVOID lp)
{
	CSocketServer*	pServer = (CSocketServer*) lp;
	CSocketServer&	Server = *pServer;

	Server.Polling();
	return 0;
}