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

/**
 * Get around the lack of RTTI by keeping a mapping of UniConfGen's to
 * UniSecureGen's.  That way we can convert a UniConfGen to the derived class
 * by just looking it up.  If it's not there, it can't be downcasted.
 */
unsigned int WvHash(const UniConfGen *);

struct SecureGen
{
    UniConfGen *key;
    UniSecureGen *data;
};
DeclareWvDict(SecureGen, UniConfGen *, key);


class UniConfPamConn : public UniConfDaemonConn
{
public:
    UniConfPamConn(WvStream *s, const UniConf &root);

    static SecureGenDict securegens;

protected:
    virtual void addcallback();
    virtual void delcallback();

    virtual void do_get(const UniConfKey &key);
    virtual void do_set(const UniConfKey &key, WvStringParm value);
    virtual void do_remove(const UniConfKey &key);
    virtual void do_subtree(const UniConfKey &key);
    virtual void do_haschildren(const UniConfKey &key);

    void deltacallback(const UniConf &cfg, const UniConfKey &key);

    void updatecred(const UniConf &key);
};

#endif // __UNICONFPAMCONN_H
