#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CPingConfig dialog
typedef struct {
  CString m_sHostToResolve;
  BOOL    m_bResolveAddressesToHostnames;
  BOOL    m_bPingTillStopped;
  int     m_nRequestsToSend;
  UCHAR   m_nTTL;
  UCHAR   m_nTOS;
  WORD    m_wDataRequestSize;
  DWORD   m_dwTimeout;
  BOOL    m_bUseRawSockets;
  BOOL    m_bDontFragment;
}ParameterInfo;

typedef struct 
{
//Member variables
	in_addr	 Address;              //The IP address of the replier
	unsigned long RTT;             //Round Trip time in Milliseconds
	unsigned long EchoReplyStatus; //here will be status of the last ping if successful
}ReplyInfo;

class CPingConfig : public CDialog
{
	DECLARE_DYNAMIC(CPingConfig)

public:
	CPingConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPingConfig();

// Dialog Data
	enum { IDD = IDD_PINGCONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CIPAddressCtrl IPAddress;
	CListBox m_Output;
	CString m_DomainName;
	int m_Timeout;
	int m_SendBuf;
	int m_TTL;
	int m_PacktoSend;
public:
	afx_msg void OnBnClickedOk();
	BOOL GetAllParameters();
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
public:
	afx_msg void OnBnClickedRadio1();
};
