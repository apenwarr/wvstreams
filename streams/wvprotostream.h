/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvProtoStream is a framework that makes it easy to communicate using
 * common command-response driven protocols.  This is supposed to be
 * flexible enough to handle FTP, HTTP, SMTP, tunnelv, Weaver rcmd, and
 * many others.
 */
#ifndef __WVPROTOSTREAM_H
#define __WVPROTOSTREAM_H

#include "wvstreamclone.h"

class WvLog;


class WvProtoStream : public WvStreamClone
{
public:
    WvProtoStream(WvStream **_cloned, WvLog *_debuglog = NULL);
    virtual ~WvProtoStream();
    
    class Token;
    class TokenList;
    
    // override uwrite() so we can log all output
    virtual size_t uwrite(const void *buffer, size_t size);

    virtual void execute();
    
    // Routines to convert an input line into a list of Tokens.
    virtual TokenList *tokenize(const char *line);
    size_t list_to_array(TokenList *tl, Token **array);
    size_t tokline(Token **array);
    
    // Convert token strings to enum values
    int tokanal(const Token *t, char **lookup,
		bool case_sensitive = false);
    
    // finite state machine
    int state;
    virtual void do_state(Token *t, size_t nt);
    virtual void switch_state(int newstate);
    
protected:
    WvLog *log;
    bool log_enable;
    
public:
    class Token
    {
    public:
	WvString data;
	size_t length;
	int extra;
	
	Token();
	Token(const char *_data, size_t _length, int _extra = 0);
	void fill(const char *_data, size_t _length, int _extra);
	~Token();
    };
    
    DeclareWvList(Token);
};


#endif // __WVPROTOSTREAM_H
