#pragma once
#include <winsock2.h>

struct socket_fd_pair
{
    SOCKET socket;
    int fd;
};

class SocketFromFDMaker
{
protected:
    HANDLE m_hThread;
    socket_fd_pair m_pair;
    SOCKET m_socket;
    bool m_wait;
public:
    SocketFromFDMaker(int fd, LPTHREAD_START_ROUTINE lpt, bool wait_for_termination = false);
    ~SocketFromFDMaker();
    SOCKET GetSocket() { return m_socket; }
};
DWORD WINAPI fd2socket_fwd(LPVOID lpThreadParameter);
DWORD WINAPI socket2fd_fwd(LPVOID lpThreadParameter);

