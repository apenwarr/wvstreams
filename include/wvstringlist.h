/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvStrings are used a lot more often than WvStringLists, so the List need
 * not be defined most of the time.  Include this file if you need it.
 *
 */
#ifndef __WVSTRINGLIST_H
#define __WVSTRINGLIST_H

#include "wvstring.h"
#include "wvlinklist.h"

DeclareWvList2(WvStringListBase, WvString);

class WvStringList : public WvStringListBase
{
public:
    
    /**
     * concatenates all elements of the list seperating on joinchars
     */
    WvString join(const char *joinchars = " ") const;
    
    /**
     * split s and form a list ignoring splitchars
     * ie. " happy birthday  to  you" split on " " will populate the list with
     * "happy"
     * "birthday"
     * "to"
     * "you"
     */
    void split(WvStringParm s, const char *splitchars = " \t\r\n",
	       int limit = 0);
    /**
     * split s and form a list creating null entries when there are multiple 
     * splitchars
     * ie " happy birthday  to  you" split on " " will populate the list with
     *  ""
     *  "happy"
     *  "birthday"
     *  ""
     *  "to"
     *  ""
     *  "you"
     *      
     */
    void splitstrict(WvStringParm s, const char *splitchars = " \t\r\n",
	       int limit = 0);
    
    /*
     * populate the list from an array of strings
     */
    void fill(const char * const *array);

    void append(WvStringParm str);
    void append(const WvString *strp, bool autofree, char *id = NULL);

    /** 
     * get the first string in the list, or an empty string if the list is empty.
     * Removes the returned string from the list.
     */
    WvString popstr();
};

#endif // __WVSTRINGLIST_H
