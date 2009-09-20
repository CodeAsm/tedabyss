// HookGame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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

	while( true )
	{
		
	}

	UnhookOneProcess2("asktao.mod");
	FreeLibrary(hLib);

	return 0;
}

