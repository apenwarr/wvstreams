// test3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "test3.h"
#include "test3Dlg.h"
#include ".\test3dlg.h"

#include "wvtcp.h"
#include "wvhttp.h"
#include "wvwinstreamclone.h"
#include "MySocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Ctest3Dlg dialog



Ctest3Dlg::Ctest3Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(Ctest3Dlg::IDD, pParent),
	m_streamlist(0), m_winstream(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ctest3Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT21, m_response);
    DDX_Control(pDX, IDC_EDIT1, m_url);
    DDX_Control(pDX, IDC_RICHEDIT22, m_received);
    DDX_Control(pDX, IDC_EDIT2, m_listenport);
    DDX_Control(pDX, IDC_EDIT4, m_conncount);
    DDX_Control(pDX, IDC_EDIT3, m_csaddr);
}

BEGIN_MESSAGE_MAP(Ctest3Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
END_MESSAGE_MAP()


// Ctest3Dlg message handlers

BOOL Ctest3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_url.SetWindowText("http://nit.ca/");
	m_conncount.SetWindowText("0");
	m_listenport.SetWindowText("6969");
	m_csaddr.SetWindowText("127.0.0.1:7070");

	m_winstream = new WvWinStreamClone( m_streamlist = new WvIStreamList() );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Ctest3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Ctest3Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Ctest3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void mycallback(WvStream &s, void *userdata)
{
    if (!s.isok())
    {
	WvString msg("(%s) %s", s.geterr(), s.errstr());
	MessageBox(NULL, msg, "WvTCPConn", MB_OK);
	return;
    }

    s.print("Hello world!\n");
    WvString line = s.blocking_getline(-1);

    s.close();
}

void Ctest3Dlg::OnBnClickedButton1()
{
    CString addstr;
    m_csaddr.GetWindowText(addstr);

    WvIPPortAddr addr(addstr);
    WvTCPConn *c = new WvTCPConn(addr);
    MySocket *s = new MySocket(c);
    m_streamlist->append(s, true);
}

void Ctest3Dlg::httpcallback(WvStream &s, void *userdata)
{
    if (!s.isok())
    {
	WvString msg("(%s) %s", s.geterr(), s.errstr());
	MessageBox(msg, "WvHTTPStream", MB_OK);
	return;
    }

    WvString line = s.blocking_getline(-1);
    if (line) 
    {
	CString l(line);
	l += "\n";
	m_response.SetSel(-1, -1);
	m_response.ReplaceSel(l);
    }

}

void Ctest3Dlg::OnBnClickedButton2()
{
    CString urltext;
    m_url.GetWindowText(urltext);

    WvUrl url(urltext.GetBuffer());
    WvHTTPStream *h = new WvHTTPStream(url);
    h->setcallback(WvStreamCallback(this, Ctest3Dlg::httpcallback), 0);
    m_streamlist->append(h, true);
    m_response.SetWindowText("");
 
}

void Ctest3Dlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    OnOK();

    delete m_winstream;
    m_winstream = 0;

}

void Ctest3Dlg::OnEnChangeEdit1()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

void Ctest3Dlg::tcpincallback(WvStream &s, void *userdata)
{
    if (!s.isok())
    {
	WvString msg("(%s) %s", s.geterr(), s.errstr());
	MessageBox(msg, "WvTCPConn", MB_OK);
	return;
    }

    WvString line = s.blocking_getline(-1);
    if (line) 
    {
	CString l(line);
	l += "\n";
	m_received.SetSel(-1, -1);
	m_received.ReplaceSel(l);
    }    
}

void Ctest3Dlg::OnBnClickedButton3()
{
    CString portstring;
    m_listenport.GetWindowText(portstring);
    int nport = atoi(portstring.GetBuffer());
    if (nport)
    {
	WvIPPortAddr addr(nport);
	WvTCPListener *listen = new WvTCPListener(addr);
	m_streamlist->append(listen, true);

	WvStreamCallback cb(this, Ctest3Dlg::tcpincallback);
	listen->auto_accept(m_streamlist, cb);
    }
    else
    {
	MessageBox("Invalid port number", "WvTCPListener");
    }
}
