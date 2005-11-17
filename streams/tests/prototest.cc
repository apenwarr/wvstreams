/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvProtoStream test.  Defines a simple language consisting of the commands
 * and states "one", "two", "three", and "four".  See if you can get to state
 * three!  Hours of fun for the whole family!
 *
 */

#include "wvprotostream.h"
#include "wvlog.h"


class ProtoTest : public WvProtoStream
{
public:
    ProtoTest(WvStream *_cloned, WvLog *_debuglog);
    virtual void do_state(Token &t1);
    virtual void switch_state(int newstate);
};


enum States {
    None = 0,
    One,
    Two,
    Three,
    Four,
};


char *toks[] = {
    "quit",
    "one",
    "Two",
    "THREE",
    "four",
    NULL
};


ProtoTest::ProtoTest(WvStream *_cloned, WvLog *_debuglog)
	: WvProtoStream(_cloned, _debuglog)
{
    switch_state(One);
}


void ProtoTest::do_state(Token &t1)
{
    enum States tok = (States)tokanal(t1, toks, false);

    if (tok < 0)
	print("ERROR Unknown command (%s)\n", t1.data);
    else
	switch_state(tok);
}


void ProtoTest::switch_state(int newstate)
{
    switch ((States)newstate)
    {
    case Three:
	if (state != Two)
	{
	    print("ERROR Only from state two!\n");
	    break;
	}
	
	// else fall through!

    case One:
    case Two:
    case Four:
	print("GO %s (%s)\n", newstate, toks[newstate]);
	break;
	
    case None:
	close();
	break;
    }

    state = newstate;
}


int main()
{
    WvLog log("prototest");
    ProtoTest ps(wvcon, &log);
    
    log("Starting...\n");
    
    while (ps.isok())
    {
	if (ps.select(100))
	    ps.callback();
    }

    ps.cloned = NULL;
}
