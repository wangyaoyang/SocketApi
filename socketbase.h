#ifndef		__WYY_SOCKET_H__
#define		__WYY_SOCKET_H__

#define	MAX_BUFF_SIZE				65536
#define	MAX_CONNECTIONS			256


#define	SOCKET_SYNCHRONOUS	0	//	blocking mode,		accept() 等待结果后才返回
#define	SOCKET_ASYNCHRONOUS	1	//	nonblocking mode，	accept() 立即返回

class CSocketBase
{
private:
	unsigned char m_host_ip[4];
public:
	CSocketBase();
	~CSocketBase();
	BOOL	CheckSocketVersion();
public:
	BOOL	Bind(SOCKET& opt_socket,int type, __int16 port, DWORD localIP,LPSTR multicastIP=NULL);
	DWORD	Accept(SOCKET listener,SOCKET& sock);
	BOOL	Connect(SOCKET& socket,int b1,int b2,int b3,int b4,int port);
	int		SendData(SOCKET& socket,LPSTR buffer,DWORD size);					/* TCP/IP */
	int		RecvData(SOCKET& socket,LPSTR buffer,DWORD size);					/* TCP/IP */
	int		SendTo(SOCKET& socket,CString ip,int port,LPSTR buffer,DWORD size);	/* UDP/IP */
	int		RecvFrom(SOCKET& socket,CString& ip,int port,LPSTR buffer,DWORD size);	/* UDP/IP */
	int		RecvFrom(SOCKET& socket,DWORD& ip,int port,LPSTR buffer,DWORD size);	/* UDP/IP */
	void	Disconnect(SOCKET& socket);
};

class CConnection : public CSocketBase
{
private:
	DWORD		m_dwRemoteAdd;
	__int16		m_PortNo;
	SOCKET		m_socket;
public:
	CConnection(__int16 PortNo);
	CConnection();
	~CConnection();
	DWORD		m_GetRemoteAdd();
	__int16		m_GetPortNo();
	void		m_SetPortNo(__int16 portNo);
public:
	BOOL		m_Connect(int b1,int b2,int b3,int b4,__int16 port);
	BOOL		m_Bind(SOCKET& listener, DWORD localIP);		//	for TCP/IP (stream)
	BOOL		m_Bind(LPSTR multicastIP, DWORD localIP);		//	for UDP/IP (datagram)
	DWORD		m_Accept(SOCKET socket);
	BOOL		m_IsConnected();
	void		m_DisConnect();
	int			m_Send(CString ip,WORD port,LPSTR buffer,DWORD size);
	int			m_Send(DWORD ip,WORD port,LPSTR buffer,DWORD size);
	int			m_Recv(CString& ip,WORD& port,LPSTR buffer,DWORD& size);
	int			m_Recv(DWORD& ip,WORD& port,LPSTR buffer,DWORD& size);
	int			m_Send(LPSTR buffer,DWORD size);
	int			m_Recv(LPSTR buffer,DWORD size);
};

class CConnectionSet		//实现链表操作，与CObList类似
{
private:
	CConnection			m_connection[MAX_CONNECTIONS];
public:
	CConnectionSet(__int16 port);
	~CConnectionSet();
public:
	void			moveFirst(int& position);
	void			moveLast(int& position);
	void			moveNext(int& position);
	void			movePrev(int& position);
	CConnection*	Lock(int position);
	void			UnLock(int position);
	void			cleanUp();
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
#endif		//__WYY_SOCKET_H__