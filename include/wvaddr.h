/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Device-independent and device-specific hardware/protocol address classes
 * that can store themselves efficiently as well as create a printable string
 * version of themselves.
 */
#ifndef __WVADDR_H
#define __WVADDR_H

#include "wvstring.h"

#ifdef ISLINUX
#include <linux/if_ether.h>
#endif

#if defined(ISBSD) || defined(ISDARWIN)
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <netinet/in_systm.h>
#include <net/ethernet.h>
#define ETH_ALEN ETHER_ADDR_LEN
#endif

#if defined(ISBSD) || defined(ISDARWIN)
#include <arpa/inet.h>
#include "if_arp.h"
#endif

#include <netinet/in.h>

typedef unsigned int __u32;
typedef short unsigned int __u16;

/**
 * Common packet encapsulation types, with the ability to convert a Linux
 * ARPHRD_* value or (struct sockaddr) sa_family value.  (Those two use the
 * same set of values.)
 */
class WvEncap
{
    static char strings[][20];	       // printable-string names per type
    static int extypes[];	       // external types (ARPHRD_*, etc)
public:
    // NOTE:  if you change enum CapType, don't forget to change extypes[]
    //   and strings[] in wvaddr.cc!
    enum CapType {
	// hardware encapsulation
	Unknown = 0,
	Loopback,
	Ethertap,
	Ethernet,
	ARCnet,
	SLIP,
	CSLIP,
	PPP,
	
	// protocol encapsulation
	IPv4,
	Unix,
	
	// END
	NUM_ENCAP_TYPES
    };
    CapType cap;
    
    WvEncap(CapType _cap = Unknown)
        { cap = _cap; }
    
    WvEncap(int extype);
    
    operator CapType () const
        { return cap; }
    
    operator WvString () const
        { return strings[cap]; }
};


/**
 * Base class for different address types, each of which will have
 * the ability to convert itself to/from a printable string, as well
 * as other type-specific abilities.
 */
class WvAddr
{
protected:
    virtual WvString printable() const = 0;
    const char *addrtype;

public:
    WvAddr();
    virtual ~WvAddr();
    static WvAddr *gen(struct sockaddr *addr);
    
    virtual WvEncap encap() const = 0;
    operator WvString() const
        { return printable(); }
    
    virtual bool isbroadcast() const;

    virtual struct sockaddr *sockaddr() const = 0;
    virtual size_t sockaddr_len() const = 0;
    virtual const unsigned char *rawdata() const;
    virtual size_t rawdata_len() const;
    
    virtual unsigned WvHash() const;
    virtual bool comparator(const WvAddr *a2) const;
    
    bool operator== (const WvAddr &a2) const
        { return addrtype == a2.addrtype && comparator(&a2); }
    bool operator!= (const WvAddr &a2) const
        { return ! (*this == a2); }
};


// useful for hash tables (see wvhashtable.h)
unsigned WvHash(const WvAddr &addr);


/**
 * A WvAddr that simply contains a printable string with a user-defined
 * encapsulation type.
 */
class WvStringAddr : public WvAddr
{
    WvString addr;
    WvEncap cap;

protected:
    virtual WvString printable() const;

public:
    WvStringAddr(WvStringParm s, const WvEncap &_cap);
    WvStringAddr(const struct sockaddr *_addr);
    virtual ~WvStringAddr();
    virtual WvEncap encap() const;
    virtual struct sockaddr *sockaddr() const;
    virtual size_t sockaddr_len() const;
    virtual const unsigned char *rawdata() const;
    virtual size_t rawdata_len() const;
};


/**
 * An ethernet address is made up of a string of hex numbers, in the form
 *     AA:BB:CC:DD:EE:FF
 */
class WvEtherAddr : public WvAddr
{
    unsigned char binaddr[ETH_ALEN];

protected:
    virtual WvString printable() const;

public:
    WvEtherAddr(const unsigned char _binaddr[ETH_ALEN] = NULL)
        { if (_binaddr) memcpy(binaddr, _binaddr, ETH_ALEN); }
    WvEtherAddr(const char string[])
        { string_init(string); }
    WvEtherAddr(WvStringParm string)
        { string_init(string); }
    void string_init(const char string[]);
    WvEtherAddr(const struct sockaddr *addr)
        { memcpy(binaddr, (void *)addr->sa_data, ETH_ALEN); }
    virtual ~WvEtherAddr();
    
    virtual WvEncap encap() const;
    virtual bool isbroadcast() const;
    virtual struct sockaddr *sockaddr() const;
    virtual size_t sockaddr_len() const;
    virtual const unsigned char *rawdata() const;
    virtual size_t rawdata_len() const;
};


/**
 * An ARCnet address is made up of a single hex number. 
 */
class WvARCnetAddr : public WvAddr
{
    unsigned char binaddr;

protected:
    virtual WvString printable() const;

public:
    WvARCnetAddr(const unsigned char _binaddr[1] = NULL)
        { if (_binaddr) binaddr = _binaddr[0]; }
    WvARCnetAddr(const char string[])
        { binaddr = strtoul(string, NULL, 16); }
    WvARCnetAddr(WvStringParm string)
    	{ binaddr = strtoul(string, NULL, 16); }
    WvARCnetAddr(const struct sockaddr *addr)
        { binaddr = ((unsigned char *)addr->sa_data)[0]; }
    virtual ~WvARCnetAddr();
    
    virtual WvEncap encap() const;
    virtual struct sockaddr *sockaddr() const;
    virtual size_t sockaddr_len() const;
    virtual const unsigned char *rawdata() const;
    virtual size_t rawdata_len() const;
};


/**
 * An IP address is made up of a "dotted quad" -- four decimal numbers in
 * the form
 *     www.xxx.yyy.zzz
 * 
 * We don't support automatic name lookups yet, but this will be the place
 * to do it when support is added.
 */
class WvIPAddr : public WvAddr
{
protected:
    virtual WvString printable() const;
public:
    unsigned char binaddr[4];

    WvIPAddr(const unsigned char _binaddr[4])
        { if (_binaddr) memcpy(binaddr, _binaddr, 4); }
    WvIPAddr(const __u32 _binaddr = 0)
        { memcpy(binaddr, &_binaddr, 4); }
    WvIPAddr(const char string[])
    	{ string_init(string); }
    WvIPAddr(WvStringParm string)
        { string_init(string); }
    void string_init(const char string[]);
    WvIPAddr(const struct sockaddr *addr)
        { memcpy(binaddr,
		 (void *)&((struct sockaddr_in *)addr)->sin_addr.s_addr, 4); }
    WvIPAddr(const WvIPAddr &_addr)
        { memcpy(binaddr, _addr.binaddr, 4); }
    virtual ~WvIPAddr();
    
    WvIPAddr operator& (const WvIPAddr &a2) const;
    WvIPAddr operator| (const WvIPAddr &a2) const;
    WvIPAddr operator^ (const WvIPAddr &a2) const;
    WvIPAddr operator~ () const;
    WvIPAddr operator+ (int n) const;
    WvIPAddr operator- (int n) const;
    __u32 s_addr() const
        { return *(__u32 *)binaddr; }
    
    virtual WvEncap encap() const;

    virtual struct sockaddr *sockaddr() const;
    virtual size_t sockaddr_len() const;
    virtual const unsigned char *rawdata() const;
    virtual size_t rawdata_len() const;
};


/**
 * An IP network comprises two WvIPAddr structures: an address and a
 * netmask. The two ANDed together comprise the "network address",
 * which, if it is correct, can be ORed with any IP address on the
 * network without changing the address.  Together, a network address
 * and netmask provide a good description of the IP addresses
 * available on a network.
 * 
 * WvIPNet internally stores a base IP address (the inherited WvIPAddr)
 * and the netmask (a member variable).
 * 
 * Note that the rawdata() function is inherited from WvIPAddr, so it does
 * not include the netmask in the raw data.
 */
class WvIPNet : public WvIPAddr
{
protected:
    WvIPAddr mask;
    virtual WvString printable() const;

public:
    WvIPNet(const WvIPNet &_net);
    WvIPNet(const char string[]) : WvIPAddr(string)
        { string_init(string); }
    WvIPNet(WvStringParm string) : WvIPAddr(string)
        { string_init(string); }
    void string_init(const char string[]);
    WvIPNet(const WvIPAddr &base, const WvIPAddr &_mask);
    
    /**
     * construct an IPNet from a base address and a number of bits in
     * the netmask.  The default of 32 gives a one-host network,
     * (netmask 255.255.255.255).
     */
    WvIPNet(const WvIPAddr &base, int bits = 32);
    
    /**
     * construct an empty IPNet for later copying (probably by operator=)
     */
    WvIPNet();
    
    virtual ~WvIPNet();
    
    /**
     * Override the hash and comparison functions
     */
    virtual unsigned WvHash() const;
    virtual bool comparator(const WvAddr *a2) const;
    
    /** 
     * Get the 'base IP address' component, netmask, network, and broadcast
     */
    WvIPAddr base() const
        { return WvIPAddr(binaddr); }
    const WvIPAddr &netmask() const
        { return mask; }
    WvIPAddr network() const
        { return *this & mask; }
    WvIPAddr broadcast() const
        { return *this | ~mask; }
    
    /**
     * adjust the netmask so that 'addr' would be included in this network
     */
    void include(const WvIPNet &addr);
    
    /**
     * determine whether the given address is already included in this net
     */
    bool includes(const WvIPNet &addr) const;
    
    /**
     * weird netmasks such as 255.0.255.0 (easy example) are almost never
     * used -- they have '0' bits in the middle.  However, using the
     * include() function will result in odd netmasks like this, since
     * it will not eliminate a '1' bit unless absolutely necessary.
     * normalize() would convert the above netmask into 255.0.0.0, which
     * is probably the netmask _really_ in use.  bits() calculates
     * the number of leading '1' bits in the normalized netmask, without
     * actually doing the normalization.
     */
    int bits() const;
    void normalize();
    
    /**
     * is this net the default gateway? (0.0.0.0/0)
     */
    bool is_default() const
        { return mask.binaddr[0] == 0; }
    
    /**
     * is it a plain host? (x.x.x.x/32)
     */
    bool is_host() const
        { return mask.binaddr[3] == 255; }
};



/**
 * An IP+Port address also includes a port number, with the resulting form
 *     www.xxx.yyy.zzz:pppp
 * 
 * Note that the rawdata() function is inherited from WvIPAddr, so it does
 * not include the port number in the raw data.
 */
class WvIPPortAddr : public WvIPAddr
{
protected:
    virtual WvString printable() const;
public:
    __u16 port;
    
    WvIPPortAddr();
    WvIPPortAddr(const unsigned char _ipaddr[4], __u16 _port = 0);
    WvIPPortAddr(const WvIPAddr &_ipaddr, __u16 _port = 0);
    WvIPPortAddr(const char string[]) : WvIPAddr(string)
        { string_init(string); }
    WvIPPortAddr(WvStringParm string) : WvIPAddr(string)
        { string_init(string); }
    void string_init(const char string[]);
    WvIPPortAddr(__u16 _port);          // assumes address 0.0.0.0, (ie local)
    WvIPPortAddr(const char string[], __u16 _port);
    
    WvIPPortAddr(struct sockaddr_in *sin) : WvIPAddr(sin->sin_addr.s_addr)
        { port = ntohs(sin->sin_port); }
    virtual ~WvIPPortAddr();
    
    virtual struct sockaddr *sockaddr() const;

    // Override the hash and comparison functions
    virtual unsigned WvHash() const;
    virtual bool comparator(const WvAddr *a2) const;
};


/**
 * A Unix domain socket address is really just a filename. 
 */
class WvUnixAddr : public WvAddr
{
protected:
    WvString sockname;
    virtual WvString printable() const;
    
public:
    WvUnixAddr(const char *_sockname);
    WvUnixAddr(WvStringParm _sockname);
    WvUnixAddr(const WvUnixAddr &_addr);
    virtual ~WvUnixAddr();
    
    virtual WvEncap encap() const;

    virtual struct sockaddr *sockaddr() const;
    virtual size_t sockaddr_len() const;
    virtual const unsigned char *rawdata() const;
    virtual size_t rawdata_len() const;
};


#endif // __WVADDR_H
