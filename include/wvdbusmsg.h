/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * WvDBusMsg and WvDBusReplyMsg are intended to be easy-to-use abstractions
 * over the low-level D-Bus DBusMessage structure. They represent messages
 * being passed around on the bus.
 */ 
#ifndef __WVDBUSMSG_H
#define __WVDBUSMSG_H

#include "wvstringlist.h"
#include <stdint.h>

struct DBusMessageIter;
struct DBusMessage;

class WvDBusMsg;

class WvDBusMsg
{
public:
    /**
     * Constructs a new WvDBus message. If destination is blank, no 
     * destination is set; this is appropriate when using D-BUS in a 
     * peer-to-peer context (no message bus).
     *
     */
    WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
              WvStringParm interface, WvStringParm method);

    /**
     * Constructs a new WvDBus message, copying it out of an old one.
     */
    WvDBusMsg(WvDBusMsg &_msg);

    /**
     * Constructs a new WvDBus message from an existing low-level D-Bus 
     * message.
     */
    WvDBusMsg(DBusMessage *_msg);

    virtual ~WvDBusMsg();

    operator DBusMessage* () const;
    
    WvString get_sender() const;
    WvString get_dest() const;
    WvString get_path() const;
    WvString get_interface() const;
    WvString get_member() const;
    operator WvString() const;
    
    void get_arglist(WvStringList &list) const;
    WvString get_argstr() const;

    /*
     * The following methods are designed to allow appending various
     * arguments to the message.
     */
    void append(const char *s);
    void append(bool b);
    void append(char c);
    void append(int16_t i);
    void append(uint16_t i);
    void append(int32_t i);
    void append(uint32_t i);
    void append(double d);
    
    class Iter
    {
    public:
	const WvDBusMsg &msg;
	DBusMessageIter *it;
	mutable WvString s;
	bool rewound;
	
	Iter(const WvDBusMsg &_msg);
	~Iter();

        /**
         * Rewinds the iterator to make it point to an imaginary element
         * preceeding the first element of the list.
         */
	void rewind();
	
	/**
	 * Returns the data type of the current element.  Not usually needed,
	 * as the iterator converts elements automatically between most types.
	 */
	int type() const;
	
        /**
         * Moves the iterator along the list to point to the next element.
         * 
         * If the iterator had just been rewound, it now points to the
         * first element of the list.
         */
	bool next();

        /**
	 * Returns: true if the current link is valid
         */
	bool cur() const;
	
	/**
	 * Get the current element as a string (possible for all types).
	 */
	WvString get_str() const;
	
	/**
	 * Get the current element as an int64_t
	 * (possible for all integer types)
	 */
	int64_t get_int() const;
	operator int64_t() const { return get_int(); }
	operator int32_t() const { return get_int(); }
	operator int16_t() const { return get_int(); }
	operator int8_t() const { return get_int(); }
	
	/**
	 * Get the current element as a uint64_t
	 * (possible for all integer types)
	 */
	uint64_t get_uint() const;
	operator uint64_t() const { return get_uint(); }
	operator uint32_t() const { return get_uint(); }
	operator uint16_t() const { return get_uint(); }
	operator uint8_t() const { return get_uint(); }
	
	/**
	 * Returns a pointer to the WvString at the iterator's current
	 * location.  Needed so that WvIterStuff() will work.
	 */
	WvString *ptr() const;
	operator WvString() const { return *ptr(); }
 	
	WvIterStuff(WvString);
    };

protected:
    mutable DBusMessage *msg;
};


class WvDBusReplyMsg : public WvDBusMsg
{
public:
    /**
     * Constructs a new reply message (a message intended to be a reply to
     * an existing D-Bus message).
     */
    WvDBusReplyMsg(DBusMessage *_msg);
    virtual ~WvDBusReplyMsg() {}
};


class WvDBusSignal : public WvDBusMsg
{
public:
    WvDBusSignal(WvStringParm objectname, WvStringParm interface, 
                 WvStringParm name);
};

#endif // __WVDBUSMSG_H
