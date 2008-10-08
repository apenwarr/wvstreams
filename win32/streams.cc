#include "streams.h"
#include "wvstring.h"
#include <assert.h>
#include <errno.h>
#include <conio.h>
#include <stdio.h>
#include <io.h>

#if _MSC_VER
// MS Visual C++ doesn't support varags preproc macros
# define DPRINTF
#else
#if 0
# define DPRINTF(x, args...) do { \
        printf(x, ## args); fflush(stdout); \
    } while (0)
#else
# define DPRINTF(x, args...) do { } while(0)
#endif
#endif


// this class changes the default libc stdout buffering to "line buffered"
// and stderr to "unbuffered", like they should be in any sane system.
// Apparently they start off as "fully buffered" in most Windows systems.
class FixLibcIoBuffers
{
public:
    FixLibcIoBuffers()
    {
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
    }
};
static FixLibcIoBuffers fixbufs;


// these versions of close/read/write try to work with both sockets and
// msvcrt file descriptors! (I hope we never get a socket with the same
// VALUE as a file descriptor!)
 

static void errcode(int err)
{
    if (err == EIO)
	err = EBADF; // sometimes we get EIO when Unix would be EBADF
    if (err == WSAENOTSOCK)
	err = EBADF; // if it's not a socket, it's also not a fd
    SetLastError(err);
    errno = err;
}


static bool is_socket(int fd)
{
    // if _get_osfhandle doesn't work, it must not be a fd, so assume it's
    // a socket.
    return (HANDLE)_get_osfhandle(fd) == INVALID_HANDLE_VALUE;
}


int close(int fd)
{
    int ret;
    if (is_socket(fd))
    {
	ret = closesocket(fd);
	errcode(GetLastError());
    }
    else
    {
	ret = _close(fd);
	errcode(errno);
    }
    return ret;
}


int read(int fd, void *buf, size_t count)
{
    int ret;
    if (is_socket(fd))
    {
	ret = recv(fd, (char *)buf, count, 0);
	errcode(GetLastError());
    }
    else
    {
	ret = _read(fd, buf, count);
	errcode(errno);
    }
    return ret;
}


int write(int fd, const void *buf, size_t count)
{
    int ret;
    if (is_socket(fd))
    {
	ret = send(fd, (char *)buf, count, 0);
	errcode(GetLastError());
    }
    else
    {
	ret = _write(fd, buf, count);
	errcode(errno);
    }
    return ret;
}


int socketpair(int family, int type, int protocol, int *sb)
{
    SOCKET insock, outsock, newsock;
    struct sockaddr_in sock_in;

    if (type != SOCK_STREAM)
	return -1;

    newsock = socket(AF_INET, type, 0);
    if (newsock == INVALID_SOCKET)
	return -1;

    sock_in.sin_family = AF_INET;
    sock_in.sin_port = 0;
    sock_in.sin_addr.s_addr = INADDR_ANY;
    if (bind(newsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
	return -1;

    int len = sizeof (sock_in);
    if (getsockname(newsock, (struct sockaddr *)&sock_in, &len) < 0)
	return -1;

    if (listen(newsock, 2) < 0)
	return -1;

    outsock = socket(AF_INET, type, 0);
    if (outsock == INVALID_SOCKET)
	return -1;

    sock_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(outsock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
	return -1;

    /* For stream sockets, accept the connection and close the listener */
    len = sizeof(sock_in);
    insock = accept(newsock, (struct sockaddr *)&sock_in, &len);
    if (insock == INVALID_SOCKET)
	return -1;

    if (closesocket(newsock) < 0)
	return -1;

    sb[0] = insock;
    sb[1] = outsock;
    return 0;
}


static void CALLBACK completion(DWORD error, DWORD nread, LPOVERLAPPED ov)
{
}


static size_t fake_read(int fd, void *buf, size_t len)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    INPUT_RECORD p;
    DPRINTF("fake_read(%d/%d,%p,%d) = ", fd, (int)h, buf, (int)len);
    
    DWORD ret = 0;
    OVERLAPPED ov;
    memset(&ov, 0, sizeof(ov));
    ov.Offset = SetFilePointer(h, 0, NULL, FILE_CURRENT);
    
    if (PeekNamedPipe(h, NULL, 0, NULL, &ret, NULL))
    {
	// cygwin sshd/telnetd uses named pipes for stdin.  We have to
	// support these separately.  Getting stuck in ReadFile on a named
	// pipe appears to freeze up gethostbyname() for some reason on win2k!
	DPRINTF("(stdin is a pipe)\n");
	while (PeekNamedPipe(h, NULL, 0, NULL, &ret, NULL) && !ret)
	{
	    DPRINTF(".");
	    Sleep(100);
	}
	ReadFile(h, buf, len, &ret, NULL);
    }
    else if (PeekConsoleInput(h, &p, 1, &ret))
    {
	// a typical stdin/out pair refers to a console.  Unfortunately,
	// console I/O is stupid: you can poll it to see if it's ready, but
	// if you have it in line mode, then it's not *really* ready.
	// ReadConsole/ReadFile will only return after the user hits enter.
	// Unfortunately, it seems the only way around this is to disable
	// line/echo mode and fake it ourselves.  Hopefully this isn't too
	// ugly...
	DPRINTF("(stdin is a console)\n");
	
	size_t used = 0;
	char *xbuf = (char *)buf;
	HANDLE hout = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE,
				 FILE_SHARE_READ | FILE_SHARE_WRITE,
				 NULL, OPEN_EXISTING, 0, 0);
	
	DWORD conmode = 0;
	GetConsoleMode(h, &conmode);
	SetConsoleMode(h, conmode & 
	       ~(ENABLE_LINE_INPUT | ENABLE_MOUSE_INPUT | ENABLE_ECHO_INPUT));
	
	while (PeekConsoleInput(h, &p, 1, &ret))
	{
	    DWORD tmp;
	    if (ret)
	    {
		ReadConsoleInput(h, &p, 1, &ret);
		assert(ret);
		if (p.EventType == KEY_EVENT && p.Event.KeyEvent.bKeyDown)
		{
		    int key = p.Event.KeyEvent.uChar.AsciiChar;
		    if (key == '\r') // end of line
		    {
			xbuf[used++] = '\n';
			WriteConsole(hout, "\r\n", 2, &tmp, NULL);
			ret = used;
			break;
		    }
		    else if (key == '\b' && used > 0)
		    {
			used--;
			WriteConsole(hout, "\b \b", 3, &tmp, NULL);
		    }
		    else if (key && used < len-1)
		    {
			xbuf[used++] = key;
			WriteConsole(hout, xbuf+used-1, 1, &tmp, NULL);
		    }
		}
	    }
	    else
	    {
		DPRINTF(".");
		WaitForSingleObjectEx(h, 1000, true);
	    }
	}
	
	CloseHandle(hout);
    }
    else
    {
	// stdin might be redirected from a file, in which case we can
	// probably safely (heh) assume it'll never block.  Still, try
	// ReadFileEx with a timeout first and see if that works.
	DPRINTF("(stdin is a file)\n");
	while (!ret)
	{
	    DPRINTF(".");
	    int rv = 0;
	    if (ReadFileEx(h, buf, 0, &ov, &completion))
	    {
		rv = SleepEx(1000, true);
		CancelIo(h);
		DPRINTF("(rv is %d)\n", rv);
		if (rv == WAIT_IO_COMPLETION)
		{
		    ReadFile(h, buf, len, &ret, NULL);
		    break;
		}
		else if (!rv) // timed out
		    Sleep(1); // ensure lock is released for nonzero time (1ms)
		else
		    return 0; // unknown problem: assume EOF
	    }
	    else
	    {
		// can't do ReadFileEx: probably stupid Win9x.
		ReadFile(h, buf, len, &ret, NULL);
		break;
	    }
	}
    }
    
    DPRINTF("[%d]\n", ret);
    return ret;
}


DWORD WINAPI fd2socket_fwd(LPVOID lpThreadParameter)
{
//    return 0;
    DWORD retval = 0;
    const int BUFSIZE = 512;
    socket_fd_pair *pair = (socket_fd_pair *)lpThreadParameter;
    
    // fprintf(stderr, "forwarding %d -> %d\n", 
    //         pair->fd, pair->socket); fflush(stderr);
    
    char buf[BUFSIZE];
    while (true)
    {
	char *ptr = buf;
	
	size_t bytes = fake_read(pair->fd, ptr, BUFSIZE);
	if (bytes <= 0) { retval = bytes; break; }
	while (bytes > 0)
	{
	    int written = send(pair->socket, ptr, bytes, 0);
	    if (written < 0) { retval = written; break; }

	    bytes -= written;
	    ptr += written;
	}
    }

    shutdown(pair->socket, SD_BOTH);
    closesocket(pair->socket);
    // fprintf(stderr, "TERMINATING-%d\n", pair->fd); fflush(stderr);
    return retval;
}


DWORD WINAPI socket2fd_fwd(LPVOID lpThreadParameter)
{
    DWORD retval = 0;
    const int BUFSIZE = 512;
    socket_fd_pair *pair = (socket_fd_pair *)lpThreadParameter;

    char buf[BUFSIZE];
    while (true)
    {
	char *ptr = buf;
	int bytes = recv(pair->socket, ptr, BUFSIZE, 0);
	if (bytes <= 0) { retval = bytes; break; }
	while (bytes > 0)
	{
	    int written = _write(pair->fd, ptr, bytes);
	    if (written < 0) { retval = written; break; }
	    bytes -= written;
	    ptr += written;
	}
    }
    shutdown(pair->socket, SD_BOTH);
    closesocket(pair->socket);
    // fprintf(stderr, "TERMINATING-%d\n", pair->fd); fflush(stderr);
    return retval;
}


SocketFromFDMaker::SocketFromFDMaker(int fd,
		     LPTHREAD_START_ROUTINE lpStartAddress, bool wait)
    : m_hThread(0), m_socket(INVALID_SOCKET), m_wait(wait)
{
    // might do this twice
    WSAData wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);

    int s[2], result;
    result = socketpair(AF_INET, SOCK_STREAM, 0, s);
    assert(result == 0);

    m_pair.fd = fd;
    m_pair.socket = s[0];
    m_socket = s[1];

    DWORD threadid;
    m_hThread = CreateThread(
	NULL,
	0,
	lpStartAddress,
	&m_pair,
	0,
	&threadid
    );
    assert(m_hThread);
}


SocketFromFDMaker::~SocketFromFDMaker()
{
    int result;
    // fprintf(stderr, "shutting down #%d\n", m_socket);
    if (m_socket != INVALID_SOCKET)
    {
	result = shutdown(m_socket, SD_BOTH);
	
	// this assertion will fail if someone has already closed the
	// socket; eg. if you give the socket to a WvFDStream and then let
	// him close it.  But you shouldn't do that, because nobody is
	// supposed to close stdin/stdout/stderr!
	if (result != 0)
	{
	    int e = GetLastError();
	    if (e == WSASYSNOTREADY || e == WSANOTINITIALISED)
	    {
		fprintf(stderr, "Abnormal termination.  Skipping cleanup.\n");
		_exit(42);
	    }
	    else
	    {
		fprintf(stderr,
			"ERROR! Socket #%d was already shut down! (%d)\n",
			m_socket, e);
		assert(result == 0);
	    }
	}
	    
	if (m_wait) // wait for socket->fd copier
	{
	    // wait for thread to terminate.  Since it's reading from a
	    // socket (m_wait==true), this will be safe, because we know
	    // it'll die politely when it should.
	    WaitForSingleObject(m_hThread, INFINITE);
	}
	else
	{
	    // FIXME: fd->socket copier will never die politely.  It gets
	    // stuck in _read(), which enters a critical section and
	    // then blocks until input is available.  Unfortunately, it's
	    // impossible to make input *not* available.
	    // 
	    // TerminateThread() is generally evil, and doesn't help here
	    // anyway: it just leaves that critical section locked forever, so
	    // no operation on that fd will *ever* finish.
	    // 
	    // Answer: just do nothing.  Someone will clean up the broken
	    // thread eventually, I guess.  (ExitProcess() claims to do
	    // this, but I hope it doesn't get stuck in a critical section...)
	}
        
	close(m_socket);
    }
    CloseHandle(m_hThread);
}
