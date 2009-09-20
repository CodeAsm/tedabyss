// HookGame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

//IP Header Structure
typedef struct ip_hdr
{
	unsigned char  ip_header_len:4;  // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
	unsigned char  ip_version   :4;  // 4-bit IPv4 version
	unsigned char  ip_tos;           // IP type of service
	unsigned short ip_total_length;  // Total length
	unsigned short ip_id;            // Unique identifier 

	unsigned char  ip_frag_offset   :5;        // Fragment offset field

	unsigned char  ip_more_fragment :1;
	unsigned char  ip_dont_fragment :1;
	unsigned char  ip_reserved_zero :1;

	unsigned char  ip_frag_offset1;    //fragment offset

	unsigned char  ip_ttl;           // Time to live
	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
	unsigned short ip_checksum;      // IP checksum
	unsigned int   ip_srcaddr;       // Source address
	unsigned int   ip_destaddr;      // Source address
}   IPV4_HDR;

//UDP Header Structure
typedef struct udp_hdr
{
	unsigned short source_port;     // Source port no.
	unsigned short dest_port;       // Dest. port no.
	unsigned short udp_length;      // Udp packet length
	unsigned short udp_checksum;    // Udp checksum (optional)
}   UDP_HDR;

// TCP Header Structure
typedef struct tcp_header 
{ 
	unsigned short source_port;  // source port 
	unsigned short dest_port;    // destination port 
	unsigned int   sequence;     // sequence number - 32 bits 
	unsigned int   acknowledge;  // acknowledgement number - 32 bits 

	unsigned char  ns   :1;          //Nonce Sum Flag Added in RFC 3540.
	unsigned char  reserved_part1:3; //according to rfc
	unsigned char  data_offset:4;    /*The number of 32-bit words in the TCP header. 
									 This indicates where the data begins. 
									 The length of the TCP header is always a multiple 
									 of 32 bits.*/

	unsigned char  fin  :1;      //Finish Flag
	unsigned char  syn  :1;      //Synchronise Flag
	unsigned char  rst  :1;      //Reset Flag
	unsigned char  psh  :1;      //Push Flag 
	unsigned char  ack  :1;      //Acknowledgement Flag 
	unsigned char  urg  :1;      //Urgent Flag

	unsigned char  ecn  :1;      //ECN-Echo Flag
	unsigned char  cwr  :1;      //Congestion Window Reduced Flag

	////////////////////////////////

	unsigned short window;          // window 
	unsigned short checksum;        // checksum 
	unsigned short urgent_pointer;  // urgent pointer 
}   TCP_HDR;

//ICMP Header Structure
typedef struct icmp_hdr    
{
	BYTE type;          // ICMP Error type
	BYTE code;          // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
}   ICMP_HDR;

struct sockaddr_in source,dest;

//Its free!
IPV4_HDR *iphdr;
TCP_HDR  *tcpheader;
UDP_HDR  *udpheader;
ICMP_HDR *icmpheader;


typedef int (WINAPI *FuncHookOneProcess2)(HWND hwndNotify, char *exe_name);
typedef int (WINAPI *FuncUnhookOneProcess2)(char *exe_name);
typedef int (WINAPI *FuncHookAllProcess)();
typedef int (WINAPI *FuncUnhookAllProcess)();

int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE hLib;
	hLib =LoadLibrary( _T("HookAPINT.dll") );
	if(hLib ==NULL)
	{
		printf("Load HookAPINT.dll failed.");
		return false;
	}

	FuncHookOneProcess2 HookOneProcess2 =(FuncHookOneProcess2)GetProcAddress(hLib, "HookOneProcess2");
	FuncUnhookOneProcess2 UnhookOneProcess2 =(FuncUnhookOneProcess2)GetProcAddress(hLib, "UnhookOneProcess2");
	FuncHookAllProcess HookAllProcess =(FuncHookAllProcess)GetProcAddress(hLib, "HookAllProcess");
	FuncUnhookAllProcess UnhookAllProcess =(FuncUnhookAllProcess)GetProcAddress(hLib, "UnhookAllProcess");

	int retVal = 0;
	retVal = HookOneProcess2( NULL, "asktao.mod" );
	while( retVal < 0 )
	{
		return 0;
	}

	WSADATA wsa;
	//Initialise Winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("WSAStartup() failed.\n");
		return 1;
	}
	printf("Initialised!\n");

	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	clientService.sin_port = htons( 2001 );

	//create a socket to receive the duplicated socket
	if (bind( ListenSocket, (SOCKADDR*) &clientService, sizeof(clientService)) == SOCKET_ERROR) 
	{
		printf("bind() failed.\n");
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	if ( listen( ListenSocket, 5 ) == SOCKET_ERROR) 
	{
		printf("Error listening on socket.\n");
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	SOCKET AcceptSocket;
	printf("Waiting for client to connect...\n");

	AcceptSocket = accept( ListenSocket, NULL, NULL );
	if (AcceptSocket == INVALID_SOCKET) 
	{
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else 
	{
		printf("Client connected.\n");
	}

	char Buffer[4096];
	INT	count = recv(AcceptSocket,Buffer,4096,0);

	//Create the socketInformation out of the received Bytes.
	WSAPROTOCOL_INFO DupSocketInfo;
	memcpy( &DupSocketInfo, Buffer, count );
	SOCKET DuplicatedSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP,&DupSocketInfo, 0, 0 );


	int Steps;
	while( true )
	{
		//cout << "Input Steps:";
		//cin >> Steps;
		//if( Steps <= 0 ) break;
		//for( int i = 0; i < Steps; ++i )
		//{

		//}
	}

	WSACleanup();
	UnhookOneProcess2("asktao.mod");
	FreeLibrary(hLib);

	return 0;
}

