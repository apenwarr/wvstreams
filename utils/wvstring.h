/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 * 
 * Implementation of a simple and efficient printable-string class.  It
 * leaves out many of the notational conveniences provided by other string
 * classes, because they waste too much CPU time and space.
 * 
 * It does the one thing really missing from char* strings, that is,
 * dynamic buffer management.
 * 
 * The 'str' member is the actual (char*) string.  You should never need
 * to access it directly.
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


#define __WVS_FORM(n) const WvString &__wvs_##n = __wvs_n
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

struct WvStringBuf;
class WvString;

// WvStringBuf used for char* strings that have not been cloned.
extern WvStringBuf __wvs_nb;

// just an empty string
extern const WvString __wvs_n;


struct WvStringBuf
{
    size_t size;        // string length - if zero, use strlen!!
    unsigned links;	// number of WvStrings using this buf.
    char data[1];	// optional room for extra string data
};


// the _actual_ space taken by a WvStringBuf, without the data[] array
// (which is variable-sized, not really 1 byte)
#define WVSTRINGBUF_SIZE(s) (s->data - (char *)s)


class WvString
{
    WvStringBuf *buf;
    char *str;
    
    void unlink();
    void link(WvStringBuf *_buf, const char *_str);
    
    WvStringBuf *alloc(size_t size);
    void newbuf(size_t size);

public:
    WvString();      // fill blank strings later with operator= or setsize()
    void setsize(size_t i);
    WvString(const WvString &s); // Copy constructor
    WvString(const char *_str);

    // NOTE: make sure that 32 bytes is big enough for your longest int.
    // This is true up to at least 64 bits.
    WvString(int i); // auto-render int 'i' into a string

    // when this is called, we assume output.str == NULL; it will be filled.
    static void do_format(WvString &output, char *format, const WvString **a);
    
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
	
	if (&__wvs_a0  != &__wvs_n) x[ 0] = &__wvs_a0;
	if (&__wvs_a1  != &__wvs_n) x[ 1] = &__wvs_a1;
	if (&__wvs_a2  != &__wvs_n) x[ 2] = &__wvs_a2;
	if (&__wvs_a3  != &__wvs_n) x[ 3] = &__wvs_a3;
	if (&__wvs_a4  != &__wvs_n) x[ 4] = &__wvs_a4;
	if (&__wvs_a5  != &__wvs_n) x[ 5] = &__wvs_a5;
	if (&__wvs_a6  != &__wvs_n) x[ 6] = &__wvs_a6;
	if (&__wvs_a7  != &__wvs_n) x[ 7] = &__wvs_a7;
	if (&__wvs_a8  != &__wvs_n) x[ 8] = &__wvs_a8;
	if (&__wvs_a9  != &__wvs_n) x[ 9] = &__wvs_a9;
	if (&__wvs_a10 != &__wvs_n) x[10] = &__wvs_a10;
	if (&__wvs_a11 != &__wvs_n) x[11] = &__wvs_a11;
	if (&__wvs_a12 != &__wvs_n) x[12] = &__wvs_a12;
	if (&__wvs_a13 != &__wvs_n) x[13] = &__wvs_a13;
	if (&__wvs_a14 != &__wvs_n) x[14] = &__wvs_a14;
	if (&__wvs_a15 != &__wvs_n) x[15] = &__wvs_a15;
	if (&__wvs_a16 != &__wvs_n) x[16] = &__wvs_a16;
	if (&__wvs_a17 != &__wvs_n) x[17] = &__wvs_a17;
	if (&__wvs_a18 != &__wvs_n) x[18] = &__wvs_a18;
	if (&__wvs_a19 != &__wvs_n) x[19] = &__wvs_a19;
	
	buf = NULL;
	do_format(*this, __wvs_format.str, x);
    }
    
    ~WvString();
    
    void append(const WvString &s);
    void append(WVSTRING_FORMAT_DECL);
    size_t len() const;

    WvString &operator= (const WvString &s2);
    
    // make the buf and str pointers owned only by this WvString.
    WvString &unique();

    // string comparison
    bool operator== (const WvString &s2) const;
    bool operator!= (const WvString &s2) const;
    bool operator== (const char *s2) const;
    bool operator!= (const char *s2) const;
    
    // not operator is 'true' if string is empty
    bool operator! () const;

    // pointer arithmetic
    const char *operator+ (int i) const
        { return str + i; }
    const char *operator- (int i) const
        { return str - i; }
    
    // auto-convert WvString to (const char *), when needed.
    operator const char*() const
        { return str; }
    
    // make the string editable, and return a non-const (char*)
    char *edit()
        { return unique().str; }
    
    // used to convert WvString to int, when needed.
    // we no longer provide a typecast, because it causes annoyance.
    int num() const
        { return atoi(str); }
    
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
