/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * A WvInterface manages a particular network interface.  It is _allowed_
 * to have more than one WvInterface instance referring to the same
 * physical interface, because that is more convenient.
 */
#ifndef __WVINTERFACE_H
#define __WVINTERFACE_H

#include "wvaddr.h"
#include "wvhashtable.h"
#include "wvlog.h"

struct ifreq;
struct rtentry;

class WvInterface
{
    WvAddr *my_hwaddr;
    WvIPNet *my_ipaddr;
    
    WvLog err;
    
    // get/set information about an interface
    int getinfo(struct ifreq *ifr, int ioctl_num);
    
    // used by addroute()/delroute()
    void fill_rte(struct rtentry *rte, char *ifname,
		  const WvIPNet &dest, const WvIPAddr &gw,
		  int metric);
    
public:
    WvString name;
    bool valid;
    
    WvInterface(const WvString &_name);
    ~WvInterface();
    
    // forget all stored information about the address(es) of this interface
    void rescan();
    
    // get the hardware address of this interface
    const WvAddr &hwaddr();
    
    // get the local IP net of this interface
    const WvIPNet &ipaddr();
    
    // get the point-to-point IP address of this interface
    const WvIPAddr dstaddr();
    
    // get the current kernel flags
    int getflags();
    
    // toggle kernel flags on this netdevice.  Be careful!
    int setflags(int clear, int set);

    // set the interface state up or down.
    bool isup();
    void up(bool enable);

    // turn promiscuous (see-all-packets) mode on or off.
    bool ispromisc();
    void promisc(bool enable);
    
    // Set the local address, netmask, and broadcast of this interface
    // and set a route to the local net.
    int setipaddr(const WvIPNet &addr);
    
    // add a route to the given network through this interface.
    int addroute(const WvIPNet &dest, int metric = 0);
    int addroute(const WvIPNet &dest, const WvIPAddr &gw, int metric = 0);

    // delete a route to the given network through this interface.
    int delroute(const WvIPNet &dest, int metric = 0);
    int delroute(const WvIPNet &dest, const WvIPAddr &gw, int metric = 0);
    
    // add an ARP entry on this interface
    bool isarp();
    int addarp(const WvIPNet &proto, const WvAddr &hw, bool proxy);
};

DeclareWvDict2(WvInterface, WvString, name,
	       WvLog *log;
	       void setup()
	           { log = new WvLog("Net Interface", WvLog::Info);
		       update(); }
	       void shutdown()
	           { delete log; }
	       void update();
	       bool islocal(const WvAddr &addr);
	       bool on_local_net(const WvIPNet &addr);
	       );

#endif // __WVINTERFACE_H
