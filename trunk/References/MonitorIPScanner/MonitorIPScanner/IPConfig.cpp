// IPConfig.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "IPConfig.h"


#define NO_OF_INTERFACE 5
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


// CIPConfig dialog

IMPLEMENT_DYNAMIC(CIPConfig, CDialog)

CIPConfig::CIPConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CIPConfig::IDD, pParent)
	, m_MacAddress(_T(""))
	, m_IPAddress(_T(""))
	, m_SubnetMask(_T(""))
	, m_DefaultGateway(_T(""))
	, m_PreferedDNS(_T(""))
	, m_AlternateDNS(_T(""))
	, m_DhcpIP(_T(""))
	, m_DhcpSubnet(_T(""))
	, m_dhcp(FALSE)
	, m_dns(FALSE)
{

}

CIPConfig::~CIPConfig()
{
}

void CIPConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADAPTERNAME, m_AdapterName);
	DDX_Text(pDX, IDC_EDIT1, m_MacAddress);
	DDX_Text(pDX, IDC_EDIT2, m_IPAddress);
	DDX_Text(pDX, IDC_EDIT3, m_SubnetMask);
	DDX_Text(pDX, IDC_EDIT4, m_DefaultGateway);
	DDX_Text(pDX, IDC_EDIT5, m_PreferedDNS);
	DDX_Text(pDX, IDC_EDIT6, m_AlternateDNS);
	DDX_Text(pDX, IDC_EDIT7, m_DhcpIP);
	DDX_Text(pDX, IDC_EDIT8, m_DhcpSubnet);
	DDX_Check(pDX, IDC_CHECK1, m_dhcp);
	DDX_Check(pDX, IDC_CHECK2, m_dns);
}


BEGIN_MESSAGE_MAP(CIPConfig, CDialog)
	ON_CBN_SELCHANGE(IDC_ADAPTERNAME, &CIPConfig::OnCbnSelchangeAdaptername)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CIPConfig message handlers
BOOL CIPConfig::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateIPConfigScreen();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIPConfig::UpdateIPConfigScreen()
{
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO) * 10;

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen);
	
	if (dwRetVal != ERROR_SUCCESS) 
		return;

	pAdapter = &pAdapterInfo[0];		// get pointer to the first structure

	do
	{
		UpdateData(TRUE);
		m_AdapterName.AddString(pAdapter->Description);
		UpdateData(FALSE);
	} while ((pAdapter = pAdapter->Next) != NULL);
	m_AdapterName.SetCurSel(0);
	UpdateAdapterInfo();
}

void CIPConfig::UpdateAdapterInfo()
{
	PIP_ADAPTER_INFO pAdapter = NULL;
	int curpos = m_AdapterName.GetCurSel();
	pAdapter = &pAdapterInfo[curpos];		// get pointer to the first structure

	char mac[30], buf[10];
	int i = 0;
	memset(mac, 0x00, sizeof(mac));
	memset(buf, 0x00, sizeof(buf));
	for( i =0 ; i< 8; i++)
	{
		if(pAdapter->Address[i])
		{
			sprintf_s(buf, ":%2x", pAdapter->Address[i]);
			strcat_s(mac, buf);
		}
		else
		{
			if(i != 0)
				strcat_s(mac, ":00");
			else
				strcat_s(mac, "00");
		}
	}
	UpdateData(TRUE);
	m_MacAddress = mac;
	m_IPAddress = pAdapter->IpAddressList.IpAddress.String;
	m_SubnetMask = pAdapter->IpAddressList.IpMask.String;
	m_DefaultGateway = pAdapter->GatewayList.IpAddress.String;
	m_DhcpIP = pAdapter->DhcpServer.IpAddress.String;
	m_DhcpSubnet = pAdapter->DhcpServer.IpMask.String;
	if(pAdapter->DhcpEnabled)
		m_dhcp = TRUE;
	else
		m_dhcp = FALSE;
	UpdateData(FALSE);
	UpdateDNSInfo();
}
void CIPConfig::UpdateDNSInfo()
{
    PBYTE   m_pBuffer = NULL;
    ULONG   m_ulSize;
    DWORD   m_dwResult;

	PFIXED_INFO     pFixedInfo;
//    PIP_ADDR_STRING pAddrString;

    GetNetworkParams((PFIXED_INFO) m_pBuffer, &m_ulSize);

    m_pBuffer = new BYTE[m_ulSize];
    if (NULL != m_pBuffer)
    {
        m_dwResult = GetNetworkParams((PFIXED_INFO) m_pBuffer, &m_ulSize);
        if (m_dwResult == NO_ERROR)
            pFixedInfo = (PFIXED_INFO) m_pBuffer;
		UpdateData(TRUE);
		m_PreferedDNS = pFixedInfo->DnsServerList.IpAddress.String;
		m_AlternateDNS = pFixedInfo->DnsServerList.IpMask.String;
		UpdateData(FALSE);
        
    /*    pAddrString = pFixedInfo->DnsServerList.Next;
        while (NULL != pAddrString)
        {
            m_strText.Format("                 %s\r\n", pAddrString->IpAddress.String);
            m_edit1.ReplaceSel(m_strText);
            
            pAddrString = pAddrString->Next;
        }*/
	}
}
void CIPConfig::OnCbnSelchangeAdaptername()
{
	// TODO: Add your control notification handler code here
	UpdateAdapterInfo();
}

void CIPConfig::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
}
