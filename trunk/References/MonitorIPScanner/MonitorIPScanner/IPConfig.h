#pragma once
#include "afxwin.h"


// CIPConfig dialog

class CIPConfig : public CDialog
{
	DECLARE_DYNAMIC(CIPConfig)

public:
	CIPConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIPConfig();

// Dialog Data
	enum { IDD = IDD_IPCONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	IP_ADAPTER_INFO pAdapterInfo[10];

	virtual	BOOL OnInitDialog();
	void UpdateIPConfigScreen();
	void UpdateAdapterInfo();
	void UpdateDNSInfo();
public:
	CComboBox m_AdapterName;
	CString m_MacAddress;
	CString m_IPAddress;
	CString m_SubnetMask;
	CString m_DefaultGateway;
	CString m_PreferedDNS;
	CString m_AlternateDNS;
	CString m_DhcpIP;
	CString m_DhcpSubnet;
	BOOL m_dhcp;
	BOOL m_dns;
public:
	afx_msg void OnCbnSelchangeAdaptername();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
