/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session which is authenticated through PAM.
 */
#ifndef __UNICONFPAMCONN_H
#define __UNICONFPAMCONN_H

#include "uniconfdaemonconn.h"
#include "unisecuregen.h"
#include "uniconfroot.h"

class WvPamStream;

class UniConfPamConn : public WvStreamClone
{
public:
    UniConfPamConn(WvStream *s, const UniConf &root, UniPermGen *perms);

protected:
    WvPamStream *pam;
    UniConfRoot newroot;
};

#endif // __UNICONFPAMCONN_H
