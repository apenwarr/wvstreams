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
 * The 'str' member is the actual (char*) string.  Modify it at will.  You
 * can pass a 'maxlen' parameter to the constructor to make the 'str' buffer
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
    void fillme(const char *_str)
    { 
	if (_str == NULL) { str = NULL; return; }
	str = new char[strlen(_str) + WVSTRING_EXTRA];
	strcpy(str, _str); 
    }

    // when this is called, we assume output.str == NULL; it will be filled.
    static void do_format(WvString &output, char *format, const WvString **a);

public:
    char *str;

    // just an empty string
    static const WvString wv_null;
    
    WvString()      // fill blank strings ASAP by operator= or setsize()
        { str = NULL; }
    void setsize(size_t i)
        { if (str) delete[] str; str = new char[i]; }
    WvString(const WvString &s) // Copy constructor
        { fillme(s.str); }
    WvString(const char *_str)
        { fillme(_str); }
    WvString(int i) // auto-render int 'i' into a string
        { str = new char[16]; snprintf(str, 16, "%d", i); }

    // Now, you are probably thinking to yourself, "Boy, does this ever look
    // ridiculous."  And indeed it does.  However, it is completely type-safe
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
	    
	    str = NULL;
	    do_format(*this, __wvs_format.str, x);
	}
    
    
    ~WvString()
        { if (str) delete[] str; }

    // we need to be able to concatenate strings
    WvString operator+(const WvString &s2) const
    {
	WvString e;
	e.str = new char[strlen(str) + strlen(s2.str) + WVSTRING_EXTRA];
	strcpy(e.str, str);
	strcat(e.str, s2.str);
	return e;
    }
    
    WvString& operator= (const WvString &s2)
    {
	if (str) delete[] str;
	if (s2.str != str)
	    fillme(s2.str);
	else
	    str = NULL;
	return *this;
    }

    // string comparison
    bool operator== (const WvString &s2) const
	{ return (str==s2.str) || (str && s2.str && !strcmp(str, s2.str)); }
    bool operator!= (const WvString &s2) const
	{ return (str!=s2.str) && str && s2.str && strcmp(str, s2.str); }
    
    // auto-convert WvString to int, when needed.
    operator int() const
        { return atoi(str); }
};

#endif // __WVSTRING_H
