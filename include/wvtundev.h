/* 
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc. 
 * 
 * WvTunDev provides a convenient way of using Linux tunnel devices.
 *
 * If you don't have the /dev/net/tun device, try doing: 
 *          mknod /dev/net/tun c 10 200
 */
#ifndef __WV_TUNDEV_H
#define __WV_TUNDEV_H

#include "wvfile.h"
#include "wvinterface.h"
#include "wvaddr.h"

class WvTunDev : public WvFile
{
    public:
        WvTunDev(const WvIPNet &addr, int mtu = 1400);
        WvString ifcname;
    private:
        void init(const WvIPNet &addr, int mtu);
};


#endif
