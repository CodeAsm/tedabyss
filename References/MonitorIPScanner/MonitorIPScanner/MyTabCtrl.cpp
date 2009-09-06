// MyTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MonitorIPScanner.h"
#include "MyTabCtrl.h"

#include "HostConfig.h"
#include "IPConfig.h"
#include "MonAdmin.h"
#include "NetworkConfig.h"
#include "PingConfig.h"
#include "SendMsg.h"
// CMyTabCtrl

IMPLEMENT_DYNAMIC(CMyTabCtrl, CTabCtrl)

CMyTabCtrl::CMyTabCtrl()
{
	m_TabPages[0] = new CHostConfig;
	m_TabPages[1] = new CIPConfig;
	m_TabPages[2] = new CMonAdmin;
	m_TabPages[3] = new CNetworkConfig;
	m_TabPages[4] = new CPingConfig;
	m_TabPages[5] = new CSendMsg;
}

CMyTabCtrl::~CMyTabCtrl()
{
		for(int nCount=0; nCount < NO_OF_TABS; nCount++){
		delete m_TabPages[nCount];
	}

}


BEGIN_MESSAGE_MAP(CMyTabCtrl, CTabCtrl)
		ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CMyTabCtrl message handlers

void CMyTabCtrl::InitialiseTabCtrl()
{
	m_tabCurrent=0;

	m_TabPages[0]->Create(IDD_HOSTCONFIG_DLG, this);
	m_TabPages[1]->Create(IDD_IPCONFIG_DLG, this);
	m_TabPages[2]->Create(IDD_MONADMINCONFIG_DLG, this);
	m_TabPages[3]->Create(IDD_NETWORK_DLG, this);
	m_TabPages[4]->Create(IDD_PINGCONFIG_DLG, this);
	m_TabPages[5]->Create(IDD_SENDMSG_DLG, this);

	m_TabPages[0]->ShowWindow(SW_SHOW);
	m_TabPages[1]->ShowWindow(SW_HIDE);
	m_TabPages[2]->ShowWindow(SW_HIDE);
	m_TabPages[3]->ShowWindow(SW_HIDE);
	m_TabPages[4]->ShowWindow(SW_HIDE);
	m_TabPages[5]->ShowWindow(SW_HIDE);

	SetTabPage();
}

void CMyTabCtrl::SetTabPage()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-1;

	m_TabPages[0]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	for(int nCount=1; nCount < NO_OF_TABS; nCount++){
		m_TabPages[nCount]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
}

void CMyTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect tabRect, itemRect;

	CTabCtrl::OnLButtonDown(nFlags, point);

	if(m_tabCurrent != GetCurFocus()){
		m_TabPages[m_tabCurrent]->ShowWindow(SW_HIDE);
		m_tabCurrent=GetCurFocus();
		m_TabPages[m_tabCurrent]->ShowWindow(SW_SHOW);
		m_TabPages[m_tabCurrent]->SetFocus();

	}
}