/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Implementation of a simple and efficient printable-string class.  It
 * leaves out many of the notational conveniences provided by other string
 * classes, because they waste too much CPU time and space.
 * 
 * It does the one thing really missing from char* strings, that is,
 * dynamic buffer management.
 * 
 * The '_st' member is the actual (char*) string.  Modify it at will.  You
 * can pass a 'maxlen' parameter to the constructor to make the '_st' buffer
 * larger than it initially needs to be.
 */
#ifndef __WVSTRING_H
#define __WVSTRING_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * 1 byte for terminating NUL, 4 more to kludge around libc5+efence
 * incompatibility with strcat().
 */
#define WVSTRING_EXTRA 5


#define __WVS_FORM(n) const WvString &__wvs_##n = WvString::wv_null
#define WVSTRING_FORMAT_DECL const WvString &__wvs_format, \
		const WvString &__wvs_a0, \
		__WVS_FORM( a1), __WVS_FORM( a2), __WVS_FORM( a3), \
		__WVS_FORM( a4), __WVS_FORM( a5), __WVS_FORM( a6), \
		__WVS_FORM( a7), __WVS_FORM( a8), __WVS_FORM( a9), \
		__WVS_FORM(a10), __WVS_FORM(a11), __WVS_FORM(a12), \
		__WVS_FORM(a13), __WVS_FORM(a14), __WVS_FORM(a15), \
		__WVS_FORM(a16), __WVS_FORM(a17), __WVS_FORM(a18), \
		__WVS_FORM(a19)
#define WVSTRING_FORMAT_CALL __wvs_format, __wvs_a0, \
		__wvs_a1, __wvs_a2, __wvs_a3, __wvs_a4, __wvs_a5, \
		__wvs_a6, __wvs_a7, __wvs_a8, __wvs_a9, __wvs_a10, \
		__wvs_a11, __wvs_a12, __wvs_a13, __wvs_a14, __wvs_a15, \
		__wvs_a16, __wvs_a17, __wvs_a18, __wvs_a19

class WvString
{
    void fillme(const char *__st)
    { 
	if (__st == NULL) { _st = NULL; return; }
	_st = new char[strlen(__st) + WVSTRING_EXTRA];
	strcpy(_st, __st); 
    }

    // when this is called, we assume output._st == NULL; it will be filled.
    static void do_format(WvString &output, char *format, const WvString **a);

public:
    char *_st;

    // just an empty string
    static const WvString wv_null;

    WvString()      // fill blank strings ASAP by operator= or setsize()
        { _st = NULL; }
    void setsize(size_t i)
        { if (_st) delete[] _st; _st = new char[i]; }
    WvString(const WvString &s) // Copy constructor
        { fillme(s._st); }
    WvString(const char *__st)
        { fillme(__st); }
    WvString(int i) // auto-render int 'i' into a string
        { _st = new char[16]; snprintf(_st, 16, "%d", i); }

    // Now, you are probably thinking to yourself: Boy, does this ever look
    // ridiculous.  And indeed it does.  However, it is completely type-safe
    // and when inline functions are enabled, it reduces automatically to its
    // minimum possible implementation.  (ie. all extra comparisons with
    // wv_null go away if the parameter really _is_ wv_null,
    // and that is the default!)
    //
    // I failed to find a way to optimize out the comparisons for parameters
    // that _are_ provided, however.
    //
    // There is a small problem, which is that only up to 20 (numbers 0 to
    // 19) additional arguments are allowed.  Luckily, no one has ever used
    // that many on one "printf"-type line in the history of the world.
    //
    WvString(WVSTRING_FORMAT_DECL)
        {
	    const WvString *x[20];
	    
	    if (&__wvs_a0  != &wv_null) x[ 0] = &__wvs_a0;
	    if (&__wvs_a1  != &wv_null) x[ 1] = &__wvs_a1;
	    if (&__wvs_a2  != &wv_null) x[ 2] = &__wvs_a2;
	    if (&__wvs_a3  != &wv_null) x[ 3] = &__wvs_a3;
	    if (&__wvs_a4  != &wv_null) x[ 4] = &__wvs_a4;
	    if (&__wvs_a5  != &wv_null) x[ 5] = &__wvs_a5;
	    if (&__wvs_a6  != &wv_null) x[ 6] = &__wvs_a6;
	    if (&__wvs_a7  != &wv_null) x[ 7] = &__wvs_a7;
	    if (&__wvs_a8  != &wv_null) x[ 8] = &__wvs_a8;
	    if (&__wvs_a9  != &wv_null) x[ 9] = &__wvs_a9;
	    if (&__wvs_a10 != &wv_null) x[10] = &__wvs_a10;
	    if (&__wvs_a11 != &wv_null) x[11] = &__wvs_a11;
	    if (&__wvs_a12 != &wv_null) x[12] = &__wvs_a12;
	    if (&__wvs_a13 != &wv_null) x[13] = &__wvs_a13;
	    if (&__wvs_a14 != &wv_null) x[14] = &__wvs_a14;
	    if (&__wvs_a15 != &wv_null) x[15] = &__wvs_a15;
	    if (&__wvs_a16 != &wv_null) x[16] = &__wvs_a16;
	    if (&__wvs_a17 != &wv_null) x[17] = &__wvs_a17;
	    if (&__wvs_a18 != &wv_null) x[18] = &__wvs_a18;
	    if (&__wvs_a19 != &wv_null) x[19] = &__wvs_a19;
	    
	    _st = NULL;
	    do_format(*this, __wvs_format._st, x);
	}

    ~WvString()
        { if (_st) delete[] _st; }

    // we need to be able to concatenate strings
    WvString operator+(const WvString &s2) const
    {
	WvString e;
	e._st = new char[strlen(_st) + strlen(s2._st) + WVSTRING_EXTRA];
	strcpy(e._st, _st);
	strcat(e._st, s2._st);
	return e;
    }
    
    WvString& operator= (const WvString &s2)
    {
	if (_st) delete[] _st;
	if (s2._st != _st)
	    fillme(s2._st);
	else
	    _st = NULL;
	return *this;
    }

    // string comparison
    bool operator== (const WvString &s2) const
	{ return (_st==s2._st) || (_st && s2._st && !strcmp(_st, s2._st)); }
    bool operator!= (const WvString &s2) const
	{ return (_st!=s2._st) && (!_st || !s2._st || strcmp(_st, s2._st)); }

    bool operator== (const char *s2) const
        { return (_st==s2) || (_st && s2 && !strcmp(_st, s2)); }
    bool operator!= (const char *s2) const
	{ return (_st!=s2) && (!_st || !s2 || strcmp(_st, s2)); }
    
    // not operator is 'true' if string is empty
    bool operator! () const
        { return !_st || !_st[0]; }

    // pointer arithmetic
    bool operator+ (int i) const
        { return _st + i; }
    bool operator- (int i) const
        { return _st - i; }
    
    // auto-convert WvString to (char *), when needed.
    operator const char*() const
        { return _st; }
    char *edit()
        { return _st; }
    
    // used to convert WvString to int, when needed.
    // we no longer provide a typecast, because it causes annoyance.
    int num() const
        { return atoi(_st); }
};


inline bool operator== (const char *s1, const WvString &s2)
{
    return s2 == s1;
}


inline bool operator!= (const char *s1, const WvString &s2)
{
    return s2 != s1;
}


#endif // __WVSTRING_H
