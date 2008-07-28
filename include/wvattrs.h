#ifndef __WVATTRS_H
#define __WVATTRS_H

#include "wvstring.h"

class WvAttrs
{
    char *attrlist;
    unsigned int attrlen;

    char *_getattr(WvStringParm name) const;
public:
    WvAttrs();
    WvAttrs(const WvAttrs &copy);
    virtual ~WvAttrs();

    void setattr(WvStringParm name, WvStringParm value);
    WvString getattr(WvStringParm name) const;
};

#endif
