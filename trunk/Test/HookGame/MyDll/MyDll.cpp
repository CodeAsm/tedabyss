// MyDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <winsock.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <shellapi.h>
#include "mydll.h"
#include <ras.h>
#include <iostream>

using namespace std;

#define DEFAULT_PORT 9012
#define TEST_SERVER "117.34.76.15"

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

void WriteLog(char *fmt,...)
{
	va_list args;
	char modname[200];

	char temp[5000];
	HANDLE hFile;

	GetModuleFileNameA(NULL, modname, sizeof(modname));

	if((hFile =CreateFileA("c:\\hookapi.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) <0)
	{
		return;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);

	wsprintfA(temp, "mydll.dll:%s:", modname);
	DWORD dw;
	WriteFile(hFile, temp, strlen(temp), &dw, NULL);

	va_start(args,fmt);
	vsprintf(temp, fmt, args);
	va_end(args);

	WriteFile(hFile, temp, strlen(temp), &dw, NULL);

	wsprintfA(temp, "\r\n");
	WriteFile(hFile, temp, strlen(temp), &dw, NULL);

	CloseHandle(hFile);
}

void GetFileName(char *fname)
{
	char temp[200];

	GetModuleFileNameA(NULL, temp, sizeof(temp));
	int i =strlen(temp);
	while(i >0 && temp[i-1] !='\\' && temp[i-1] !=':') i--;

	strcpy(fname, &temp[i]);
}

int WriteBinData(char *function, char *buf, int len)
{
	char mod_name[100];
	char fname[128];
	if(len <=0) return 0;

	GetFileName(mod_name);
	if( strstr( mod_name, "asktao" ) != NULL )
	{
		wsprintfA(fname, "c:\\%s.log", mod_name);
		HANDLE hFile;

		if((hFile =CreateFileA(fname, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) <0)
		{
			WriteLog("open file %s failed", fname);
			return -1;
		}
		SetFilePointer(hFile, 0, NULL, FILE_END);

		GamePacket gamePacket( (BYTE*)buf, len );
		ActionPacket* pAction = gamePacket.GetAction();
		MovePacket* pMove = (MovePacket*)pAction;
		LocationPoint  Locations[1];
		GamePacket DummyPacket;

		switch( gamePacket.GetActionType() )
		{
		case EActionType_Move:
			char temp[2048];
			wsprintfA(temp, "\r\n(%s,len=%d) ", function, len);
			DWORD dw;
			WriteFile(hFile, temp, strlen(temp), &dw, NULL);

			for(int i =0; i<len; i++)
			{
				wsprintfA(temp, "%02x", buf[i]&0x00FF);
				WriteFile(hFile, temp, strlen(temp), &dw, NULL);
			}

			SYSTEMTIME  SysTime;
			GetSystemTime( &SysTime );
			wsprintfA(temp, "\r\n[Move] SeverInfo: %x, Area:%x, StepNum:%x, FaceDir:%x, TimeStamp:%x, SystemTime:%x:%x:%x, CalcTimeStamp:%x", 
				pMove->GetServerInfo(), pMove->GetArea(), pMove->GetStepNum(), pMove->GetFaceDir(), pMove->GetTimeStamp(), 
				SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds, gamePacket.CalcTimeStamp() );
			WriteFile(hFile, temp, strlen(temp), &dw, NULL);
			for ( int i = 0; i < (int)pMove->GetStepNum(); ++i )
			{
				wsprintfA(temp, "\r\nLocation[%d]: %d, %d", i, pMove->GetLocation(i).X, pMove->GetLocation(i).Y );
				WriteFile(hFile, temp, strlen(temp), &dw, NULL);
			}

			Locations[0].Y = pMove->GetLocation(pMove->GetStepNum()-1).Y-1;
			DummyPacket.SetActionPacket( new MovePacket( pMove->GetServerInfo(), pMove->GetArea(), 1, Locations, 0, DummyPacket.CalcTimeStamp() ) );

			//send(s, (char *)DummyPacket.GetData(), DummyPacket.GetSize(), flags);

			break;
		default:
			break;
		}

		CloseHandle(hFile);
	}

	return 0;
}



int WINAPI mysocket(int af, int type, int protocol)
{
	//Sleep(500);// test for multithread
	printf("debug mysocket, af=%d, type=%d, protocol=%d", af, type, protocol);

	return socket(af, type, protocol);
}

int WINAPI mysend(SOCKET s,	char *buf, int len,	int flags)
{
	int ret;

	WriteBinData("send", buf, len);
	ret =send(s, (char *)buf, len, flags);

	int err;
	if(ret <=0) err =WSAGetLastError();
	// other process...
	if(ret <=0) WSASetLastError(err);

	return ret;
}

MYAPIINFO myapi_info[] =
{
#ifdef WINNT
	{"WS2_32.DLL", "socket", 3, "mysocket"},
	//{"WS2_32.DLL", "accept", 3, "myaccept"},
	//{"WS2_32.DLL", "connect", 3, "myconnect"},
	//{"WS2_32.DLL", "recv", 4, "myrecv"},
	{"WS2_32.DLL", "send", 4, "mysend"},
	//{"WS2_32.DLL", "sendto", 6, "mysendto"},
	//{"WS2_32.DLL", "recvfrom", 6, "myrecvfrom"},
	//{"WS2_32.DLL", "gethostbyname", 1, "mygethostbyname"},
#else
	{"WSOCK32.DLL", "socket", 3, "mysocket"},
	//{"WSOCK32.DLL", "accept", 3, "myaccept"},
	//{"WSOCK32.DLL", "connect", 3, "myconnect"},
	//{"WSOCK32.DLL", "recv", 4, "myrecv"},
	{"WSOCK32.DLL", "send", 4, "mysend"},
	//{"WSOCK32.DLL", "sendto", 6, "mysendto"},
	//{"WSOCK32.DLL", "recvfrom", 6, "myrecvfrom"},
	//{"WSOCK32.DLL", "gethostbyname", 1, "mygethostbyname"},
#endif
	{NULL}
};

MYAPIINFO *GetMyAPIInfo()
{
	return &myapi_info[0];
}
