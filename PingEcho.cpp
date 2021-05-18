#include "StdAfx.h"
#include "pingecho.h"
#include "ipexport.h"
#include "icmpapi.h"
#include "CommonFacility.h"

// CPingEchoThread

typedef struct _iphdr 
{
	unsigned int h_len:4; // Length of the header
	unsigned int version:4; // Version of IP
	unsigned char tos; // Type of service
	unsigned short total_len; // Total length of the packet
	unsigned short ident; // Unique identifier
	unsigned short frag_and_flags; // Flags
	unsigned char ttl; // Time to live
	unsigned char proto; // Protocol (TCP, UDP etc)
	unsigned short checksum; // IP checksum

	unsigned int sourceIP;
	unsigned int destIP;
} IPHEADER;

//
// ICMP header structure
//
typedef struct _icmphdr 
{
	BYTE i_type;
	BYTE i_code; // Type sub code
	USHORT i_cksum;
	USHORT i_id;
	USHORT i_seq;
	// This is not the standard header, but we reserve space for time
	ULONG timestamp;
} ICMPHEADER;

//
// IP option header - use with socket option IP_OPTIONS
//
typedef struct _ipoptionhdr
{
	unsigned char code; // Option type
	unsigned char len; // Length of option hdr
	unsigned char ptr; // Offset into options
	unsigned long addr[9]; // List of IP addrs
} IPOPTIONHEADER;


////////////////////////////////////////////////////////
CPingEcho::CPingEcho(void)
{
	this->m_nRecv = 0;
	this->m_nTimeDelay = 0;
	this->m_nPingCounter = 0;
	memset( m_sIpReply,0x00,32 );
}

CPingEcho::~CPingEcho(void)
{
}


bool CPingEcho::m_Init()
{
	//WSADATA		wsaData;
	//DWORD		err = 0;

	//err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//if(err == SOCKET_ERROR)
	//{
	//	printf("\nWSAStartup Failed\n");
	//	return false;
	//}
	return true;
}

#define		MAX_ICMP_DATA_SIZE	1024
int CPingEcho::m_Ping(char* sIpAddress,int nDataSize,int nTimeOut)
{
	if( nDataSize <= 0 ) return 1;
	if( nTimeOut <= 0 ) return 1;
	if( !m_Init() ) GetLastError();
	if( nDataSize > MAX_ICMP_DATA_SIZE ) nDataSize = MAX_ICMP_DATA_SIZE;
	if( nDataSize < 1 ) nDataSize = 1;

	HANDLE	hIcmpFile = IcmpCreateFile();
	if( hIcmpFile && INVALID_HANDLE_VALUE != hIcmpFile ) {
		int			nErrorCode = 0;
		BYTE		RequestData[MAX_ICMP_DATA_SIZE];
		size_t		nReplySize = sizeof(ICMP_ECHO_REPLY) + nDataSize;
		char		ReplyBuffer[sizeof(ICMP_ECHO_REPLY) + MAX_ICMP_DATA_SIZE];
		
		memset( RequestData,'E',nDataSize );
		memset( ReplyBuffer,0x00,nReplySize );
		DWORD	nReply = IcmpSendEcho(	hIcmpFile,		inet_addr(sIpAddress),
										RequestData,	nDataSize,
										NULL,//PIP_OPTION_INFORMATION RequestOptions,
										ReplyBuffer,	nReplySize,
										nTimeOut
										);
		PICMP_ECHO_REPLY	pReply = (PICMP_ECHO_REPLY)ReplyBuffer;
		if( nReply > 0 && 0 == pReply->Status ) {
			CString			szIP = IPconvertN2A(pReply->Address,false);
			m_nRecv			= pReply->DataSize;
			m_nTimeDelay	= pReply->RoundTripTime;
			WideStr_MultiByteStr( szIP,m_sIpReply,sizeof(m_sIpReply) );
		} else {
			nErrorCode = GetLastError();
			if( 0 == nErrorCode ) nErrorCode = pReply->Status;
		}
		IcmpCloseHandle( hIcmpFile );
		return nErrorCode;
	} else {
		return GetLastError();
	}
}

long CPingEcho::m_Ping(CListCtrl& list, unsigned char* b,int nDataSize,int nTimeOut,bool& bFirstPing) {
	if( nDataSize <= 0 ) return -1;
	if( nTimeOut <= 0 ) return -1;

	char					RequestData[10240];
	ICMP_ECHO_REPLY			ReplyBuff[10240];

	if (b == NULL || b[0] == 0 || b[3] == 0) return -1;
	memset( RequestData,'E',nDataSize );
	memset( ReplyBuff,0x00,16*sizeof(ICMP_ECHO_REPLY) );
	IPAddr	DestinationAddress = ((b[3] << 24) & 0xff000000) |
								 ((b[2] << 16) & 0x00ff0000) |
								 ((b[1] << 8 ) & 0x0000ff00) |
								 ((b[0] << 0 ) & 0x000000ff);
	
	HANDLE	hIcmpFile = IcmpCreateFile();
	if( hIcmpFile ) {
		int		nItem = list.GetItemCount();
		CString	szEcho,szError;
		DWORD	nReply = IcmpSendEcho(	hIcmpFile,
										DestinationAddress,
										RequestData,
										nDataSize,
										NULL,//PIP_OPTION_INFORMATION RequestOptions,
										ReplyBuff,
										sizeof(ReplyBuff),
										nTimeOut
										);
		//CloseHandle( hIcmpFile );
		if( bFirstPing ) {
			this->m_nPingCounter = 0;
			bFirstPing = false;
			list.InsertItem(nItem, _T("========"));
			return 0;
		} else if( nReply > 0 ) {
			int b4 = (ReplyBuff[0].Address >> 24) & 0x000000ff;
			int b3 = (ReplyBuff[0].Address >> 16) & 0x000000ff;
			int b2 = (ReplyBuff[0].Address >> 8 ) & 0x000000ff;
			int b1 = (ReplyBuff[0].Address >> 0 ) & 0x000000ff;
			szEcho.Format(_T("%d.%d.%d.%d, seq=% 5d, "), b1,b2,b3,b4, this->m_nPingCounter ++);
			switch (ReplyBuff[0].Status) {
			case IP_SUCCESS:
				szEcho = _T("From ") + szEcho;
				szError.Format(_T("%d bytes, time<=%d ms"),
						ReplyBuff[0].DataSize, ReplyBuff[0].RoundTripTime);
				break;
			case IP_BUF_TOO_SMALL:			szError = _T("The reply buffer was too small.");			break;
			case IP_DEST_NET_UNREACHABLE:	szError = _T("The destination network was unreachable.");	break;
			case IP_DEST_HOST_UNREACHABLE:	szError = _T("The destination host was unreachable.");		break;
			case IP_DEST_PROT_UNREACHABLE:	szError = _T("The destination protocol was unreachable.");	break;
			case IP_DEST_PORT_UNREACHABLE:	szError = _T("The destination port was unreachable.");		break;
			case IP_NO_RESOURCES:			szError = _T("Insufficient IP resources were available.");	break;
			case IP_BAD_OPTION:			szError = _T("A bad IP option was specified.");			break;
			case IP_HW_ERROR:				szError = _T("A hardware error occurred.");				break;
			case IP_PACKET_TOO_BIG:			szError = _T("The packet was too big.");					break;
			case IP_REQ_TIMED_OUT:			szError = _T("The request timed out.");					break;
			case IP_BAD_REQ:				szError = _T("A bad request.");							break;
			case IP_BAD_ROUTE:				szError = _T("A bad route.");							break;
			case IP_TTL_EXPIRED_TRANSIT:	szError = _T("The time to live (TTL) expired in transit.");	break;
			case IP_TTL_EXPIRED_REASSEM:	szError = _T("The time to live expired during fragment\
														 reassembly.");						break;
			case IP_PARAM_PROBLEM:			szError = _T("A parameter problem. ");					break;
			case IP_SOURCE_QUENCH:			szError = _T("Datagrams are arriving too fast to be processed \
													and datagrams may have been discarded.");		break;
			case IP_OPTION_TOO_BIG:			szError = _T("An IP option was too big. ");				break;
			case IP_BAD_DESTINATION:		szError = _T("A bad destination. ");						break;
			case IP_GENERAL_FAILURE:		szError = _T("A general failure. This error can be returned \
														 for some malformed ICMP packets.");		break;
			default:						szError = _T("Unknown error.");
			}
			
			list.InsertItem(nItem, szEcho + szError);
			list.EnsureVisible(nItem, TRUE);
			return (ReplyBuff[0].Status <= 0) ? ReplyBuff[0].RoundTripTime : (-ReplyBuff[0].Status);
		} else {
			szEcho.Format(_T("%d.%d.%d.%d, seq=% 5d,    Timeout !!!"),
							b[0],b[1],b[2],b[3], this->m_nPingCounter ++);	//ReplyBuff[0].DataSize
			list.InsertItem(nItem, szEcho);
			list.EnsureVisible(nItem, TRUE);
			return -1;
		}
	} else return (-1 * GetLastError());
}
