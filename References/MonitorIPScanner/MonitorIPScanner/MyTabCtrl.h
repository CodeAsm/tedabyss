#pragma once

#define NO_OF_TABS	6
// CMyTabCtrl

class CMyTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CMyTabCtrl)

public:
	CMyTabCtrl();
	virtual ~CMyTabCtrl();

public:
	CDialog *m_TabPages[NO_OF_TABS];

public:
	int m_tabCurrent;

	void InitialiseTabCtrl();
	void SetTabPage();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:
	DECLARE_MESSAGE_MAP()
};


