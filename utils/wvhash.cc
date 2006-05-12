#include "wvhash.h"

// Note: this hash function is case-insensitive since it ignores the
// bit in ASCII that defines case.  You may want to take advantage of this.
unsigned WvHash(const char *s)
{
    unsigned hash = 0, slide, andval;
    if (!s) return 0;
    
    slide = sizeof(hash)*8 - 5;
    andval = 0x1F << slide;
    
    while (*s)
	hash = (hash<<4) ^ (*(s++) & 0x1F) ^ ((hash & andval) >> slide);
    
    return hash;
}

unsigned WvHash(WvStringParm s)
{
    return !s ? 0 : WvHash((const char *)s);
}

// FIXME: does this suck?
unsigned WvHash(const int &i)
{
    return i;
}


unsigned WvHash(const void *p)
{
    return reinterpret_cast<unsigned>(p);
}

