/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniDefGen is a UniConfGen for retrieving data with defaults
 * 
 * Usable with the moniker default:
 */

#ifndef __UNIDEFGEN_H
#define __UNIDEFGEN_H

#include "unifiltergen.h"

/*
 * The defaults are stored and accessed by using a * in the keyname. The *
 * can represent either a segment of the path, or can can be left as a key
 * in a path so that any search to that path returns a result.
 *
 * (note: odd spacing is only to avoid putting * and / directly together)
 *
 * For example, you could set /twister/expression/ * /reality =
 * "/tmp/twister" and then a search for /twister/expression/bob/reality
 * would return the result.
 *
 * There is no limitation on where the * can be placed or the number of *s.
 * For example: /twister/ * / * / reality / *
 * would be an acceptable (though somewhat insane) use. This would make it
 * so a search to /twister/(whatever)/(whatever)/reality/(whatever) would
 * always return a key.
 *
 * If a more absolute path exists, then it will be returned instead of the
 * defaults. Precedence is given to matches existing closer to the end of
 * the key.
 */
class UniDefGen : public UniFilterGen
{
    WvString finddefault(UniConfKey key, UniConfKey keypart = "");

public:
    UniDefGen(UniConfGen *gen) : UniFilterGen(gen) { }

    /***** Overridden members *****/

    virtual WvString get(const UniConfKey &key);
};

#endif // __UNIDEFGEN_H
