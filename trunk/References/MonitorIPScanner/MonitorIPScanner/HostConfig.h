#pragma once


// CHostConfig dialog

class CHostConfig : public CDialog
{
	DECLARE_DYNAMIC(CHostConfig)

public:
	CHostConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHostConfig();

// Dialog Data
	enum { IDD = IDD_HOSTCONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL GetSystemInfo(CString& SysInfo);
	VOID UpdateHostName();
	INT GetSystemMemoryUsage();
public:
	afx_msg void OnBnClickedSysteminfo();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
public:
	CString m_HostName;
public:
	ULONGLONG m_Pagefile;
	ULONGLONG m_PeakPageFile;
	ULONGLONG m_VirtualPage;
	ULONGLONG m_PeakVirtualSize;
	ULONGLONG m_PageFaultCount;
	ULONGLONG m_WorkSetSize;
	ULONGLONG m_PeakWorkSetSize;
};
