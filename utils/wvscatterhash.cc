/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *  
 * A hash table container.
 */ 

#include "wvscatterhash.h"
#include <assert.h>

// Prime numbers close to powers of 2
const unsigned WvScatterHashBase::prime_numbers[]
    = {2u, 5u, 11u, 17u,
       31u, 67u, 127u, 251u,
       509u, 1021u, 2039u, 4093u,
       8191u, 16381u, 32749u, 65521u,
       131071u, 262139u, 524287u, 1048573u,
       2097143u, 4194301u, 8388593u, 16777213u,
       33554393u, 67108859u, 134217689u, 268435399u,
       536870909u, 1073741789u, 2147483647u, 4294967281u};

// we do not accept the _numslots value directly.  Instead, we find the
// next number of xslots which is >= _numslots and take the closest prime
// number
WvScatterHashBase::WvScatterHashBase(unsigned _numslots)
{
    num = 0;
    used = 0;

    if (_numslots == 0)
        prime_index = 0;
    else
    {
        prime_index = 1;
        while ((_numslots >>= 1) != 0)
            prime_index++;
    }

    numslots = prime_numbers[prime_index];
    xslots = new Slot[numslots];
    xstatus = new Status[numslots];
    memset(xslots, 0, numslots * sizeof(xslots[0]));
    memset(xstatus, 0, numslots * sizeof(xstatus[0]));
}

size_t WvScatterHashBase::slowcount() const 
{   
    unsigned count = 0;
    for (unsigned index = 0; index < numslots; index++)
    {
        if (IS_OCCUPIED(xstatus[index]))
            count++;
    }

    return count;
}

void WvScatterHashBase::rebuild()
{
    if (!(numslots * REBUILD_LOAD_FACTOR <= used + 1))
        return;

    unsigned oldnumslots = numslots;

    if (numslots * RESIZE_LOAD_FACTOR <= num + 1) 
        numslots = prime_numbers[++prime_index];

    Slot *tmpslots = xslots;
    Status *tmpstatus = xstatus;
    xslots = new Slot[numslots];
    xstatus = new Status[numslots];
    memset(xslots, 0, numslots * sizeof(xslots[0]));
    memset(xstatus, 0, numslots * sizeof(xstatus[0]));
    used = num = 0;

    for (unsigned i = 0; i < oldnumslots; i++)
    {
        if (IS_OCCUPIED(tmpstatus[i]))
            _add(tmpslots[i], IS_AUTO_FREE(tmpstatus[i]));
    }

    deletev tmpslots;
    deletev tmpstatus;
}

void WvScatterHashBase::_add(void *data, bool auto_free)
{
    _add(data, do_hash(data), auto_free);
}

void WvScatterHashBase::_add(void *data, unsigned hash, bool auto_free)
{
    rebuild();
    unsigned slot = hash % numslots;

    if (IS_OCCUPIED(xstatus[slot]))
    {
        unsigned attempt = 0;
        unsigned hash2 = second_hash(hash);

        while (IS_OCCUPIED(xstatus[slot]))
            slot = curhash(hash, hash2, ++attempt);
    }

    num++;
    if (!IS_DELETED(xstatus[slot]))
        used++;

    xslots[slot] = data;
    xstatus[slot] = auto_free ? 3 : 2;
}

void WvScatterHashBase::_remove(const void *data, unsigned hash)
{
    unsigned res = genfind(data, hash);

    if (res != null_idx)
    {
        if (IS_AUTO_FREE(xstatus[res]))
            do_delete(xslots[res]);
	xstatus[res] = 1;
        num--;
    }
}

void WvScatterHashBase::_zap()
{
    for (unsigned i = 0; i < numslots; i++)
    {
        if (IS_AUTO_FREE(xstatus[i]))
            do_delete(xslots[i]);

        xstatus[i] = 0;
    }
    
    used = num = 0;
}

void WvScatterHashBase::_set_autofree(const void *data,
    unsigned hash, bool auto_free)
{
    unsigned res = genfind(data, hash);

    if (res != null_idx)
        xstatus[res] = auto_free ? 3 : 2;
}

bool WvScatterHashBase::_get_autofree(const void *data, unsigned hash)
{
    unsigned res = genfind(data, hash);

    if (res != null_idx)
        return IS_AUTO_FREE(xstatus[res]);

    assert(0 && "You checked auto_free of a nonexistant thing.");
    return false;
}

unsigned WvScatterHashBase::genfind(const void *data, unsigned hash) const
{
    unsigned slot = hash % numslots;

    if (IS_OCCUPIED(xstatus[slot]) && compare(data, xslots[slot]))
        return slot;

    unsigned attempt = 0;
    unsigned hash2 = second_hash(hash);

    while (xstatus[slot])
    {
        slot = curhash(hash, hash2, ++attempt);

        if (IS_OCCUPIED(xstatus[slot]) && compare(data, xslots[slot]))
            return slot;
    } 

    return null_idx;
}


void *WvScatterHashBase::genfind_or_null(const void *data, unsigned hash) const
{
    unsigned slot = genfind(data, hash);
    if (slot == null_idx)
	return NULL;
    else
	return xslots[slot];
}
