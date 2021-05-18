#ifndef		__WYY_THREAD_H__
#define		__WYY_THREAD_H__

HANDLE	StartSocketThread(LPTHREAD_START_ROUTINE pThread,LPVOID pParam);
UINT	ListenConnections(LPVOID lp);
UINT	DataTransmission(LPVOID lp);
BOOL	TestEventState(HANDLE& hEvent);

#endif		//__WYY_THREAD_H__