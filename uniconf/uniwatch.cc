/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A class that does add_callback when created and del_callback when
 * destroyed.  See uniwatch.h
 */
#include "uniwatch.h"
#include "uniconfroot.h"


UniWatch::UniWatch(const UniConf &_cfg, const UniConfCallback &_cb,
		   bool _recurse)
    : cfg(_cfg), cb(_cb), recurse(_recurse)
{
    cfg.add_callback(this, cb, recurse);
}


UniWatch::UniWatch(const UniConf &_cfg, bool *b, bool _recurse)
    : cfg(_cfg), cb(WvBoundCallback<UniConfCallback, bool *>
		    (&UniConfRoot::setbool_callback, b)),
    recurse(_recurse)
{
    cfg.add_callback(this, cb, recurse);
}


UniWatch::~UniWatch()
{
    cfg.del_callback(this, recurse);
}
