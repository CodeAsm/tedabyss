#include "stdafx.h"

#include <winsock2.h>
#include <io.h>
#include <stdio.h>

#include "DuplicateSocket.h"

DuplicateSocket::DuplicateSocket()
{
	bTransferred = false;
	dwProcessId = 0;

	ZeroMemory(&WSAProtocolInfo, sizeof(WSAPROTOCOL_INFO));

	char fname[128];
	wsprintfA(fname, "c:\\DuplicateSocket.log");

	if((hLogFile =CreateFileA(fname, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) <0)
	{
		//WriteLog("open file %s failed", fname);
		return;
	}
	SetFilePointer(hLogFile, 0, NULL, FILE_END);
}

DuplicateSocket::~DuplicateSocket()
{
	CloseHandle(hLogFile);
}

void DuplicateSocket::GetTargetProcess()
{
	HWND hwnd = FindWindowA( "ConsoleWindowClass", "e:\\My Documents\\My Projects\\TedAbyss\\trunk\\Test\\HookGame\\HookGame\\Bin\\HookGame.exe" );
	GetWindowThreadProcessId( hwnd, &dwProcessId );

	char temp[256];
	DWORD dw;
	wsprintfA(temp, "\r\n[Duplicate Socket] FindWindow Result: Handle: %x, PID: %x", hwnd, dwProcessId );
	WriteFile(hLogFile, temp, strlen(temp), &dw, NULL);
}

bool DuplicateSocket::TransportToTarget()
{
	if( !bTransferred )
	{
		char temp[256];
		DWORD dw;
		SOCKET transportSocket;
		transportSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		sockaddr_in clientService;
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
		clientService.sin_port = htons( 2001 );
		if(connect(transportSocket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR)
		{
			wsprintfA(temp, "\r\n[ConnectError] %ld", WSAGetLastError() );
			WriteFile(hLogFile, temp, strlen(temp), &dw, NULL);
			WSACleanup();
			return false;
		}
		INT sendSize = sizeof(WSAProtocolInfo);
		if (send(transportSocket, (char*) &WSAProtocolInfo, sendSize, 0) == SOCKET_ERROR) 
		{
			wsprintfA(temp, "\r\n[SendError] %ld", WSAGetLastError() );
			WriteFile(hLogFile, temp, strlen(temp), &dw, NULL);
			WSACleanup();
			return false;
		}
		wsprintfA(temp, "\r\n[SendSocket] Size: %d", sendSize );
		WriteFile(hLogFile, temp, strlen(temp), &dw, NULL);

		// shutdown the connection since no more data will be sent
		if (shutdown(transportSocket, SD_SEND) == SOCKET_ERROR) 
		{
			wsprintfA(temp, "\r\n[ShutdownError] %ld", WSAGetLastError() );
			WriteFile(hLogFile, temp, strlen(temp), &dw, NULL);
			WSACleanup();
			return false;
		}
		closesocket(transportSocket);
		bTransferred = true;

		return true;
	}

	return false;
}

bool DuplicateSocket::Duplicate(SOCKET original)
{
	GetTargetProcess();

	// Duplicate
	WSADuplicateSocket( original, dwProcessId, &WSAProtocolInfo );

	return true;
}