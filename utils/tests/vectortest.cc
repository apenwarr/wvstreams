/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvVector test.
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "wvvector.h"

#define DATASET 10000

struct InstrumentedType
{
    static int total_instances;
    static int live_instances;
    int num;
    
    InstrumentedType(int _num)
    {
        total_instances++;
        live_instances++;
	num = _num;
    }

    ~InstrumentedType()
    {
        assert(live_instances > 0);
        live_instances--;
	num = 0xDEADBEEF;
    }
};

int InstrumentedType::total_instances, InstrumentedType::live_instances;


void test(const char *name)
{
    typedef WvVector<InstrumentedType> Vector;
    printf("Testing: %s\n", name);

    {
        puts("  empty vector with zero initial capacity");
        Vector v(true);
        assert(v.count() == 0);
        assert(v.isempty());
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
        v.compact();
        assert(v.count() == 0);
        assert(v.isempty());
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
    }
    
    {
        puts("  empty vector with non-zero initial capacity");
        Vector v(true);
        v.setcapacity(10);
        assert(v.count() == 0);
        assert(v.isempty());
        assert(v.capacity() == 10);
        assert(v.ptr() != NULL);
        v.compact();
        assert(v.count() == 0);
        assert(v.isempty());
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
    }

    {
        puts("  one-element vector");
        Vector v(true);
	v.setcapacity(10);
	InstrumentedType *t = new InstrumentedType(5676);
	v.append(t);
        assert(v.count() == 1);
        assert(!v.isempty());
        assert(v.capacity() >= 1);
        assert(v.ptr() != NULL);

        v.compact();
        assert(v.count() == 1);
        assert(!v.isempty());
        assert(v.capacity() == 1);
        assert(v.ptr() != NULL);
        assert(v[0] == t);

        v.remove(0);
        assert(v.count() == 0);
        assert(v.isempty());
        v.compact();
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);

        v.zap();
        assert(v.count() == 0);
        assert(v.isempty());
        v.compact();
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
    }

    {
        puts("  stack behaviour");
        Vector v(true);
        for (int i = 0; i < DATASET; ++i)
        {
            v.append(new InstrumentedType(i));
            assert(v.count() == i + 1);
        }
        
        for (int i = 0; i < DATASET; ++i)
            assert(v[i]->num == i);
            
        v.compact();
        assert(v.capacity() == DATASET);
        
        for (int i = 0; i < DATASET; ++i)
            assert(v[i]->num == i);
            
        for (int i = 0; i < DATASET; ++i)
        {
	    assert(v.last());
            assert(v.last()->num == DATASET - i - 1);
	    v.remove_last();
            assert(v.count() == DATASET - i - 1);
        }
    }

    puts("Success!\n");
}

int main(int argc, char **argv)
{
    // test vector 
    test("Vector");

    assert(InstrumentedType::total_instances != 0);
    assert(InstrumentedType::live_instances == 0);
    puts("Success again!");

    return 0;
}
