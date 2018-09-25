/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Portable standins for getuid() and friends.  See wvuid.h.
 */ 
#include "wvautoconf.h"
#include "wvuid.h"
#include <unistd.h>

#if WIN32


WvString wv_username_from_uid(wvuid_t uid)
{
    // FIXME not implemented
    return WvString::null;
}


wvuid_t wv_uid_from_username(WvString username)
{
    // FIXME not implemented
    return WVUID_INVALID;
}


wvuid_t wvgetuid()
{
    // FIXME not implemented
    return WVUID_INVALID;
}


#else // not WIN32


WvString wv_username_from_uid(wvuid_t uid)
{
    char buf[1024];
    struct passwd pwbuf, *userinfo;
    
    if (getpwuid_r(uid, &pwbuf, buf, sizeof(buf), &userinfo) == 0)
	return userinfo->pw_name;
    else
	return WvString::null;
}


wvuid_t wv_uid_from_username(WvString username)
{
    char buf[1024];
    struct passwd pwbuf, *userinfo;
    
    if (getpwnam_r(username, &pwbuf, buf, sizeof(buf), &userinfo) != 0)
	return userinfo->pw_uid;
    else
	return WVUID_INVALID;
}


wvuid_t wvgetuid()
{
    return getuid();
}


#endif
