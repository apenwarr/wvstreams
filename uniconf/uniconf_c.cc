/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides a C binding for UniConf.
 */
#include "uniconf.h"
#include "uniconfroot.h"


uniconf_t uniconf_init(const char* _moniker)
{
    return new UniConfRoot(_moniker);
}


void uniconf_free(uniconf_t _uniconf)
{
    assert(_uniconf);

    delete static_cast<UniConfRoot*>(_uniconf);
}


const char* uniconf_get(uniconf_t _uniconf, const char* _key)
{
    UniConfRoot* uniconf = static_cast<UniConfRoot*>(_uniconf);

    return strdup((*uniconf)[WvString(_key)].get());
}


void uniconf_set(uniconf_t _uniconf,
		 const char* _key, const char* _value)
{
    UniConfRoot* uniconf = static_cast<UniConfRoot*>(_uniconf);

    return (*uniconf)[_key].set(_value);
}

