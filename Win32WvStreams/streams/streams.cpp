// streams.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "streams.h"
#include <assert.h>
#include <wvstring.h>

// this little trick allows us to define our own close/read/write  
// (in streams.cpp) that optionally call _close/_read/_write (defined in <io.h>)
#define __STDC__ 1 // prevents io.h from dllimporting close/read/write
#include <io.h>

// these versions of close/read/write try to work with bot sockets and msvcrt 
// file descriptors! (I hope we never get a socket with the same VALUE
// as a file descriptor!)
int close(int fd)
{
    int retval = 0;
    if (((retval = closesocket(fd)) < 0) && (GetLastError() == WSAENOTSOCK))
    { 
	// fd is not a socket, perhaps its a file descriptor?
	retval = _close(fd);
	if (retval < 0) 
	    SetLastError(errno); // save the "real" errno
    } 
    return retval;
}

int read(int fd, void *buf, size_t count)
{
    int retval = 0;
    if (((retval = recv(fd, (char *) buf, count, 0)) < 0) && (GetLastError() == WSAENOTSOCK))
    {
	// fd is not a socket, perhaps its a file descriptor?
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
	// fd is not a socket, perhaps its a file descriptor?
	retval = _write(fd, buf, count);
	if (retval < 0) 
	    SetLastError(errno); // save the "real" errno
    }
    return retval;
}

int socketpair (int family, int type, int protocol, int *sb)
{
    int res = -1;
    SOCKET insock, outsock, newsock;
    struct sockaddr_in sock_in;

    assert(type == SOCK_STREAM);
    newsock = socket (AF_INET, type, 0);
    assert(newsock != INVALID_SOCKET);
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = 0;
    sock_in.sin_addr.s_addr = INADDR_ANY;
    assert(bind (newsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) >= 0);
    int len = sizeof (sock_in);
    assert(getsockname (newsock, (struct sockaddr *) &sock_in, &len) >= 0);
    if (type == SOCK_STREAM)
	listen (newsock, 2);
    outsock = socket (AF_INET, type, 0);
    assert(outsock != INVALID_SOCKET);
    sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    assert(connect(outsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) >= 0);

    /* For stream sockets, accept the connection and close the listener */
    len = sizeof (sock_in);
    insock = accept (newsock, (struct sockaddr *) &sock_in, &len);
    assert(insock != INVALID_SOCKET);

    closesocket (newsock);

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
    int result = WSAStartup(MAKEWORD(2,0), &wsaData);

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
//	assert(result == 0);
	// wait for thread to terminate
	// if (m_wait)
	//	WaitForSingleObject(m_hThread, /* INFINITE */ 2000);
        
	close(m_socket);
    }
    CloseHandle(m_hThread);
}
