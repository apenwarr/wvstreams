/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
  
#ifndef __WVINTERFACE_H
#define __WVINTERFACE_H

#include "wvaddr.h"
#include "wvhashtable.h"
#include "wvlog.h"

struct ifreq;
struct rtentry;

/** 
 * A WvInterface manages a particular network interface.  It is _allowed_
 * to have more than one WvInterface instance referring to the same
 * physical interface, because that is more convenient.
 */
class WvInterface
{
    WvAddr *my_hwaddr;
    WvIPNet *my_ipaddr;
    
    WvLog err;
    
    /**
     * get/set information about an interface
     */
    int getinfo(struct ifreq *ifr, int ioctl_num);
    
    /**
     * used by addroute()/delroute()
     */
    void fill_rte(struct rtentry *rte, char *ifname,
		  const WvIPNet &dest, const WvIPAddr &gw,
		  int metric);
    
public:
    WvString name;
    bool valid;
    
    WvInterface(WvStringParm _name);
    ~WvInterface();
    
    /**
     * forget all stored information about the address(es) of this interface
     */
    void rescan();
    
    /**
     * get the hardware address of this interface
     */
    const WvAddr &hwaddr();
    
    /**
     * get the local IP net of this interface
     */
    const WvIPNet &ipaddr();
    
    /**
     * get the point-to-point IP address of this interface
     */
    const WvIPAddr dstaddr();
    
    /**
     * get the current kernel flags
     */
    int getflags();
    
    /**
     * toggle kernel flags on this netdevice.  Be careful!
     */
    int setflags(int clear, int set);

    /**
     * set the interface state up or down.
     */
    bool isup();
    void up(bool enable);

    /**
     * turn promiscuous (see-all-packets) mode on or off.
     */
    bool ispromisc();
    void promisc(bool enable);
    
    /**
     * Set the local address, netmask, and broadcast of this interface
     * and set a route to the local net.
     */
    int setipaddr(const WvIPNet &addr);
    
    /**
     * Set the MTU of the interface.
     */
    int setmtu(int mtu);
    
    /**
     * add a route to the given network through this interface.
     */
    int addroute(const WvIPNet &dest, int metric = 0,
		 WvStringParm table = "default");
    int addroute(const WvIPNet &dest, const WvIPAddr &gw, int metric = 0,
		 WvStringParm table = "default");

    /**
     * delete a route to the given network through this interface.
     */
    int delroute(const WvIPNet &dest, int metric = 0,
		 WvStringParm table = "default");
    int delroute(const WvIPNet &dest, const WvIPAddr &gw, int metric = 0,
		 WvStringParm table = "default");
    
    /**
     * add an ARP entry on this interface
     */
    bool isarp();
    int addarp(const WvIPNet &proto, const WvAddr &hw, bool proxy);
};

DeclareWvDict3(WvInterface, WvInterfaceDictBase, WvString, name, );

class WvInterfaceDict
{
public:
    WvLog log;
    static WvInterfaceDictBase slist;
    static int links;
    
    class Iter : public WvInterfaceDictBase::Iter
    {
    public:
	Iter(WvInterfaceDict &l) 
	    : WvInterfaceDictBase::Iter(l.slist)
	    { }
    };
    
    WvInterfaceDict();
    ~WvInterfaceDict();
    
    void update();
    bool islocal(const WvAddr &addr);
    bool on_local_net(const WvIPNet &addr);

    WvInterface *operator[] (WvStringParm str)
        { return slist[str]; }
    
    operator WvInterfaceDictBase ()
        { return slist; }
};

#endif // __WVINTERFACE_H
