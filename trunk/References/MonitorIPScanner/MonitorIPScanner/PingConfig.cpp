// PingConfig.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "PingConfig.h"
#include <process.h>

// CPingConfig dialog
ParameterInfo PingOption;
ReplyInfo	PingReply;
CPingConfig	*Ping;
HANDLE hThread;
DWORD  dwThreadID;
BOOL	ThreadFlag;
IMPLEMENT_DYNAMIC(CPingConfig, CDialog)

CPingConfig::CPingConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CPingConfig::IDD, pParent)
	, m_DomainName(_T(""))
	, m_Timeout(5000)
	, m_SendBuf(32)
	, m_TTL(30)
	, m_PacktoSend(3)
{
	  hThread = INVALID_HANDLE_VALUE;
	  dwThreadID = 0;
	  PingOption.m_bResolveAddressesToHostnames = FALSE;
	  PingOption.m_bPingTillStopped = TRUE;
	  PingOption.m_nRequestsToSend = 3;
	  PingOption.m_nTTL = 30;
	  PingOption.m_wDataRequestSize = 32;
	  PingOption.m_dwTimeout = 5000;
	  PingOption.m_bUseRawSockets = FALSE;
	  PingOption.m_nTOS = 0;
	  PingOption.m_bDontFragment  = FALSE;
}

CPingConfig::~CPingConfig()
{
}

void CPingConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, IPAddress);
	DDX_Text(pDX, IDC_EDIT1, m_DomainName);
	DDX_Text(pDX, IDC_EDIT2, m_Timeout);
	DDX_Text(pDX, IDC_EDIT5, m_SendBuf);
	DDX_Text(pDX, IDC_EDIT4, m_TTL);
	DDX_Text(pDX, IDC_EDIT3, m_PacktoSend);
	DDX_Control(pDX, IDC_LIST1, m_Output);
}


BEGIN_MESSAGE_MAP(CPingConfig, CDialog)
	ON_BN_CLICKED(IDOK, &CPingConfig::OnBnClickedOk)
	ON_WM_SHOWWINDOW()
	ON_WM_KILLFOCUS()
	ON_BN_CLICKED(IDC_RADIO1, &CPingConfig::OnBnClickedRadio1)
END_MESSAGE_MAP()


// CPingConfig message handlers
VOID PingDestinationAddress( LPVOID lpParam)
{
	// TODO: Add your control notification handler code here
	USES_CONVERSION;
	char Buffer[200];
	int NoOfPacket;

	NoOfPacket = 0;
	memset(Buffer, 0x00, sizeof(Buffer));
	unsigned long	addr = inet_addr(PingOption.m_sHostToResolve);

	if (addr == INADDR_NONE)
	{
		//Not a dotted address, then do a lookup of the name
		hostent* hp = gethostbyname(PingOption.m_sHostToResolve);
		if (hp)
			memcpy(&addr, hp->h_addr, hp->h_length);
		else
		{
			sprintf_s(Buffer, "Could not resolve the host name %s", PingOption.m_sHostToResolve);
			::SendDlgItemMessage(Ping->m_hWnd, IDC_LIST1, LB_ADDSTRING, (WPARAM) NULL, (LPARAM) Buffer);
			_endthread();
			return;
		}
	}

	while(1)
	{
		HANDLE hIP = IcmpCreateFile();
		if (hIP == INVALID_HANDLE_VALUE)
		{
			sprintf_s(Buffer, "Could not get a valid ICMP handle");
			::SendDlgItemMessage(Ping->m_hWnd, IDC_LIST1, LB_ADDSTRING, (WPARAM) NULL, (LPARAM) Buffer);
			break;
		}
		//Set up the option info structure
		IP_OPTION_INFORMATION OptionInfo;
		memset(&OptionInfo, 0, sizeof(OptionInfo));
		OptionInfo.Ttl = PingOption.m_nTTL;
		OptionInfo.Tos = PingOption.m_nTOS;
		if (PingOption.m_bDontFragment)
			OptionInfo.Flags = IP_FLAG_DF;

		//Set up the data which will be sent
		unsigned char* pBuf = new unsigned char[PingOption.m_wDataRequestSize];
		memset(pBuf, 'S', PingOption.m_wDataRequestSize);

		//Do the actual Ping
		DWORD dwReplySize = sizeof(ICMP_ECHO_REPLY) + max(PingOption.m_wDataRequestSize, 8);
		unsigned char* pReply = new unsigned char[dwReplySize];
		ICMP_ECHO_REPLY* pEchoReply = reinterpret_cast<ICMP_ECHO_REPLY*>(pReply);

		DWORD nRecvPackets = IcmpSendEcho(hIP, addr, pBuf, PingOption.m_wDataRequestSize, &OptionInfo, pReply, dwReplySize, PingOption.m_dwTimeout);

		BOOL bSuccess = (nRecvPackets == 1);

		//Check the data we got back is what was sent
		if (bSuccess)
		{
			char* pReplyData = static_cast<char*>(pEchoReply->Data);
			for (int i = 0; i < pEchoReply->DataSize && bSuccess; i++)
				bSuccess = (pReplyData[i] == 'S');

			memset(Buffer, 0x00, sizeof(Buffer));
			if (!bSuccess)
			{
				DWORD dwError = GetLastError();
				switch(dwError)
				{
				case ERROR_UNEXP_NET_ERR:
					sprintf_s(Buffer, "An unexpected network error occurred");
					break;
				case ERROR_BAD_NET_RESP:
					sprintf_s(Buffer, "The specified server cannot perform the requested operation");
					break;
				case WAIT_TIMEOUT:
					sprintf_s(Buffer, "Requested Time Out...");
					break;
				case ERROR_NETWORK_UNREACHABLE:
					sprintf_s(Buffer, "The network location cannot be reached");
					break;
				case ERROR_CONNECTION_ACTIVE:
					sprintf_s(Buffer, "An invalid operation was attempted on an active network connection");
					break;
				default:
					sprintf_s(Buffer, "Unidentified error occurs on network");
					break;
				}
			}
			else
				sprintf_s(Buffer, "Reply from %s : bytes : %d time<%dms TTL = %d", PingOption.m_sHostToResolve, pEchoReply->DataSize, pEchoReply->RoundTripTime, pEchoReply->Options.Ttl);
			::SendDlgItemMessage(Ping->m_hWnd, IDC_LIST1, LB_ADDSTRING, (WPARAM) NULL, (LPARAM) Buffer);
			IcmpCloseHandle(hIP);
			delete [] pBuf;
			delete [] pReply;
			Sleep(500);
			NoOfPacket++;
			if(ThreadFlag == TRUE || PingOption.m_nRequestsToSend == NoOfPacket)
			{
				::SetDlgItemText(Ping->m_hWnd, IDOK, "Ping");
				_endthread();
				return ;
			}
		}
	}
	_endthread();
	return ;
}

void CPingConfig::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	char Buf[10];
	memset(Buf, 0x00, sizeof(Buf));
	::GetDlgItemText(this->m_hWnd, IDOK, Buf, sizeof(Buf));
	if(strcmp(Buf, "Ping") == 0)
	{
		char Buffer[100];
		::SetDlgItemText(this->m_hWnd, IDOK, "Stop");
		if(!GetAllParameters())
			return;
		for(int i = m_Output.GetCount()+1 ; i >= 0 ; i--)
			m_Output.DeleteString(i);

		if(Ping == NULL)
		{
			Ping = new CPingConfig;
			Ping = this;
		}
		memset(Buffer, 0x00, sizeof(Buffer));
		sprintf_s(Buffer, "Pinging %s with %d bytes of data...", PingOption.m_sHostToResolve, PingOption.m_wDataRequestSize);
		m_Output.AddString(_T(Buffer));
		ThreadFlag = FALSE;
		if(!_beginthread(&PingDestinationAddress, 0, NULL)) //hThread != INVALID_HANDLE_VALUE)
		{
			m_Output.AddString(_T("Ping Thread cannot be initiated..."));
		}
	}
	else if(strcmp(Buf, "Stop") == 0)
	{
		ThreadFlag = TRUE;
		//::SetDlgItemText(this->m_hWnd, IDOK, "Ping");
	}
}


BOOL CPingConfig::GetAllParameters()
{
	// TODO: Add your control notification handler code here
	BYTE Field1, Field2, Field3, Field4;
	BOOL m_IP;
	m_IP = ::IsDlgButtonChecked(this->m_hWnd, IDC_RADIO1);
	UpdateData(TRUE);
	if(m_IP == BST_CHECKED)
	{
		if(IPAddress.IsBlank())
		{
			AfxMessageBox("Enter the IP address...", 0, 0);
			return FALSE;
		}
		IPAddress.GetAddress(Field1, Field2, Field3, Field4);
		PingOption.m_sHostToResolve.Format("%d.%d.%d.%d", Field1, Field2, Field3, Field4);
	}
	else
	{
		if(m_DomainName.IsEmpty())
		{
			AfxMessageBox("Enter the Domain Name...", 0, 0);
			return FALSE;
		}
		PingOption.m_sHostToResolve = m_DomainName;
	}

	PingOption.m_nRequestsToSend = m_PacktoSend;
	PingOption.m_nTTL = m_TTL;
	PingOption.m_wDataRequestSize = m_SendBuf;
	PingOption.m_dwTimeout = m_Timeout;
	UpdateData(FALSE);
	return TRUE;
}

void CPingConfig::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	::SetDlgItemText(this->m_hWnd, IDOK, "Ping");
	::CheckRadioButton(this->m_hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
	// TODO: Add your message handler code here
}

void CPingConfig::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);

	ThreadFlag = TRUE;
	if(hThread != INVALID_HANDLE_VALUE)
		_endthread();
	::SetDlgItemText(this->m_hWnd, IDOK, "Ping");
	delete Ping;
	
	// TODO: Add your message handler code here
}

void CPingConfig::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
}
