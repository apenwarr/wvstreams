/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Small, efficient, type-safe hash table (also known as "dictionary")
 * class.  These are typically used to store a reasonably large number
 * of objects (in no particular order) and find them quickly when needed.
 * 
 * Two semi-distinct types of hash tables are supported:  tables and
 * dictionaries.
 * 
 * TABLE EXAMPLE:
 *       DeclareWvTable(WvString);
 *       int main()
 *       {
 *           WvString s("foo"), s2("blue"), s3("foo");
 *           WvStringTable t(10);  // suggested size: 10 elements
 *           t.add(&s); t.add(&s2);
 *           printf("%s %s\n", t[s]->str, t[s3]->str); // prints "foo" "foo"
 *           ...
 *       }
 * Here, the WvStrings "foo" and "blue" are stored in the table, and then
 * "foo" is looked up twice (both table searches return &s).  The suggested
 * table size is given as 10 elements; this is only a suggestion, and
 * places no limit on the number of elements (though if you have many more
 * than this, it will get slower).
 * 
 * To match an element, the WvString operator== function is used.  That
 * means this particular example is rather foolish since if you already
 * have the search string, you do not need to find it in the table.  Objects
 * with more specific == operators might have more luck.
 * 
 * 
 * DICTIONARY EXAMPLE:
 *       class IntStr
 *       {
 *           int *key;
 *           WvString data;
 *       }
 *       DeclareWvDict(IntStr, int, key[0]);
 * 
 *       ...
 *       IntStrDict d(100);
 * 
 * Here, we are creating a dictionary that stores strings indexed by
 * integers.  d[5] might return the address of IntStr number 5, which
 * in turn contains WvString number 5.  When matching elements in this case,
 * a comparison is only done on key[0] of the two elements; thus, it is
 * not the operator== of IntStr that is important, but rather the operator==
 * for int.  (In this case, the operator== of int is predefined.)
 * 
 * The only reason *key is declared as a pointer in this example is to
 * demonstrate how to use pointer-based keys with our syntax.  In this case
 * it would certainly make more sense to use "int key;" and
 * DeclareWvDict(IntStr, key).  Note though, that "int *key;" and
 * DeclareWvDict(IntStr, key) is almost certainly not what you want, since
 * it would compare the POINTERS, not their values.
 * 
 * 
 * NOTES:
 *    - g++ does templates badly, or we would use those.
 * 
 *    - This class acts a lot like the WvList class in wvlinklist.h,
 *        possibly because it is based on an array of WvLists.  You
 *        should see wvlinklist.h for many interesting usage notes.
 *        (We support iterators in exactly the same way that WvLists do.)
 * 
 */
#ifndef __WVHASHTABLE_H
#define __WVHASHTABLE_H

#include "wvlinklist.h"

// no need to #include wvstring.h just for this
class WvString;

// predefined hashing functions (note: string hashes are case-insensitive)
unsigned WvHash(const WvString &s);
unsigned WvHash(const char *s);

// this base class has some non-inlined functions that work for all
// data types.
class WvHashTable
{
protected:
    typedef bool Comparator(const void *, const void *);
    
    WvHashTable(unsigned _numslots);
    void setup()
        { /* default: do nothing */ }
    void shutdown()
        { /* default: do nothing */ }
    WvLink *prevlink(WvList *slots, const void *data,
		     unsigned hash, Comparator *comp);
    void *genfind(WvList *slots, const void *data,
		  unsigned hash, Comparator *comp);
public:
    unsigned numslots;
    WvList *slots;
    
    size_t count() const;

    // base class for the auto-declared hash table iterators
    class IterBase
    {
    public:
	WvHashTable *tbl;
	unsigned tblindex;
	WvLink *link;
	
	IterBase(WvHashTable &_tbl)
            { tbl = &_tbl; }
	void rewind()
            { tblindex = 0; link = &tbl->slots[0].head; }
	WvLink *next();
	WvLink *cur() const
            { return link; }
    };
};


// this is ugly.

#define __WvDict_base(_classname_, _type_, _ftype_, _field_, _extra_)	\
class _classname_ : public WvHashTable					\
{									\
protected:								\
    DeclareWvList(_type_);						\
									\
    unsigned hash(const _type_ *data)					\
	{ return WvHash((*data) _field_); }				\
    static bool comparator(const void *key, const void *elem)		\
        { return *(_ftype_ *)key == (*(_type_ *)elem) _field_; }	\
									\
public:									\
    _classname_(unsigned _numslots) : WvHashTable(_numslots)		\
        { slots = new _type_##List[numslots]; setup(); }		\
    									\
    _type_##List *sl()							\
	{ return (_type_##List *)slots; }				\
    									\
    ~##_classname_()							\
        { shutdown(); delete[] sl(); }					\
    									\
    void add(_type_ *data, bool auto_free)				\
        { sl()[hash(data) % numslots].append(data, auto_free); }	\
    									\
    _type_ *operator[] (const _ftype_ &key)				\
        { return (_type_ *)genfind(slots, &key, WvHash(key), comparator); } \
									\
    void remove(const _type_ *data)					\
    {									\
	unsigned h = hash(data);					\
	WvLink *l = prevlink(slots, &(*data) _field_, h, comparator);	\
	if (l && l->next) sl()[h % numslots].unlink_after(l);		\
    }									\
									\
    class Iter : public IterBase					\
    {									\
    public:								\
	Iter(_classname_ &_tbl) : IterBase(_tbl)			\
	    { }								\
	_type_ *data() const						\
	    { return (_type_ *)link->data; }				\
    };									\
    									\
public:									\
    _extra_								\
};


#define DeclareWvDict2(_type_, _ftype_, _field_, _extra_) \
	__WvDict_base(_type_##Dict, _type_, _ftype_, . _field_, _extra_)
#define DeclareWvDict(_type_, _ftype_, _field_) \
	DeclareWvDict2(_type_, _ftype_, _field_, )

#define DeclareWvTable2(_type_, _extra_) \
	__WvDict_base(_type_##Table, _type_, _type_, , _extra_)
#define DeclareWvTable(_type_) DeclareWvTable2(_type_, )


#endif // __WVHASHTABLE_H
