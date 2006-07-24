
#include "wvwinstreamclone.h"

ATOM WvWinStreamClone::s_aClass = 0;
WvWinStreamClone::WndVector WvWinStreamClone::s_wndpool;
WvWinStreamClone::WndStreamMap WvWinStreamClone::s_wndmap;

HWND WvWinStreamClone::alloc_wnd()
{
    if (s_wndpool.empty())
    {
	HWND hWnd = CreateWindow(
	    "WvWinStreamClone",
	    "WvWinStreamWindowName",
	    WS_POPUP | WS_DISABLED,
	    CW_USEDEFAULT, // initial x position
	    CW_USEDEFAULT, // initial y position
	    CW_USEDEFAULT, // initial x extent
	    CW_USEDEFAULT, // initial y extent
	    HWND_MESSAGE,
	    NULL,
	    NULL,
	    NULL
	);
	assert(hWnd);
	s_wndpool.push_back(hWnd);
    }

    HWND hWnd = s_wndpool.back();
    s_wndpool.pop_back();

    // associate window with this instance
    s_wndmap[hWnd] = this;

    return hWnd;
}

void WvWinStreamClone::free_wnd(HWND w)
{
    s_wndpool.push_back(w);
}

DWORD WvWinStreamClone::Initialize()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WvWinStreamClone::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = NULL; 
    wc.hCursor = NULL;
    wc.hbrBackground = NULL; 
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WvWinStreamClone";

    s_aClass = RegisterClass(&wc);
    if (!s_aClass)
    {
	DWORD error = GetLastError();
	return error;
    }
    return 0;
}

WvWinStreamClone::WvWinStreamClone(WvStream * _cloned) :
    WvStreamClone(_cloned), m_pending_callback(false), m_select_in_progress(false),
    m_msec_timeout(500)
{
    memset(&m_si, 0, sizeof(m_si));
    m_hWnd = alloc_wnd();
    pre_poll();
}

WvWinStreamClone::~WvWinStreamClone()
{
    free_wnd(m_hWnd);
}

// for each socket in "set", add the "event" to the its associated mask in sockmap 
void WvWinStreamClone::select_set(SocketEventsMap &sockmap, fd_set *set, long event )
{
    for (unsigned i=0; i<set->fd_count; i++)
    {
	SOCKET &socket = set->fd_array[i];
	sockmap[socket] |= event;
    }

    FD_ZERO(set);
}

void WvWinStreamClone::pre_poll()
{
    bool sure = this->_build_selectinfo(m_si, m_msec_timeout, 
					false, false, false, true);
    
    if (sure)
    {
	m_pending_callback = true;
	m_si.msec_timeout = 0;
    }

    // We must only call WSAAsyncSelect once per socket, so we need
    // to collect all the events from each set first, grouping them by
    // socket rather than by event
    SocketEventsMap sockmap;
    this->select_set(sockmap, &m_si.read, FD_READ);
    this->select_set(sockmap, &m_si.write, FD_WRITE);
    this->select_set(sockmap, &m_si.except, FD_OOB);

    // Call WSAAsyncSelect, asking the OS to send us a message when the socket
    // becomes readable | writable | exceptional
    for (SocketEventsMap::iterator i = sockmap.begin(); i!=sockmap.end(); ++i)
    {
	SOCKET socket = (*i).first;
	long events = (*i).second;
	
    	int result = ::WSAAsyncSelect(socket, m_hWnd, WM_SELECT, 
	                              events | FD_CONNECT | FD_CLOSE | FD_ACCEPT);
	assert(result == 0);
    }

    // alarm
    ::KillTimer(m_hWnd, TIMER_ID);
    if (m_si.msec_timeout >= 0)
    {
	::SetTimer(m_hWnd, TIMER_ID, m_si.msec_timeout, NULL);
    }

    m_select_in_progress = true;
}

void WvWinStreamClone::post_poll()
{
    bool sure = this->_process_selectinfo(m_si, true);
    
    if (sure || m_pending_callback)
    {
        m_pending_callback = false;
        callback();
        if (globalstream) globalstream->callback();
    }
}

void WvWinStreamClone::select_callback(SOCKET socket, int events, int error)
{
    if (events | FD_READ) FD_SET(socket, &m_si.read);
    if (events | FD_WRITE) FD_SET(socket, &m_si.write);
    if (events | FD_OOB) FD_SET(socket, &m_si.except);
    m_pending_callback = true;

    if (m_select_in_progress)
    {
	::PostMessage(m_hWnd, WM_DONESELECT, 0, 0);
	m_select_in_progress = false;
    }
}

LRESULT CALLBACK WvWinStreamClone::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DONESELECT:
    {	
	WvWinStreamClone *_this = s_wndmap[hwnd];
	assert(_this);
	_this->post_poll();
	_this->pre_poll();
	
	break;
    }

    case WM_SELECT:
    {
	WvWinStreamClone *_this = s_wndmap[hwnd];	
	assert(_this);
	SOCKET socket = wParam;
	int events = WSAGETSELECTEVENT(lParam);
	int error = WSAGETSELECTERROR(lParam);
	_this->select_callback( socket, events, error );
	
        break;
    }

    case WM_TIMER:
    {
	::PostMessage(hwnd, WM_DONESELECT, 0, 0);

	break;
    }

    default:
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}



void WvWinStreamClone::setclone(IWvStream *newclone)
{
    WvStreamClone::setclone(newclone);
    
    if (newclone != NULL)
        my_type = WvString("WvWinStreamClone:%s", newclone->wstype());
    else
        my_type = "WvWinStreamClone:(none)";
}
