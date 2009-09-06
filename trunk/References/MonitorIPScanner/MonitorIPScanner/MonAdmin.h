#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CMonAdmin dialog

class CMonAdmin : public CDialog
{
	DECLARE_DYNAMIC(CMonAdmin)

public:
	CMonAdmin(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMonAdmin();

// Dialog Data
	enum { IDD = IDD_MONADMINCONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
public:
	VOID UpdateTcpTableList();
public:
	CListBox m_TcpTable;
	CListBox m_RetrieveInfo;
public:
	afx_msg void OnBnClickedOk3();
public:
	CIPAddressCtrl m_ipaddr;
public:
	afx_msg void OnBnClickedOk2();
public:
	CButton m_meminfo;
	CButton m_arp;
	CButton m_protocol;
	CButton m_routing;
	CButton m_ipok;
	CButton m_Retrieve;
public:
	afx_msg void OnLbnSelchangeList1();
public:
	CProgressCtrl m_progress;

	void CreateServerSocket();
	void GetIpAddressForConnection();
	void CreateClientSocket();
	void DisplayReceivedData();
	void DisplayProtocolStats();
	void DisplayRoutingDetails();
	void DisplayARPDetails();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
