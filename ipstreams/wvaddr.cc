/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 * 
 * Device-independent and device-specific hardware/protocol address
 * classes that can store themselves efficiently as well as create a
 * printable string version of themselves.
 */
#include "wvaddr.h"
#include <sys/socket.h>
#include <net/if_arp.h>

// workaround for functions called sockaddr() -- oops.
typedef struct sockaddr sockaddr_bin;

/* A list of Linux ARPHRD_* types, one for each element of CapType. */
int WvEncap::extypes[] = {
    // hardware encapsulation
    0, // Unknown
    ARPHRD_LOOPBACK,
    0, // Ethertap
    ARPHRD_ETHER,
    ARPHRD_ARCNET,
    ARPHRD_SLIP,
    ARPHRD_CSLIP,
    ARPHRD_PPP,
    
    // protocol encapsulation
    AF_INET, // IPv4
};


/* Printable strings corresponding to each element of CapType */
char WvEncap::strings[][20] = {
    // hardware encapsulation
    "Unknown",
    "Loopback",
    "Ethertap",
    "Ethernet",
    "ARCnet",
    "SLIP",
    "CSLIP",
    "PPP",
    
    // protocol encapsulation
    "IP", // IPv4
};


/* Figure out the CapType corresponding to a Linux ARPHRD_* type or
 * sockaddr sa_family type.
 */
WvEncap::WvEncap(int extype)
{
    for (int count=0; count < NUM_ENCAP_TYPES; count++)
    {
	if (extype == extypes[count])
	{
	    cap = (CapType)count;
	    return;
	}
    }
    cap = Unknown;
}


/* Find the hash value of a WvAddr, for use with WvHashTable */
unsigned WvHash(const WvAddr &addr)
{
    return addr.WvHash();
}


WvAddr::WvAddr()
{
    addrtype = "WvAddr";
}


WvAddr::~WvAddr()
{
}


/* Create an object of the appropriate WvAddr-derived class based on the
 * address and type stored in 'addr'.
 */
WvAddr *WvAddr::gen(struct sockaddr *addr)
{
    WvEncap encap(addr->sa_family);
    
    switch (encap.cap)
    {
    case WvEncap::Loopback:
	return new WvStringAddr("Loopback", WvEncap::Loopback);
	
    case WvEncap::IPv4:
	return new WvIPPortAddr((sockaddr_in *)addr);
	
    case WvEncap::ARCnet:
	return new WvARCnetAddr(addr);
	
    case WvEncap::Ethertap:
    case WvEncap::Ethernet:
	return new WvEtherAddr(addr);

    default:
	return new WvStringAddr("Unknown", WvEncap::Unknown);
    }
}


bool WvAddr::isbroadcast() const
{
    return false; // default is no support for broadcasts
}


const unsigned char *WvAddr::rawdata() const
{
    return NULL;
}


size_t WvAddr::rawdata_len() const
{
    return 0;
}


unsigned WvAddr::WvHash() const
{
    unsigned hash = 0;
    const unsigned char *cptr, *raw = rawdata();
    int len = rawdata_len(), width;
   
    if (!raw || !len) return 0;
    width = (sizeof(hash)*8 / len) + 1;
    
    for (cptr = raw; len; len--)
	hash = (hash << width) ^ *(cptr++);
    return hash;
}


bool WvAddr::comparator(const WvAddr *a2) const
{
    const unsigned char *raw1, *raw2;
    size_t len;
    
    len = rawdata_len();
    if (len != a2->rawdata_len())
	return false;
    
    raw1 = rawdata();
    raw2 = a2->rawdata();
    
    if (!raw1 && !raw2) return true;
    if (!raw1 || !raw2) return false;
    
    return !memcmp(raw1, raw2, len);
}


WvStringAddr::WvStringAddr(const WvString &s, const WvEncap &_cap)
	: addr(s), cap(_cap)
{
    addr.unique();
}


WvStringAddr::WvStringAddr(const sockaddr_bin *_addr)
	: addr((char *)_addr->sa_data), cap(_addr->sa_family)
{
    addr.unique();
}


WvEncap WvStringAddr::encap() const
{
    return cap;
}


const unsigned char *WvStringAddr::rawdata() const
{
    return (const unsigned char *)(const char *)addr;
}


size_t WvStringAddr::rawdata_len() const
{
    return strlen(addr);
}


sockaddr_bin *WvStringAddr::sockaddr() const
{
    sockaddr_bin *sa = new sockaddr_bin;
    memset(sa, 0, sizeof(*sa));
    strncpy(sa->sa_data, addr, sizeof(sa->sa_data));
    return sa;
}


size_t WvStringAddr::sockaddr_len() const
{
    return sizeof(sockaddr_bin);
}


WvString WvStringAddr::printable() const
{
    return addr;
}


/* create a WvEtherAddr from a printable string in the format:
 *      AA:BB:CC:DD:EE:FF  (six hex numbers, separated by colons)
 */
WvEtherAddr::WvEtherAddr(char const string[])
{
    char *endptr = NULL;
    unsigned char *cptr = binaddr;
    
    memset(binaddr, 0, ETH_ALEN);
    for (int count=0; count < ETH_ALEN; count++)
    {
	*cptr++ = strtoul(endptr ? endptr : string, &endptr, 16);
	if (!endptr || endptr==string) break;
	endptr++;
    }
}


/* Generate a printable version of an ethernet address. */
WvString WvEtherAddr::printable() const
{
    char s[ETH_ALEN*3], *cptr = s;
    
    for (int count = 0; count < ETH_ALEN; count++)
    {
	if (cptr > s)
	    *cptr++ = ':';
	sprintf(cptr, "%02X", binaddr[count]);
	cptr += 2;
    }
    *cptr = 0;

    return WvString("%s", s); // create a dynamic WvString
}


WvEncap WvEtherAddr::encap() const
{
    return WvEncap(WvEncap::Ethernet);
}


bool WvEtherAddr::isbroadcast() const
{
    for (int count = 0; count < ETH_ALEN; count++)
	if (binaddr[count] != 0xFF)
	    return false;
    return true;
}


const unsigned char *WvEtherAddr::rawdata() const
{
    return binaddr;
}


size_t WvEtherAddr::rawdata_len() const
{
    return ETH_ALEN;
}


sockaddr_bin *WvEtherAddr::sockaddr() const
{
    sockaddr_bin *sa = new sockaddr_bin;
    memset(sa, 0, sizeof(*sa));
    sa->sa_family = ARPHRD_ETHER;
    memcpy(sa->sa_data, binaddr, ETH_ALEN);
    return sa;
}


size_t WvEtherAddr::sockaddr_len() const
{
    return sizeof(sockaddr_bin);
}


WvString WvARCnetAddr::printable() const
{
    WvString s("  ");
    sprintf(s.edit(), "%02X", binaddr);
    return s;
}


WvEncap WvARCnetAddr::encap() const
{
    return WvEncap(WvEncap::ARCnet);
}


const unsigned char *WvARCnetAddr::rawdata() const
{
    return &binaddr;
}


size_t WvARCnetAddr::rawdata_len() const
{
    return 1;
}


sockaddr_bin *WvARCnetAddr::sockaddr() const
{
    sockaddr_bin *sa = new sockaddr_bin;
    memset(sa, 0, sizeof(*sa));
    sa->sa_family = ARPHRD_ARCNET;
    sa->sa_data[0] = binaddr;
    return sa;
}


size_t WvARCnetAddr::sockaddr_len() const
{
    return sizeof(sockaddr_bin);
}


/* create an IP address from a dotted-quad string.  Maybe someday we'll
 * support hostnames too with gethostbyname, but not yet.
 */
WvIPAddr::WvIPAddr(const char string[])
{
    const char *iptr, *nptr;
    unsigned char *cptr = binaddr;

    memset(binaddr, 0, 4);
    nptr = string;
    for (int count=0; count < 4 && nptr; count++)
    {
	iptr = nptr;
	nptr = strchr(iptr, '.');
	if (nptr) nptr++;
	*cptr++ = strtol(iptr, NULL, 10);
	if (!nptr) break;
    }
}


/* Generate a printable version of an IP address. */
WvString WvIPAddr::printable() const
{
    return WvString("%s.%s.%s.%s",
		    binaddr[0], binaddr[1], binaddr[2], binaddr[3]);
}


/* AND two IP addresses together (handle netmasks) */
WvIPAddr WvIPAddr::operator& (const WvIPAddr &a2) const
{
    unsigned char obin[4];
    
    for (int count=0; count<4; count++)
	obin[count] = binaddr[count] & a2.binaddr[count];
    return WvIPAddr(obin);
}


/* OR two IP addresses together (for broadcasts, etc) */
WvIPAddr WvIPAddr::operator| (const WvIPAddr &a2) const
{
    unsigned char obin[4];
    
    for (int count=0; count<4; count++)
	obin[count] = binaddr[count] | a2.binaddr[count];
    return WvIPAddr(obin);
}


/* XOR two IP addresses together (for binary operations) */
WvIPAddr WvIPAddr::operator^ (const WvIPAddr &a2) const
{
    unsigned char obin[4];
    
    for (int count=0; count<4; count++)
	obin[count] = binaddr[count] ^ a2.binaddr[count];
    return WvIPAddr(obin);
}


/* invert all the bits of an IP address (for useful binary operations) */
WvIPAddr WvIPAddr::operator~ () const
{
    unsigned char obin[4];
    
    for (int count=0; count<4; count++)
	obin[count] = ~binaddr[count];
    return WvIPAddr(obin);
}


/* add an integer value to an IP address:
 *   eg. 192.168.42.255 + 1 == 192.168.43.0
 */
WvIPAddr WvIPAddr::operator+ (int n) const
{
    __u32 newad = htonl(ntohl(s_addr()) + n);
    return WvIPAddr((unsigned char *)&newad);
}


WvIPAddr WvIPAddr::operator- (int n) const
{
    __u32 newad = htonl(ntohl(s_addr()) - n);
    return WvIPAddr((unsigned char *)&newad);
}


WvEncap WvIPAddr::encap() const
{
    return WvEncap(WvEncap::IPv4);
}


const unsigned char *WvIPAddr::rawdata() const
{
    return binaddr;
}


size_t WvIPAddr::rawdata_len() const
{
    return 4;
}


/* Generate a struct sockaddr (suitable for sendto()) from this IP
 * address.  Don't forget to delete it after you're done!
 */
sockaddr_bin *WvIPAddr::sockaddr() const
{
    sockaddr_in *sin = new sockaddr_in;
    
    memset(sin, 0, sizeof(*sin));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = s_addr();
    sin->sin_port = 0;
    return (sockaddr_bin *)sin;
}


size_t WvIPAddr::sockaddr_len() const
{
    return sizeof(sockaddr_in);
}


static char wvipnet[] = "WvIPNet";

WvIPNet::WvIPNet()
{
    addrtype = wvipnet;
}


WvIPNet::WvIPNet(const WvIPNet &_net)
	: WvIPAddr(_net), mask(_net.netmask())
{
    addrtype = wvipnet;
}


WvIPNet::WvIPNet(const char string[]) : WvIPAddr(string)
{
    char *maskptr;
    int bits;
    __u32 imask;

    addrtype = wvipnet;

    maskptr = strchr(string, '/');
    if (!maskptr)
    {
	mask = WvIPAddr("255.255.255.255");
	return;
    }
    
    maskptr++;
    
    if (strchr(maskptr, '.'))
	mask = WvIPAddr(maskptr);
    else
    {
	bits = atoi(maskptr);
	if (bits > 0)
	    imask = htonl(~(((__u32)1 << (32-bits)) - 1)); // see below
	else
	    imask = 0;
	mask = WvIPAddr((unsigned char *)&imask);
    }
}


WvIPNet::WvIPNet(const WvIPAddr &base, const WvIPAddr &_mask)
	: WvIPAddr(base), mask(_mask)
{
    addrtype = wvipnet;
}


WvIPNet::WvIPNet(const WvIPAddr &base, int bits)
	: WvIPAddr(base)
{
    __u32 imask;
    if (bits > 0) // <<32 is a bad idea!
	imask = htonl(~(((__u32)1 << (32-bits)) - 1));
    else
	imask = 0;
    mask = WvIPAddr((unsigned char *)&imask);
    addrtype = wvipnet;
}


WvString WvIPNet::printable() const
{
    return WvString("%s/%s", WvIPAddr::printable(), bits());
}


unsigned WvIPNet::WvHash() const
{
    return WvIPAddr::WvHash() + mask.WvHash();
}


bool WvIPNet::comparator(const WvAddr *a2) const
{
    return WvIPAddr::comparator(a2) && mask.comparator(&((WvIPNet *)a2)->mask);
}


void WvIPNet::include(const WvIPNet &addr)
{
    mask = mask & addr.mask & ~(*this ^ addr);
}


bool WvIPNet::includes(const WvIPNet &addr) const
{
    return ((addr.base() & mask) == network()
	    &&  mask == (mask & addr.mask));
}


int WvIPNet::bits() const
{
    int bits = 0;
    __u32 val = ntohl(mask.s_addr());
    
    do
    {
	bits += val >> 31;
    } while ((val <<= 1) & (1 << 31));
    
    return bits;
}


void WvIPNet::normalize()
{
    if (bits() > 0)
    {
	__u32 val = htonl(~(((__u32)1 << (32-bits())) - 1));
	mask = WvIPAddr((unsigned char *)&val);
    }
}


static char wvipportaddr[] = "WvIPPortAddr";


WvIPPortAddr::WvIPPortAddr()
{
    port = 0;
    addrtype = wvipportaddr;
}


WvIPPortAddr::WvIPPortAddr(const unsigned char _ipaddr[4], __u16 _port)
			: WvIPAddr(_ipaddr)
{
    port = _port;
    addrtype = wvipportaddr;
}


WvIPPortAddr::WvIPPortAddr(const WvIPAddr &_ipaddr, __u16 _port)
			: WvIPAddr(_ipaddr)
{
    port = _port;
    addrtype = wvipportaddr;
}


WvIPPortAddr::WvIPPortAddr(const char string[]) 
                              : WvIPAddr(string)
{
    const char *cptr = strchr(string, ':');
    if (!cptr)
	cptr = strchr(string, ' ');
    if (!cptr)
	cptr = strchr(string, '\t');
    
    port = cptr ? atoi(cptr+1) : 0;
    
    addrtype = wvipportaddr;
}


WvIPPortAddr::WvIPPortAddr(__u16 _port)
                              : WvIPAddr("0.0.0.0")
{
    port = _port;
    addrtype = wvipportaddr;
}


WvIPPortAddr::WvIPPortAddr(const char string[], __u16 _port)
                              : WvIPAddr(string)
{
    port = _port;
    addrtype = wvipportaddr;
}


/* Generate a printable version of an IP+Port Address. */
WvString WvIPPortAddr::printable() const
{
    return WvString("%s:%s", WvIPAddr::printable(), WvString(port));
}


/* Generate a struct sockaddr (suitable for sendto()) from this IP+Port
 * address.  Don't forget to delete it after you're done!
 */
sockaddr_bin *WvIPPortAddr::sockaddr() const
{
    sockaddr_in *sin = (sockaddr_in *)WvIPAddr::sockaddr();
    sin->sin_port = htons(port);
    return (sockaddr_bin *)sin;
}


unsigned WvIPPortAddr::WvHash() const
{
    return WvIPAddr::WvHash() + port;
}


bool WvIPPortAddr::comparator(const WvAddr *a2) const
{
    return WvIPAddr::comparator(a2) && port == ((WvIPPortAddr *)a2)->port;
}


