/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a connection between the UniConf client and daemon.
 */
#ifndef __UNICONFCONN_H
#define __UNICONFCONN_H

#include "uniconfkey.h"
#include "uniconfgen.h"
#include "wvstreamclone.h"
#include "wvistreamlist.h"
#include "wvbuf.h"
#include "wvlog.h"

#define DEFAULT_UNICONF_DAEMON_TCP_PORT 4111
#define DEFAULT_UNICONF_DAEMON_SSL_PORT 4112

/**
 * Represents a connection to a UniConf daemon via any WvStream.
 * Makes several operations much simpler, such as TCL
 * encoding/decoding of lists, filling of the operation buffer and
 * comparison for UniConf operations.
 */
class UniClientConn : public WvStreamClone
{
    WvDynBuf msgbuf;

protected:
    WvLog log;
    bool closed;
    
public:
    WvConstStringBuffer payloadbuf; /*!< holds the previous command payload */
    enum Command
    {
        NONE = -2, /*!< used to signal no command received */
        INVALID = -1, /*!< used to signal invalid command */
        
        // requests
        REQ_NOOP, /*!< noop ==> OK */
        REQ_GET, /*!< get <key> ==> VAL ... OK / FAIL */
        REQ_SET, /*!< set <key> <value> ==> OK / FAIL */
        REQ_REMOVE, /*!< del <key> ==> OK / FAIL */
        REQ_SUBTREE, /*!< subt <key> ==> VAL ... OK / FAIL <*/
        REQ_HASCHILDREN, /*!< hchild <key> => HCHILD <key> TRUE / FALSE */
        REQ_QUIT, /*!< quit ==> OK */
        REQ_HELP, /*!< help ==> TEXT ... OK / FAIL */

        // command completion replies
        REPLY_OK, /*!< OK */
        REPLY_FAIL, /*!< FAIL */
        REPLY_CHILD, /*!< HCHILD <key> TRUE / FALSE */
        REPLY_ONEVAL, /*!< ONEVAL <key> <value> */

        // partial replies
        PART_VALUE, /*!< VAL <key> <value> */
        PART_TEXT, /*!< TEXT <text> */

        // events
        EVENT_HELLO, /*!< HELLO <message> */
        EVENT_NOTICE, /*!< NOTICE <key> <oldval> <newval> */
    };
    static const int NUM_COMMANDS = EVENT_NOTICE + 1;
    struct CommandInfo
    {
        const char *name;
        const char *description;
    };
    static const CommandInfo cmdinfos[NUM_COMMANDS];

    /** Create a wrapper around the supplied WvStream. */
    UniClientConn(IWvStream *_s, WvStringParm dst = WvString::null);
    virtual ~UniClientConn();

    virtual bool pre_select(SelectInfo &si);
    virtual bool isok() const;
    virtual void close();

    /**
     * Reads a command from the connection.
     * The payload is stored in UniClientConn::payloadbuf.
     * Returns: the command code, NONE, or INVALID
     */
    Command readcmd();

    /**
     * Reads the next argument from the command payload.
     * Returns: the argument or WvString::null
     */
    WvString readarg();

    /**
     * Writes a command to the connection.
     * "command" is the command
     * "payload" is the payload
     */
    void writecmd(Command command, WvStringParm payload = WvString::null);

    /**
     * Writes a REPLY_OK message.
     * "payload" is the payload, defaults to ""
     */
    void writeok(WvStringParm payload = "");

    /**
     * Writes a REPLY_FAIL message.
     * "payload" is the payload, defaults to ""
     */
    void writefail(WvStringParm payload = "");

    /**
     * Writes a PART_VALUE message.
     * "key" is the key
     * "value" is the value
     */
    void writevalue(const UniConfKey &key, WvStringParm value);

    /**
     * Writes a PART_VALUE message.
     * "key" is the key
     * "value" is the value
     */ 
    void writeonevalue(const UniConfKey &key, WvStringParm value);

    /**
     * Writes a PART_TEXT message.
     * "text" is the text
     */
    void writetext(WvStringParm text);

private:
    /** Reads a message from the connection. */
    WvString readmsg();

    /** Writes a message to the connection. */
    void writemsg(WvStringParm message);
};

#endif // __UNICONFCONN_H
