#include "streams.h"
#include "wvstring.h"
#include <assert.h>
#include <errno.h>
#include <conio.h>

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
	
	size_t bytes = _read(pair->fd, ptr, BUFSIZE);
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

    int s[2];
    socketpair(AF_INET, SOCK_STREAM, 0, s);

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
    if (m_socket != INVALID_SOCKET)
    {
	result = shutdown(m_socket, SD_BOTH);
	
	// this assertion will fail if someone has already closed the
	// socket; eg. if you give the socket to a WvFDStream and then let
	// him close it.  But you shouldn't do that, because nobody is
	// supposed to close stdin/stdout/stderr!
	assert(result == 0);
	    
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
