/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * A WvInterface stores information about a particular network interface.
 */
#include "wvinterface.h"
#include "wvpipe.h"
#include "wvfile.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <net/route.h>
#include <unistd.h>
#include <linux/sockios.h>
#include <errno.h>

#define min(x,y) ((x) < (y) ? (x) : (y))


WvInterfaceDictBase WvInterfaceDict::slist(15);
int WvInterfaceDict::links = 0;


WvInterface::WvInterface(const WvString &_name) 
	: err("Net Interface", WvLog::Error), name(_name)
{
    name.unique();
    my_hwaddr = my_ipaddr = NULL;
    valid = true;
}


WvInterface::~WvInterface()
{
    rescan();
}


int WvInterface::getinfo(struct ifreq *ifr, int ioctl_num)
{
    int sock, retval;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr->ifr_name, name, IFNAMSIZ-1);
    ifr->ifr_name[IFNAMSIZ-1] = 0;
    ifr->ifr_addr.sa_family = AF_INET;
    
    retval = ioctl(sock, ioctl_num, ifr);
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
	if (getinfo(&ifr, SIOCGIFHWADDR))
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
	if (getinfo(&ifr, SIOCGIFADDR) || getinfo(&ifr2, SIOCGIFNETMASK))
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
    if (!(getflags() & IFF_POINTOPOINT) || getinfo(&ifr, SIOCGIFDSTADDR))
	return WvIPAddr();
    else
	return WvIPAddr(&ifr.ifr_dstaddr);
}


int WvInterface::getflags()
{
    struct ifreq ifr;
    int sock, errnum;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr.ifr_name, name, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = 0;

    if (ioctl(sock, SIOCGIFFLAGS, &ifr))
    {
	errnum = errno;
	if (errnum != EACCES && errnum != EPERM)
	    err.perror(WvString("GetFlags %s", name));
	close(sock);
	return errnum;
    }
    
    close(sock);
    return ifr.ifr_flags;
}


int WvInterface::setflags(int clear, int set)
{
    struct ifreq ifr;
    int sock, errnum;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr.ifr_name, name, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = 0;

    if (ioctl(sock, SIOCGIFFLAGS, &ifr))
    {
	errnum = errno;
	if (errnum != EACCES && errnum != EPERM)
	    err.perror(WvString("GetFlags %s", name));
	close(sock);
	return errnum;
    }
    
    if (((ifr.ifr_flags & ~clear) | set) != ifr.ifr_flags)
    {
	ifr.ifr_flags &= ~clear;
	ifr.ifr_flags |= set;
	
	if (ioctl(sock, SIOCSIFFLAGS, &ifr))
	{
	    errnum = errno;
	    if (errnum != EACCES && errnum != EPERM)
		err.perror(WvString("SetFlags %s", name));
	    close(sock);
	    return errnum;
	}
    }
    
    close(sock);
    return 0;
}


void WvInterface::up(bool enable)
{
    setflags(IFF_UP, enable ? IFF_UP : 0);
    rescan();
}


bool WvInterface::isup()
{
    return (getflags() & IFF_UP) ? 1 : 0;
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
    int sock, errnum;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    strncpy(ifr.ifr_name, name, IFNAMSIZ-1);
    ifr.ifr_name[IFNAMSIZ-1] = 0;

    ifr.ifr_mtu = mtu;
    
    if (ioctl(sock, SIOCSIFMTU, &ifr))
    {
	errnum = errno;
	if (errnum != EACCES && errnum != EPERM)
	    err.perror(WvString("SetMTU %s", name));
	close(sock);
	return errnum;
    }
    
    close(sock);
    return 0;
}


// Fill a routing table entry with the given information.
void WvInterface::fill_rte(struct rtentry *rte, char ifname[17],
			   const WvIPNet &dest, const WvIPAddr &gw,
			   int metric)
{
    struct sockaddr *net, *mask, *gwaddr;
    size_t len;
    bool is_direct = (gw == WvIPAddr());
    bool is_host = (dest.bits() == 32);
    
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


int WvInterface::addroute(const WvIPNet &dest, const WvIPAddr &gw,
			   int metric)
{
    struct rtentry rte;
    char ifname[17];
    int sock;
    WvString gwstr(gw), metr(metric);
    const char *argv[] = {
	"ip", "route", "add",
	"default",
	"table", "default",
	"dev", name,
	"via", gwstr,
	"metric", metr,
	"scope", "global",
	NULL
    };
    
    if (gw == WvIPAddr()) // no gateway; change the scope name
	argv[13] = "link";
    
    if (dest.bits() == 0)
    {
	err(WvLog::Debug2, "addroute: ");
	for (int i = 0; argv[i]; i++)
	    err(WvLog::Debug2, "%s ", argv[i]);
	err(WvLog::Debug2, "\n");
	
	if (WvPipe(argv[0], argv, false, false, false).finish() != 242)
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
	if (errno != EACCES && errno != EPERM && errno != EEXIST)
	    err.perror(WvString("AddRoute %s %s (up=%s)",
				name, dest, isup()));
	close(sock);
	return -1;
    }
    
    close(sock);
    return 0;
}


// add a route with no gateway, ie. direct to interface
int WvInterface::addroute(const WvIPNet &dest, int metric)
{
    return addroute(dest, WvIPAddr(), metric);
}


int WvInterface::delroute(const WvIPNet &dest, const WvIPAddr &gw,
			   int metric)
{
    struct rtentry rte;
    char ifname[17];
    int sock;
    WvString gwstr(gw), metr(metric);
    const char *argv[] = {
	"ip", "route", "del",
	"default",
	"table", "default",
	"dev", name,
	"via", gwstr,
	"metric", metr,
	"scope", "global",
	NULL
    };
    
    if (gw == WvIPAddr()) // no gateway; change the scope name
	argv[13] = "link";
    
    if (dest.bits() == 0)
    {
	err(WvLog::Debug2, "addroute: ");
	for (int i = 0; argv[i]; i++)
	    err(WvLog::Debug2, "%s ", argv[i]);
	err(WvLog::Debug2, "\n");
	
	if (WvPipe(argv[0], argv, false, false, false).finish() == 0)
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
int WvInterface::delroute(const WvIPNet &dest, int metric)
{
    return delroute(dest, WvIPAddr(), metric);
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
		    | (proxy && dest.bits()==32 ? ATF_NETMASK : 0));
    
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
	WvInterface &ifc(i);
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
	WvInterface &ifc = i;
	if (!ifc.valid) continue;
	
	if (ifc.isup() && WvIPAddr(ifc.ipaddr()) != zero
	  && ifc.ipaddr().includes(addr))
	    return true;
    }
    
    return false;
}
