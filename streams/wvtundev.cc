#include <sys/ioctl.h>
#include <sys/socket.h> 
#include <linux/if_tun.h> 
#include <linux/if.h>
#include <string.h> 

#include "wvlog.h"

#include "wvtundev.h"


WvTunDev::WvTunDev(WvConf &cfg)
    : WvFile("/dev/net/tun", O_RDWR)
{
    const WvIPNet addr(cfg.get("Global", "IPAddr", "192.168.42.42"), 32);
    init(addr);
}

WvTunDev::WvTunDev(const WvIPNet &addr)
    : WvFile("/dev/net/tun", O_RDWR)
{
    init(addr);
}

void WvTunDev::init(const WvIPNet &addr)
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
    iface.up(true);
    ifcname = ifr.ifr_name;
    log.app = ifcname;

    log(WvLog::Debug2, "Now up (%s).\n", addr);
}    
