/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A WvInterface stores information about a particular network interface.
 */

#include "wvinterface.h"
#if 1
// FIXME: this file doesn't compile on anything other than Linux

#include "wvsubproc.h"
#include "wvfile.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <unistd.h>
#include <errno.h>
#include <linux/sockios.h>

#define _LINUX_IF_H /* Hack to prevent loading linux/if.h */
#include <linux/wireless.h>

#define min(x,y) ({ \
    const typeof(x) _x = (x); \
    const typeof(y) _y = (y); \
    (void) (&_x == &_y); \
    _x < _y ? _x : _y; })

WvInterfaceDictBase WvInterfaceDict::slist(15);
int WvInterfaceDict::links = 0;


WvInterface::WvInterface(WvStringParm _name) :
    err("Net Interface", WvLog::Error), name(_name)
{
    my_hwaddr = my_ipaddr = NULL;
    valid = true;
}


WvInterface::~WvInterface()
{
    rescan();
}


int WvInterface::req(int ioctl_num, struct ifreq *ifr)
{
    int sock, retval;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr->ifr_name, name, IFNAMSIZ-1);
    ifr->ifr_name[IFNAMSIZ-1] = 0;
    
    retval = ioctl(sock, ioctl_num, ifr);
    if (retval == -1)
        retval = errno;
    close(sock);
    return retval;
}

// For Wireless Interfaces...
int WvInterface::req(int ioctl_num, struct iwreq *ifr)
{
    int sock, retval;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr->ifr_name, name, IFNAMSIZ-1);
    ifr->ifr_name[IFNAMSIZ-1] = 0;
    
    retval = ioctl(sock, ioctl_num, ifr);
    if (retval)
	retval = errno;
    close(sock);
    return retval;
}
                                                

// forget all stored information about the address(es) of this interface
void WvInterface::rescan()
{
    if (my_hwaddr)
    {
        delete my_hwaddr;
        my_hwaddr = NULL;
    }
    
    if (my_ipaddr)
    {
        delete my_ipaddr;
        my_ipaddr = NULL;
    }
}


// get the hardware address of this interface
const WvAddr &WvInterface::hwaddr()
{
    struct ifreq ifr;
    
    if (!my_hwaddr)
    {
        if (req(SIOCGIFHWADDR, &ifr))
            my_hwaddr = new WvStringAddr("Unknown", WvEncap::Unknown);
        else
            my_hwaddr = WvAddr::gen(&ifr.ifr_hwaddr);
    }
    return *my_hwaddr;
}


// get the local IP net of this interface
const WvIPNet &WvInterface::ipaddr()
{
    struct ifreq ifr, ifr2;
    
    if (!my_ipaddr)
    {
        ifr.ifr_addr.sa_family = AF_INET;
        ifr2.ifr_netmask.sa_family = AF_INET;
        if (req(SIOCGIFADDR, &ifr) || req(SIOCGIFNETMASK, &ifr2))
            my_ipaddr = new WvIPNet();
        else
            my_ipaddr = new WvIPNet(&ifr.ifr_addr, &ifr2.ifr_netmask);
    }
    
    return *my_ipaddr;
}


// get the point-to-point IP address of this interface
const WvIPAddr WvInterface::dstaddr()
{
    struct ifreq ifr;
    ifr.ifr_dstaddr.sa_family = AF_INET;
    if (!(getflags() & IFF_POINTOPOINT) || req(SIOCGIFDSTADDR, &ifr))
        return WvIPAddr();
    else
        return WvIPAddr(&ifr.ifr_dstaddr);
}


int WvInterface::getflags()
{
    struct ifreq ifr;
    int retval = req(SIOCGIFFLAGS, &ifr);
    if (retval)
        valid = false;
    return ifr.ifr_flags;
}


int WvInterface::setflags(int clear, int set)
{
    struct ifreq ifr;

    int retval = req(SIOCGIFFLAGS, &ifr);
    if (retval)
        return retval;
    int newflags = (ifr.ifr_flags & ~clear) | set;
    if (newflags != ifr.ifr_flags)
    {
        ifr.ifr_flags = newflags;
        retval = req(SIOCSIFFLAGS, &ifr);
        if (retval && retval != EACCES && retval != EPERM)
            err.perror(WvString("SetFlags %s", name));
    }
    return retval;
}


void WvInterface::up(bool enable)
{
    setflags(IFF_UP, enable ? IFF_UP : 0);
    rescan();
}


bool WvInterface::isup()
{
    return (valid && (getflags() & IFF_UP)) ? 1 : 0;
}


void WvInterface::promisc(bool enable)
{
    setflags(IFF_PROMISC, enable ? IFF_PROMISC : 0);
}


bool WvInterface::ispromisc()
{
    return (getflags() & IFF_PROMISC) ? 1 : 0;
}


int WvInterface::setipaddr(const WvIPNet &addr)
{
    struct ifreq ifr;
    struct sockaddr *sa;
    size_t len;
    int sock;
    WvIPAddr none;
    
    if (addr != ipaddr())
	err(WvLog::Info, "Changing %s address to %s (%s bits)\n", name,
	    addr.base(), addr.bits());
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr.ifr_name, name, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = 0;
    ifr.ifr_addr.sa_family = AF_INET;

    len = min(sizeof(sockaddr), addr.sockaddr_len());

    sa = addr.sockaddr();
    memcpy(&ifr.ifr_addr, sa, len);
    delete sa;
    if (ioctl(sock, SIOCSIFADDR, &ifr))
    {
	if (errno != EACCES && errno != EPERM)
	    err.perror(WvString("SetIfAddress %s", name));
	close(sock);
	return -1;
    }
    
    // 2.1 kernels error when we try to change netmask/broadcast for
    // a 0.0.0.0 address.
    if (addr.base() != none)
    {
	sa = addr.netmask().sockaddr();
	memcpy(&ifr.ifr_netmask, sa, len);
	delete sa;
	if (ioctl(sock, SIOCSIFNETMASK, &ifr))
	{
	    if (errno != EACCES && errno != EPERM)
		err.perror(WvString("SetNetmask %s", name));
	    close(sock);
	    return -1;
	}
	
	if (!strchr(name, ':')) // otherwise, an alias, and no broadcast addr!
	{
	    sa = addr.broadcast().sockaddr();
	    memcpy(&ifr.ifr_broadaddr, sa, len);
	    delete sa;
	    if (ioctl(sock, SIOCSIFBRDADDR, &ifr))
	    {
		if (errno != EACCES && errno != EPERM)
		    err.perror(WvString("SetBroadcast %s", name));
		close(sock);
		return -1;
	    }
	}
    }
    
    // addroute(addr); // not necessary on 2.1 and higher kernels
    close(sock);

    rescan();
    return 0;
}


int WvInterface::setmtu(int mtu)
{
    struct ifreq ifr;
    ifr.ifr_mtu = mtu;
    int retval = req(SIOCSIFMTU, &ifr);
    if (retval && retval != EACCES && retval != EPERM)
        err.perror(WvString("SetMTU %s", name));
    return retval;
}


int WvInterface::sethwaddr(const WvAddr &addr)
{
    struct ifreq ifr;
    sockaddr *saddr = addr.sockaddr();
    memcpy(& ifr.ifr_hwaddr, saddr, addr.sockaddr_len());
    delete saddr;

    bool wasup = isup();
    if (wasup)
        up(false);
        
    int retval = req(SIOCSIFHWADDR, &ifr);
    if (retval && retval != EACCES && retval != EPERM)
        err.perror(WvString("SetHWAddr %s", name));

    if (wasup)
        up(true);

    rescan();
    return retval;
}


// Fill a routing table entry with the given information.
void WvInterface::fill_rte(struct rtentry *rte, char ifname[17],
			   const WvIPNet &dest, const WvIPAddr &gw,
			   int metric)
{
    struct sockaddr *net, *mask, *gwaddr;
    size_t len;
    bool is_direct = (gw == WvIPAddr());
    bool is_host = dest.is_host();
    
    memset(rte, 0, sizeof(struct rtentry));
    rte->rt_metric = metric + 1;
    
    strncpy(ifname, name, 17);
    ifname[17-1] = 0;
    rte->rt_dev = ifname;

    len = min(sizeof(sockaddr), dest.sockaddr_len());
    
    net = dest.network().sockaddr();
    memcpy(&rte->rt_dst, net, len);
    delete net;
    
    if (!is_host)
    {
	mask = dest.netmask().sockaddr();
	memcpy(&rte->rt_genmask, mask, len);
	delete mask;
    }

    if (!is_direct)
    {
	gwaddr = gw.sockaddr();
	memcpy(&rte->rt_gateway, gwaddr, len);
	delete gwaddr;
    }
    
    rte->rt_flags = (RTF_UP
		    | (is_host ? RTF_HOST : 0)
		    | (is_direct ? 0 : RTF_GATEWAY));
}


int WvInterface::really_addroute(const WvIPNet &dest, const WvIPAddr &gw,
		 const WvIPAddr &src, int metric, WvStringParm table,
		 bool shutup)
{
    struct rtentry rte;
    char ifname[17];
    int sock;
    WvString deststr(dest), gwstr(gw), metr(metric), srcstr(src);

    // FIXME: There has got to be a better way to do this.
    const char * const argvnosrc[] = {
	"ip", "route", "add",
	deststr,
	"table", table,
	"dev", name,
	"via", gwstr,
	"metric", metr,
	NULL
    };

    const char * const argvsrc[] = {
	"ip", "route", "add",
	deststr,
	"table", table,
	"dev", name,
	"via", gwstr,
        "src", srcstr,
	"metric", metr,
	NULL
    };

    WvIPAddr zero;
    const char * const * argv;
    if (src != zero)
        argv = argvsrc;
    else
        argv = argvnosrc;

    if (dest.is_default() || table != "default")
    {
	err(WvLog::Debug2, "addroute: ");
	for (int i = 0; argv[i]; i++)
	    err(WvLog::Debug2, "%s ", argv[i]);
	err(WvLog::Debug2, "\n");
	
        WvSubProc checkProc;
        checkProc.startv(*argv, argv);
        checkProc.wait(-1);

	//if (WvPipe(argv[0], argv, false, false, false).finish() != 242)
        if (checkProc.estatus != 242)
	{
	    // added a default route via the subprogram
	    // 242 is the magic "WvPipe could not exec program..." exit code.
	    return 0;
	}
    }
    
    // if we get here, it is not a default route or the 'ip' command is
    // broken somehow.

    fill_rte(&rte, ifname, dest, gw, metric);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ioctl(sock, SIOCADDRT, &rte))
    {
	if (errno != EACCES && errno != EPERM && errno != EEXIST
	  && errno != ENOENT)
	{
	    if (!shutup)
		err.perror(WvString("AddRoute '%s' %s (up=%s)",
				    name, dest, isup()));
	}
	close(sock);
	return -1;
    }
    
    close(sock);
    return 0;
}


int WvInterface::addroute(const WvIPNet &dest, const WvIPAddr &gw,
                          const WvIPAddr &src, int metric, WvStringParm table)
{
    WvIPAddr zero;
    int ret;
    
    // The kernel (2.4.19) sometimes tries to protect us from ourselves by
    // not letting us create a route via 'x' if 'x' isn't directly reachable
    // on the same interface.  This is non-helpful to us in some cases,
    // particularly with FreeSwan's screwy lying kernel routes.  Anyway,
    // the kernel people weren't clever enough to check that the routing
    // table *stays* self-consistent, so we add an extra route, then we
    // create our real route, and then we delete the extra route again.
    // Blah.
    // 
    // Using metric 255 should make it not the same as any other route.
    if (gw != zero)
	really_addroute(gw, zero, zero, 255, "default", true);
    ret = really_addroute(dest, gw, src, metric, table, false);
    if (gw != zero)
	delroute(gw, zero, 255, "default");
    
    return ret;
}


// add a route with no gateway, ie. direct to interface
int WvInterface::addroute(const WvIPNet &dest, int metric,
			  WvStringParm table)
{
    return addroute(dest, WvIPAddr(), WvIPAddr(), metric, table);
}


int WvInterface::delroute(const WvIPNet &dest, const WvIPAddr &gw,
			  int metric, WvStringParm table)
{
    struct rtentry rte;
    char ifname[17];
    int sock;
    WvString deststr(dest), gwstr(gw), metr(metric);
    const char *argv[] = {
	"ip", "route", "del",
	deststr,
	"table", table,
	"dev", name,
	"via", gwstr,
	"metric", metr,
	NULL
    };
    
    if (dest.is_default() || table != "default")
    {
	err(WvLog::Debug2, "addroute: ");
	for (int i = 0; argv[i]; i++)
	    err(WvLog::Debug2, "%s ", argv[i]);
	err(WvLog::Debug2, "\n");
	
        WvSubProc checkProc;
        checkProc.startv(*argv, (char * const *)argv);
        checkProc.wait(-1);

	//if (WvPipe(argv[0], argv, false, false, false).finish() == 0)
        if (!WEXITSTATUS(checkProc.estatus))
	{
	    // successfully deleted a default route via the subprogram
	    return 0;
	}
    }

    fill_rte(&rte, ifname, dest, gw, metric);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ioctl(sock, SIOCDELRT, &rte))
    {
	if (errno != EACCES && errno != EPERM && errno != EEXIST)
	    err.perror(WvString("DelRoute %s", name));
	close(sock);
	return -1;
    }
    
    close(sock);
    return 0;
}


// delete a route with no gateway, ie. direct to interface
int WvInterface::delroute(const WvIPNet &dest, int metric, WvStringParm table)
{
    return delroute(dest, WvIPAddr(), metric, table);
}


// add an ARP or proxy ARP entry on this interface
int WvInterface::addarp(const WvIPNet &dest, const WvAddr &hw, bool proxy)
{
    int sock;
    struct arpreq ar;
    struct sockaddr *sa;
    size_t len;
    
    sa = dest.network().sockaddr();
    len = min(dest.sockaddr_len(), sizeof(ar.arp_pa));
    memcpy(&ar.arp_pa, sa, len);
    delete sa;
    
    sa = hw.sockaddr();
    len = min(hw.sockaddr_len(), sizeof(ar.arp_ha));
    memcpy(&ar.arp_ha, sa, len);
    delete sa;
    
    sa = dest.netmask().sockaddr();
    len = min(dest.sockaddr_len(), sizeof(ar.arp_netmask));
    memcpy(&ar.arp_netmask, sa, len);
    delete sa;
    
    strncpy(ar.arp_dev, name, sizeof(ar.arp_dev));
    
    ar.arp_flags = (ATF_COM | ATF_PERM
		    | (proxy ? ATF_PUBL : 0)
		    | (proxy && dest.is_host() ? ATF_NETMASK : 0));
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ioctl(sock, SIOCSARP, &ar))
    {
	if (errno != EACCES && errno != EPERM)
	    err.perror(WvString("AddARP %s", name));
	close(sock);
	return -1;
    }
    
    close(sock);
    return 0;
}


bool WvInterface::isarp()
{
    int f = getflags();
    return !(f & (IFF_NOARP | IFF_LOOPBACK)) && (f & IFF_BROADCAST);
}


static char *find_ifname(char *line)
{
    if (!line) return NULL;
    
    // skip leading whitespace
    while (*line==' ') line++;

    // everything before the last colon is the device name
    char *cptr = strrchr(line, ':');
    if (!cptr)
	return NULL;
    *cptr = 0;
    return line;
}


////////////////////////////////////////////// WvInterfaceDict


WvInterfaceDict::WvInterfaceDict() : log("Net Interface", WvLog::Info)
{
    links++;
    update();
}


WvInterfaceDict::~WvInterfaceDict()
{
    links--;
    
    if (!links)
	slist.zap();
}


// auto-fill the list of interfaces using the list from /proc/net/dev.
//
// I wish there was a better way to do this, but the SIOCGIFCONF ioctl
// ignores 'down' interfaces, which is not what we want.
//
void WvInterfaceDict::update()
{
    int sock;
    struct ifconf ifconf;
    char buf[sizeof(ifconf.ifc_req) * 100]; // room for 100 interfaces
    WvLog err(log.split(WvLog::Error));
    WvFile procdev("/proc/net/dev", O_RDONLY);
    char *ifname;

	
    // mark all interfaces in list invalid for now
    Iter i(*this);
    for (i.rewind(); i.next(); )
	i().valid = false;
    

    // get list of all non-aliased interfaces from /proc/net/dev
    
    
    // skip the two header lines
    procdev.getline(-1); procdev.getline(-1);
    
    // add/validate existing interfaces
    while ((ifname = find_ifname(procdev.getline(-1))) != NULL)
    {
	WvString s(ifname);
	WvInterface *ifc = (*this)[s];
	
	if (!ifc)
	{
	    ifc = new WvInterface(ifname);
	    slist.add(ifc, true);
	    log(WvLog::Debug3, "Found %-16s  [%s]\n", ifname, ifc->hwaddr());
	}
	else
	    ifc->rescan();
	ifc->valid = true;
    }

    
    // get list of "up" and aliased interfaces with SIOCGIFCONF ioctl
    
    
    ifconf.ifc_buf = buf;
    ifconf.ifc_len = sizeof(buf);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (! ioctl(sock, SIOCGIFCONF, &ifconf))
    {
	int count, max = ifconf.ifc_len / sizeof(ifconf.ifc_req[0]);
	
	for (count = 0; count < max; count++)
	{
	    struct ifreq &ifr = ifconf.ifc_req[count];
	    WvInterface *ifc = (*this)[ifr.ifr_name];
	    
	    if (!ifc)
	    {
		ifc = new WvInterface(ifr.ifr_name);
		slist.add(ifc, true);
	    }
	    else
		ifc->rescan();
	    ifc->valid = true;
	}
    }
    close(sock);
}


// determine if the given address belongs to the local system
bool WvInterfaceDict::islocal(const WvAddr &addr)
{
    static WvIPAddr bcast("255.255.255.255"); // always a local address!
    
    if (addr == bcast)
	return true;
    
    Iter i(*this);
    for (i.rewind(); i.next(); )
    {
	WvInterface &ifc(*i);
	if (!ifc.valid) continue;
	
	if (ifc.ipaddr() == addr || ifc.ipaddr().base() == addr
	  || ifc.ipaddr().broadcast() == addr)
	    return true;

	if (ifc.hwaddr() == addr)
	    return true;
    }
    
    return false;
}


bool WvInterfaceDict::on_local_net(const WvIPNet &addr)
{
    WvIPAddr zero;
    
    if (islocal(addr))
	return true;
    
    Iter i(*this);
    for (i.rewind(); i.next(); )
    {
	WvInterface &ifc = *i;
	if (!ifc.valid) continue;
	
	if (ifc.isup() && WvIPAddr(ifc.ipaddr()) != zero
	  && ifc.ipaddr().includes(addr))
	    return true;
    }
    
    return false;
}

#else

WvInterfaceDictBase WvInterfaceDict::slist(15);

int WvInterface::getinfo(struct ifreq *ifr, int ioctl_num) { return 0; }
void WvInterface::fill_rte(struct rtentry *rte, char *ifname,
                  const WvIPNet &dest, const WvIPAddr &gw,
                  int metric) {}

WvInterface::WvInterface(WvStringParm _name) :err("fake") {}
WvInterface::~WvInterface() {}

void WvInterface::rescan() {}
const WvIPNet &WvInterface::ipaddr() { return *(new WvIPNet()); }
const WvIPAddr WvInterface::dstaddr() { return *(new WvIPAddr()); }
int WvInterface::getflags() { return 0; }
int WvInterface::setflags(int clear, int set) { return 0; }
bool WvInterface::isup() { return true; }
void WvInterface::up(bool enable) {}
bool WvInterface::ispromisc() { return true; }
void WvInterface::promisc(bool enable) {}
int WvInterface::setipaddr(const WvIPNet &addr) { return 0; }
int WvInterface::setmtu(int mtu) { return 0; }
int WvInterface::addroute(const WvIPNet &dest, int metric = 0,
                 WvStringParm table = "default") { return 0; }
int WvInterface::addroute(const WvIPNet &dest, const WvIPAddr &gw,
                 int metric = 0, WvStringParm table = "default") { return 0; }
int WvInterface::delroute(const WvIPNet &dest, int metric = 0,
                 WvStringParm table = "default") { return 0; }
int WvInterface::delroute(const WvIPNet &dest, const WvIPAddr &gw,
                 int metric = 0, WvStringParm table = "default") { return 0; }
bool WvInterface::isarp() { return true; }
int WvInterface::addarp(const WvIPNet &proto, const WvAddr &hw, bool proxy)
                 { return 0; }

WvInterfaceDict::WvInterfaceDict() :log("fake") {}
WvInterfaceDict::~WvInterfaceDict() {}

void WvInterfaceDict::update() {}
bool WvInterfaceDict::islocal(const WvAddr &addr) { return true; }
bool WvInterfaceDict::on_local_net(const WvIPNet &addr) { return true; }

#endif
