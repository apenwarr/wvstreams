#include "wvtest.h"
#include "wvvector.h"
#include <cstdio>

// BEGIN vectortest.cc definition
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
        if (!(live_instances > 0))
            WVPASS(live_instances > 0);
        live_instances--;
	num = 0xDEADBEEF;
    }
};

int InstrumentedType::total_instances, InstrumentedType::live_instances;

typedef class WvVector<InstrumentedType> Vector;

void test(const char *name)
{

    // empty vector with zero initial capacity
    {
        Vector v(0);
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.get_capacity() == 0);
        v.compact();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.get_capacity() == 0);
    }

    // empty vector with non-zero initial capacity
    {
        Vector v(10);
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.get_capacity() == 10);
        v.compact();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.get_capacity() == 0);
    }

    // one element vector
    {
        Vector v;
	InstrumentedType *t = new InstrumentedType(5676);
	v.append(t, true);
        WVPASS(v.count() == 1);
        WVPASS(!v.isempty());
        WVPASS(v.get_capacity() >= 1);
	WVPASS(v[0] == t);

        v.compact();
        WVPASS(v.count() == 1);
        WVPASS(!v.isempty());
        WVPASS(v.get_capacity() == 1);
        WVPASS(v[0] == t);

        v.remove(0);
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        v.compact();
        WVPASS(v.get_capacity() == 0);

        v.zap();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        v.compact();
        WVPASS(v.get_capacity() == 0);
    }

    // Insert an element in slot 2, and then remove it.
    {
	Vector v;
	InstrumentedType *a = new InstrumentedType(1);
	InstrumentedType *b = new InstrumentedType(2);
	InstrumentedType *c = new InstrumentedType(3);

	v.append(c, true);
	WVPASS(v[0] == c);

	v.prepend(a, true);
	WVPASS(v[0] == a);
	WVPASS(v[1] == c);
	WVPASS(v.count() == 2);

	v.insert(1, b, true);
	WVPASS(v[0] == a);
	WVPASS(v[1] == b);
	WVPASS(v[2] == c);
	WVPASS(v.count() == 3);

	v.remove(1);
	WVPASS(v[1] == c);
	WVPASS(v.count() == 2);

        v.zap();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        v.compact();
        WVPASS(v.get_capacity() == 0);
    }

    // stack behaviour
    {
        Vector v;
        bool passed = true;
        for (int i = 0; i < DATASET; ++i)
        {
            v.append(new InstrumentedType(i), true);
            if (!(v.count() == i + 1))
                passed = false;
        }
        WVPASS(passed);
	WVPASS(v.count() == DATASET);

        passed = true;
        for (int i = 0; i < DATASET; ++i)
            if (!(v[i]->num == i))
                passed = false;
        WVPASS(passed);

        v.compact();
	WVPASS(v.count() == DATASET);
        WVPASS(v.get_capacity() == DATASET);
        passed = true;
        for (int i = 0; i < DATASET; ++i)
            if (!(v[i]->num == i))
                passed = false;
        WVPASS(passed);

        passed = true;
        for (int i = 0; i < DATASET; ++i)
        {
	    if (!(v.last()))
                passed = false;
            if (!(v.last()->num == DATASET - i - 1))
                passed = false;
	    v.remove_last();
            if (!(v.count() == DATASET - i - 1))
                passed = false;
        }
        WVPASS(passed);

	WVPASS(v.count() == 0);
    }
}

// END vectortest.cc definition

WVTEST_MAIN("vectortest.cc")
{
    // test vector
    test("Vector");

    WVPASS(InstrumentedType::total_instances != 0);
    printf("%d\n", InstrumentedType::live_instances);
    WVPASS(InstrumentedType::live_instances == 0);
}

int compare_int(const int *a, const int *b)
{
    printf ("%d, %d\n", *a, *b);
    if (*a == *b)
    {
	printf ("Equal\n");
	return 0;
    }
    else if (*a > *b)
    {
	printf ("Greater\n");
	return 1;
    }
    else
    {
	printf ("Lesser\n");
	return -1;
    }
}

int comp_int(const void *_a, const void *_b)
{
    int *a = *(int **)_a;
    int *b = *(int **)_b;
    if (*a == *b)
	return 0;
    else if (*a > *b)
	return 1;
    else
	return -1;
}

WVTEST_MAIN("wvvector - sorting")
{
    WvVector<int> v;

    v.append(new int(5), true);
    v.append(new int(10), true);
    v.append(new int(1), true);
    v.append(new int(6), true);
    v.append(new int(15), true);
    v.qsort(&compare_int);

    WVPASSEQ(*v[0], 1);
    WVPASSEQ(*v[1], 5);
    WVPASSEQ(*v[2], 6);
    WVPASSEQ(*v[3], 10);
    WVPASSEQ(*v[4], 15);
    WVPASSEQ(v.count(), 5);
}
