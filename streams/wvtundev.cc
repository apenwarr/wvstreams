#include <sys/ioctl.h>
#include <sys/socket.h> 
#include <linux/if_tun.h> 
#include <linux/if.h>
#include <string.h> 

#include "wvtundev.h"
#include "wvlog.h"


WvTunDev::WvTunDev() : WvFile("/dev/net/tun", O_RDWR)
{
    WvLog log("New tunnel", WvLog::Critical);
    if (rwfd < 0)
    {
        log("Could not open /dev/net/tun: %s\n", strerror(errno)); 
        seterr(errno);
        return;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_NO_PI;
    ifr.ifr_flags |= IFF_TUN;

    if (ioctl(rwfd, TUNSETIFF, (void *) &ifr) < 0)
    {
        log("Could not initialize the interface: %s\n", strerror(errno));
        rwfd = -1;
        seterr(errno);
        return;
    }

    ifc_name = ifr.ifr_name;
    log.app = ifc_name;

    log(WvLog::Debug2, "Initialized successfully.\n");
}    
