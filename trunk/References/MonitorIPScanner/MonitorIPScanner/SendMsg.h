#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CSendMsg dialog
#define NO_OF_SOCKET	10
typedef struct
{
	char ClientName[20];
	SOCKET ServerSocket;
	sockaddr_in Serv_Sockaddr;
}Server_Socket;

class CSendMsg : public CDialog
{
	DECLARE_DYNAMIC(CSendMsg)

public:
	CSendMsg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSendMsg();

// Dialog Data
	enum { IDD = IDD_SENDMSG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_PortNo;
	CIPAddressCtrl m_IPAddr;
	CString m_Msg;
	CListBox m_UserList;
	CListBox m_MsgList;
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
public:
	SOCKET	Sock, ClientSocket;
	int addlen;
	sockaddr_in Server_Addr, Client_Addr;

	void StartClientApplication();
	void StartServerApplication();

};
