/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * UniConf daemon support.
 */
#ifndef __UNICONFCONN_H
#define __UNICONFCONN_H

#include "wvstreamclone.h"
#include "wvbuffer.h"

#define DEFAULT_UNICONF_DAEMON_TCP_PORT 4111

/**
 * Represents a connection to a UniConf daemon via any WvStream.
 * Makes several operations much simpler, such as TCL
 * encoding/decoding of lists, filling of the operation buffer and
 * comparison for UniConf operations.
 */
class UniConfConn : public WvStreamClone
{
public:
    /**
     * Create a wrapper around the supplied WvStream.
     */
    UniConfConn(WvStream *_s);
    
    /**
     * Default destructor
     */
    virtual ~UniConfConn();

    /**
     * Gets a tcl line from the incoming buffer.
     */
    WvString gettclline();
    
    /**
     * Fills the incoming buffer with all the data which is waiting to be read.
     */
    virtual void fillbuffer();
    
    /**
     * Empty callback function
     */
    virtual void execute();
    
    /**
     * Returns whether or not the underlying stream is actually useable.
     */
    virtual bool isok() const;

    /*** Protocol opcodes ***/
    
    static const WvString UNICONF_GET;
        /*!< Sent on a get request. */
    static const WvString UNICONF_SET;
        /*!< Sent on a set request. */
    static const WvString UNICONF_DEL;
        /*!< Sent on a delete request. */
    static const WvString UNICONF_SUBTREE;
        /*!< Sent on a request for an immediate subtree. */
    static const WvString UNICONF_RECURSIVESUBTREE;
        /*!< Sent on a request for a recursive subtree. */
    static const WvString UNICONF_REGISTER;
        /*!< Sent on a request to add a watch to a specific key. */
    
    static const WvString UNICONF_QUIT;
        /*!< Sent to close this connection. */
    static const WvString UNICONF_HELP;
        /*!< Sent to request a list of available commands. */
    
    static const WvString UNICONF_RETURN;
        /*!< Returned to indicate a new value. */
    static const WvString UNICONF_FORGET;
        /*!< Returned to indicate a subtree is out of date. */
    static const WvString UNICONF_SUBTREE_RETURN;
        /*!< Returned when a subtree result is returned. */
    static const WvString UNICONF_OK;
        /*!< Returned when the requested operation can be / has been completed. */
    static const WvString UNICONF_FAIL;
        /*!< Returned when the requested operation cannot be completed. */

protected:

    WvDynamicBuffer incomingbuff;
private:
};

#endif // __UNICONFCONN_H
