// winstreams.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "winsock2.h"
#include "assert.h"

class RunWinSockInitialize
{
    WSAData wsaData;

public:
    RunWinSockInitialize()
    {
	int result = WSAStartup(MAKEWORD(2,0), &wsaData);
	assert(result == 0);
    }
};

RunWinSockInitialize __runinitialize;
