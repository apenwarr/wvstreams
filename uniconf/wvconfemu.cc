/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#include "wvconfemu.h"

void WvConfEmu::notify(const UniConf &_uni, const UniConfKey &_key)
{
    WvList<SetBool>::Iter i(setbools);
    WvString section(_key.first());
    WvString key(_key.removefirst());

    if (hold)
	return;

    i.rewind();
    while (i.next())
	if (((i->section && !i->section) || !strcasecmp(i->section, section))
	    && ((i->key && !i->key) || !strcasecmp(i->key, key)))
	    *(i->b) = true;
}


WvConfEmu::WvConfEmu(const UniConf& _uniconf):
    uniconf(_uniconf), sections(42), hold(false)
{
    uniconf.add_callback(this,
			 UniConfCallback(this, &WvConfEmu::notify),
			 true);
}


void WvConfEmu::zap()
{
    uniconf.remove();
}


void WvConfEmu::load_file(WvStringParm filename)
{
    UniConfRoot new_uniconf(WvString("ini:%s", filename));

    hold = true;
    new_uniconf.copy(uniconf, true);
    hold = false;
}


WvConfigSectionEmu *WvConfEmu::operator[] (WvStringParm sect)
{
    WvConfigSectionEmu* section = sections[sect];

    if (!section && uniconf[sect].exists())
    {
	section = new WvConfigSectionEmu(uniconf[sect], sect);
	sections.add(section, true);
    }

    return section;
}


void WvConfEmu::add_setbool(bool *b, WvStringParm _section, WvStringParm _key)
{
    WvList<SetBool>::Iter i(setbools);

    i.rewind();
    while (i.next())
    {
	if (i->b == b
	    && i->section == _section
	    && i->key == _key)
	    return;
    }

    setbools.append(new SetBool(b, _section, _key), true);
}


const char *WvConfEmu::get(WvStringParm section, WvStringParm entry,
			   const char *def_val)
{
    return uniconf[section][entry].get(def_val);
}


void WvConfEmu::setint(WvStringParm section, WvStringParm entry, int value)
{
    uniconf[section][entry].setint(value);
}


void WvConfEmu::set(WvStringParm section, WvStringParm entry,
		    const char *value)
{
    uniconf[section][entry].set(value);
}


void WvConfEmu::maybeset(WvStringParm section, WvStringParm entry,
			 const char *value)
{
    if (get(section, entry, 0) == 0)
	set(section, entry, value);
}

