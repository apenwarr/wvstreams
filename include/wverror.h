/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A class for managing error numbers and strings.
 */ 
#ifndef __WVERROR_H
#define __WVERROR_H

#include "wvstring.h"

/**
 * A class for managing error numbers and strings.
 *
 * It can have either a system error value, like those defined
 * in errno.h, or an arbitrary error string.  In either case, it
 * can return a string representation of the error message.
 */
class WvError
{
protected:
    int errnum;
    WvString errstring;

public:
    WvError()
        { noerr(); }
    virtual ~WvError();

    /**
     * By default, returns true if geterr() == 0.
     * Might be overridden so that isok() == false even though no
     * error code has been specified.
     */
    virtual bool isok() const
        { return errnum == 0; }

    /**
     * If isok() is false, return the system error number corresponding to
     * the error, -1 for a special error string (which you can obtain with
     * errstr()) or 0 on end of file.  If isok() is true, returns an
     * undefined number.
     */ 
    virtual int geterr() const
        { return errnum; }
    virtual WvString errstr() const;
    
    /**
     * Set the errnum variable -- we have an error.  If called more than
     * once, seterr() doesn't change the error code away from the previous
     * one.  That way, we remember the _original_ cause of our problems.
     * 
     * Subclasses may want to override seterr(int) to shut themselves down
     * (eg. WvStream::close()) when an error condition is set.
     * 
     * Note that seterr(WvString) will call seterr(-1).
     */
    virtual void seterr(int _errnum);
    void seterr(WvStringParm specialerr);
    void seterr(WVSTRING_FORMAT_DECL)
        { seterr(WvString(WVSTRING_FORMAT_CALL)); }
    void seterr(const WvError &err);
    
    /** Reset our error state - there's no error condition anymore. */
    void noerr()
        { errnum = 0; errstring = WvString::null; }
};


#endif // __WVERROR_H
