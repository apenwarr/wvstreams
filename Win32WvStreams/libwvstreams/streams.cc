// streams.cpp : Defines the entry point for the DLL application.
//
#include "streams.h"
#include "wvstring.h"
#include <assert.h>
#include <errno.h>

// these versions of close/read/write try to work with both sockets and msvcrt 
// file descriptors! (I hope we never get a socket with the same VALUE
// as a file descriptor!)
int close(int fd)
{
    int retval = _close(fd), err = errno;
    if (retval < 0 && err == EBADF)
    { 
	// fd is not a normal fd; perhaps it's a socket?
	retval = closesocket(fd);
	if (retval < 0 && GetLastError() == WSAENOTSOCK)
	    SetLastError(err); // save the original errno
    } 
    return retval;
}

int read(int fd, void *buf, size_t count)
{
    int retval = 0;
    if (((retval = recv(fd, (char *) buf, count, 0)) < 0) && (GetLastError() == WSAENOTSOCK))
    {
	// fd is not a socket; perhaps it's a file descriptor?
	retval = _read(fd, buf, count);
	if (retval < 0) 
	    SetLastError(errno); // save the "real" errno
    }
    return retval;
}

int write(int fd, const void *buf, size_t count)
{
    int retval = 0;
    if (((retval = send(fd, (char *) buf, count, 0)) < 0) && (GetLastError() == WSAENOTSOCK))
    {
	// fd is not a socket; perhaps it's a file descriptor?
	retval = _write(fd, buf, count);
	if (retval < 0) 
	    SetLastError(errno); // save the "real" errno
    }
    return retval;
}

int socketpair(int family, int type, int protocol, int *sb)
{
    SOCKET insock, outsock, newsock;
    struct sockaddr_in sock_in;

    if (type != SOCK_STREAM)
	return -1;

    newsock = socket (AF_INET, type, 0);
    if (newsock == INVALID_SOCKET)
	return -1;

    sock_in.sin_family = AF_INET;
    sock_in.sin_port = 0;
    sock_in.sin_addr.s_addr = INADDR_ANY;
    if (bind (newsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
	return -1;

    int len = sizeof (sock_in);
    if (getsockname (newsock, (struct sockaddr *) &sock_in, &len) < 0)
	return -1;

    if (listen (newsock, 2) < 0)
	return -1;

    outsock = socket (AF_INET, type, 0);
    if (outsock == INVALID_SOCKET)
	return -1;

    sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    if (connect(outsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
	return -1;

    /* For stream sockets, accept the connection and close the listener */
    len = sizeof (sock_in);
    insock = accept (newsock, (struct sockaddr *) &sock_in, &len);
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
    DWORD retval = 0;
    const int BUFSIZE = 512;
    socket_fd_pair *pair = (socket_fd_pair *) lpThreadParameter;

    char buf[BUFSIZE];
    while (true)
    {
	char *ptr = buf;
	int bytes = _read(pair->fd, ptr, BUFSIZE);
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
    return retval;
}

DWORD WINAPI socket2fd_fwd(LPVOID lpThreadParameter)
{
    DWORD retval = 0;
    const int BUFSIZE = 512;
    socket_fd_pair *pair = (socket_fd_pair *) lpThreadParameter;

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
    return retval;
}

SocketFromFDMaker::SocketFromFDMaker(int fd, LPTHREAD_START_ROUTINE lpStartAddress, bool wait)
    : m_hThread(0), m_socket(INVALID_SOCKET), m_wait(wait)
{
    // might do this twice
    WSAData wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);

    int s[2];
    socketpair(AF_INET, SOCK_STREAM, 0, s);

    m_pair.fd = _dup(fd);
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
	// ugly ugly ugly workaround for bug 5449
	for (int i = 0; i != 10; ++i)
	    Sleep(50);
//	assert(result == 0);
	// wait for thread to terminate
	// if (m_wait)
	//	WaitForSingleObject(m_hThread, /* INFINITE */ 2000);
        
	close(m_socket);
    }
    CloseHandle(m_hThread);
}
