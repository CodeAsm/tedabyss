// NetworkConfig.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "NetworkConfig.h"
#include "ProtocolStat.h"
#include "pdh.h"

// CNetworkConfig dialog
#define MAXLEN_IFDESCR 256


typedef DWORD (_stdcall *TGetIfTable) (
  MIB_IFTABLE *pIfTable,  // buffer for interface table 
  ULONG *pdwSize,         // size of buffer
  BOOL bOrder             // sort the table by index?
);

typedef DWORD (_stdcall *TGetNumberOfInterfaces) (
  PDWORD pdwNumIf  // pointer to number of interfaces
);


int  TableFlag = 0;
TGetIfTable pGetIfTable;
TGetNumberOfInterfaces pGetNumberOfInterfaces;


IMPLEMENT_DYNAMIC(CNetworkConfig, CDialog)

CNetworkConfig::CNetworkConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkConfig::IDD, pParent)
	, m_RecvBytes(0)
	,m_RecvDiscard(0)
	,m_RecvError(0)
	,m_RecvUKProto(0)
	,m_RecvUPack(0)
	,m_RecvNUPack(0)
	,m_SendBytes(0)
	,m_SendDiscard(0)
	,m_SendError(0)
	,m_SendUKProto(0)
	,m_SendUPack(0)
	,m_SendNUPack(0)
{

}

CNetworkConfig::~CNetworkConfig()
{
}

void CNetworkConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_Adapter);
	DDX_Text(pDX, IDC_EDIT1, m_RecvBytes);
	DDX_Text(pDX, IDC_EDIT3, m_RecvDiscard);
	DDX_Text(pDX, IDC_EDIT5, m_RecvError);
	DDX_Text(pDX, IDC_EDIT7, m_RecvUKProto);
	DDX_Text(pDX, IDC_EDIT9, m_RecvUPack);
	DDX_Text(pDX, IDC_EDIT11, m_RecvNUPack);
	DDX_Text(pDX, IDC_EDIT2, m_SendBytes);
	DDX_Text(pDX, IDC_EDIT4, m_SendDiscard);
	DDX_Text(pDX, IDC_EDIT6, m_SendError);
	DDX_Text(pDX, IDC_EDIT8, m_SendUKProto);
	DDX_Text(pDX, IDC_EDIT10, m_SendUPack);
	DDX_Text(pDX, IDC_EDIT12, m_SendNUPack);
}

BEGIN_MESSAGE_MAP(CNetworkConfig, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CNetworkConfig::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CNetworkConfig::OnCbnSelchangeCombo1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CNetworkConfig::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &CNetworkConfig::OnBnClickedOk2)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


// CNetworkConfig message handlers

void CNetworkConfig::UpdateIPConfigScreen()
{
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO) * 10;

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen);
	
	if (dwRetVal != ERROR_SUCCESS) 
		return;

	for(int  i = 0; i <= m_Adapter.GetCount(); i++)
		m_Adapter.DeleteString(i);
	pAdapter = &pAdapterInfo[0];		// get pointer to the first structure

	do
	{
		UpdateData(TRUE);
		m_Adapter.AddString(pAdapter->Description);
		UpdateData(FALSE);
	} while ((pAdapter = pAdapter->Next) != NULL);
	m_Adapter.SetCurSel(0);
	GetAllEntryValue();
}

void CNetworkConfig::GetAllEntryValue()
{
/*	MIB_IFROW pIfRow;
	DWORD m_dwResult;
	
	m_dwResult = GetIfEntry(&pIfRow);*/


	DWORD m_dwAdapters;
	MIB_IFTABLE *m_pTable;

	m_pTable=NULL;
	m_dwAdapters=0;

	pGetIfTable=(TGetIfTable)GetProcAddress(LoadLibrary("Iphlpapi.dll"),"GetIfTable");
	pGetNumberOfInterfaces=(TGetNumberOfInterfaces)GetProcAddress(LoadLibrary("Iphlpapi.dll"),"GetNumberOfInterfaces");

	m_pTable=NULL;

	if (!pGetIfTable || !pGetNumberOfInterfaces)
	{
		AfxMessageBox("Error loading Iphlpapi.dll");
		return;
	}
	ULONG uRetCode = pGetIfTable(m_pTable,&m_dwAdapters,TRUE);

	m_pTable=new MIB_IFTABLE[m_dwAdapters];
	pGetIfTable(m_pTable,&m_dwAdapters,TRUE);

	char szDescr1[MAXLEN_IFDESCR];
	m_Adapter.GetLBText(m_Adapter.GetCurSel(), szDescr1);

	for (UINT i=0;i<m_pTable->dwNumEntries;i++)
	{
		MIB_IFROW Row = m_pTable->table[i];

		char szDescr[MAXLEN_IFDESCR];
		memcpy(szDescr,Row.bDescr,Row.dwDescrLen);
		szDescr[Row.dwDescrLen]=0;

		if(strcmp(szDescr1, szDescr) == 0)
		{
			UpdateData(TRUE);
			m_RecvBytes = Row.dwInOctets;
			m_RecvDiscard = Row.dwInDiscards;
			m_RecvError =  Row.dwInErrors;
			m_RecvUKProto = Row.dwInUnknownProtos;
			m_RecvUPack = Row.dwInUcastPkts;
			m_RecvNUPack = Row.dwInNUcastPkts;

			m_SendBytes = Row.dwOutOctets;
			m_SendDiscard = Row.dwOutDiscards;
			m_SendError = Row.dwOutErrors;
			m_SendUPack = Row.dwOutUcastPkts;
			m_SendNUPack = Row.dwOutNUcastPkts;
			UpdateData(FALSE);		
		}
	}
}




void CNetworkConfig::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	TableFlag = 1;
	CProtocolStat dlg;
	dlg.DoModal();
}

void CNetworkConfig::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	GetAllEntryValue();
}

void CNetworkConfig::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	GetAllEntryValue();

	CDialog::OnTimer(nIDEvent);
}

void CNetworkConfig::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	TableFlag = 2;
	CProtocolStat dlg;
	dlg.DoModal();
}

void CNetworkConfig::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	TableFlag = 3;
	CProtocolStat dlg;
	dlg.DoModal();
}

void CNetworkConfig::OnDestroy()
{
	CDialog::OnDestroy();
	
	KillTimer(1);
	// TODO: Add your message handler code here
}


void CNetworkConfig::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	SetTimer(1, 1500, NULL);
	UpdateIPConfigScreen();
}

void CNetworkConfig::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);

	KillTimer(1);
	// TODO: Add your message handler code here
}
