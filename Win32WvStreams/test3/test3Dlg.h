// test3Dlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "wvistreamlist.h"
#include "wvwinstreamclone.h"

// Ctest3Dlg dialog
class Ctest3Dlg : public CDialog
{
// Construction
public:
	Ctest3Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TEST3_DIALOG };

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

	CAsyncSocket m_socket;
public:
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    void httpcallback(WvStream &s, void *userdata);
    void tcpincallback(WvStream &s, void *userdata);

    CRichEditCtrl m_response;
    CEdit m_url;
private:
    WvIStreamList *m_streamlist;
    WvWinStreamClone *m_winstream;
public:
    afx_msg void OnBnClickedOk();
    CRichEditCtrl m_received;
    CEdit m_listenport;
    CEdit m_conncount;
    afx_msg void OnEnChangeEdit1();
    afx_msg void OnBnClickedButton3();
    CEdit m_csaddr;
};
