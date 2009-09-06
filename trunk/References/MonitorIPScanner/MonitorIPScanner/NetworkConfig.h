#pragma once
#include "afxwin.h"


// CNetworkConfig dialog

class CNetworkConfig : public CDialog
{
	DECLARE_DYNAMIC(CNetworkConfig)

public:
	CNetworkConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNetworkConfig();

// Dialog Data
	enum { IDD = IDD_NETWORK_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_Adapter;

	IP_ADAPTER_INFO pAdapterInfo[10];

	void UpdateIPConfigScreen();
	void GetAllEntryValue();
public:
	int m_RecvBytes;
	int m_RecvDiscard;
	int m_RecvError;
	int m_RecvUKProto;
	int m_RecvUPack;
	int m_RecvNUPack;

	int m_SendBytes;
	int m_SendDiscard;
	int m_SendError;
	int m_SendUKProto;
	int m_SendUPack;
	int m_SendNUPack;
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
public:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};
