/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Implementation of a simple and efficient printable-string class.
 * 
 * It leaves out many of the notational conveniences provided by other
 * string classes, because they waste too much CPU time and space.
 * It does the one thing really missing from char* strings, that is,
 * dynamic buffer management.
 * 
 * The 'str' member is the actual (char*) string.  You should never
 * need to access it directly.
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


#define __WVS_FORM(n) WvStringParm __wvs_##n = WvFastString::null
#define WVSTRING_FORMAT_DECL WvStringParm __wvs_format, \
		WvStringParm __wvs_a0, \
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
class WvFastString;
class WvString;
class QString; // for operator QString()
class QCString;

// all WvFastString objects are const - they should _only_ be created
// automatically by automatic typecasting in parameter passing.  So let's
// create a handy alias.
typedef const WvFastString &   WvStringParm;



struct WvStringBuf
{
    size_t size;        // string length - if zero, use strlen!!
    unsigned links;	// number of WvStrings using this buf.
    char data[1];	// optional room for extra string data
};


// the _actual_ space taken by a WvStringBuf, without the data[] array
// (which is variable-sized, not really 1 byte)
#define WVSTRINGBUF_SIZE(s) (s->data - (char *)s)

/**
 * A WvFastString acts exactly like a WvString, but can take (const char *)
 * strings without needing to allocate any memory, thus making it faster.
 * 
 * When we copy to a normal WvString object, _then_ we allocate the memory.
 * If that never happens, we never need to allocate.
 * 
 * DON'T CREATE INSTANCES OF THIS!  It's mostly useful for parameter passing,
 * and for that you should use WvStringParm.  You can get yourself into _big_
 * trouble if you have an instance of a WvFastString created from a (char *)
 * object and then you modify the original (char *).
 * 
 * For almost all purposes, use WvString instead.  At worst, it's a bit slower.
 */
class WvFastString
{
    friend class WvString; // so WvString can access members of _other_ objects
    
protected:
    WvStringBuf *buf;
    char *str;
    
    // WvStringBuf used for char* strings that have not been cloned.
    static WvStringBuf nullbuf;
    
public:
    // a null string, converted to char* as "(nil)"
    static const WvFastString null;

    /**
     * Create an empty, NULL string.  In the past, these were dangerous
     * and could only be filled with operator= or setsize(); nowadays, NULL
     * strings are explicitly allowed, since it's useful to express the
     * difference between a zero-length string and a NULL result.
     */
    WvFastString();
    void setsize(size_t i);
    
    /**
     * Copy constructor.  We can safely copy from a normal WvString like this
     * too, since no special behaviour is required in this direction.  (Note
     * that copying from a WvFastString to a WvString _does_ require special
     * care!)
     */
    WvFastString(const WvFastString &s);
    WvFastString(const WvString &s);
    
    /**
     * Create a string out of a (char *)-style string _without_ copying any
     * memory.  It's fast, but we have to trust that the _str won't change
     * for the lifetime of our WvFastString.  That's usually safe, if you
     * didn't use WvFastString where you should have used a WvString.
     */
    WvFastString(const char *_str);
    
    /**
     * Create a string out of a Qt library QString.  If you use this,
     * you need to link with libwvqt.so.
     */
    WvFastString(const QString &s);
    WvFastString(const QCString &s);

    /**
     * NOTE: make sure that 32 bytes is big enough for your longest
     * int.  This is true up to at least 64 bits.
     */
    WvFastString(int i);
    
    /** when this is called, we assume output.str == NULL; it will be filled. */
    static void do_format(WvFastString &output, const char *format,
			  const WvFastString * const *a);
    
    
    /**
     * Now, you're probably thinking to yourself: Boy, does this ever
     * look ridiculous.  And indeed it does.  However, it is
     * completely type-safe and when functions are enabled, it
     * reduces automatically to its minimum possible implementation.
     * (ie. all extra comparisons with wv_null go away if the
     * parameter really _is_ wv_null, and that is the default!)
     *
     * I failed to find a way to optimize out the comparisons for
     * parameters that _are_ provided, however.
     *
     * There is a small problem, which is that only up to 20 (numbers
     * 0 to 19) additional arguments are allowed.  Luckily, no one has
     * ever used that many on one "printf"-type line in the history of
     * the world.
     */
    WvFastString(WVSTRING_FORMAT_DECL) 
    {
	const WvFastString *x[20];
	
	if (&__wvs_a0  != &null) x[ 0] = &__wvs_a0;
	if (&__wvs_a1  != &null) x[ 1] = &__wvs_a1;
	if (&__wvs_a2  != &null) x[ 2] = &__wvs_a2;
	if (&__wvs_a3  != &null) x[ 3] = &__wvs_a3;
	if (&__wvs_a4  != &null) x[ 4] = &__wvs_a4;
	if (&__wvs_a5  != &null) x[ 5] = &__wvs_a5;
	if (&__wvs_a6  != &null) x[ 6] = &__wvs_a6;
	if (&__wvs_a7  != &null) x[ 7] = &__wvs_a7;
	if (&__wvs_a8  != &null) x[ 8] = &__wvs_a8;
	if (&__wvs_a9  != &null) x[ 9] = &__wvs_a9;
	if (&__wvs_a10 != &null) x[10] = &__wvs_a10;
	if (&__wvs_a11 != &null) x[11] = &__wvs_a11;
	if (&__wvs_a12 != &null) x[12] = &__wvs_a12;
	if (&__wvs_a13 != &null) x[13] = &__wvs_a13;
	if (&__wvs_a14 != &null) x[14] = &__wvs_a14;
	if (&__wvs_a15 != &null) x[15] = &__wvs_a15;
	if (&__wvs_a16 != &null) x[16] = &__wvs_a16;
	if (&__wvs_a17 != &null) x[17] = &__wvs_a17;
	if (&__wvs_a18 != &null) x[18] = &__wvs_a18;
	if (&__wvs_a19 != &null) x[19] = &__wvs_a19;
	
	link(&nullbuf, NULL);
	do_format(*this, __wvs_format.str, x);
    }
    
    ~WvFastString();
    
    /*
     * Figure out the length of this string.  ==0 if NULL or empty.
     */
    size_t len() const;

protected:
    // this doesn't exist - it's just here to keep it from being auto-created
    // by stupid C++.
    WvFastString &operator= (const WvFastString &s2);
    
    // connect/disconnect ourselves from a WvStringBuf.
    void link(WvStringBuf *_buf, const char *_str);
    void unlink();
    
    // allocate new space for buffers - needed only by the (int i) constructor,
    // for now.
    WvStringBuf *alloc(size_t size);
    void newbuf(size_t size);
    
public:
    // string comparison
    bool operator== (WvStringParm s2) const;
    bool operator!= (WvStringParm s2) const;
    bool operator== (const char *s2) const;
    bool operator!= (const char *s2) const;
    
    /** the not operator is 'true' if string is empty */
    bool operator! () const;

    // pointer arithmetic
    const char *operator+ (int i) const
        { return str + i; }
    const char *operator- (int i) const
        { return str - i; }
    
    /** auto-convert WvString to (const char *), when needed. */
    operator const char*() const
        { return str; }
    
    /**
     * return a (const char *) for this string.  The typecast operator does
     * this automatically when needed, but sometimes (especially with varargs
     * like in printf()) that isn't convenient enough.
     */
    const char *cstr() const
        { return str; }
    
    /**
     * return a Qt library QString containing the contents of this string.
     * You need to link to libwvqt.so if you use this.
     */
    operator QString() const;
    
    /**
     * used to convert WvString to int, when needed.
     * we no longer provide a typecast, because it causes annoyance.
     */
    int num() const
        { return str ? atoi(str) : 0; }

    /** returns true if this string is null */
    bool isnull() const
        { return str == NULL; }
};


/**
 * WvString is an implementation of a simple and efficient
 * printable-string class. It leaves out many of the notational
 * conveniences provided by other string classes, because they waste
 * too much CPU time and space.
 *
 * It does the one thing really missing from char* strings, that is,
 * dynamic buffer management.
 *
 * When you copy one WvString to another, it does _not_ duplicate the
 * buffer; it just creates another pointer to it. To really duplicate
 * the buffer, call the unique() member function.
 *
 * To change the contents of a WvString, you need to run its edit()
 * member function, which executes unique() and then returns a char*
 * pointer to the WvString contents.
 *
 * The most annoying side-effect of this implementation is that if you
 * construct a WvString from a char* buffer or static string, WvString
 * won't duplicate it. Usually this is okay and much faster (for
 * example, if you just want to print a static string). However, if
 * you construct a WvString from a dynamic variable, changing the
 * dynamic variable will change the WvString unless you run unique()
 * or edit(). Worse still, deleting the dynamic variable will make
 * WvString act unpredictably.
 *
 * But it does cut out extra dynamic memory allocation for the most
 * common cases, and it almost always avoids manual 'new' and 'delete'
 * of string objects.
 */
class WvString : public WvFastString
{
public:
    // an empty string, converted to char* as ""
    static const WvString empty;
 
    WvString() {} // nothing special needed
    WvString(int i) : WvFastString(i) { } // nothing special
    
    /**
     * Magic copy constructor for "fast" char* strings.  When we copy from
     * a "fast" string to a real WvString, we might need to allocate memory
     * (equivalent to unique()) so the original char* can be safely changed
     * or destroyed.
     */
    WvString(const WvString &s)
    	{ copy_constructor(s); }
    WvString(const WvFastString &s)
        { copy_constructor(s); }
    
    /**
     * Create a WvString out of a char* string.  We always allocate memory
     * and make a copy here.  To avoid memory copies, you can (carefully)
     * use a WvFastString.  To just have quick parameter passing, use a
     * WvStringParm instead.
     */
    WvString(const char *_str);

    /**
     * Create a WvString out of a Qt library QString.  You have to link with
     * libwvqt.so if you want to use this.
     */
    WvString(const QString &);
    WvString(const QCString &);

    WvString(WVSTRING_FORMAT_DECL) : WvFastString(WVSTRING_FORMAT_CALL)
        { }
    
    void append(WvStringParm s);
    void append(WVSTRING_FORMAT_DECL)
        { append(WvString(WVSTRING_FORMAT_CALL)); }
    
    WvString &operator= (int i);
    WvString &operator= (const WvFastString &s2);
    WvString &operator= (const char *s2)
        { return *this = WvFastString(s2); }
    
    /** make the buf and str pointers owned only by this WvString. */
    WvString &unique();

    /** make the string editable, and return a non-const (char*) */
    char *edit()
        { return unique().str; }
    
protected:
    void copy_constructor(const WvFastString &s);

};


inline bool operator== (const char *s1, WvStringParm s2)
{
    return s2 == s1;
}


inline bool operator!= (const char *s1, WvStringParm s2)
{
    return s2 != s1;
}

#endif // __WVSTRING_H
