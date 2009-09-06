// MonitorIPScannerDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "MyTabCtrl.h"

// CMonitorIPScannerDlg dialog
class CMonitorIPScannerDlg : public CDialog
{
// Construction
public:
	CMonitorIPScannerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MONITORIPSCANNER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMyTabCtrl m_MainTabCtrl;
public:
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnBnClickedOk();
public:
	CString m_Main;
public:
	afx_msg void OnBnClickedCancel();
};
