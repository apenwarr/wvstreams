/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Implementation of wvsocketpair(), a portable way to call socketpair().
 */
#include "wvsocketpair.h"
#include <fcntl.h>

#ifndef _WIN32
# include <sys/socket.h>
#else
# include <winsock2.h>
#endif

#ifdef _WIN32
int socketpair(int family, int type, int protocol, int *sb);
#endif

int wvsocketpair(int type, int socks[2])
{
    // NOTE: a fake socketpair() call is provided by wvstreams for win32.
    // The main advantage of wvsocketpair is it avoids the weird mess of
    // includes, ifdefs, and prototypes above.
    return socketpair(PF_UNIX, type, 0, socks);
}
