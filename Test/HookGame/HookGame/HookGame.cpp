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


void SwapEndian16( BYTE* InBytes )
{
	BYTE tempByte;
	tempByte = InBytes[0];
	InBytes[0] = InBytes[1];
	InBytes[1] = tempByte;
}

void SwapEndian32( BYTE* InBytes )
{
	BYTE tempByte;
	tempByte = InBytes[0];
	InBytes[0] = InBytes[3];
	InBytes[3] = tempByte;
	tempByte = InBytes[1];
	InBytes[1] = InBytes[2];
	InBytes[2] = tempByte;
}

void SwapEndian16( WORD* InBytes )
{
	SwapEndian16( (BYTE*)InBytes );
}

void SwapEndian32( DWORD* InBytes )
{
	SwapEndian32( (BYTE*)InBytes );
}

enum EActionType
{
	EActionType_Idel	= 0x10B2,
	EActionType_Move	= 0xF0C2,
	EActionType_Unknown = 0xFFFF
};

enum EAreaType
{
	EAreaType_LanXianZhen		= 0x03e8,
	EAreaType_LanXianZhenWai	= 0x07d0,
	EAreaType_WoLongPo			= 0x0bb8,
	EAreaType_GuanDaoNan		= 0x0fa0,
	EAreaType_TianYongChen		= 0x1388,
	EAreaType_GuanDaoBei		= 0x5dc0,
	EAreaType_Unknown			= 0xFFFF
};

enum EFaceDir
{
	EFaceDir_Left = 0,
	EFaceDir_UpLeft,
	EFaceDir_Up,
	EFaceDir_UpRight,
	EFaceDir_Right,
	EFaceDir_DownRight,
	EFaceDir_Down,
	EFaceDir_DownLeft,
	EFaceDir_Unknown
};

class ActionPacket
{
public:
	ActionPacket( BYTE* InData, INT Length, BOOL bNeedSwap = TRUE )
	{
		PacketSize = Length >= 2 ? Length : 2;
		Data = new BYTE[PacketSize];
		memcpy(Data,InData,PacketSize);
	}
	ActionPacket() {};

	virtual EActionType GetType() { return EActionType_Unknown; }

	BYTE* GetData()
	{
		return &Data[0];
	}

	WORD GetSize()
	{
		return PacketSize;
	}

	virtual ~ActionPacket()
	{
		if(Data)
		{
			delete [] Data;
		}
	}

protected:
	BOOL						bSwapEndian;
	BYTE*						Data;
	WORD						PacketSize;
};

struct LocationPoint
{
	LocationPoint(WORD InX = 0, WORD InY = 0) { X = InX; Y = InY; }
	WORD	X;
	WORD	Y;
};

class MovePacket : public ActionPacket
{
public:
	MovePacket( BYTE* InData, INT Length, BOOL bNeedSwap = TRUE )
		: ActionPacket( InData, Length, bNeedSwap )
	{
		INT index = 2;
		memcpy(&ServerInfo,&Data[index],4);
		if( bSwapEndian ) SwapEndian32( &ServerInfo );
		index += 4;
		memcpy(&Area,&Data[index],4);
		if( bSwapEndian ) SwapEndian32( &Area );
		index += 4;
		memcpy(&StepNum,&Data[index],2);
		if( bSwapEndian ) SwapEndian16( &StepNum );
		index += 2;

		for(INT i=0;i<StepNum;i++)
		{
			memcpy(&Locations[i].X,&Data[index],2);
			if( bSwapEndian ) SwapEndian16( &Locations[i].X );
			index += 2;
			memcpy(&Locations[i].Y,&Data[index],2);
			if( bSwapEndian ) SwapEndian16( &Locations[i].Y );
			index += 2;
		}

		memcpy(&FaceDir,&Data[index],2);
		if( bSwapEndian ) SwapEndian16( &FaceDir );
		index += 2;
		memcpy(&TimeStamp,&Data[index],4);
		if( bSwapEndian ) SwapEndian32( &TimeStamp );
		index += 4;
	}

	MovePacket( DWORD InServerInfo, DWORD InArea, WORD InStepNum, LocationPoint* InLocations, WORD InFaceDir, DWORD InTimeStamp, BOOL bNeedSwap = TRUE )
		:ActionPacket()
	{
		bSwapEndian = bNeedSwap;
		ServerInfo = InServerInfo;
		Area = InArea;
		StepNum = InStepNum;
		for( int i = 0; i < StepNum; ++i )  Locations[i] = InLocations[i];
		FaceDir = InFaceDir;
		TimeStamp = InTimeStamp;
		GenerateData();
	}

	void GenerateData()
	{
		PacketSize = 18+4*StepNum;
		Data = new BYTE[PacketSize];

		INT index = 0;
		WORD ActionType = (WORD)EActionType_Move;
		memcpy(&Data[index],&ActionType,2);
		if( bSwapEndian ) SwapEndian16( &Data[index] );
		memcpy(&Data[index],&ServerInfo,4);
		if( bSwapEndian ) SwapEndian32( &Data[index] );
		index += 4;
		memcpy(&Data[index],&Area,4);
		if( bSwapEndian ) SwapEndian32( &Data[index] );
		index += 4;
		memcpy(&Data[index],&StepNum,2);
		if( bSwapEndian ) SwapEndian16( &Data[index] );
		index += 2;

		for(INT i=0;i<StepNum;i++)
		{
			memcpy(&Data[index],&Locations[i].X,2);
			if( bSwapEndian ) SwapEndian16( &Data[index] );
			index += 2;
			memcpy(&Data[index],&Locations[i].Y,2);
			if( bSwapEndian ) SwapEndian16( &Data[index] );
			index += 2;
		}

		memcpy(&Data[index],&FaceDir,2);
		if( bSwapEndian ) SwapEndian16( &Data[index] );
		index += 2;
		memcpy(&Data[index],&TimeStamp,4);
		if( bSwapEndian ) SwapEndian32( &Data[index] );
		index += 4;
	}

	virtual EActionType GetType() { return EActionType_Move; }

	DWORD GetServerInfo() { return ServerInfo; }
	DWORD GetArea() { return Area; }
	DWORD GetStepNum() { return StepNum; }
	LocationPoint GetLocation(int LocationIndex) { if( LocationIndex<StepNum ) return Locations[LocationIndex]; return LocationPoint(); }
	EFaceDir GetFaceDir() { return (EFaceDir)FaceDir; }
	DWORD GetTimeStamp() { return TimeStamp; }

private:
	DWORD				ServerInfo;
	DWORD				Area;
	WORD				StepNum;
	LocationPoint		Locations[100];
	WORD				FaceDir;
	DWORD				TimeStamp;
};

class GamePacket
{
public:
	GamePacket(BYTE* InData, INT Length, BOOL bNeedSwap = TRUE)
	{
		bSwapEndian = bNeedSwap;
		ActionPack = NULL;
		PacketSize = Length >= 4 ? Length : 4;
		Data = new BYTE[PacketSize];
		if( PacketSize == Length )
		{
			memcpy(Data,InData,PacketSize);
			ParseData();
		}
	}

	GamePacket(BOOL bNeedSwap = TRUE)
	{
		bSwapEndian = bNeedSwap;
		PacketSize = 0;
		Data = NULL;
		ActionPack = NULL;
	}

	virtual ~GamePacket()
	{
		if(Data)
		{
			delete [] Data;
		}
		if(ActionPack)
		{
			delete ActionPack;
			ActionPack = NULL;
		}
	}

	BYTE* GetData()
	{
		return &Data[0];
	}

	WORD GetSize()
	{
		return PacketSize;
	}

	void ParseData()
	{
		INT index = 0;
		PackedHeader = 0xFFFFFFFF;
		memcpy(&PackedHeader,&Data[index],4);
		if( bSwapEndian ) SwapEndian32( &PackedHeader );
		index += 4;
		if( IsCorrectPacket() )
		{
			memcpy(&TimeStamp,&Data[index],4);
			if( bSwapEndian ) SwapEndian32( &TimeStamp );
			index += 4;
			if( !TimeStampRecorded )
			{
				SYSTEMTIME SysTime;
				GetSystemTime( &SysTime );
				TimeStampOffset = ((SysTime.wHour*60+SysTime.wMinute)*60+SysTime.wSecond)*1000 + SysTime.wMilliseconds;
				TimeStampOffset = TimeStamp - TimeStampOffset;
				TimeStampRecorded = true;
			}
			memcpy(&ActionLength,&Data[index],2);
			if( bSwapEndian ) SwapEndian16( &ActionLength );
			index += 2;

			ParseExtraActionData( &Data[index], ActionLength );
		}
	}

	void ParseExtraActionData( BYTE* InData, INT Length )
	{
		if( Length >= 2 )
		{
			WORD ActionType = EActionType_Unknown;
			memcpy(&ActionType,&InData[0],2);
			if( bSwapEndian ) SwapEndian16( &ActionType );
			
			switch( ActionType )
			{
			case EActionType_Idel:
				break;
			case EActionType_Move:
				ActionPack = new MovePacket( InData, Length, bSwapEndian );
				break;
			default:
				break;
			}
		}
	}
	
	ActionPacket* GetAction() { return ActionPack; }
	
	// Used for manually setup game packet.
	void GenerateData()
	{
		TimeStamp = CalcTimeStamp();
		ActionLength = 0;
		if( ActionPack )	ActionLength = ActionPack->GetSize();
		PacketSize = 10 + ActionLength;
		Data = new BYTE[PacketSize];

		INT index = 0;
		memcpy(&Data[index],&FixedHeader,4);
		if( bSwapEndian ) SwapEndian32(&Data[index]);
		index += 4;
		memcpy(&Data[index],&TimeStamp,4);
		if( bSwapEndian ) SwapEndian32(&Data[index]);
		index += 4;
		memcpy(&Data[index],&ActionLength,2);
		if( bSwapEndian ) SwapEndian32(&Data[index]);
		index += 2;

		if( ActionPack )
		{
			memcpy(&Data[index],ActionPack->GetData(),ActionLength);
		}
	}

	void SetActionPacket( ActionPacket* InAction )
	{
		ActionPack = InAction;
		GenerateData();
	}

	DWORD CalcTimeStamp()
	{
		SYSTEMTIME SysTime;
		GetSystemTime( &SysTime );
		return TimeStampOffset + ((SysTime.wHour*60+SysTime.wMinute)*60+SysTime.wSecond)*1000 + SysTime.wMilliseconds;
	}
	BOOL IsCorrectPacket() { return PackedHeader == FixedHeader && PacketSize >= 10; }
	EActionType GetActionType() { return ActionPack != NULL ? ActionPack->GetType() : EActionType_Unknown; }

private:

private:
	BYTE*						Data;
	WORD						PacketSize;

	BOOL						bSwapEndian;
	static const DWORD			FixedHeader = 0x4d5a0000;
	DWORD						PackedHeader;
	DWORD						TimeStamp;
	WORD						ActionLength;
	class ActionPacket*			ActionPack;

	static bool					TimeStampRecorded;
	static DWORD				TimeStampOffset;
};

bool GamePacket::TimeStampRecorded = false;
DWORD GamePacket::TimeStampOffset = 0;

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
		FreeLibrary(hLib);
		return 0;
	}

	WSADATA wsa;
	//Initialise Winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("WSAStartup() failed.\n");
		UnhookOneProcess2("asktao.mod");
		FreeLibrary(hLib);
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
		UnhookOneProcess2("asktao.mod");
		FreeLibrary(hLib);
		return 1;
	}

	if ( listen( ListenSocket, 5 ) == SOCKET_ERROR) 
	{
		printf("Error listening on socket.\n");
		closesocket(ListenSocket);
		WSACleanup();
		UnhookOneProcess2("asktao.mod");
		FreeLibrary(hLib);
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
		UnhookOneProcess2("asktao.mod");
		FreeLibrary(hLib);
		return 1;
	}
	else 
	{
		printf("Client connected.\n");
	}

	char Buffer[4096];
	// receive socket
	WSAPROTOCOL_INFO DupSocketInfo;
	INT	recvSize = recv(AcceptSocket,(char*)Buffer,4096,0);
	if(recvSize == 0)
	{
		printf("reveived data size 0!!!");
		WSACleanup();
		UnhookOneProcess2("asktao.mod");
		FreeLibrary(hLib);
		return 1;
	}

	memcpy(&DupSocketInfo, &Buffer[0], sizeof(WSAPROTOCOL_INFO));
	MovePacket original_packet((BYTE*)&Buffer[sizeof(WSAPROTOCOL_INFO)],recvSize);

	SYSTEMTIME  catched_sys_time;
	GetSystemTime( &catched_sys_time );


	//Create the socketInformation out of the received Bytes.
	SOCKET DuplicatedSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP,&DupSocketInfo, 0, 0 );
	

	while( true )
	{
		Sleep(1020);

		WORD step_num = 10;
		LocationPoint location_ = original_packet.GetLocation(original_packet.GetStepNum()-1);
		LocationPoint points[10];
		points[0].X = location_.X;
		points[0].Y = location_.Y+1; 
		for (INT i=1;i<step_num;i++)
		{
			points[i].X = points[0].X;
			points[i].Y = points[0].Y+i;
		}

		SYSTEMTIME SysTime;
		GetSystemTime( &SysTime );
		DWORD time_stamp__ = ((SysTime.wHour*60+SysTime.wMinute)*60+SysTime.wSecond)*1000 + SysTime.wMilliseconds - 
			((catched_sys_time.wHour*60+catched_sys_time.wMinute)*60+catched_sys_time.wSecond)*1000 + catched_sys_time.wMilliseconds 
			+ original_packet.GetTimeStamp();
		MovePacket test_packet(original_packet.GetServerInfo(),original_packet.GetArea(),step_num,points,original_packet.GetFaceDir(),time_stamp__);
		if (send(DuplicatedSocket, (char*) test_packet.GetData(), sizeof(test_packet), 0) == SOCKET_ERROR) 
		{
			printf("\r\n[SendError] %ld", WSAGetLastError());
			//WSACleanup();
			//return 1;
		}		
		
		break;
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

