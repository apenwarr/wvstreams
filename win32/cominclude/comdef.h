#ifndef __COMDEF_H
#define __COMDEF_H

//#warning "Using hideously broken fake comdef.h"

class WvComSmartBase
{
public:
    void *p;
};


// FIXME: completely untested and random
template <typename I>
class WvComSmart : public WvComSmartBase
{
public:
    WvComSmart(IUnknown *ptr = 0, bool addref = false)
    {
	p = ptr;
    }
    
    WvComSmart(const WvComSmartBase &b)
    { 
	p = b.p;
    }
    
    bool operator== (const void *b) const
    {
	return p == b;
    }
    
    bool operator!= (const void *b) const
    {
	return p != b;
    }
    
    I *operator-> ()
    {
	return (I *)p;
    }
    
    operator I* ()
    {
	return (I *)p;
    }
    
    I **operator& ()
    {
	return (I **)&p;
    }
};

#define _COM_SMARTPTR_TYPEDEF(x, y) typedef WvComSmart<x> x##Ptr;

// FIXME
extern CLSID xblah;
_COM_SMARTPTR_TYPEDEF(IUnknown, xblah);
_COM_SMARTPTR_TYPEDEF(IDispatch, xblah);

class _com_error
{
public:
    const char *ErrorMessage();
};

#define __uuidof(x) (xblah)

#endif // _COMDEF_H
