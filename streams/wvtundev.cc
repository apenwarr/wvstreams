/* 
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc. 
 * 
 * WvTunDev provides a convenient way of using Linux tunnel devices.
 *
 * If you don't have the /dev/net/tun device, try doing: 
 *          mknod /dev/net/tun c 10 200
 */
#include <sys/ioctl.h>
#include <sys/socket.h> 
#include <linux/if_tun.h> 
#include <linux/if.h>
#include <string.h> 

#include "wvlog.h"
#include "wvtundev.h"

WvTunDev::WvTunDev(const WvIPNet &addr, int mtu)
    : WvFile("/dev/net/tun", O_RDWR)
{
    init(addr, mtu);
}

void WvTunDev::init(const WvIPNet &addr, int mtu)
{
    WvLog log("New tundev", WvLog::Debug2);
    if (rwfd < 0)
    {
        log("Could not open /dev/net/tun: %s\n", strerror(errno)); 
        seterr(errno);
        return;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_NO_PI | IFF_TUN;

    if (ioctl(rwfd, TUNSETIFF, (void *) &ifr) < 0 ||
        ioctl(rwfd, TUNSETNOCSUM, 1) < 0)
    {
        log("Could not initialize the interface: %s\n", strerror(errno));
        rwfd = -1;
        seterr(errno);
        return;
    }
    
    WvInterface iface(ifr.ifr_name);
    iface.setipaddr(addr);
    iface.setmtu(mtu);
    iface.up(true);
    ifcname = ifr.ifr_name;
    log.app = ifcname;

    log(WvLog::Debug2, "Now up (%s).\n", addr);
}    
