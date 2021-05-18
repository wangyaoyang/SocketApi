#pragma once

#define	MBX_NAME_NETSVC_PROCESSER		_T("MBX_NAME_NET_SVC_PROCESSER")
#define	MBX_NAME_NETSVC_ICMP_SCANNER	_T("MBX_NAME_NET_SVC_ICMP_SCANNER")
#define	MBX_NAME_NETSVC_SNMP_MANAGER	_T("MBX_NAME_NET_SVC_SNMP_MANAGER")

#define		COLOR_GREEN_BLACK		1
#define		COLOR_BLACK_GREEN		2
#define		COLOR_BLUE_BLACK		3
#define		COLOR_BLACK_BLUE		4
#define		COLOR_RED_BLACK			5
#define		COLOR_BLACK_RED			6
#define		COLOR_WHITE_BLACK		7
#define		COLOR_BLACK_WHITE		8

#define		REQUEST_UNKNOWN			0x00000000
#define		REQUEST_ICMP_PING		0x00001000
#define		REQUEST_SNMP_PING		0x00002000
#define		REQUEST_SNMP_MIB_II		0x00003000
#define		REQUEST_ENTERPRISE		0x00004000
////////////////////////////////////////////////////
#define		RESPONSE_SNMP_SUCCESS		0x80000000
#define		RESPONSE_SNMP_NO_MORE		0x70000000
#define		RESPONSE_SNMP_ERROR			0xFFFFFFFF
#define		RESPONSE_SNMP_FAILURE		0xFFFFFFFE
#define		RESPONSE_SNMP_TRAP_HEAD		0x10000000
#define		RESPONSE_SNMP_TRAP_BODY		0x20000000
#define		RESPONSE_SNMP_TRAP_OVER		0x30000000
#define		RESPONSE_SNMP_ENTERPRISE	0x40000000

#define		REQUEST_SNMP_SET			0x00000001
#define		REQUEST_SNMP_GET			0x00000002
#define		REQUEST_SNMP_GETNEXT		0x00000003
#define		REQUEST_SNMP_TABLE			0x00000004
#define		REQUEST_SNMP_CHILDREN		0x00000004
#define		REQUEST_SNMP_TABLE_ROW		0x00000005
#define		REQUEST_SNMP_INSTANCES		0x00000005
#define		REQUEST_SNMP_TABLE_COL		0x00000006
///////////////////////////////////////////////////
#define		RESPONSE_SNMP_SET			0x00010000
#define		RESPONSE_SNMP_GET			0x00020000
#define		RESPONSE_SNMP_GETNEXT		0x00030000
#define		RESPONSE_SNMP_TABLE			0x00040000
#define		RESPONSE_SNMP_CHILDREN		0x00040000
#define		RESPONSE_SNMP_TABLE_ROW		0x00050000
#define		RESPONSE_SNMP_INSTANCES		0x00050000
#define		RESPONSE_SNMP_TABLE_COL		0x00060000

#define		REQUEST_USER_SELECT			0x00000007
#define		REQUEST_USER_INSERT			0x00000008
#define		REQUEST_USER_UPDATE			0x00000009
#define		REQUEST_USER_DELETE			0x0000000A
#define		REQUEST_LOG_ON				0x0000000B
#define		REQUEST_INIT_MIT			0x0000000C
#define		REQUEST_INIT_MIB			0x0000000D
#define		REQUEST_LOAD_MIB			0x0000000E

#define		REQUEST_QUERY_VT			0x0000000F
#define		REQUEST_EVENT_ACK			0x00000010
///////////////////////////////////////////////////
#define		RESPONSE_QUERY_VT			0x000F0000
#define		RESPONSE_EVENT_ACK			0x00100000

#define	MIN(x,y)		(x < y ? x : y)
#define	MAX(x,y)		(x > y ? x : y)

#ifndef	u_int8_t
#define	u_int8_t	unsigned char
#define	u_int16_t	unsigned short
#define	u_int32_t	unsigned int
#endif

#ifndef	ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6  
#endif

class CSocketDatagram;
class CCommonFacility
{
public:
	CCommonFacility(void);
	~CCommonFacility(void);
public:
	void m_EnableTrace(bool bEnable,LPSTR sIP);
public:
	static void	m_TRACE(LPCSTR msg,DWORD color = COLOR_WHITE_BLACK);
	static void	m_TRACE(CString msg,DWORD color = COLOR_WHITE_BLACK);
};

#define V_INT8			0x81
#define V_INT16			0x82
#define V_CHAR			0x83
#define V_UCHAR			0x84
#define V_FLOAT			0x85
#define V_DOUBLE			0x86
#define	MAX_LENGTH_NAME	32

#include "Winsnmp.h"

void SOCKET_TRACE(char* msg);
int		WideStr_MultiByteStr( CString szString,LPSTR sString,int nMax );
CString	MultiByteStr_WideStr( LPSTR sString );
CString	IPconvertN2ANew(DWORD nIP,bool b0_in_hh=true);
CString	IPconvertN2A(DWORD nIP,bool b0_in_hh=true);
DWORD	IPconvertA2N(CString szIP,bool b0_in_hh=true);
char Convert_Bcd2A( char* bcd,double* flt );
bool Convert_A2Bcd( double fltParam,char* bcdParam,char unit);
short AsnName2shortName( smiOID& asnName,short sName[] );
void interchangeWordArray( short wordArray[],int nWords );
void wordArray2textString( short wordArray[],int nWords,char* textString,bool exchange );
CString WordArray2DisplayString( short wordArray[],int nWords,bool exchange );
void DisplayString2WordArray( short wordArray[],int& nWords,LPCTSTR displayString,bool exchang );
long Ping(CListBox& list, char* sIpAddress,int nDataSize,int nTimeOut,bool& bFirstPing);
