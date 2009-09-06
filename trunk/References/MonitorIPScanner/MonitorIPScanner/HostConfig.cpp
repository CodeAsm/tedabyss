// HostConfig.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "HostConfig.h"

#include <atlbase.h>
#include <ntsecapi.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define IDS_REG_KEY_MSINFO_PATH1	_T( "Software\\Microsoft\\Shared Tools\\MSInfo" )
#define IDS_REG_KEY_MSINFO_PATH2	_T( "Software\\Microsoft\\Shared Tools Location" )
#define IDS_REG_VAL_MSINFO_PATH1	_T( "Path" )
#define IDS_REG_VAL_MSINFO_PATH2	_T( "MSInfo" )
#define IDS_MSINFO_EXE_NAME			_T( "MSInfo32.exe" )

#define SystemProcessesAndThreadsInformation	5
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
typedef LONG	KPRIORITY;

#pragma comment(lib, "ntdll.lib")

ULONG VirtualPage, PeakVirtualSize, PageFile, PeakPageFile, PageFaultCount, PeakWorkSetSize, WorkSetSize;

typedef struct _CLIENT_ID {
    DWORD	    UniqueProcess;
    DWORD	    UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

typedef struct _VM_COUNTERS {
    SIZE_T	    PeakVirtualSize;
    SIZE_T	    VirtualSize;
    ULONG	    PageFaultCount;
    SIZE_T	    PeakWorkingSetSize;
    SIZE_T	    WorkingSetSize;
    SIZE_T	    QuotaPeakPagedPoolUsage;
    SIZE_T	    QuotaPagedPoolUsage;
    SIZE_T	    QuotaPeakNonPagedPoolUsage;
    SIZE_T	    QuotaNonPagedPoolUsage;
    SIZE_T	    PagefileUsage;
    SIZE_T	    PeakPagefileUsage;
} VM_COUNTERS;


typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG			WaitTime;
    PVOID			StartAddress;
    CLIENT_ID	    ClientId;
    KPRIORITY	    Priority;
    KPRIORITY	    BasePriority;
    ULONG			ContextSwitchCount;
    LONG			State;
    LONG			WaitReason;
} SYSTEM_THREAD_INFORMATION, * PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION_NT4 {
    ULONG			NextEntryDelta;
    ULONG			ThreadCount;
    ULONG			Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ProcessName;
    KPRIORITY	    BasePriority;
    ULONG			ProcessId;
    ULONG			InheritedFromProcessId;
    ULONG			HandleCount;
    ULONG			Reserved2[2];
    VM_COUNTERS	    VmCounters;
    SYSTEM_THREAD_INFORMATION  Threads[1];
} SYSTEM_PROCESS_INFORMATION_NT4, * PSYSTEM_PROCESS_INFORMATION_NT4;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG			NextEntryDelta;
    ULONG			ThreadCount;
    ULONG			Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ProcessName;
    KPRIORITY	    BasePriority;
    ULONG			ProcessId;
    ULONG			InheritedFromProcessId;
    ULONG			HandleCount;
    ULONG			Reserved2[2];
    VM_COUNTERS	    VmCounters;
    IO_COUNTERS	    IoCounters;
    SYSTEM_THREAD_INFORMATION  Threads[1];
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_VALID_ATTRIBUTES    0x000003F2L

extern "C"
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation(
						 IN UINT SystemInformationClass,
						 IN OUT PVOID SystemInformation,
						 IN ULONG SystemInformationLength,
						 OUT PULONG ReturnLength OPTIONAL
						 );

// CHostConfig dialog
IMPLEMENT_DYNAMIC(CHostConfig, CDialog)

CHostConfig::CHostConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CHostConfig::IDD, pParent)
	, m_HostName(_T(""))
	, m_Pagefile(0)
	, m_PeakPageFile(0)
	, m_VirtualPage(0)
	, m_PeakVirtualSize(0)
	, m_PageFaultCount(0)
	, m_WorkSetSize(0)
	, m_PeakWorkSetSize(0)
{

}

CHostConfig::~CHostConfig()
{
}

void CHostConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HOSTNAME, m_HostName);
	DDX_Text(pDX, IDC_HOSTNAME2, m_Pagefile);
	DDX_Text(pDX, IDC_HOSTNAME3, m_PeakPageFile);
	DDX_Text(pDX, IDC_HOSTNAME4, m_VirtualPage);
	DDX_Text(pDX, IDC_HOSTNAME5, m_PeakVirtualSize);
	DDX_Text(pDX, IDC_HOSTNAME6, m_PageFaultCount);
	DDX_Text(pDX, IDC_HOSTNAME7, m_WorkSetSize);
	DDX_Text(pDX, IDC_HOSTNAME8, m_PeakWorkSetSize);
}


BEGIN_MESSAGE_MAP(CHostConfig, CDialog)
	ON_BN_CLICKED(ID_SYSTEMINFO, &CHostConfig::OnBnClickedSysteminfo)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CHostConfig message handlers


void CHostConfig::OnBnClickedSysteminfo()
{
	CString m_SysInfo = _T("");

	if(GetSystemInfo(m_SysInfo))
	{
		UINT Res = (UINT) ::ShellExecute(GetSafeHwnd(), _T("open"), m_SysInfo, NULL, NULL, SW_SHOWNORMAL);
		if( Res <= HINSTANCE_ERROR )
		{
			AfxMessageBox( _T( "Error Executing \"MsInfo32.exe\" !" ), 
				MB_OK | MB_ICONEXCLAMATION );
		}

	}
	else
	{
		AfxMessageBox( _T( "\"MsInfo32.exe\" cannot be found !" ), 
				MB_OK | MB_ICONEXCLAMATION );
	}

	// TODO: Add your control notification handler code here
}

BOOL CHostConfig::GetSystemInfo(CString& SysInfo)
{
	SysInfo.Empty();
	LPTSTR pszPath = SysInfo.GetBuffer(MAX_PATH);
	CRegKey Reg;
	DWORD dwSize = MAX_PATH;
	// Try to find "MSInfo32.exe" at the first location
	LONG nRet = Reg.Open(HKEY_LOCAL_MACHINE, IDS_REG_KEY_MSINFO_PATH1, KEY_READ);

	if(nRet == ERROR_SUCCESS)
	{
		#if ( _MFC_VER >= 0x0700 )
			nRet = Reg.QueryStringValue( IDS_REG_VAL_MSINFO_PATH1, pszPath, &dwSize );
		#else
			nRet = reg.QueryValue( pszPath, IDS_REG_VAL_MSINFO_PATH1, &dwSize );
		#endif
	}
	if ( nRet != ERROR_SUCCESS )
	{
		// If first attemp fails then try to find "MSInfo32.exe" 
		// at the second location:
		
		nRet = Reg.Open( HKEY_LOCAL_MACHINE, IDS_REG_KEY_MSINFO_PATH2, KEY_READ );

        if ( nRet == ERROR_SUCCESS )
        {
			#if ( _MFC_VER >= 0x0700 )
				Reg.QueryStringValue( IDS_REG_VAL_MSINFO_PATH2, pszPath, &dwSize );
			#else
				reg.QueryValue( pszPath, IDS_REG_VAL_MSINFO_PATH2, &dwSize );
			#endif
			
			// The second location does not contain the full
			// path (exe name is missing), complete it:

			if ( nRet == ERROR_SUCCESS )
				VERIFY( ::PathAppend( pszPath, IDS_MSINFO_EXE_NAME ) );

			Reg.Close(); 
		}
	}
	
	SysInfo.ReleaseBuffer();
	SysInfo.FreeExtra();
	
    // Check for valid file.
	return ::PathFileExists( SysInfo );
}
VOID CHostConfig::UpdateHostName()
{
	// TODO: Add extra initialization here
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	char Buf[100];
	 
	wVersionRequested = MAKEWORD( 2, 2 );
	 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return;
	}

	memset(Buf, 0x00, sizeof(Buf));
	gethostname(Buf, sizeof(Buf));
	UpdateData(TRUE);
	m_HostName = Buf;
	UpdateData(FALSE);
}




void CHostConfig::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	UpdateHostName();
	GetSystemMemoryUsage();
}


INT CHostConfig::GetSystemMemoryUsage()
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	ULONG cbBuffer = 0x8000;
	LPVOID pBuffer = NULL;
	NTSTATUS Status;

	do
	{
		pBuffer = malloc(cbBuffer);
		if(pBuffer == NULL)
		{
			AfxMessageBox(_T("Not enough memory"), 0, 0);
			return FALSE;
		}

		Status = ZwQuerySystemInformation(
			SystemProcessesAndThreadsInformation,
			pBuffer, cbBuffer, NULL);
		if(Status == STATUS_INFO_LENGTH_MISMATCH)
		{
			free(pBuffer);
			cbBuffer *= 2;
		}
		else if(!NT_SUCCESS(Status))
		{
			AfxMessageBox(_T("ZwQuerySystemInformation failed with status 0x%08X"), 0, 0);
			free(pBuffer);
			return 1;
		}
	}while (Status == STATUS_INFO_LENGTH_MISMATCH);
	
	PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION) pBuffer;
	
	VirtualPage	= PeakVirtualSize = PageFile = PeakPageFile	= 
	PageFaultCount = PeakWorkSetSize = WorkSetSize = 0;

	for(;;)
	{
		VirtualPage		+= (ULONG) pInfo->VmCounters.VirtualSize;
		PeakVirtualSize	+= (ULONG) pInfo->VmCounters.PeakVirtualSize;
		PageFile		+= (ULONG) pInfo->VmCounters.PagefileUsage;
		PeakPageFile	+= (ULONG) pInfo->VmCounters.PeakPagefileUsage;
		PageFaultCount	+= (ULONG) pInfo->VmCounters.PageFaultCount;
		PeakWorkSetSize	+= (ULONG) pInfo->VmCounters.PeakWorkingSetSize;
		WorkSetSize		+= (ULONG) pInfo->VmCounters.WorkingSetSize;

		if (pInfo->NextEntryDelta == 0)
			break;

		// find the address of the next process structure
		pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo)
						+ pInfo->NextEntryDelta);			
	}

	UpdateData(TRUE);
	m_Pagefile			=	PageFile;
	m_PeakPageFile		=	PeakPageFile;
	m_VirtualPage		=	VirtualPage;
	m_PeakVirtualSize	=	PeakVirtualSize;
	m_PageFaultCount	=	PageFaultCount;
	m_WorkSetSize		=	WorkSetSize;
	m_PeakWorkSetSize	=	PeakWorkSetSize;
	UpdateData(FALSE);
}	

