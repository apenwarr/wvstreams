/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 */

/** /file
 * A UniConf hierarchical key path abstraction.
 */
#ifndef __UNICONFKEY_H
#define __UNICONFKEY_H

#include "wvstring.h"

/**
 * Represents a UniConf key which is a path in a hierarchy structured much
 * like the traditional Unix filesystem.
 * <p>
 * <ul>
 * <li>Segments in the path are delimited by slashes.</li>
 * <li>The root of the tree is identified by a single slash.</li>
 * <li>Keys are case insensitive yet preserve case information.</li>
 * <li>An initial slash is added if it was missing or if the
 *     string was empty.</li>
 * <li>Paired slashes are converted to single slashes.</li>
 * <li>Trailing slashes are discarded.</li>
 * </ul>
 * </p><p>
 * The following paths are equivalent when canonicalized:
 * <ul>
 * <li>/foo/key <em>(the canonical representation)</em></li>
 * <li>/Foo/Key <em>(also canonical but preserves case)</em></li>
 * <li>foo/key <em>(converted to /foo/key)<li>
 * <li>/foo//key <em>(converted to /foo/key)<li>
 * <li>/foo/key/ <em>(converted to /foo/key)<li>
 * </ul>
 * </p><p>
 * Keys that may contain slashes or nulls should be escaped in
 * some fashion prior to constructing a UniConfKey object.
 * Simply prefixing slashes with backslashes is inadequate
 * because UniConfKey does not invest any special meaning in
 * backslash.
 * </p>
 */
class UniConfKey
{
    WvString path;

public:
    static UniConfKey EMPTY; /*!< represents "/" (root) */
    static UniConfKey ANY;   /*!< represents "*" */

    /**
     * Constructs an empty UniConfKey (also known as root).
     */
    UniConfKey();

    /**
     * Constructs a UniConfKey from a string.
     * <p>
     * See the rules above for information about how the key string
     * is canonicalized.
     * </p>
     * @param key the key as a string
     */
    inline UniConfKey(WvStringParm key)
    {
        init(key);
    }

    /**
     * Constructs a UniConfKey from a string.
     * <p>
     * See the rules above for information about how the key string
     * is canonicalized.  This constructor only exists to help out the
     * C++ compiler with its automatic type conversions.
     * </p>
     * @param key the key as a string
     */
    inline UniConfKey(const char *key)
    {
        init(key);
    }   

    /**
     * Copies a UniConfKey.
     * @param other the key to copy
     */
    UniConfKey(const UniConfKey &other);

    /**
     * Constructs a UniConfKey by concatenating two keys.
     * @param path the initial part of the new path
     * @param key the tail of the new path
     */
    UniConfKey(const UniConfKey &path, const UniConfKey &key);

    /**
     * Appends a path to this path.
     * @param other the path
     */
    void append(const UniConfKey &other);

    /**
     * Prepends a path to this path.
     * @param other the path
     */
    void prepend(const UniConfKey &other);

    /**
     * Returns true if this path has zero segments (also known as root).
     * @return numsegments() == 0
     */
    bool isempty() const;

    /**
     * Returns the number of segments in this path.
     * <p>
     * The number of segments is equal to the number of slashes
     * in the path unless the path is "/" (the root), which has
     * zero segments.
     * </p>
     * @return the number of segments
     */
    int numsegments() const;

    /**
     * Returns the specified segment of the path.
     * @param i the segment index
     * @return the segment
     */
    UniConfKey segment(int i) const;

    /**
     * Returns the path formed by the n first segments of this path.
     * @param n the number of segments
     * @return the path
     */
    UniConfKey first(int n = 1) const;

    /**
     * Returns the path formed by the n last segments of this path.
     * @param n the number of segments
     * @return the path
     */
    UniConfKey last(int n = 1) const;

    /**
     * Returns the path formed by removing the first n segments of
     *   this path.
     * @param n the number of segments
     * @return the path
     */
    UniConfKey removefirst(int n = 1) const;

    /**
     * Returns the path formed by removing the last n segments of
     *   this path.
     * @param n the number of segments
     * @return the path
     */
    UniConfKey removelast(int n = 1) const;

    /**
     * Returns a range of segments.
     * @param i the first segment index, beginning if <= 0
     * @param j the last segment index, end if >= numsegments()
     * @return the path, empty if j <= i
     */
    UniConfKey range(int i, int j) const;

    /**
     * Returns the canonical string representation of the path.
     * <p>
     * If the UniConfKey was constructed in part or whole from
     * strings, then the string returned here will have the same
     * case information as those strings but the arrangement of
     * slashes may differ.  That is, the identity
     * <code>UniConfKey(string).printable() == key<code> <em>
     * does not hold</em>.
     * </p>
     * @return the path as a string
     */
    WvString printable() const;
    inline operator WvString() const
        { return printable(); }

    /**
     * Returns the path as a string without the leading slash.
     * @return the path as a string
     */
    WvString strip() const;

    /**
     * Assigns this path to equal another.
     * @param other the other path
     */
    UniConfKey &operator= (const UniConfKey &other);

    /**
     * Determines if two paths are equal.
     * Uses case-insensitive matching on the path string.
     * @param other the other path
     */
    bool operator== (const UniConfKey &other) const;
    inline bool operator!= (const UniConfKey &other) const
        { return ! (*this == other); }

    /**
     * Produces a total ordering of paths.
     * Uses case-insensitive matching on the path string.
     * @param other the other path
     */
    bool operator< (const UniConfKey &other) const;

    class Iter;

protected:
    void init(WvStringParm key);
};

/**
 * An iterator over the segments of a key.
 */
class UniConfKey::Iter
{
    const UniConfKey &key;
    int segment;
    int numsegments;
    
public:
    Iter(const UniConfKey &_key) :
        key(_key) { }

    void rewind()
    {
        segment = -1;
        numsegments = key.numsegments();
    }
    
    bool next()
    {
        segment += 1;
        return segment < numsegments;
    }
    
    UniConfKey operator*() const
    {
        return key.segment(segment);
    }
    inline UniConfKey operator()() const
    {
        return **this;
    }
};

#endif // __UNICONFKEY_H
