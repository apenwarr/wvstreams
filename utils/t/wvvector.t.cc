#include "wvtest.h"
#include "wvvector.h"

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


void test(const char *name)
{
    typedef WvVector<InstrumentedType> Vector;

    // empty vector with zero initial capacity
    {
        Vector v(true);
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.capacity() == 0);
        WVPASS(v.ptr() == NULL);
        v.compact();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.capacity() == 0);
        WVPASS(v.ptr() == NULL);
    }
    
    // empty vector with non-zero initial capacity
    {
        Vector v(true);
        v.setcapacity(10);
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.capacity() == 10);
        WVPASS(v.ptr() != NULL);
        v.compact();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        WVPASS(v.capacity() == 0);
        WVPASS(v.ptr() == NULL);
    }

    // one element vector
    {
        Vector v(true);
	v.setcapacity(10);
	InstrumentedType *t = new InstrumentedType(5676);
	v.append(t);
        WVPASS(v.count() == 1);
        WVPASS(!v.isempty());
        WVPASS(v.capacity() >= 1);
        WVPASS(v.ptr() != NULL);

        v.compact();
        WVPASS(v.count() == 1);
        WVPASS(!v.isempty());
        WVPASS(v.capacity() == 1);
        WVPASS(v.ptr() != NULL);
        WVPASS(v[0] == t);

        v.remove(0);
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        v.compact();
        WVPASS(v.capacity() == 0);
        WVPASS(v.ptr() == NULL);

        v.zap();
        WVPASS(v.count() == 0);
        WVPASS(v.isempty());
        v.compact();
        WVPASS(v.capacity() == 0);
        WVPASS(v.ptr() == NULL);
    }

    // stack behaviour
    {
        Vector v(true);
        bool passed = true;
        for (int i = 0; i < DATASET; ++i)
        {
            v.append(new InstrumentedType(i));
            if (!(v.count() == i + 1))
                passed = false;
        }
        WVPASS(passed);
        
        passed = true;
        for (int i = 0; i < DATASET; ++i)
            if (!(v[i]->num == i))
                passed = false;
            
        WVPASS(passed);
        v.compact();
        WVPASS(v.capacity() == DATASET);
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
    }
}

// END vectortest.cc definition
 
WVTEST_MAIN("vectortest.cc")
{
    // test vector 
    test("Vector");

    WVPASS(InstrumentedType::total_instances != 0);
    WVPASS(InstrumentedType::live_instances == 0);
}
