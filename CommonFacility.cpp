#include "StdAfx.h"
#include "CommonFacility.h"
#include <IPHlpApi.h>
#include <IcmpAPI.h>

/////////////////////////////////////////////////////////////////////////////////////////
CCommonFacility::CCommonFacility(void)
{
}

CCommonFacility::~CCommonFacility(void)
{
}

#define	W_TRACE(fmt,...)	printf(fmt, ##__VA_ARGS__)
void SOCKET_TRACE(char* msg)
{
//	char	sMessage[2048];
//
//	memset(	sMessage,0x00,2048 );
//#ifdef	UNICODE
//	WideStr_MultiByteStr( msg,sMessage,2047 );
//#else	//UNICODE
//	strcpy_s( sMessage,2048,msg.GetBuffer() );	//strcpy_s( sMessage,2047,msg.GetBuffer() );
//#endif	//UNICODE
	W_TRACE(msg);
}

void SOCKET_TRACE(LPCSTR msg,DWORD color)
{
	WORD	attr = 0;
	switch( color )
	{
	case COLOR_GREEN_BLACK:	attr = FOREGROUND_GREEN;	break;
	case COLOR_BLACK_GREEN:	attr = BACKGROUND_GREEN;	break;
	case COLOR_BLUE_BLACK:	attr = FOREGROUND_BLUE;		break;
	case COLOR_BLACK_BLUE:	attr = BACKGROUND_BLUE;		break;
	case COLOR_RED_BLACK:	attr = FOREGROUND_RED;		break;
	case COLOR_BLACK_RED:	attr = BACKGROUND_RED;		break;
	case COLOR_WHITE_BLACK:	attr = 0x000f;				break;
	case COLOR_BLACK_WHITE:	attr = 0x00f0;				break;
	default:				attr = 0x00f0;
	}
	if(	color )
	{
		TRACE(msg);
		//SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE),attr );
		//printf(msg);
		//SetConsoleTextAttribute( GetStdHandle(STD_OUTPUT_HANDLE),COLOR_WHITE_BLACK );
		//LPCTSTR		szName = AfxGetAppName();
		//CString		szMail = CString("MBX_NAME_HCINM_SERVICE_MANAGER_") + szName;
		//static_TraceMail.SendMail( szMail,(LPSTR)msg,(DWORD)strlen(msg) );
	}
}

CString ip_2_string(DWORD ip) {
	CString	szIP;
	u_char*	uc_ip = (u_char*) &ip;
	szIP.Format(_T("%d.%d.%d.%d"), uc_ip[0],uc_ip[1],uc_ip[2],uc_ip[3]);
	return szIP;
}

CString mac_2_string(unsigned char* mac) {
	CString	szMAC;
	if (mac) {
		szMAC.Format(_T("%02X:%02X:%02X:%02X:%02X:%02X"),
			mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	}
	return szMAC;
}

unsigned long string_2_ip(CString szIP) {
	static unsigned char ip[4];
	int		nIP = 0;
	CString	szByte;

	memset (ip,0x00,4);
	for(int i = 0; i < 4 && !szIP.IsEmpty(); i ++) {
		int len = szIP.GetLength();
		int sep = szIP.Find(_T("."));
		if (sep > 0) {
			szByte = szIP.Left(sep);
			szIP = szIP.Right(len - sep - 1);
		} else if (i == 3) {	//this is the last byte
			szByte = szIP;
			szIP.Empty();
		} else break;	//something wrong
		swscanf_s(szByte,_T("%d"),&nIP);
		ip[i] = (unsigned char) nIP;
	}
	return *(unsigned long*)ip;
}

unsigned char* string_2_mac(CString szMac) {
	static unsigned char mac[ETHER_ADDR_LEN];
	int		nMac = 0;
	CString	szByte;

	memset (mac,0x00,ETHER_ADDR_LEN);
	for(int i = 0; i < ETHER_ADDR_LEN && !szMac.IsEmpty(); i ++) {
		int len = szMac.GetLength();
		int sep = szMac.Find(_T(":"));
		if (sep > 0) {
			szByte = szMac.Left(sep);
			szMac = szMac.Right(len - sep - 1);
		} else if (i == (ETHER_ADDR_LEN - 1)) {	//this is the last byte
			szByte = szMac;
			szMac.Empty();
		} else break;	//something wrong
		swscanf_s(szByte,_T("%x"),&nMac);
		mac[i] = (unsigned char) nMac;
	}
	return mac;
}

u_int16_t in_cksum (u_int16_t * addr, int len)  
{  
    int     nleft = len;  
    u_int32_t sum = 0;  
    u_int16_t *w = addr;  
    u_int16_t answer = 0;  
  
    /* 
    * Our algorithm is simple, using a 32 bit accumulator (sum), we add 
    * sequential 16 bit words to it, and at the end, fold back all the 
    * carry bits from the top 16 bits into the lower 16 bits. 
    */  
    while (nleft > 1) {  
        sum += *w++;  
        nleft -= 2;  
    }  
    /* mop up an odd byte, if necessary */  
    if (nleft == 1) {  
        * (unsigned char *) (&answer) = * (unsigned char *) w;  
        sum += answer;  
    }  
  
    /* add back carry outs from top 16 bits to low 16 bits */  
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */  
    sum += (sum >> 16);     /* add carry */  
    answer = ~sum;     /* truncate to 16 bits */  
    return (answer);
}  


char Convert_Bcd2A( char* bcd,double* flt )
{
	double	result = 0.0f;
	char	flag,unit;
	char	bit100,bit10,bit1,bit01,bit001;

	if( bcd[0] & 0x80 ) flag = -1; else flag = 1;
	bit001 = bcd[1] & 0x0F;
	bit01 = (bcd[1] & 0xF0) >> 4;
	bit1 = bcd[2] & 0x0F;
	bit10 = (bcd[3] & 0xF0) >> 4;
	bit100 = bcd[3] & 0x0F;
	unit = (bcd[2] & 0xF0)>>4;

	result = bit100 * 100 + bit10 * 10 + bit1 + bit01 * 0.1 + bit001 * 0.01;
	*flt = result * flag;

	return unit;
}

bool Convert_A2Bcd( double fltParam,char* bcdParam,char unit)
{
	int		decimal=0;
	int		sign=0;
	int		b100=0;
	int		b10=0;
	int		b1=0;
	int		b01=0;
	int		b001=0;
	char	strParam[8];

	memset( bcdParam, 0, 4 );
	memset( strParam, 0, 8 );
	if( _fcvt_s( strParam, 8, fltParam, 2, &decimal, &sign ) != 0 ) return false;
	if( fltParam >= 1000 ) return false;
	if( sign ) bcdParam[0] |= 0x80;
	if( decimal>2 ) b100 = strParam[ decimal-3 ] - 48; else b10 = 0;
	if( decimal>1 ) b10 = strParam[ decimal-2 ] - 48; else b10 = 0;
	if( decimal>0 ) b1 = strParam[ decimal-1 ] - 48; else b1 = 0;
	if( (b01 = strParam[ decimal ] - 48)<0 ) b01 = 0;
	if( (b001 = strParam[ decimal+1 ] - 48)<0 ) b001 = 0;
	bcdParam[1] = (b01 << 4) | b001;
	bcdParam[2] = (unit << 4 ) | b1;
	bcdParam[3] = (b10 << 4) | b100;
	return true;
}  

////////////////////////////////////////////////////////////////////////////
short AsnName2shortName( smiOID& asnName,short sName[] )
{
	short*	sOid = sName;
	short	nOid = (short)asnName.len;
	short	i = 0;
	for( i = 0; i < nOid && i < MAX_LENGTH_NAME; i ++ )
	{
		sOid[i] = (short)asnName.ptr[i];
	}
	interchangeWordArray( sName,i );
	return (i * sizeof(short));
}

void interchangeWordArray( short wordArray[],int nWords )
{
	short	lo = 0;
	short	hi = 0;
	for( int i = 0; i < nWords; i ++ )
	{
		lo =  0x00ff&wordArray[i];
		hi = (0xff00&wordArray[i]) >> 8;
		wordArray[i] = (lo<<8) | hi;
	}
}

void wordArray2textString( short wordArray[],int nWords,char* textString,bool exchange )
{
	memset( textString,0x00, 4*nWords + 3 );
	sprintf_s( textString,4*nWords + 3,"0x" );
	for( int i = 0; i < nWords; i ++ )
	{
		int		lo = exchange ? (0x000000ff&wordArray[i]) << 8 : wordArray[i];
		int		hi = exchange ? (0x0000ff00&wordArray[i]) >> 8 : wordArray[i];
		int		word = (0x0000ffff) & (hi | lo);
		sprintf_s( textString,4*nWords + 3,"%s%04x",textString,word );
	}
}

CString WordArray2DisplayString( short wordArray[],int nWords,bool exchange )
{
	CString	szDisplay;
	for( int i = 0; i < nWords; i ++ )
	{
		short	lo = exchange ? (0x00ff&wordArray[i]) << 8 : wordArray[i];
		short	hi = exchange ? (0xff00&wordArray[i]) >> 8 : wordArray[i];
		short	word = hi | lo;
		CString	szNum;	szNum.Format(_T("%d."),word );
		szDisplay += szNum;
	}
	return szDisplay;
}


void DisplayString2WordArray( short wordArray[],int& nWords,LPCTSTR displayString,bool exchang )
{
	CString	dispStr = displayString;
	int		nCount = 0,nDotPos = -1;

	while( (nDotPos = dispStr.Find('.')) != -1 && nCount < nWords )
	{
		CString	sNumber = dispStr.Left(nDotPos);
		int		nNumber = 0;
		int		nLength = dispStr.GetLength();
#ifdef	UNICODE
		swscanf_s( sNumber,_T("%d"),&nNumber );
#else
		sscanf_s( sNumber,_T("%d"),&nNumber );
#endif	//UNICODE
		short	lo = exchang ? (nNumber & 0x000000ff)<<8 : nNumber & 0x00ff;
		short	hi = exchang ? (nNumber & 0x0000ff00)>>8 : nNumber & 0xff00;
		short	word = hi | lo;
		wordArray[ nCount ++ ] = word;
		dispStr = dispStr.Right( nLength - nDotPos - 1 );
	}
	nWords = nCount;
}

//¡ã?192.168.001.012??¨º?¡Áa?¡¥?a192.168.1.2??¨º?
CString	IPconvertN2ANew(DWORD nIP,bool b0_in_hh)
{
	DWORD	b0 = 0x000000ff & (nIP >> 24);
	DWORD	b1 = 0x000000ff & (nIP >> 16);
	DWORD	b2 = 0x000000ff & (nIP >>  8);
	DWORD	b3 = 0x000000ff & (nIP);
	CString	szIP;
	if( b0_in_hh )
		//szIP.Format( _T("%03d.%03d.%03d.%03d"),b0,b1,b2,b3 );
		szIP.Format( _T("%d.%d.%d.%d"),b0,b1,b2,b3 );
	else 
		//szIP.Format( _T("%03d.%03d.%03d.%03d"),b3,b2,b1,b2 );	//where is b0
		szIP.Format( _T("%d.%d.%d.%d"),b3,b2,b1,b0 );
	return szIP;
}

CString	IPconvertN2A(DWORD nIP,bool b0_in_hh)
{
	DWORD	b0 = 0x000000ff & (nIP >> 24);
	DWORD	b1 = 0x000000ff & (nIP >> 16);
	DWORD	b2 = 0x000000ff & (nIP >>  8);
	DWORD	b3 = 0x000000ff & (nIP);
	CString	szIP;
	if( b0_in_hh )
		szIP.Format( _T("%03d.%03d.%03d.%03d"),b0,b1,b2,b3 );
	else 
		szIP.Format( _T("%03d.%03d.%03d.%03d"),b3,b2,b1,b0 );	//where is b0
	return szIP;
}

DWORD	IPconvertA2N(CString szIP,bool b0_in_hh)
{
	DWORD	b0 = 0;
	DWORD	b1 = 0;
	DWORD	b2 = 0;
	DWORD	b3 = 0;
	if( szIP )
	{
#ifdef	UNICODE
		if( b0_in_hh ) swscanf_s( szIP,_T("%d.%d.%d.%d"),&b0,&b1,&b2,&b3 );
		else swscanf_s( szIP,_T("%d.%d.%d.%d"),&b3,&b2,&b1,&b0 );
#else
		if( b0_in_hh ) sscanf_s( szIP,_T("%d.%d.%d.%d"),&b0,&b1,&b2,&b3 );
		else sscanf_s( szIP,_T("%d.%d.%d.%d"),&b3,&b2,&b1,&b0 );
#endif	//UNICODE
	}
	b0 = 0xff000000 & (b0<<24);
	b1 = 0x00ff0000 & (b1<<16);
	b2 = 0x0000ff00 & (b2<< 8);
	b3 = 0x000000ff & b3;
	DWORD	nIP = b0 | b1 | b2 | b3;
	return nIP;
}

int WideStr_MultiByteStr( CString szString,LPSTR sString,int nMax )
{
	if (!szString.IsEmpty() && nMax >= szString.GetLength()) {
		memset( sString,0x00,nMax );
#ifdef	UNICODE
		size_t	len = wcslen(szString);
		return WideCharToMultiByte( 936,WC_COMPOSITECHECK,szString,len,sString,nMax,NULL,NULL);
#else
		strcpy_s( sString,nMax,szString.GetBuffer() );	//strcpy_s( sString,nMax-1,szString.GetBuffer() );
		return strlen(sString);
#endif
	}
	return 0;
}

CString MultiByteStr_WideStr( LPSTR sString )
{
	if( sString == NULL ) return _T("");
	int			nString = (int)strlen( sString );
	wchar_t*	wsBuffer = new wchar_t[nString+1];

	wmemset( wsBuffer,0, nString+1 );
	MultiByteToWideChar( 936,MB_COMPOSITE,sString,nString,wsBuffer,nString );
	CString		szWideStr = CString( wsBuffer );
	delete [] wsBuffer;
	return szWideStr;
}

static long static_nPing;
long Ping(CListBox& list, char* sIpAddress,int nDataSize,int nTimeOut,bool& bFirstPing) {
	if( nDataSize <= 0 ) return -1;
	if( nTimeOut <= 0 ) return -1;
	int	b1 = 0,b2 = 0,b3 = 0,b4 = 0;
	char						RequestData[10240];
	ICMP_ECHO_REPLY			ReplyBuff[10240];

	memset( RequestData,'E',nDataSize );
	memset( ReplyBuff,0x00,16*sizeof(ICMP_ECHO_REPLY) );
	sscanf_s( sIpAddress,"%d.%d.%d.%d",&b1,&b2,&b3,&b4 );
	IPAddr	DestinationAddress = ((b4 << 24) & 0xff000000) |
								 ((b3 << 16) & 0x00ff0000) |
								 ((b2 << 8 ) & 0x0000ff00) |
								 ((b1 << 0 ) & 0x000000ff);
	
	HANDLE	hIcmpFile = IcmpCreateFile();
	if( hIcmpFile ) {
		int		nItem = 0;
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
		if ((nItem = list.GetCount()) >= 100) {
			list.DeleteString(0);
			nItem --;
		}
		if( bFirstPing ) {
			static_nPing = 0;
			bFirstPing = false;
			list.AddString(_T("========"));
			return 0;
		} else if( nReply > 0 ) {
			b4 = (ReplyBuff[0].Address >> 24) & 0x000000ff;
			b3 = (ReplyBuff[0].Address >> 16) & 0x000000ff;
			b2 = (ReplyBuff[0].Address >> 8 ) & 0x000000ff;
			b1 = (ReplyBuff[0].Address >> 0 ) & 0x000000ff;
			szEcho.Format(_T("%d.%d.%d.%d, seq=% 5d, "), b1,b2,b3,b4, static_nPing ++);
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
			
			list.AddString(szEcho + szError);
			if (nItem > 0) {
				list.SetCurSel(nItem - 1);
				list.SetCurSel(-1);
			}
			return (ReplyBuff[0].Status <= 0) ? ReplyBuff[0].RoundTripTime : (-ReplyBuff[0].Status);
		} else {
			szEcho.Format(_T("%d.%d.%d.%d, seq=% 5d,    Timeout !!!"), b1,b2,b3,b4, static_nPing ++);	//ReplyBuff[0].DataSize
			list.AddString(szEcho);
			if (nItem > 0) {
				list.SetCurSel(nItem - 1);
				list.SetCurSel(-1);
			}
			return -1;
		}
	} else return (-1 * GetLastError());
}
