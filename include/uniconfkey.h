/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniConfKeys are paths in the UniConf hierarchy.
 */
#ifndef __UNICONFKEY_H
#define __UNICONFKEY_H

#include "wvstring.h"
#include "wvlinklist.h"

/**
 * Represents a UniConf key which is a path in a hierarchy structured much
 * like the traditional Unix filesystem.
 * 
 * - Segments in the path are delimited by slashes.
 * - The root of the tree is identified by a single slash.
 * - Keys are case insensitive yet preserve case information.
 * - An initial slash is added if it was missing or if the string was empty.
 * - Paired slashes are converted to single slashes.
 * - Trailing slashes are discarded.
 * 
 * The following paths are equivalent when canonicalized:
 * 
 * - /foo/key (the canonical representation)
 * - /Foo/Key (also canonical but preserves case)
 * - foo/key (converted to /foo/key)
 * - /foo//key (converted to /foo/key)
 * - /foo/key/ (converted to /foo/key)
 * 
 * Keys that may contain slashes or nulls should be escaped in some fashion
 * prior to constructing a UniConfKey object. Simply prefixing slashes with
 * backslashes is inadequate because UniConfKey does not give any special
 * meaning to backslash.
 */
class UniConfKey
{
    WvString path;

public:
    static UniConfKey EMPTY; /*!< represents "/" (root) */
    static UniConfKey ANY;   /*!< represents "*" */

    /**
     * Constructs an empty UniConfKey (the 'root').
     */
    UniConfKey();

    /**
     * Constructs a UniConfKey from a string.
     * 
     * See the rules above for information about how the key string
     * is canonicalized.
     * 
     * @param key the key as a string
     */
    UniConfKey(WvStringParm key)
        { init(key); }

    /**
     * Constructs a UniConfKey from a string.
     * 
     * See the rules above for information about how the key string
     * is canonicalized.  This constructor only exists to help out the
     * C++ compiler with its automatic type conversions.
     * 
     * @param key the key as a string
     */
    UniConfKey(const char *key)
        { init(key); }   
    
    /**
     * Constructs a UniConfKey from an int.
     */
    UniConfKey(int key)
        { init(key); }

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
     * Returns true if the key contains a wildcard.
     */
    bool iswild() const;

    /**
     * Returns the number of segments in this path.
     * 
     * The number of segments is equal to the number of slashes
     * in the path unless the path is "/" (the root), which has
     * zero segments.
     * 
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
     * 
     * If the UniConfKey was constructed in part or whole from
     * strings, then the string returned here will have the same
     * case information as those strings but the arrangement of
     * slashes may differ.  That is, the identity
     * <code>UniConfKey(string).printable() == key<code> 
     * does not hold.
     * 
     * @return the path as a string
     */
    WvString printable() const;
    inline operator WvString() const
        { return printable(); }

    /**
     * Assigns this path to equal another.
     * @param other the other path
     */
    UniConfKey &operator= (const UniConfKey &other);

    /**
     * Compares two paths lexicographically.
     * Uses case-insensitive matching on the path string to produce
     * a total ordering of all paths.
     * @param other the other path
     * @return 0 if *this == other, < 0 if *this < other, else > 0
     */
    int compareto(const UniConfKey &other) const;

    /**
     * Determines if the key matches a pattern.
     * Patterns are simply keys that may have path segments consiting
     * entirely of "*".  Using wildcards to represent part of a
     * segment or optional segments is not currently supported.
     * @param pattern the pattern
     * @return true if the key matches, false otherwise
     */
    bool matches(const UniConfKey &pattern) const;

    /**
     * Determines if two paths are equal.
     * @param other the other path
     * @return true in that case
     */
    inline bool operator== (const UniConfKey &other) const
        { return compareto(other) == 0; }
        
    /**
     * Determines if two paths are unequal.
     * @param other the other path
     * @return true in that case
     */
    inline bool operator!= (const UniConfKey &other) const
        { return ! (*this == other); }

    /**
     * Determines if this path precedes the other lexicographically.
     * @param other the other path
     * @return true in that case
     */
    bool operator< (const UniConfKey &other) const
        { return compareto(other) < 0; }

    class Iter;

protected:
    void init(WvStringParm key);
};


DeclareWvList(UniConfKey);

/**
 * An iterator over the segments of a key.
 */
class UniConfKey::Iter
{
    const UniConfKey &key;
    int seg, max;
    UniConfKey curseg;
    
public:
    Iter(const UniConfKey &_key) : key(_key) 
        { }

    void rewind()
        { seg = -1; max = key.numsegments(); }
    
    bool cur()
        { return seg >= 0 && seg < max; }
    
    bool next()
        { seg++; curseg = key.segment(seg); return cur(); }
    
    const UniConfKey *ptr() const
        { return &curseg; }
    
    WvIterStuff(const UniConfKey);
};

#endif // __UNICONFKEY_H
