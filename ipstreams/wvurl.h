/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvURL is a simple URL-parsing class with built-in (though still somewhat
 * inconvenient) DNS resolution.
 */ 
#ifndef __WVURL_H
#define __WVURL_H

#include "wvstring.h"
#include "wvresolver.h"

class WvIPPortAddr;

class WvUrl
{
public:
    WvUrl(WvStringParm url);
    WvUrl(const WvUrl &url);
    ~WvUrl();
    
    bool isok() const
        { return port != 0 && (resolving || addr != NULL); }
    WvStringParm errstr() const
        { return err; }
    bool resolve(); // dns-resolve the hostname (returns true if done)

    operator WvString () const;
    
    // not actually defined - this just prevents accidental copying
    const WvUrl &operator= (const WvUrl &);
    
    // ONLY valid if resolve() returns true!
    const WvIPPortAddr &getaddr() const
        { return *addr; }
    WvStringParm getfile() const
        { return file; }
    WvStringParm gethost() const
        { return hostname; }
    int getport() const
        { return port; }
    
protected:
    WvString hostname;
    int port;
    bool resolving;
    WvResolver dns;
    WvIPPortAddr *addr;
    WvString file, err;
};


// backwards compatibility
typedef WvUrl WvURL;

#endif // __WVURL_H
