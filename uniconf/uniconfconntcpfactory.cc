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
    // the connection is not established until select completes
    conn->select(0, true, true, false);
    if (conn->isok())
        return new UniConfConn(conn);
    else
        return NULL;
}
