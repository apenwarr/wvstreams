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
    return new UniConfConn(new WvTCPConn(addr));
}
