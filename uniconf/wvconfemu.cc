/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#include "wvconfemu.h"
#include "uniconfini.h"

WvConf::WvConf(WvStringParm _filename, int _create_mode)
    : ev(h), filename(_filename)
{
    h.generator = new UniConfIniFile(&h, filename);
    h.load();
}


WvConf::~WvConf()
{
    save();
}


const char *WvConf::get(WvStringParm section, WvStringParm entry,
			const char *def_val = NULL)
{
    UniConf *res = h.find(UniConfKey(section, entry));
    if (!res || !*res)
	return def_val;
    else
	return res->cstr();
}


void WvConf::setint(WvStringParm section, WvStringParm entry, int value)
{
    h.find_make(UniConfKey(section, entry))->set(value);
    run_all_callbacks();
}


void WvConf::set(WvStringParm section, WvStringParm entry, const char *value)
{
    h.find_make(UniConfKey(section, entry))->set(value);
    run_all_callbacks();
}


void WvConf::add_setbool(bool *b, WvStringParm section, WvStringParm entry)
{
    WvFastString s(!!section ? section : WvFastString("*"));
    WvFastString e(!!entry ? entry : WvFastString("*"));
    ev.add_setbool(b, UniConfKey(s, e));
}


void WvConf::del_setbool(bool *b, WvStringParm section, WvStringParm entry)
{
    WvFastString s(!!section ? section : WvFastString("*"));
    WvFastString e(!!entry ? entry : WvFastString("*"));
    ev.del_setbool(b, UniConfKey(s, e));
}


