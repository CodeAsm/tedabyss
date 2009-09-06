#pragma once
#include "afxwin.h"


// CProtocolStat dialog

class CProtocolStat : public CDialog
{
	DECLARE_DYNAMIC(CProtocolStat)

public:
	CProtocolStat(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProtocolStat();

// Dialog Data
	enum { IDD = IDD_IPSTATISTICS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	void GetIPStatistics();
	void GetRoutingTable();
	void GetARPTable();

public:
	CListBox m_ipstats;
};
