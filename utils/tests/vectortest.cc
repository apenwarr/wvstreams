/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** /file
 * WvVector test.
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "wvvector.h"

#define DATASET 10000

template<class T>
struct InstrumentedType
{
    static const T INIT = 0;
    static const T GUARD = 0xDEADBEEF;
    static int total_instances;
    static int live_instances;

    T holder;
    
    InstrumentedType()
    {
        total_instances += 1;
        live_instances += 1;
        holder = INIT;
    }
    
    InstrumentedType(const InstrumentedType &other) :
        holder(other.holder)
    {
        assert(holder != GUARD);
        total_instances += 1;
        live_instances += 1;
    }

    InstrumentedType(const T &value) :
        holder(value)
    {
        assert(value != GUARD);
        total_instances += 1;
        live_instances += 1;
    }

    ~InstrumentedType()
    {
        assert(live_instances > 0);
        live_instances -= 1;
        holder = GUARD;
    }

    InstrumentedType &operator= (const T &value)
    {
        assert(holder != GUARD);
        assert(value != GUARD);
        holder = value;
        return *this;
    }

    operator T ()
    {
        return holder;
    }
};

template<class T>
int InstrumentedType<T>::total_instances = 0;
 
template<class T>
int InstrumentedType<T>::live_instances = 0;


template<class T, class BOps>
void test(const char *name)
{
    typedef WvVector<T, BOps> Vector;
    printf("Testing: %s\n", name);

    {
        puts("  empty vector with zero initial capacity");
        Vector v;
        assert(v.size() == 0);
        assert(v.isempty());
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
        v.compact();
        assert(v.size() == 0);
        assert(v.isempty());
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
    }
    
    {
        puts("  empty vector with non-zero initial capacity");
        Vector v;
        v.setcapacity(10);
        assert(v.size() == 0);
        assert(v.isempty());
        assert(v.capacity() == 10);
        assert(v.ptr() != NULL);
        v.compact();
        assert(v.size() == 0);
        assert(v.isempty());
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
    }

    {
        puts("  one element vector");
        Vector v(1);
        assert(v.size() == 1);
        assert(! v.isempty());
        assert(v.capacity() >= 1);
        assert(v.ptr() != NULL);
        v[0] = 5;
        assert(v[0] == 5);
        assert(* v.ptr() == 5);

        v.compact();
        assert(v.size() == 1);
        assert(! v.isempty());
        assert(v.capacity() == 1);
        assert(v.ptr() != NULL);
        assert(v[0] == 5);

        v.remove(0);
        assert(v.size() == 0);
        assert(v.isempty());
        v.compact();
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);

        v.setsize(1);
        assert(v.size() == 1);
        assert(! v.isempty());
        assert(v.capacity() >= 1);
        assert(v.ptr() != NULL);
        v[0] = 5;
        assert(v[0] == 5);
        assert(* v.ptr() == 5);
        
        v.setsize(0);
        assert(v.size() == 0);
        assert(v.isempty());
        v.compact();
        assert(v.capacity() == 0);
        assert(v.ptr() == NULL);
    }

    {
        puts("  stack behaviour");
        Vector v;
        for (int i = 0; i < DATASET; ++i)
        {
            v.pushback(i);
            assert(v.size() == i + 1);
        }
        
        for (int i = 0; i < DATASET; ++i)
            assert(v[i] == i);
            
        v.compact();
        assert(v.capacity() == DATASET);
        
        for (int i = 0; i < DATASET; ++i)
            assert(v[i] == i);
            
        for (int i = 0; i < DATASET; ++i)
        {
            assert(v.popback() == DATASET - i - 1);
            assert(v.size() == DATASET - i - 1);
        }
    }

    {
        // We expect this to be slow because vector is not
        // optimized to handle front insertion/removal
        puts("  reverse stack behaviour (slow)");
        Vector v;
        for (int i = 0; i < DATASET; ++i)
        {
            v.pushfront(i);
            assert(v.size() == i + 1);
        }
        
        for (int i = 0; i < DATASET; ++i)
            assert(v[i] == DATASET - i - 1);
            
        v.compact();
        assert(v.capacity() == DATASET);
        
        for (int i = 0; i < DATASET; ++i)
            assert(v[i] == DATASET - i - 1);
            
        for (int i = 0; i < DATASET; ++i)
        {
            assert(v.popfront() == DATASET - i - 1);
            assert(v.size() == DATASET - i - 1);
        }
    }
    
    // Bored... enough for now...
    // FIXME: need tests for insertion, removal, and truncation

    puts("Success!\n");
}

typedef InstrumentedType<int> IInt;

int main(int argc, char **argv)
{
    // test vector using shallow block ops
    test<int, ShallowBlockOps<int> >("Shallow block ops");

    // test vector using deep block ops
    test<int, DeepBlockOps<int> >("Deep block ops");

    // test vector using instrumented block ops
    test<IInt, DeepBlockOps<IInt> >("Instrumented block ops");

    assert(IInt::total_instances != 0);
    assert(IInt::live_instances == 0);
    puts("Success!");

    return 0;
}
