/*  
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *  
 * A hash table container.
 */ 

#include "wvscatterhash.h"


// Prime numbers close to powers of 2
const unsigned WvScatterHashBase::prime_numbers[] = {2u, 5u, 11u, 17u, 31u, 67u, 127u, 251u, 509u, 1021u, 2039u, 4093u, 8191u, 16381u, 32749u, 65521u, 131071u, 262139u, 524287u, 1048573u, 2097143u, 4194301u, 8388593u, 16777213u, 33554393u, 67108859u, 134217689u, 268435399u, 536870909u, 1073741789u, 2147483647u, 4294967281u};

WvScatterHashBase::pair WvScatterHashBase::null_pair;

// we do not accept the _numslots value directly.  Instead, we find the
// next number of slots which is >= _numslots and take the closest prime number
WvScatterHashBase::WvScatterHashBase(unsigned _numslots)
{
    num = 0;
    used = 0;

    if (_numslots == 0)
        prime_index = 2;
    else
    {
        prime_index = 1;
        while ((_numslots >>= 1) != 0)
            prime_index++;
    }

    numslots = prime_numbers[prime_index];
    slots = new pair[numslots];
    memset(slots, 0, numslots * sizeof(pair));
}

size_t WvScatterHashBase::slowcount() const 
{   
    unsigned count = 0;
    for (unsigned index = 0; index < numslots; index++)
    {
        if (IS_OCCUPIED(slots[index]))
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

    pair *tmpslots = slots;
    slots = new pair[numslots];
    memset(slots, 0, numslots * sizeof(pair));
    used = num = 0;

    for (unsigned i = 0; i < oldnumslots; i++)
    {
        if (IS_OCCUPIED(tmpslots[i]))
            _add(tmpslots[i].data, IS_AUTO_FREE(tmpslots[i]));
    }

    delete[] tmpslots;
}

void WvScatterHashBase::_add(void *data, bool auto_free)
{
    _add(data, do_hash(data), auto_free);
}

void WvScatterHashBase::_add(void *data, unsigned hash, bool auto_free)
{
    rebuild();
    unsigned slot = hash % numslots;

    if (IS_OCCUPIED(slots[slot]))
    {
        unsigned attempt = 0;
        unsigned hash2 = second_hash(hash);

        while (IS_OCCUPIED(slots[slot]))
            slot = curhash(hash, hash2, ++attempt);
    }

    num++;
    if (!IS_DELETED(slots[slot]))
        used++;

    slots[slot].data = data;
    slots[slot].status = auto_free ? 3 : 2;
}

void WvScatterHashBase::_remove(const void *data, unsigned hash)
{
    pair *res = genfind(data, hash);

    if (res != &null_pair)
    {
        if (IS_AUTO_FREE((*res)))
            do_delete(res->data);
        res->status = 1;
        num--;
    }
}

void WvScatterHashBase::_zap()
{
    for (unsigned i = 0; i < numslots; i++)
    {
        if (IS_AUTO_FREE(slots[i]))
            do_delete(slots[i].data);

        slots[i].status = 0;
    }
}

void WvScatterHashBase::_set_autofree(const void *data,
    unsigned hash, bool auto_free)
{
    pair *res = genfind(data, hash);

    if (res)
        res->status = auto_free ? 3 : 2;
}

bool WvScatterHashBase::_get_autofree(const void *data, unsigned hash)
{
    pair *res = genfind(data, hash);

    if (res)
        return IS_AUTO_FREE((*res));

    assert(0 && "You checked auto_free of a nonexistant thing.");
    return false;
}

struct WvScatterHashBase::pair *WvScatterHashBase::genfind
    (const void *data, unsigned hash)
{
    unsigned slot = hash % numslots;

    if (IS_OCCUPIED(slots[slot]) && compare(data, slots[slot].data))
        return &slots[slot];

    unsigned attempt = 0;
    unsigned hash2 = second_hash(hash);

    while (slots[slot].status)
    {
        slot = curhash(hash, hash2, ++attempt);

        if (IS_OCCUPIED(slots[slot]) && compare(data, slots[slot].data))
            return &slots[slot];
    } 

    return &null_pair;
}
