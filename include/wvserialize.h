/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Code to serialize objects into WvBufs, and more code to read WvBufs and
 * construct objects from them.
 */
#ifndef __WVSERIALIZE_H
#define __WVSERIALIZE_H

#include "wvbuf.h"
#include <stdint.h>
#include <netinet/in.h>

/**
 * Encode an object as an array of bytes and put it into a WvBuf.  This
 * function just calls an overloaded _wv_serialize() function.  There was
 * really no need for a template here at all, except for symmetry with
 * wv_deserialize() which does need one.
 */
template <typename T>
inline void wv_serialize(WvBuf &buf, T t)
{
    _wv_serialize(buf, t);
}


/**
 * A serializer for passing objects by reference.  This one is used 
 * if the _wv_serialize() for this type of object expects the object to
 * be passed by reference.  Otherwise, the default implementation is used.
 */
template <typename T>
inline void wv_serialize(WvBuf &buf, T &t)
{
    _wv_serialize(buf, t);
}


/** Serialize a 32-bit int in network byte order. */
inline void _wv_serialize(WvBuf &buf, int32_t i)
{
    int32_t i2 = htonl(i);
    buf.put(&i2, sizeof(i2));
}


/** Serialize a 32-bit unsigned int in network byte order. */
inline void _wv_serialize(WvBuf &buf, uint32_t i)
{
    _wv_serialize(buf, (int32_t)i);
}


/** Serialize a 16-bit int in network byte order. */
inline void _wv_serialize(WvBuf &buf, int16_t i)
{
    int16_t i2 = htons(i);
    buf.put(&i2, sizeof(i2));
}


/** Serialize a 16-bit unsigned int in network byte order. */
inline void _wv_serialize(WvBuf &buf, uint16_t i)
{
    _wv_serialize(buf, (int16_t)i);
}


/** Note: signed char, unsigned char, and char are all different types! */
inline void _wv_serialize(WvBuf &buf, char c)
{
    buf.put(&c, sizeof(c));
}


/** Note: signed char, unsigned char, and char are all different types! */
inline void _wv_serialize(WvBuf &buf, signed char c)
{
    _wv_serialize(buf, (char)c);
}


/** Note: signed char, unsigned char, and char are all different types! */
inline void _wv_serialize(WvBuf &buf, unsigned char c)
{
    _wv_serialize(buf, (char)c);
}


/**
 * Serialize a WvString. The string serializer is guaranteed to not insert
 * any nuls (character 0) into the output stream except for the
 * string-terminating one, which is always present.  This makes
 * deserialization easy.
 */
inline void _wv_serialize(WvBuf &buf, WvStringParm s)
{
    if (!s.isnull())
	buf.putstr(s);
    buf.put("", 1); // terminating nul
}




/** Deserialize an object.  See wv_deserialize(). */
template <typename T>
    T _wv_deserialize(WvBuf &buf);


/**
 * Deserialize a complex nested templated object.  See wv_deserialize().
 * 
 * This class is needed because partial template specialization only works
 * on classes, not on functions.  So in order to define a generic deserializer
 * for, say, WvList<T>, we have to have a class with a member function.  Sigh.
 */
template <typename T>
class WvDeserialize
{
public:
    static T go(WvBuf &buf)
	{ return _wv_deserialize<T>(buf); }
};


/**
 * Deserialize an object: read bytes from a buffer, and return an object
 * constructed from that.
 * 
 * Note that there is no default deserializer.  You have to specialize this
 * template for every data type you might want to deserialize.  We do define
 * some for a few standard C types.
 * 
 * Implementation note:
 * If you define a deserializer for your own type, name it _wv_deserialize()
 * (with the underscore).  If you're unlucky, you may need to define a
 * WvDeserialize class instead.
 * 
 * Note that if you have a data structure, you probably want to
 * wv_deserialize<MyType *>(buf) instead of wv_deserialize<MyType>(buf) to
 * avoid extra copies.  You'll have to define _wv_deserialize() appropriately,
 * of course.  Pointer-based _wv_deserialize() functions allocate memory,
 * so you'll have to 'delete' the returned object yourself.
 */
template <typename T>
inline T wv_deserialize(WvBuf &buf)
{
    return WvDeserialize<T>::go(buf);
}


/** Deserialize a 32-bit int in network byte order. */
template <>
inline int32_t _wv_deserialize<int32_t>(WvBuf &buf)
{
    if (buf.used() < sizeof(int32_t))
	return 0;
    else
	return ntohl(*(int32_t *)buf.get(sizeof(int32_t)));
}

/** Deserialize a 32-bit unsigned int in network byte order. */
template <>
inline uint32_t _wv_deserialize<uint32_t>(WvBuf &buf)
{
    return (uint32_t)_wv_deserialize<int32_t>(buf);
}


/** Deserialize a 16-bit int in network byte order. */
template <>
inline int16_t _wv_deserialize<int16_t>(WvBuf &buf)
{
    if (buf.used() < sizeof(int16_t))
	return 0;
    else
	return ntohs(*(int16_t *)buf.get(sizeof(int16_t)));
}

/** Deserialize a 16-bit unsigned int in network byte order. */
template <>
inline uint16_t _wv_deserialize<uint16_t>(WvBuf &buf)
{
    return (uint16_t)_wv_deserialize<int16_t>(buf);
}


/** Deserialize a single character. */
template <>
inline char _wv_deserialize<char>(WvBuf &buf)
{
    if (buf.used() < sizeof(char))
	return 0;
    else
	return *(char *)buf.get(sizeof(char));
}

/** Deserialize a single character (signed char != char). */
template <>
inline signed char _wv_deserialize<signed char>(WvBuf &buf)
{
    return (signed char)_wv_deserialize<char>(buf);
}

/** Deserialize a single unsigned character (unsigned char != char). */
template <>
inline unsigned char _wv_deserialize<unsigned char>(WvBuf &buf)
{
    return (unsigned char)_wv_deserialize<char>(buf);
}


/**
 * Deserialize a WvString.  Stops at (and includes) the terminating nul
 * (zero) character.  Serialized WvStrings are guaranteed not to contain nul
 * except as the last character.
 */
template <>
extern WvString _wv_deserialize<WvString>(WvBuf &buf);

#endif // __WVSERIALIZE_H
