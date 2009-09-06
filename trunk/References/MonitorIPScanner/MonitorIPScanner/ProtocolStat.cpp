// ProtocolStat.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "ProtocolStat.h"

// CProtocolStat dialog
extern int 	TableFlag;
#define ARP_TABLE_SIZE   400

IMPLEMENT_DYNAMIC(CProtocolStat, CDialog)

CProtocolStat::CProtocolStat(CWnd* pParent /*=NULL*/)
	: CDialog(CProtocolStat::IDD, pParent)
{

}

CProtocolStat::~CProtocolStat()
{
}

void CProtocolStat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPSTATS, m_ipstats);
}


BEGIN_MESSAGE_MAP(CProtocolStat, CDialog)
END_MESSAGE_MAP()


// CProtocolStat message handlers
BOOL CProtocolStat::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(	TableFlag == 1)
		GetIPStatistics();
	else if(TableFlag == 2)
		GetRoutingTable();
	else if(TableFlag == 3)
		GetARPTable();


	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CProtocolStat::GetIPStatistics()
{
    MIB_IPSTATS     IPStats;
	DWORD m_dwResult;
	CString m_strText;

	m_dwResult = GetIpStatistics(&IPStats);

	m_ipstats.AddString(_T("IP Statistics"));
	m_ipstats.AddString(_T("...................."));
	m_ipstats.AddString(_T(""));

   if (m_dwResult == NO_ERROR)
    {
        m_strText.Format("Forwarding: %lu", IPStats.dwForwarding);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Default TTL: %lu", IPStats.dwDefaultTTL);
        m_ipstats.AddString(m_strText);

		m_strText.Format("Datagrams received: %lu", IPStats.dwInReceives);
        m_ipstats.AddString(m_strText);

		m_strText.Format("Header errors received: %lu", IPStats.dwInHdrErrors);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Address errors received: %lu", IPStats.dwInAddrErrors);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Datagrams forwarded: %lu", IPStats.dwForwDatagrams);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Unknown protocol datagrams: %lu", IPStats.dwInUnknownProtos);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Discarded datagrams received: %lu", IPStats.dwInDiscards);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Delivered datagrams received: %lu", IPStats.dwInDelivers);
        m_ipstats.AddString(m_strText);

		m_strText.Format("Discarded datagrams sent: %lu", IPStats.dwOutDiscards);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Route-less datagrams: %lu", IPStats.dwOutNoRoutes);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Orphanded fragmented datagrams: %lu", IPStats.dwReasmTimeout);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Datagrams requiring reassembly: %lu", IPStats.dwReasmReqds);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Successful datagram reassemblies: %lu", IPStats.dwReasmOks);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Failed datagram reassemblies: %lu", IPStats.dwReasmFails);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Datagrams fragmented: %lu", IPStats.dwFragFails);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Successful datagram fragmentations: %lu", IPStats.dwFragOks);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Failed datagram fragmentations: %lu", IPStats.dwFragFails);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Interfaces: %lu", IPStats.dwNumIf);
        m_ipstats.AddString(m_strText);

        m_strText.Format("IP addresses: %lu", IPStats.dwNumAddr);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Routes in routing table: %lu", IPStats.dwNumRoutes);
        m_ipstats.AddString(m_strText);
    }
    else
    {
        m_strText.Format("GetIpStatistics() failed.  Result = %lu", m_dwResult);
        m_ipstats.AddString(m_strText);
    }

}

void CProtocolStat::GetRoutingTable()
{
    MIB_IPFORWARDROW    IPForwardRow;
    in_addr             ia;
	DWORD m_dwResult;
	CString m_strText;
    char                *szProtocols[4] = {"Other", "Local", "Network Mgmt", "ICMP"};
    char                *szTypes[4] = {"Other", "Invalid", "Not final", "Final"};
	 

	m_dwResult = GetBestRoute(inet_addr(_T("207.219.70.31")), 0, &IPForwardRow);


 	m_ipstats.AddString(_T("Routing Table Entries"));
	m_ipstats.AddString(_T("........................"));
	m_ipstats.AddString(_T(""));
    if (m_dwResult == NO_ERROR)
    {
        ia.S_un.S_addr = IPForwardRow.dwForwardDest;
        m_strText.Format("Destination IP address: %s", inet_ntoa(ia));
        m_ipstats.AddString(m_strText);

        ia.S_un.S_addr = IPForwardRow.dwForwardMask;
        m_strText.Format("Destination subnet mask: %s", inet_ntoa(ia));
        m_ipstats.AddString(m_strText);

        m_strText.Format("Multi-path route conditions: %lu", IPForwardRow.dwForwardPolicy);
        m_ipstats.AddString(m_strText);

        ia.S_un.S_addr = IPForwardRow.dwForwardNextHop;
        m_strText.Format("Next hop IP address: %s", inet_ntoa(ia));
        m_ipstats.AddString(m_strText);

        m_strText.Format("Interface's index: %lu", IPForwardRow.dwForwardIfIndex);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Route type: %s", szTypes[IPForwardRow.dwForwardType - 1]);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Generating protocol: %s", szProtocols[IPForwardRow.dwForwardProto - 1]);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Route age (sec): %lu", IPForwardRow.dwForwardAge);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Next hop system number: %lu", IPForwardRow.dwForwardNextHopAS);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", IPForwardRow.dwForwardMetric1);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", IPForwardRow.dwForwardMetric2);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", IPForwardRow.dwForwardMetric3);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", IPForwardRow.dwForwardMetric4);
        m_ipstats.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", IPForwardRow.dwForwardMetric5);
        m_ipstats.AddString(m_strText);
    }
    else
    {
        m_strText.Format("GetBestRoute() failed.  Result = %lu", m_dwResult);
        m_ipstats.AddString(m_strText);
    }
}
void CProtocolStat::GetARPTable()
{
	ULONG nSize = ARP_TABLE_SIZE;
  
    PMIB_IPNETTABLE pMib = (PMIB_IPNETTABLE) malloc(sizeof(MIB_IPNETTABLE)+sizeof(MIB_IPNETROW)*nSize);

	DWORD dwRet = GetIpNetTable(pMib,&nSize,TRUE);     

     if (nSize > ARP_TABLE_SIZE) 
	 {
         AfxMessageBox(_T("[Warning] Insufficient Memory(allocated %d needed %d)"),
			    ARP_TABLE_SIZE, nSize);
        
          nSize = ARP_TABLE_SIZE;

    } else {
         nSize = (unsigned long) pMib->dwNumEntries ;
    }

	char ipaddr[20], macaddr[20], buf[100];

	sprintf_s(buf, "ARP Table ( %d Entries) ", nSize);
	m_ipstats.AddString(buf);
	m_ipstats.AddString(_T("------------------------------------------------------------------"));
	m_ipstats.AddString(_T("Internet Address          Physical Address                 Type"));

	for (int i =0; i < (int) nSize;i++) 
    {

		sprintf_s(ipaddr,"%d.%d.%d.%d",
			    ( pMib->table[i].dwAddr&0x0000ff),    ((pMib->table[i].dwAddr&0xff00)>>8),
                ((pMib->table[i].dwAddr&0xff0000)>>16),(pMib->table[i].dwAddr>>24)
		);
		sprintf_s(macaddr, "%02x-%02x-%02x-%02x-%02x-%02x",
			   pMib->table[i].bPhysAddr[0],pMib->table[i].bPhysAddr[1],
			   pMib->table[i].bPhysAddr[2],pMib->table[i].bPhysAddr[3],
			   pMib->table[i].bPhysAddr[4],pMib->table[i].bPhysAddr[5]
		);

		if(pMib->table[i].dwType == 3) 
			sprintf_s(buf, "%-25s  %-25s       Dynamic",ipaddr,macaddr);
		else if (pMib->table[i].dwType == 4) 
			sprintf_s(buf, "%-25s  %-25s       Static",ipaddr,macaddr);
		
		m_ipstats.AddString(buf);
	}
}