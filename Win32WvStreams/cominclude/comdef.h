#ifndef __COMDEF_H
#define __COMDEF_H

// FIXME: hey, that's not "smart"!
template<typename T> class CComPtr { };
#define _COM_SMARTPTR_TYPEDEF(x, y) class x; typedef x *x##Ptr

// FIXME
extern CLSID xblah;
#define __uuidof(x) (xblah)

#endif // _COMDEF_H
