/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
#ifndef __WVCALLBACK_H
#define __WVCALLBACK_H

#include "wvfunctor.h"

// DeclareWvCallback is how you create a new type of callback.  The options
// are:
//    n - the number of parameters the callback takes (only 0..4 are allowed)
//    ret - the return value of the callback
//    type - the name of the callback type
//    parms... - the types of the parameters.  There are 'n' of these.
// 
// Example:
//     DeclareWvCallback(3, void, TriCallback, Object*, Blob&, bool);
//     
// NOTE!!
// 
// For some reason, running this inside a class definition causes an internal
// compiler error (g++ 2.95.4).  So you'll have to declare your callbacks
// at the top level, rather than inside your class where it might seem to
// make more sense.  Oh well, at least it's less typing that way...
// 
#define DeclareWvCallback(n, ret, type, parms...) \
    typedef WvFunctor<ret, ## parms> type;


// Use wvcallback() to actually generate a new callback object, given the
// callback type, object type, and member function.  The callback type needs
// to have been declared earlier with DeclareWvCallback.
// 
// Example:
//     CallBackType cb = wvcallback(CallbackType, myobject, MyObject::Func);
//     
// You can pass the result of this call to anything that's expecting a
// callback of the appropriate type; you don't need to actually create the
// object first.
// 
#define wvcallback(cbname, instance, func) \
    cbname(& func, & instance)


// Some types of callbacks are so common we'll just declare them here.

DeclareWvCallback(0, void, VoidCallback);


#endif // __WVCALLBACK_H
