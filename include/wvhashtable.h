/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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
 *    - This class acts a lot like the WvList class in wvlinklist.h,
 *        possibly because it is based on an array of WvLists.  You
 *        should see wvlinklist.h for many interesting usage notes.
 *        (We support iterators in exactly the same way that WvLists do.)
 * 
 */
#ifndef __WVHASHTABLE_H
#define __WVHASHTABLE_H

#include "wvlinklist.h"
#include "wvstring.h"

// predefined hashing functions (note: string hashes are case-insensitive)
unsigned WvHash(WvStringParm s);
unsigned WvHash(const char *s);
unsigned WvHash(const int &i);

// this base class has some non-inlined functions that work for all
// data types.
class WvHashTableBase
{
protected:
    typedef bool Comparator(const void *, const void *);
    
    WvHashTableBase(unsigned _numslots);
    WvHashTableBase(const WvHashTableBase &t); // copy constructor - not defined anywhere!
    WvHashTableBase& operator= (const WvHashTableBase &t);
    void setup()
        { /* default: do nothing */ }
    void shutdown()
        { /* default: do nothing */ }
    WvLink *prevlink(WvListBase *slots, const void *data,
		     unsigned hash, Comparator *comp);
    void *genfind(WvListBase *slots, const void *data,
		  unsigned hash, Comparator *comp);
public:
    unsigned numslots;
    WvListBase *slots;
    
    size_t count() const;

    // base class for the auto-declared hash table iterators
    class IterBase
    {
    public:
	WvHashTableBase *tbl;
	unsigned tblindex;
	WvLink *link;
	
	IterBase(WvHashTableBase &_tbl)
            { tbl = &_tbl; }
	void rewind()
            { tblindex = 0; link = &tbl->slots[0].head; }
	WvLink *next();
	WvLink *cur() const
            { return link; }
    };
};


// this used to be ugly, but now it's just kind of weird and hacky.

typedef const void *WvFieldPointer(const void *obj);

template <class _type_, class _ftype_, WvFieldPointer *fptr>
class WvHashTable : public WvHashTableBase
{
protected:
    //static const _ftype_ *fptr(const _type_ *obj)
    //    { return (const _ftype_*)(((const char *)obj) + _fieldofs_); }
    unsigned hash(const _type_ *data)
	{ return WvHash(*(const _ftype_ *)fptr(data)); }
    static bool comparator(const void *key, const void *elem)
        { return *(_ftype_ *)key == *(const _ftype_ *)fptr((const _type_ *)elem); }

public:
    WvHashTable(unsigned _numslots) : WvHashTableBase(_numslots)
        { slots = new WvList<_type_>[numslots]; setup(); }

    WvList<_type_> *sl()
	{ return (WvList<_type_> *)slots; }

    ~WvHashTable()
        { shutdown(); delete[] sl(); }

    void add(_type_ *data, bool auto_free)
        { sl()[hash(data) % numslots].append(data, auto_free); }

    _type_ *operator[] (const _ftype_ &key)
        { return (_type_ *)genfind(slots, &key, WvHash(key), comparator); }

    void remove(const _type_ *data)
    {
	unsigned h = hash(data);
	WvLink *l = prevlink(slots, fptr(data), h, comparator);
	if (l && l->next) sl()[h % numslots].unlink_after(l);
    }

    void zap()
    {
	delete[] sl();
	slots = new WvList<_type_>[numslots];
    }

    class Iter : public WvHashTableBase::IterBase
    {
    public:
	Iter(WvHashTable &_tbl) : IterBase(_tbl)
	    { }
	_type_ *ptr() const
	    { return (_type_ *)link->data; }
	WvIterStuff(_type_);
    };

    typedef class WvSorter<_type_,WvHashTableBase,WvHashTableBase::IterBase>
	Sorter;
};


// the _hack container class is necessary because if DeclareWvDict is run
// inside a class definition, the typedef can't access this function for
// some reason (g++ bug or c++ bug?  The world is left wondering...)
//
// Of course, the typedef _itself_ wouldn't be necessary if I could make the
// new class's constructor call the template's constructor by name.  I'm
// almost _certain_ that's a g++ bug.
//
#define __WvDict_base(_classname_, _type_, _ftype_, _field_, _extra_)	\
    struct _classname_##_hack 						\
    { 									\
        static inline const void *_classname_##_fptr_(const void *obj) 	\
	    { return &((*(const _type_ *)obj) _field_); } 		\
    }; 									\
									\
    typedef WvHashTable<_type_, _ftype_, 				\
			_classname_##_hack::_classname_##_fptr_> 	\
			_classname_##Base; 				\
    									\
    class _classname_ : public _classname_##Base 			\
    { 									\
    public: 								\
	_classname_(unsigned _numslots) : _classname_##Base(_numslots)	\
		{ }							\
	void add(_type_ *data, bool auto_free)				\
		{ _classname_##Base::add(data, auto_free); };		\
	_extra_								\
    };


#define DeclareWvDict3(_type_, _newname_, _ftype_, _field_, _extra_) 	\
	__WvDict_base(_newname_, _type_, _ftype_, . _field_, _extra_)
#define DeclareWvDict2(_type_, _ftype_, _field_, _extra_)		\
        DeclareWvDict3(_type_, _type_##Dict, _ftype_, _field_, _extra_)
#define DeclareWvDict(_type_, _ftype_, _field_) 			\
	DeclareWvDict2(_type_, _ftype_, _field_, )

#define DeclareWvTable3(_type_, _newname_, _extra_)			\
	__WvDict_base(_newname_, _type_, _type_, , _extra_)
#define DeclareWvTable2(_type_, _extra_) 				\
	DeclareWvTable3(_type_, _type_##Table, _extra_)
#define DeclareWvTable(_type_) DeclareWvTable2(_type_, )


#endif // __WVHASHTABLE_H
