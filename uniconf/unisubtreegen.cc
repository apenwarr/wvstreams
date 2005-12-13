/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A UniConfGen for returning only a particular subtree of a given generator.
 */
#include "unisubtreegen.h"
#include "wvbuf.h"
#include "wvtclstring.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(UniSubtreeGen);


static IUniConfGen *creator(WvStringParm s)
{
    WvConstInPlaceBuf buf(s, s.len());
    WvString one(wvtcl_getword(buf)), two(wvtcl_getword(buf));

    if (!one) one = "";
    if (!two) two = "";
    
    return new UniSubtreeGen(wvcreate<IUniConfGen>(one), two);
}

static const UUID uuid = {0x7daeda00, 0x5752, 0x420d,
			  {0x83, 0x68, 0x12, 0xce, 0x6e, 0x89, 0x89, 0x40}};
static WvMoniker<IUniConfGen> subtreereg("subtree", uuid, creator);


UniSubtreeGen::UniSubtreeGen(IUniConfGen *gen, const UniConfKey &_subkey)
    : UniFilterGen(gen), subkey(_subkey)
{
    // nothing special
}


UniConfKey UniSubtreeGen::keymap(const UniConfKey &key)
{
    if (key == UniConfKey())
        return subkey;
    else if (subkey == UniConfKey())
        return key;
    else
        return UniConfKey(subkey, key);
}

UniConfKey UniSubtreeGen::reversekeymap(const UniConfKey &key)
{
    if (key.numsegments() >= subkey.numsegments())
        return UniConfKey(key.removefirst(subkey.numsegments()));

    return key;
}
