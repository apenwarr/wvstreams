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

/**
 * Specialization of WvBufferBase for unsigned char type
 * buffers intended for use with raw memory buffers.
 * Refines the interface to add support for untyped pointers.
 * Adds some useful string operations.
 */
class WvBufferBase<unsigned char> :
    public WvBufferBaseCommonImpl<unsigned char>
{
public:
    explicit WvBufferBase(WvBufferStore *store) :
        WvBufferBaseCommonImpl<unsigned char>(store) { }

    /**
     * Copies a WvString into the buffer, excluding the null-terminator.
     * @param str the string
     */
    void putstr(WvStringParm str);
    void putstr(WVSTRING_FORMAT_DECL)
        { putstr(WvString(WVSTRING_FORMAT_CALL)); }

    /**
     * Returns the entire buffer as a null-terminated WvString.
     * 
     * If the buffer contains null characters, they will seem to
     * prematurely terminate the string.
     * 
     * After this operation, ungettable() >= length of the string.
     * 
     * @return the buffer contents as a string
     */
    WvString getstr();

    /*** Get/put characters as integer values ***/

    /**
     * Returns a single character from the buffer as an int.
     * 
     * The same constraints apply as for get(1).
     * 
     * @return the character
     */
    int getch()
        { return int(get()); }

    /**
     * Puts a single character into the buffer as an int.
     * 
     * The same constraints apply as for alloc(1).
     * 
     * @param ch the character
     */
    void putch(int ch)
        { put((unsigned char)ch); }

    /**
     * Peeks a single character from the buffer as an int.
     * 
     * The same constraints apply as for peek(offset, 1).
     * 
     * @param offset the offset
     * @return the character
     */
    int peekch(int offset = 0)
        { return int(peek(offset)); }
    
    /**
     * Returns the number of characters that would have to be read
     * to find the first instance of the character.
     * @param ch the character
     * @return the number of bytes, or zero if the character is not
     *         in the buffer
     */
    size_t strchr(int ch);

    /**
     * Returns the number of leading buffer elements that match
     * any of those in the list.
     * @param bytelist the list bytes to search for
     * @param numbytes the number of bytes in the list
     * @return the number of leading buffer elements that match
     */
    size_t match(const void *bytelist, size_t numbytes)
        { return _match(bytelist, numbytes, false); }
        
    /**
     * Returns the number of leading buffer elements that match
     * any of those in the list.
     * @param chlist a string of characters to search for
     * @return the number of leading buffer elements that match
     */
    size_t match(const char *chlist)
        { return match(chlist, strlen(chlist)); }

    /**
     * Returns the number of leading buffer elements that do not
     * match any of those in the list.
     * @param bytelist the list bytes to search for
     * @param numbytes the number of bytes in the list
     * @return the number of leading buffer elements that don't match
     */
    size_t notmatch(const void *bytelist, size_t numbytes)
        { return _match(bytelist, numbytes, true); }

    /**
     * Returns the number of leading buffer elements that do not
     * match any of those in the list.
     * @param chlist a string of characters to search for
     * @return the number of leading buffer elements that don't match
     */
    size_t notmatch(const char *chlist)
        { return notmatch(chlist, strlen(chlist)); }

    /*** Overload put() and move() to accept void pointers ***/
    
    void put(unsigned char value)
        { WvBufferBaseCommonImpl<unsigned char>::put(value); }
    void put(const void *data, size_t count)
        { WvBufferBaseCommonImpl<unsigned char>::put(
            (const unsigned char*)data, count); }
    void move(void *data, size_t count)
        { WvBufferBaseCommonImpl<unsigned char>::move(
            (unsigned char*)data, count); }
    void poke(void *data, int offset, size_t count)
        { WvBufferBaseCommonImpl<unsigned char>::poke(
            (unsigned char*)data, offset, count); }

private:
    // moved here to avoid ambiguities between the match variants
    size_t _match(const void *bytelist, size_t numbytes, bool reverse);
};



/***** Declarations for some commonly used memory buffers *****/

/**
 * The in place raw memory buffer type.
 * Refines the interface to add support for untyped pointers.
 */
class WvInPlaceBuffer : public WvInPlaceBufferBase<unsigned char>
{
public:
    WvInPlaceBuffer(void *_data, size_t _avail, size_t _size,
        bool _autofree = false) :
        WvInPlaceBufferBase<unsigned char>((unsigned char*)_data,
            _avail, _size, _autofree) { }
    explicit WvInPlaceBuffer(size_t _size) :
        WvInPlaceBufferBase<unsigned char>(_size) { }
    WvInPlaceBuffer() :
        WvInPlaceBufferBase<unsigned char>() { }
    void reset(void *_data, size_t _avail, size_t _size,
        bool _autofree = false)
    {
        WvInPlaceBufferBase<unsigned char>::reset(
            (unsigned char*)_data, _avail, _size, _autofree);
    }
};

/**
 * The const in place raw memory buffer type.
 * Refines the interface to add support for untyped pointers.
 */
class WvConstInPlaceBuffer : public WvConstInPlaceBufferBase<unsigned char>
{
public:
    WvConstInPlaceBuffer(const void *_data, size_t _avail) :
        WvConstInPlaceBufferBase<unsigned char>(
            (const unsigned char*)_data, _avail) { }
    WvConstInPlaceBuffer() :
        WvConstInPlaceBufferBase<unsigned char>() { }
    void reset(const void *_data, size_t _avail)
    {
        WvConstInPlaceBufferBase<unsigned char>::reset(
            (const unsigned char*)_data, _avail);
    }
};

/**
 * The circular in place raw memory buffer type.
 * Refines the interface to add support for untyped pointers.
 */
class WvCircularBuffer : public WvCircularBufferBase<unsigned char>
{
public:
    WvCircularBuffer(void *_data, size_t _avail, size_t _size,
        bool _autofree = false) :
        WvCircularBufferBase<unsigned char>((unsigned char*)_data,
            _avail, _size, _autofree) { }
    explicit WvCircularBuffer(size_t _size) :
        WvCircularBufferBase<unsigned char>(_size) { }
    WvCircularBuffer() :
        WvCircularBufferBase<unsigned char>() { }
    void reset(void *_data, size_t _avail, size_t _size,
        bool _autofree = false)
    {
        WvCircularBufferBase<unsigned char>::reset(
            (unsigned char*)_data, _avail, _size, _autofree);
    }
};

/**
 * The base raw memory buffer type.
 */
typedef WvBufferBase<unsigned char> WvBuffer;

/**
 * The dynamically resizing raw memory buffer type.
 */
typedef WvDynamicBufferBase<unsigned char> WvDynamicBuffer;

/**
 * The empty raw memory buffer type.
 */
typedef WvEmptyBufferBase<unsigned char> WvEmptyBuffer;

/**
 * The raw memory buffer cursor type.
 */
typedef WvBufferCursorBase<unsigned char> WvBufferCursor;

/**
 * The raw memory buffer view type.
 */
typedef WvBufferViewBase<unsigned char> WvBufferView;

/**
 * A raw memory read-only buffer backed by a constant WvString
 */
class WvConstStringBuffer : public WvConstInPlaceBuffer
{
    WvString xstr;

public:
    /**
     * Creates a new buffer backed by a constant string.
     *
     * @param _str the string
     */
    explicit WvConstStringBuffer(WvStringParm _str);

    /**
     * Creates a new empty buffer backed by a null string.
     */
    WvConstStringBuffer();

    /**
     * Resets the buffer contents to a new string.
     *
     * @param _str the new string
     */
    void reset(WvStringParm _str);

    /**
     * Returns the string that backs the array
     *
     * @return the string
     */
    WvString str()
        { return xstr; }
};

#endif // __WVBUFFER_H
