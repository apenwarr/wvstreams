/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Specializations of the generic buffering API and a few new buffers.
 */
#ifndef __WVBUFFER_H
#define __WVBUFFER_H
 
#include "wvbufferbase.h"
#include "wvstring.h"

/***** Specialization for 'unsigned char' buffers *****/

class WvBufferBase<unsigned char> :
    public WvBufferBaseCommonImpl<unsigned char>
{
public:
    WvBufferBase(WvBufferStore *store) :
        WvBufferBaseCommonImpl<unsigned char>(store) { }

    /**
     * Copies a WvString into the buffer, excluding the null-terminator.
     */
    void putstr(WvStringParm str);
    void putstr(WVSTRING_FORMAT_DECL)
        { putstr(WvString(WVSTRING_FORMAT_CALL)); }

    /**
     * Returns the entire buffer as a null-terminated WvString.
     * If the buffer contains null characters, they will seem to
     *   terminate the string.
     * The returned string is only valid until the next non-const
     *   buffer member is called.  Copy the string if you need to
     *   keep it for longer than that.
     *
     * After this operation, ungettable() >= length of the string.
     */
    WvFastString getstr();

    /**
     * Get/put characters as integer values.
     */
    inline int getch() { return int(get()); }
    inline void putch(int ch) { put((unsigned char)ch); }
    inline int peekch(int offset = 0) { return int(peek(offset)); }
    
    /**
     * Returns the number of bytes that would have to be read to find
     * the first character 'ch', or zero if 'ch' is not in the buffer.
     */
    size_t strchr(int ch);

    /**
     * Returns the number of leading bytes that match any in the list.
     */
    inline size_t match(const void *bytelist, size_t numbytes)
        { return _match(bytelist, numbytes, false); }
    size_t match(const char *chlist)
        { return match(chlist, strlen(chlist)); }

    /**
     * Returns the number of leading bytes that do not match any in the list.
     */
    inline size_t notmatch(const void *bytelist, size_t numbytes)
        { return _match(bytelist, numbytes, true); }
    size_t notmatch(const char *chlist)
        { return notmatch(chlist, strlen(chlist)); }

    /**
     * Overload put() and move() to accept void pointers.
     */
    inline void put(unsigned char value)
        { WvBufferBaseCommonImpl<unsigned char>::put(value); }
    inline void put(const void *data, size_t count)
        { WvBufferBaseCommonImpl<unsigned char>::put(
            (const unsigned char*)data, count); }
    inline void move(void *data, size_t count)
        { WvBufferBaseCommonImpl<unsigned char>::move(
            (unsigned char*)data, count); }
    inline void poke(void *data, int offset, size_t count)
        { WvBufferBaseCommonImpl<unsigned char>::poke(
            (unsigned char*)data, offset, count); }

private:
    // moved here to avoid ambiguities between the match variants
    size_t _match(const void *bytelist, size_t numbytes, bool reverse);
};


/**
 * Declarations for some commonly used raw byte memory buffers.
 */
class WvInPlaceBuffer : public WvInPlaceBufferBase<unsigned char>
{
public:
    inline WvInPlaceBuffer(void *_data, size_t _avail, size_t _size,
        bool _autofree = false) :
        WvInPlaceBufferBase<unsigned char>((unsigned char*)_data,
            _avail, _size, _autofree) { }
    inline WvInPlaceBuffer(size_t _size) :
        WvInPlaceBufferBase<unsigned char>(_size) { }
    inline WvInPlaceBuffer() :
        WvInPlaceBufferBase<unsigned char>() { }
    inline void reset(void *_data, size_t _avail, size_t _size,
        bool _autofree = false)
    {
        WvInPlaceBufferBase<unsigned char>::reset(
            (unsigned char*)_data, _avail, _size, _autofree);
    }
};

class WvConstInPlaceBuffer : public WvConstInPlaceBufferBase<unsigned char>
{
public:
    inline WvConstInPlaceBuffer(const void *_data, size_t _avail) :
        WvConstInPlaceBufferBase<unsigned char>(
            (const unsigned char*)_data, _avail) { }
    inline WvConstInPlaceBuffer() :
        WvConstInPlaceBufferBase<unsigned char>() { }
    inline void reset(const void *_data, size_t _avail)
    {
        WvConstInPlaceBufferBase<unsigned char>::reset(
            (const unsigned char*)_data, _avail);
    }
};

typedef WvInPlaceBuffer WvMiniBuffer;

typedef WvBufferBase<unsigned char> WvBuffer;

typedef WvDynamicBufferBase<unsigned char> WvDynamicBuffer;

typedef WvEmptyBufferBase<unsigned char> WvEmptyBuffer;

typedef WvBufferCursorBase<unsigned char> WvBufferCursor;


/***** A read-only buffer backed by a constant WvString *****/

class WvConstStringBuffer : public WvConstInPlaceBuffer
{
    WvString xstr;

public:
    /**
     * Creates a new buffer backed by a constant string.
     *   str - the string to use
     */
    WvConstStringBuffer(WvStringParm _str);

    /**
     * Creates a new empty buffer.
     */
    WvConstStringBuffer();

    /**
     * Resets the buffer contents to a new string.
     */
    void reset(WvStringParm _str);

    /**
     * Returns the string.
     */
    WvString str()
        { return xstr; }
};

#endif // __WVBUFFER_H
