#ifndef __WVATTRS_H
#define __WVATTRS_H

#include "wvstring.h"

class WvAttrs
{
    char *attrlist;
    unsigned int attrlen;

    char *_get(WvStringParm name) const;
public:
    WvAttrs();
    WvAttrs(const WvAttrs &copy);
    virtual ~WvAttrs();

    void set(WvStringParm name, WvStringParm value);
    WvString get(WvStringParm name) const;
};

#endif
