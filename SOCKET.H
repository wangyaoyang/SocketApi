#ifndef		__SOCKET_H__
#define		__SOCKET_H__

#include "SocketBase.h"

class CSocketServer
{
private:
	HANDLE			m_Thread;
	HANDLE			m_hListenEvent;
	CConnectionSet	m_connectionSet;
	SOCKET			m_listener;
	char*			m_buffer;
public:
	void			Polling();
	BOOL			StartListenThread();
	void			StopListenThread();
////////////////////////////////////////////////////////////////////////////
public:
	CSocketServer(char* buffer, __int16 port, DWORD localIP = 0);
	~CSocketServer();
public:
	int				Broadcast(char* buffer,DWORD size);
	int				Browse(int& index/*output*/);	//index=EOF或返回数据长度值
	int				Recv(int index/*input*/);		//为0，均表示无数据
	int				Scan(int& index/*input*/);		//扫描所有端口
	int				Send(int index/*input*/,char* buffer,DWORD size);
	int				GetConnectionCount();	//返回目前已经被占用的通道数
	DWORD			ClientAdd(int index);
	CConnection*	Lock(int index);
	void			UnLock(int index);
	BOOL			DisConnect(int index);	//断开索引为index的通道
	char*			GetBuffer();	//Browse()后用该函数取出数据缓冲区
};

class CSocketClient
{
private:
	CConnection		m_connection;
///////////////////////////////////////////////////////////////////////
private:
	char*			m_buffer;
public:
	CSocketClient(char* buffer);
	~CSocketClient();
	BOOL			Connect(int b1,int b2,int b3,int b4,__int16 port);
	void			DisConnect();
	BOOL			IsConnected();
	int				Send(char* buffer,DWORD size);
	int				Recv();
	char*			GetBuffer();
	CConnection&	GetConnection() { return m_connection; }
};

class CSocketDatagram
{
///////////////////////////////////////////////////////////////////////
private:
	CConnection		m_connection;
	char*			m_buffer;
	char			m_multicastIP[16];
public:
	CSocketDatagram(char* buffer,__int16 port);
	~CSocketDatagram();
	BOOL			Bind(LPSTR multicastIP,__int16 port = 0, DWORD localIP = 0);
	int				Send(DWORD ip,WORD port,LPSTR buffer,DWORD size);
	int				Send(LPSTR ip,WORD port,LPSTR buffer,DWORD size);
	int				Multicast(WORD port,LPSTR buffer,DWORD size);
	int				Recv(CString& ip,WORD& port);
	int				Recv(DWORD& ip,WORD& port);
	char*			GetBuffer();
	CConnection&	GetConnection() { return m_connection; }
};

#endif	//__SOCKET_H__