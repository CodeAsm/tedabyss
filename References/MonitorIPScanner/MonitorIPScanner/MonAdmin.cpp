// MonAdmin.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "MonAdmin.h"


// CMonAdmin dialog
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
typedef struct
{
	char szLocalAddr[128];
	char szRemoteAddr[128];
	char State[100];
	short iLocalPort;
	short iRemotePort;
}Tcp_Connection_Table;

typedef struct
{
	char Memory;
	char Arp; 
	char Protocol;
	char Routing;
}Packet_Info;

typedef struct
{
	MIB_IPSTATS	IPStats;
	PMIB_IPNETTABLE pMib;
    MIB_IPFORWARDROW    IPForwardRow;
}Send_Buffer;

typedef struct
{
	SOCKET ServerSocket;
}Server_Socket;

char RecvBuffer[1500], SendBuffer[1500];
Tcp_Connection_Table *_sTcpTable;
UINT ServerThread_MonAdmin(LPVOID lpVoid);
UINT ClientThread_MonAdmin(LPVOID lpVoid);
Send_Buffer	SendPack, RecvPack;
Packet_Info	Send_Packet;
sockaddr_in Server_Addr, Client_Addr;
SOCKET Sock, Cli_Sock;
int  addlen;
char Ip_Addr[20];
BOOL ReplyReceived = FALSE;

IMPLEMENT_DYNAMIC(CMonAdmin, CDialog)

CMonAdmin::CMonAdmin(CWnd* pParent /*=NULL*/)
	: CDialog(CMonAdmin::IDD, pParent)
{

}

CMonAdmin::~CMonAdmin()
{
}

void CMonAdmin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_TcpTable);
	DDX_Control(pDX, IDC_IPADDRESS2, m_ipaddr);
	DDX_Control(pDX, IDC_CHECK1, m_meminfo);
	DDX_Control(pDX, IDC_CHECK2, m_arp);
	DDX_Control(pDX, IDC_CHECK3, m_protocol);
	DDX_Control(pDX, IDC_CHECK4, m_routing);
	DDX_Control(pDX, IDOK2, m_ipok);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_LIST3, m_RetrieveInfo);
	DDX_Control(pDX, IDOK, m_Retrieve);
}


BEGIN_MESSAGE_MAP(CMonAdmin, CDialog)
	ON_BN_CLICKED(IDOK, &CMonAdmin::OnBnClickedOk)
	ON_BN_CLICKED(IDOK3, &CMonAdmin::OnBnClickedOk3)
	ON_BN_CLICKED(IDOK2, &CMonAdmin::OnBnClickedOk2)
	ON_LBN_SELCHANGE(IDC_LIST1, &CMonAdmin::OnLbnSelchangeList1)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMonAdmin message handlers

void CMonAdmin::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//m_meminfo.ShowWindow(SW_SHOW);
	m_arp.ShowWindow(SW_SHOW);
	m_protocol.ShowWindow(SW_SHOW);
	m_routing.ShowWindow(SW_SHOW);
	m_ipaddr.ShowWindow(SW_HIDE);
	m_ipok.ShowWindow(SW_HIDE);
	m_progress.ShowWindow(SW_SHOW);

	GetIpAddressForConnection();
	if(strcmp(Ip_Addr, "") != 0 && strcmp(Ip_Addr, "0.0.0.0") != 0)
		CreateClientSocket();
	else
		AfxMessageBox("Select an ip address to establish the connection", 0, 0);
}
void CMonAdmin::GetIpAddressForConnection()
{
	BYTE Field1, Field2, Field3, Field4;
	char buffer[100];

	memset(buffer, 0x00, sizeof(buffer));
	memset(Ip_Addr, 0x00, sizeof(Ip_Addr));

	if(m_TcpTable.GetCurSel() != -1)
	{
		m_TcpTable.GetText(m_TcpTable.GetCurSel(), buffer);
		strncpy_s(Ip_Addr, sizeof(Ip_Addr), buffer, 16);
	}
	else
	{
		m_ipaddr.GetAddress(Field1, Field2, Field3, Field4);
		sprintf_s(Ip_Addr,"%d.%d.%d.%d",Field1, Field2, Field3, Field4);
	}
	memset(&Send_Packet, 0x00, sizeof(Send_Packet));
	if(m_meminfo.GetState())
		Send_Packet.Memory = 1;
	if(m_arp.GetState())
		Send_Packet.Arp = 1;
	if(m_protocol.GetState())
		Send_Packet.Protocol= 1;
	if(m_routing.GetState())
		Send_Packet.Routing = 1;
}
void CMonAdmin::CreateClientSocket()
{

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		AfxMessageBox("we could not find a usable WinSock DLL.");
		return;
	}

	Client_Addr.sin_port = htons(3332);
	Client_Addr.sin_addr.S_un.S_addr = inet_addr(Ip_Addr);
	Client_Addr.sin_family = AF_INET;
	addlen = sizeof(Client_Addr);

	Cli_Sock = socket(AF_INET, SOCK_STREAM, NULL);
	if(Cli_Sock == INVALID_SOCKET)
	{
		AfxMessageBox(_T("Unable to create socket. Try Again..."));
		return;
	}

	ReplyReceived = FALSE;
	m_Retrieve.EnableWindow(FALSE);
	SetTimer(1, 125, NULL);

	AfxBeginThread(&ClientThread_MonAdmin, NULL, 0, 0, 0, NULL);
}
UINT ClientThread_MonAdmin(LPVOID lpVoid)
{
	int Res;

	while(connect(Cli_Sock,(sockaddr*)&(Client_Addr),sizeof(Client_Addr)))
	{
		if(Cli_Sock == INVALID_SOCKET)
		{
			::AfxEndThread(0);
			return FALSE;
		}
		Client_Addr.sin_port = htons(3332);
		Client_Addr.sin_addr.S_un.S_addr = inet_addr(Ip_Addr);
		Client_Addr.sin_family = AF_INET;
	}

	memset(SendBuffer, 0x00, sizeof(Send_Buffer));
	memcpy(&SendBuffer, &Send_Packet, sizeof(Send_Buffer));

	if(send(Cli_Sock, SendBuffer, sizeof(Send_Buffer), 0) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("Unable to send data through this socket... Try again"), 0, 0);
		::closesocket(Cli_Sock);
		::AfxEndThread(0);
		return FALSE;
	}

	memset(&RecvPack, 0x00, sizeof(Send_Buffer));
	memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
	while((Res = recv(Cli_Sock, RecvBuffer, sizeof(RecvBuffer), 0)) != SOCKET_ERROR)
	{
		memcpy(&RecvPack, &RecvBuffer, sizeof(Send_Buffer));
		ReplyReceived = TRUE;
		break;
	}

	::closesocket(Cli_Sock);
	::AfxEndThread(0);
	return TRUE;
}


BOOL CMonAdmin::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ipaddr.ShowWindow(SW_HIDE);
	m_ipok.ShowWindow(SW_HIDE);
	UpdateTcpTableList();
	CreateServerSocket();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
VOID CMonAdmin::UpdateTcpTableList()
{
	PMIB_TCPTABLE pTcpTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	struct in_addr IpAddr;
	char Buffer[1500];

	m_TcpTable.AddString(_T(" Remote-Ip           Rem-Port      State"));
	m_TcpTable.AddString(_T("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	pTcpTable = (MIB_TCPTABLE *) MALLOC (sizeof(MIB_TCPTABLE));
	if (pTcpTable == NULL) 
	{
		AfxMessageBox(_T("Error in allocating memory"), 0, 0);
	    return;
	}
	dwSize = sizeof(MIB_TCPTABLE);

	if((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) ==  ERROR_INSUFFICIENT_BUFFER)
	{
		FREE(pTcpTable);
		pTcpTable = (MIB_TCPTABLE *) MALLOC (dwSize);
		if(pTcpTable == NULL)
		{
			AfxMessageBox(_T("Error in Allocating Memory"), 0, 0);
			return;
		}
	}
	if((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) == NO_ERROR)
	{
		_sTcpTable = (Tcp_Connection_Table *) malloc (sizeof(Tcp_Connection_Table) * pTcpTable->dwNumEntries);
		for(int  i = 0; i < (int) pTcpTable->dwNumEntries; i++)
		{
			_sTcpTable[i].iLocalPort =  ntohs((u_short) pTcpTable->table[i].dwLocalPort);	
			_sTcpTable[i].iRemotePort = ntohs((u_short) pTcpTable->table[i].dwRemotePort);
			IpAddr.S_un.S_addr = (u_long) pTcpTable->table[i].dwLocalAddr;
			strcpy_s(_sTcpTable[i].szLocalAddr, sizeof(_sTcpTable[i].szLocalAddr), inet_ntoa(IpAddr));
			IpAddr.S_un.S_addr = (u_long) pTcpTable->table[i].dwRemoteAddr;
			strcpy_s(_sTcpTable[i].szRemoteAddr, sizeof(_sTcpTable[i].szRemoteAddr), inet_ntoa(IpAddr));
			switch(pTcpTable->table[i].dwState)
			{
			case MIB_TCP_STATE_CLOSED:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "CLOSED");
                break;
            case MIB_TCP_STATE_LISTEN:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "LISTEN");
                break;
            case MIB_TCP_STATE_SYN_SENT:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "SYN-SENT");
                break;
            case MIB_TCP_STATE_SYN_RCVD:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "SYN-RECD");
                break;
            case MIB_TCP_STATE_ESTAB:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "ESABLISHED");
                break;
            case MIB_TCP_STATE_FIN_WAIT1:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "FIN-WAIT-1");
                break;
            case MIB_TCP_STATE_FIN_WAIT2:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "FIN-WAIT-2");
                break;
            case MIB_TCP_STATE_CLOSE_WAIT:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "CLOSE-WAIT");
                break;
            case MIB_TCP_STATE_CLOSING:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "CLOSING");
                break;
            case MIB_TCP_STATE_LAST_ACK:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "LAST-ACK");
                break;
            case MIB_TCP_STATE_TIME_WAIT:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "TIME-WAIT");
                break;
            case MIB_TCP_STATE_DELETE_TCB:
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "DELETE-TCB");
                break;
            default:               
				strcpy_s(_sTcpTable[i].State, sizeof(_sTcpTable[i].State), "UNKNOWN");
                break;
			}
			memset(Buffer, 0x00, sizeof(Buffer));
			sprintf_s(Buffer,"%-17s     %-8d    %-15s", _sTcpTable[i].szRemoteAddr, _sTcpTable[i].iRemotePort, _sTcpTable[i].State);
			m_TcpTable.AddString(_T(Buffer));
		}
	}
}
void CMonAdmin::OnBnClickedOk3()
{
	// TODO: Add your control notification handler code here
	m_TcpTable.SetCurSel(-1);
	//m_meminfo.ShowWindow(SW_HIDE);
	m_arp.ShowWindow(SW_HIDE);
	m_protocol.ShowWindow(SW_HIDE);
	m_routing.ShowWindow(SW_HIDE);
	m_ipaddr.ShowWindow(SW_SHOW);
	m_ipok.ShowWindow(SW_SHOW);
}

void CMonAdmin::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	//m_meminfo.ShowWindow(SW_SHOW);
	m_arp.ShowWindow(SW_SHOW);
	m_protocol.ShowWindow(SW_SHOW);
	m_routing.ShowWindow(SW_SHOW);
	m_ipaddr.ShowWindow(SW_HIDE);
	m_ipok.ShowWindow(SW_HIDE);
}


void CMonAdmin::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
	if(m_TcpTable.GetCurSel() == 0 || m_TcpTable.GetCurSel() == 1)
		m_TcpTable.SetCurSel(2);
	//m_meminfo.ShowWindow(SW_SHOW);
	m_arp.ShowWindow(SW_SHOW);
	m_protocol.ShowWindow(SW_SHOW);
	m_routing.ShowWindow(SW_SHOW);
	m_ipaddr.ShowWindow(SW_HIDE);
	m_ipok.ShowWindow(SW_HIDE);
}

void CMonAdmin::CreateServerSocket()
{
	// TODO: Add your control notification handler code here
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		AfxMessageBox("Could not find a usable WinSock DLL.");
		return;
	}
	
	Server_Addr.sin_port = htons(3332);
	Server_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Server_Addr.sin_family = AF_INET;
	addlen = sizeof(Server_Addr);

	Sock = socket(AF_INET, SOCK_STREAM, NULL);
	if(Sock == INVALID_SOCKET)
	{
		AfxMessageBox("Unable to create socket for Monitor Admin. Try Again...");
		return;
	}

	if(bind(Sock, (const sockaddr *) &Server_Addr, addlen))
	{
		AfxMessageBox("Unable to bind socket for Monitor Admin. Try Again...");
		return;
	}
	if(listen(Sock, 5))
	{
		AfxMessageBox("Unable to create Listen socket for Monitor Admin. Try Again...");
		return;
	}
	AfxBeginThread(&ServerThread_MonAdmin, NULL, 0, 0, 0, NULL);
}

UINT ServerThread_MonAdmin(LPVOID lpVoid)
{
	SOCKET	m_Socket;
	m_Socket = accept(Sock,
		(SOCKADDR *) &Server_Addr, &addlen);

	if(m_Socket == INVALID_SOCKET)
	{
		AfxMessageBox("connection accept error for Monitor Admin. Try Again...");
		::closesocket(m_Socket);
		return FALSE;
	}
	else
	{
		AfxBeginThread(&ServerThread_MonAdmin, NULL, 0, 0, 0, NULL);

		int Res;
		memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
		while(( Res = recv(m_Socket, RecvBuffer, sizeof(RecvBuffer), 0)) != SOCKET_ERROR)
		{
			memcpy(&Send_Packet, &RecvBuffer, sizeof(Send_Buffer));
			if(Send_Packet.Memory)
			{
			}
			if(Send_Packet.Arp)
			{
				ULONG nSize = 400;	 
				SendPack.pMib = (PMIB_IPNETTABLE) malloc(sizeof(MIB_IPNETTABLE)+sizeof(MIB_IPNETROW)*nSize);
				GetIpNetTable(SendPack.pMib, &nSize, TRUE);     
			}
			if(Send_Packet.Protocol)
			{
				GetIpStatistics(&SendPack.IPStats);
			}
			if(Send_Packet.Routing)
			{
				char hostname[50];
				memset(hostname, 0x00, sizeof(hostname));
				gethostname(hostname, sizeof(hostname));
				GetBestRoute(inet_addr(hostname), 0, &SendPack.IPForwardRow);
			}

			memset(SendBuffer, 0x00, sizeof(Send_Buffer));
			memcpy(&SendBuffer, &SendPack, sizeof(Send_Buffer));
			send(m_Socket, SendBuffer, sizeof(Send_Buffer), 0);
			break;
		}
	}
	::closesocket(m_Socket);
	::AfxEndThread(0);
	return TRUE;
}

void CMonAdmin::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	static int ScrollIndex = 0;

	if(ReplyReceived == TRUE)
	{
		for(int  i = m_RetrieveInfo.GetCount(); i >= 0; i--)
			m_RetrieveInfo.DeleteString(i);
		ReplyReceived = FALSE;
		DisplayReceivedData();
		m_progress.ShowWindow(SW_HIDE);
		m_Retrieve.EnableWindow(TRUE);
		::closesocket(Cli_Sock);
		Cli_Sock = INVALID_SOCKET;
		KillTimer(1);
	}

	m_progress.SetStep(1);
	m_progress.SetPos(ScrollIndex);
	ScrollIndex++;
	if(ScrollIndex == 100)
	{
		AfxMessageBox(_T("Information cannot able to retrieve..."));
		::closesocket(Cli_Sock);
		Cli_Sock = INVALID_SOCKET;
		m_progress.ShowWindow(SW_HIDE);
		m_Retrieve.EnableWindow(TRUE);
		KillTimer(1);
	}
	CDialog::OnTimer(nIDEvent);
}
void CMonAdmin::DisplayReceivedData()
{
	if(m_arp.GetState())
		DisplayARPDetails();
	if(m_protocol.GetState())
		DisplayProtocolStats();
	if(m_routing.GetState())
		DisplayRoutingDetails();
}
void CMonAdmin::DisplayProtocolStats()
{
	CString m_strText;

   if (RecvPack.IPStats.dwForwarding)
    {
		m_RetrieveInfo.AddString(_T(""));
		m_RetrieveInfo.AddString(_T("IP Statistics"));
		m_RetrieveInfo.AddString(_T("...................."));

		m_strText.Format("Forwarding: %lu", RecvPack.IPStats.dwForwarding);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Default TTL: %lu", RecvPack.IPStats.dwDefaultTTL);
        m_RetrieveInfo.AddString(m_strText);

		m_strText.Format("Datagrams received: %lu", RecvPack.IPStats.dwInReceives);
        m_RetrieveInfo.AddString(m_strText);

		m_strText.Format("Header errors received: %lu", RecvPack.IPStats.dwInHdrErrors);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Address errors received: %lu", RecvPack.IPStats.dwInAddrErrors);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Datagrams forwarded: %lu", RecvPack.IPStats.dwForwDatagrams);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Unknown protocol datagrams: %lu", RecvPack.IPStats.dwInUnknownProtos);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Discarded datagrams received: %lu", RecvPack.IPStats.dwInDiscards);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Delivered datagrams received: %lu", RecvPack.IPStats.dwInDelivers);
        m_RetrieveInfo.AddString(m_strText);

		m_strText.Format("Discarded datagrams sent: %lu", RecvPack.IPStats.dwOutDiscards);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Route-less datagrams: %lu", RecvPack.IPStats.dwOutNoRoutes);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Orphanded fragmented datagrams: %lu", RecvPack.IPStats.dwReasmTimeout);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Datagrams requiring reassembly: %lu", RecvPack.IPStats.dwReasmReqds);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Successful datagram reassemblies: %lu", RecvPack.IPStats.dwReasmOks);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Failed datagram reassemblies: %lu", RecvPack.IPStats.dwReasmFails);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Datagrams fragmented: %lu", RecvPack.IPStats.dwFragFails);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Successful datagram fragmentations: %lu", RecvPack.IPStats.dwFragOks);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Failed datagram fragmentations: %lu", RecvPack.IPStats.dwFragFails);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Interfaces: %lu", RecvPack.IPStats.dwNumIf);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("IP addresses: %lu", RecvPack.IPStats.dwNumAddr);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Routes in routing table: %lu", RecvPack.IPStats.dwNumRoutes);
        m_RetrieveInfo.AddString(m_strText);
   }
}

void CMonAdmin::DisplayRoutingDetails()
{
	CString m_strText;
    in_addr ia;
    char *szProtocols[4] = {"Other", "Local", "Network Mgmt", "ICMP"};
    char *szTypes[4] = {"Other", "Invalid", "Not final", "Final"};

    if (RecvPack.IPForwardRow.dwForwardDest)
    {
		m_RetrieveInfo.AddString(_T(""));
 		m_RetrieveInfo.AddString(_T("Routing Table Entries"));
		m_RetrieveInfo.AddString(_T("........................"));

        ia.S_un.S_addr = RecvPack.IPForwardRow.dwForwardDest;
        m_strText.Format("Destination IP address: %s", inet_ntoa(ia));
        m_RetrieveInfo.AddString(m_strText);

        ia.S_un.S_addr = RecvPack.IPForwardRow.dwForwardMask;
        m_strText.Format("Destination subnet mask: %s", inet_ntoa(ia));
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Multi-path route conditions: %lu", RecvPack.IPForwardRow.dwForwardPolicy);
        m_RetrieveInfo.AddString(m_strText);

        ia.S_un.S_addr = RecvPack.IPForwardRow.dwForwardNextHop;
        m_strText.Format("Next hop IP address: %s", inet_ntoa(ia));
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Interface's index: %lu", RecvPack.IPForwardRow.dwForwardIfIndex);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Route type: %s", szTypes[RecvPack.IPForwardRow.dwForwardType - 1]);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Generating protocol: %s", szProtocols[RecvPack.IPForwardRow.dwForwardProto - 1]);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Route age (sec): %lu", RecvPack.IPForwardRow.dwForwardAge);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Next hop system number: %lu", RecvPack.IPForwardRow.dwForwardNextHopAS);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", RecvPack.IPForwardRow.dwForwardMetric1);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", RecvPack.IPForwardRow.dwForwardMetric2);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", RecvPack.IPForwardRow.dwForwardMetric3);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", RecvPack.IPForwardRow.dwForwardMetric4);
        m_RetrieveInfo.AddString(m_strText);

        m_strText.Format("Protocol-specific metric: %lu", RecvPack.IPForwardRow.dwForwardMetric5);
        m_RetrieveInfo.AddString(m_strText);
    }
}
void CMonAdmin::DisplayARPDetails()
{
	char ipaddr[20], macaddr[20], buf[100];

	if(RecvPack.pMib->dwNumEntries)
	{
		sprintf_s(buf, "ARP Table ( %d Entries) ", RecvPack.pMib->dwNumEntries);
		m_RetrieveInfo.AddString(buf);
		m_RetrieveInfo.AddString(_T(""));
		m_RetrieveInfo.AddString(_T("------------------------------------------------------------------"));
		m_RetrieveInfo.AddString(_T("Internet Address          Physical Address                 Type"));

		for (int i =0; i < (int) RecvPack.pMib->dwNumEntries;i++) 
		{

			sprintf_s(ipaddr,"%d.%d.%d.%d",
					( RecvPack.pMib->table[i].dwAddr&0x0000ff),    ((RecvPack.pMib->table[i].dwAddr&0xff00)>>8),
					((RecvPack.pMib->table[i].dwAddr&0xff0000)>>16),(RecvPack.pMib->table[i].dwAddr>>24)
			);
			sprintf_s(macaddr, "%02x-%02x-%02x-%02x-%02x-%02x",
				   RecvPack.pMib->table[i].bPhysAddr[0],RecvPack.pMib->table[i].bPhysAddr[1],
				   RecvPack.pMib->table[i].bPhysAddr[2],RecvPack.pMib->table[i].bPhysAddr[3],
				   RecvPack.pMib->table[i].bPhysAddr[4],RecvPack.pMib->table[i].bPhysAddr[5]
			);

			if(RecvPack.pMib->table[i].dwType == 3) 
				sprintf_s(buf, "%-25s  %-25s       Dynamic",ipaddr,macaddr);
			else if (RecvPack.pMib->table[i].dwType == 4) 
				sprintf_s(buf, "%-25s  %-25s       Static",ipaddr,macaddr);
			
			m_RetrieveInfo.AddString(buf);
		}
	}
}
