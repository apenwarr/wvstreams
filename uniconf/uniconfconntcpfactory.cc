#include "uniconfconnfactory.h"
#include "wvtcp.h"

UniConfTCPFactory::UniConfTCPFactory(WvIPPortAddr _addr) : addr(_addr)
{
}

UniConfTCPFactory::~UniConfTCPFactory()
{
}

UniConfConn *UniConfTCPFactory::open()
{
    WvTCPConn *conn = new WvTCPConn(addr);
    if (conn->isok())
        return new UniConfConn(conn);
    else
        return NULL;
}
