
#ifndef __UNICONFCONNFACTORY_H
#define __UNICONFCONNFACTORY_H

#include "wvaddr.h"
#include "uniconfconn.h"

class UniConfConnFactory
{
public:
    virtual UniConfConn *open() = 0;
};

class UniConfTCPFactory : public UniConfConnFactory
{
public:
    UniConfTCPFactory(WvIPPortAddr _addr);
    virtual ~UniConfTCPFactory();
    virtual UniConfConn *open();
private:
    WvIPPortAddr addr;
};

class UniConfUSocketFactory : public UniConfConnFactory    
{
public:
    UniConfUSocketFactory(WvUnixAddr _addr);
    virtual ~UniConfUSocketFactory();
    virtual UniConfConn *open();
private:
    WvUnixAddr addr;
};

#endif
