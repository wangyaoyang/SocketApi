#pragma once

class CListCtrl;
class CPingEcho
{
private:
	long m_nPingCounter;
public:
	int		m_nRecv;
	char	m_sIpReply[32];
	int		m_nTimeDelay;
public:
	CPingEcho(void);
	~CPingEcho(void);
	//m_Ping() return Zero means OK non-Zero means error code
public:
	bool	m_Init();
	int		m_Ping(char* sIP,int nDataSize,int nTimeOut);
	long	m_Ping(CListCtrl& list, unsigned char* b,int nDataSize,int nTimeOut,bool& bFirstPing);
};
