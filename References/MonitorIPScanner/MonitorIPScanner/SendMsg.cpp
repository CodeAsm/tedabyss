// SendMsg.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "SendMsg.h"


// CSendMsg dialog
UINT ServerThread(LPVOID lpVoid);
UINT ClientThread(LPVOID lpVoid);
Server_Socket	Ser_Sock[NO_OF_SOCKET];
int Sock_Index;
CSendMsg *SendMsgDlg;

IMPLEMENT_DYNAMIC(CSendMsg, CDialog)

CSendMsg::CSendMsg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendMsg::IDD, pParent)
	, m_PortNo(0)
	, m_Msg(_T(""))
{
}

CSendMsg::~CSendMsg()
{
}

void CSendMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_PortNo);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IPAddr);
	DDX_Text(pDX, IDC_EDIT2, m_Msg);
	DDX_Control(pDX, IDC_LIST1, m_UserList);
	DDX_Control(pDX, IDC_LIST2, m_MsgList);
}


BEGIN_MESSAGE_MAP(CSendMsg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO1, &CSendMsg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CSendMsg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_BUTTON1, &CSendMsg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSendMsg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSendMsg message handlers

void CSendMsg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	Sock = 0;
	m_IPAddr.EnableWindow(FALSE);
	::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Start Server");
	::CheckRadioButton(this->m_hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
}

void CSendMsg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	if(Sock != 0)
	{
		m_MsgList.AddString("Server Sockets were deleted...");
		::closesocket(Sock);
	}

	m_IPAddr.EnableWindow(TRUE);
	m_UserList.EnableWindow(FALSE);
	::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Start Client");

//	::CheckRadioButton(this->m_hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
}

void CSendMsg::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	m_IPAddr.EnableWindow(FALSE);
	m_UserList.EnableWindow(TRUE);
	::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Start Server");
}

void CSendMsg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	BOOL m_Sock;
	m_Sock = ::IsDlgButtonChecked(this->m_hWnd, IDC_RADIO1);
	if(m_Sock)
	{
		// Start Client Application
		char Buf[20];
		memset(Buf, 0x00, sizeof(Buf));
		::GetDlgItemText(this->m_hWnd, IDC_BUTTON1, Buf, sizeof(Buf));
		if(strcmp(Buf, "Start Client") == 0)
		{
			for(int i = m_MsgList.GetCount()+1 ; i >= 0 ; i--)
				m_MsgList.DeleteString(i);
			::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Stop Client");
			StartClientApplication();
		}
		else if(strcmp(Buf, "Stop Client") == 0)
		{
			m_MsgList.AddString("Client Sockets were deleted...");
			::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Start Client");
			::closesocket(ClientSocket);
		}
	}
	else
	{
		for(int i = m_UserList.GetCount()+1 ; i >= 0 ; i--)
			m_UserList.DeleteString(i);
		//Start Server Application
		Sock_Index = 0;
		char Buf[20];
		memset(Buf, 0x00, sizeof(Buf));
		::GetDlgItemText(this->m_hWnd, IDC_BUTTON1, Buf, sizeof(Buf));
		if(strcmp(Buf, "Start Server") == 0)
		{
			for(int i = m_MsgList.GetCount()+1 ; i >= 0 ; i--)
				m_MsgList.DeleteString(i);

			::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Stop Server");
			StartServerApplication();
		}
		else if(strcmp(Buf, "Stop Server") == 0)
		{
			m_MsgList.AddString("Server Sockets were deleted...");
			::SetDlgItemText(this->m_hWnd, IDC_BUTTON1, "Start Server");
			::closesocket(Sock);
		}
	}
}

void CSendMsg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	char SendBuf[1024], Buf[1024];
	BOOL m_Sock;
	m_Sock = ::IsDlgButtonChecked(this->m_hWnd, IDC_RADIO1);
	if(m_Sock)
	{
		// Send Client Message
		if(ClientSocket == INVALID_SOCKET)
		{
			AfxMessageBox(_T("No Socket connection established for sending data..."), 0, 0);
			m_MsgList.AddString(_T("Socket not yet created for sending data..."));
			return;
		}
		memset(SendBuf, 0x00, sizeof(SendBuf));
		UpdateData(TRUE);
		strncpy_s(SendBuf, m_Msg, m_Msg.GetLength());
		UpdateData(FALSE);
		if(!send(ClientSocket, SendBuf, sizeof(SendBuf), 0))
		{
			memset(Buf, 0x00, sizeof(Buf));
			sprintf_s(Buf, "Error in sending data. Error No : %d", GetLastError());
			m_MsgList.AddString(_T(Buf));
			return;
		}
		memset(Buf, 0x00, sizeof(Buf));
		strcat_s(Buf, "Client : ");
		strncat_s(Buf, SendBuf, sizeof(Buf));
		m_MsgList.AddString(Buf);
		UpdateData(TRUE);
		m_Msg.Empty();
		UpdateData(FALSE);
		return;
	}
	else
	{
		// Send Server Message
		memset(Buf, 0x00, sizeof(Buf));
		int User_List = m_UserList.GetCurSel();
		if(User_List <  0)
		{
			AfxMessageBox(_T("Select client to which the data has to send..."), 0, 0);
			m_MsgList.AddString(_T("Invalid Client name..."));
			return;
		}
		m_UserList.GetText(User_List, Buf );
		memset(SendBuf, 0x00, sizeof(SendBuf));
		UpdateData(TRUE);
		strncpy_s(SendBuf, m_Msg, m_Msg.GetLength());
		UpdateData(FALSE);
	
		for(int i = 0 ; i <= Sock_Index; i++)
		{
			if(strcmp(Buf, Ser_Sock[i].ClientName) == 0)
			{
				if(!send(Ser_Sock[i].ServerSocket, SendBuf, sizeof(SendBuf), 0))
				{
					memset(Buf, 0x00, sizeof(Buf));
					sprintf_s(Buf, "Error in sending data. Error No : %d", GetLastError());
					m_MsgList.AddString(_T(Buf));
					UpdateData(TRUE);
					m_Msg.Empty();
					UpdateData(FALSE);
					return;
				}
				memset(Buf, 0x00, sizeof(Buf));
				strcpy_s(Buf, Ser_Sock[i].ClientName);
				strcat_s(Buf, " - From Server : ");
				strncat_s(Buf, SendBuf, sizeof(Buf));
				m_MsgList.AddString(Buf);
				UpdateData(TRUE);
				m_Msg.Empty();
				UpdateData(FALSE);
				return;
			}
		}
		AfxMessageBox(_T("No connection established for this client..."), 0, 0);
		UpdateData(TRUE);
		m_Msg.Empty();
		UpdateData(FALSE);
		return;
	}
}

void CSendMsg::StartClientApplication()
{
	// TODO: Add your control notification handler code here

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	char IpAddr[20];
	BYTE Field1, Field2, Field3, Field4;

	if(SendMsgDlg == NULL)
	{
		SendMsgDlg = new CSendMsg;
		SendMsgDlg = this;
	}
	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		AfxMessageBox("we could not find a usable WinSock DLL.");
		return;
	}
	
	UpdateData(TRUE);
	if(m_PortNo <= 1023)
	{
		AfxMessageBox("Enter Port No greater than 1023");
		return;
	}
	memset(IpAddr, 0x00, sizeof(IpAddr));
	m_IPAddr.GetAddress(Field1, Field2, Field3, Field4);
	sprintf_s(IpAddr, "%d.%d.%d.%d", Field1, Field2, Field3, Field4);
	Client_Addr.sin_port = htons(m_PortNo);
	Client_Addr.sin_addr.S_un.S_addr = inet_addr(IpAddr);
	Client_Addr.sin_family = AF_INET;
	UpdateData(FALSE);
	addlen = sizeof(Client_Addr);

	ClientSocket = socket(AF_INET, SOCK_STREAM, NULL);
	if(ClientSocket == INVALID_SOCKET)
	{
		m_MsgList.AddString(_T("Unable to create socket. Try Again..."));
		return;
	}
	m_MsgList.AddString(_T("Socket created successfully"));

	m_MsgList.AddString(_T("Socket waiting for server connection..."));

	AfxBeginThread(&ClientThread, NULL, 0, 0, 0, NULL);
}

void CSendMsg::StartServerApplication()
{
	// TODO: Add your control notification handler code here
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	if(SendMsgDlg == NULL)
	{
		SendMsgDlg = new CSendMsg;
		SendMsgDlg = this;
	}

	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		AfxMessageBox("we could not find a usable WinSock DLL.");
		return;
	}
	
	UpdateData(TRUE);
	if(m_PortNo <= 1023)
	{
		AfxMessageBox("Enter Port No greater than 1023");
		return;
	}
	Server_Addr.sin_port = htons(m_PortNo);
	Server_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Server_Addr.sin_family = AF_INET;
	UpdateData(FALSE);
	addlen = sizeof(Server_Addr);
	Sock = socket(AF_INET, SOCK_STREAM, NULL);
	if(Sock == INVALID_SOCKET)
	{
		m_MsgList.AddString(_T("Unable to create socket. Try Again..."));
		return;
	}
	m_MsgList.AddString(_T("Socket created successfully"));
	if(bind(Sock, (const sockaddr *) &Server_Addr, addlen))
	{
		::closesocket(Sock);
		m_MsgList.AddString(_T("Unable to bind. Try Again..."));
		return;
	}
	m_MsgList.AddString(_T("Socket binded done successfully"));
	if(listen(Sock, 5))
	{
		::closesocket(Sock);
		m_MsgList.AddString(_T("Binded Socket unable to listen on the port. Try Again..."));
		return;
	}
	m_MsgList.AddString(_T("Socket waiting for client connection..."));

	AfxBeginThread(&ServerThread, NULL, 0, 0, 0, NULL);

}
UINT ServerThread(LPVOID lpVoid)
{
	char RecvBuffer[500], Buffer[1024];
	SOCKET	m_Socket;

	m_Socket = accept(SendMsgDlg->Sock,
		(SOCKADDR *) &SendMsgDlg->Server_Addr, &SendMsgDlg->addlen);

	if(m_Socket == INVALID_SOCKET)
	{
		SendMsgDlg->m_MsgList.AddString("Socket accept error...");
		::closesocket(m_Socket);
		::AfxEndThread(0);
		return FALSE;
	}
	else
	{
		AfxBeginThread(&ServerThread, NULL, 0, 0, 0, NULL);
		memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
		sprintf_s(RecvBuffer,"Connection Request from %s... Need to Accept... If YES provide a name for this connection ", inet_ntoa(SendMsgDlg->Server_Addr.sin_addr));
		int Res = AfxMessageBox(RecvBuffer, MB_YESNO, 0);
		if(Res == IDNO)
		{
			::closesocket(m_Socket);
			::AfxEndThread(0);
			return FALSE;
		}

		Sock_Index++;
		sprintf_s(Ser_Sock[Sock_Index-1].ClientName,"Client %d", Sock_Index);
		Ser_Sock[Sock_Index-1].ServerSocket = m_Socket;
		Ser_Sock[Sock_Index-1].Serv_Sockaddr = SendMsgDlg->Server_Addr;
		SendMsgDlg->m_UserList.AddString(Ser_Sock[Sock_Index-1].ClientName);

		memset(Buffer, 0x00, sizeof(Buffer));
		strcat_s(Buffer, Ser_Sock[Sock_Index-1].ClientName);
		strcat_s(Buffer, " : Connected.........");
		SendMsgDlg->m_MsgList.AddString(Buffer);

		memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
		while((Res = recv(m_Socket, RecvBuffer, sizeof(RecvBuffer), 0)) != SOCKET_ERROR)
		{
			memset(Buffer, 0x00, sizeof(Buffer));
			for(int i = 0; i <= Sock_Index; i++)
			{
				if(m_Socket == Ser_Sock[i].ServerSocket)
					strcpy_s(Buffer, Ser_Sock[i].ClientName);
			}
			strcat_s(Buffer, " : ");
			strncat_s(Buffer, RecvBuffer, sizeof(RecvBuffer));
			SendMsgDlg->m_MsgList.AddString(Buffer);
		}
		send(m_Socket, "Disconnected", 100, 0);
		memset(Buffer, 0x00, sizeof(Buffer));
		strcpy_s(Buffer, Ser_Sock[Sock_Index-1].ClientName);
		strcat_s(Buffer, ": Disconnected");
		SendMsgDlg->m_MsgList.AddString(_T(Buffer));
		char buf[20];
		for(int i = SendMsgDlg->m_UserList.GetCount()+1; i >= 0 ; i--)
		{
			memset(buf, 0x00, sizeof(buf));
			SendMsgDlg->m_UserList.GetText(i, buf);
			if(strcmp(Ser_Sock[Sock_Index-1].ClientName, buf) == 0)
				SendMsgDlg->m_UserList.DeleteString(i);
		}
	}
	::closesocket(m_Socket);
	::AfxEndThread(0);
	return TRUE;
}

UINT ClientThread(LPVOID lpVoid)
{
	char RecvBuffer[1024], Buffer[1024];
	int Res;

	while(connect(SendMsgDlg->ClientSocket,(sockaddr*)&(SendMsgDlg->Client_Addr),sizeof(SendMsgDlg->Client_Addr)))
	{
		SendMsgDlg->m_MsgList.AddString("Waiting for connection.....");
		Sleep(500);
		char IpAddr[20];
		BYTE Field1, Field2, Field3, Field4;
		memset(IpAddr, 0x00, sizeof(IpAddr));
		SendMsgDlg->m_IPAddr.GetAddress(Field1, Field2, Field3, Field4);
		sprintf_s(IpAddr, "%d.%d.%d.%d", Field1, Field2, Field3, Field4);
		SendMsgDlg->Client_Addr.sin_addr.S_un.S_addr = inet_addr(IpAddr);
	}
	SendMsgDlg->m_MsgList.AddString("Connection established with server.....");

	memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
	while((Res = recv(SendMsgDlg->ClientSocket, RecvBuffer, sizeof(RecvBuffer), 0)) != SOCKET_ERROR)
	{
		memset(Buffer, 0x00, sizeof(Buffer));
		strcat_s(Buffer, "Server : ");
		strncat_s(Buffer, RecvBuffer, sizeof(RecvBuffer));
		SendMsgDlg->m_MsgList.AddString(Buffer);
	}

	send(SendMsgDlg->ClientSocket, "Disconnected", 100, 0);
	SendMsgDlg->m_MsgList.AddString(_T("Connection Disconnected"));
	::closesocket(SendMsgDlg->ClientSocket);
	::AfxEndThread(0);
	return TRUE;
}