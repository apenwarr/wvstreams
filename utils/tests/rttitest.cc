
#include "wvrtti.h"
#include <stdio.h>
#include <assert.h>

class A
{
    WVTYPE(void)
};
class B : public A
{
    WVTYPE(A)
};
class C
{
    WVTYPE(void)
};
class D : public B
{
    WVTYPE(B)
};

int main(int argc, char **argv)
{
    assert(wvtypeof<A>().subtypeof(wvtypeof<A>()));
    assert(wvtypeof<B>().subtypeof(wvtypeof<A>()));
    assert(! wvtypeof<A>().subtypeof(wvtypeof<B>()));
    assert(wvtypeof<C>().subtypeof(wvtypeof<void>()));
    assert(wvtypeof<D>().subtypeof(wvtypeof<void>()));
    assert(! wvtypeof<C>().subtypeof(wvtypeof<A>()));
    assert(! wvtypeof<C>().subtypeof(wvtypeof<D>()));
    assert(wvtypeof<D>().subtypeof(wvtypeof<A>()));
    assert(wvtypeof<void>().subtypeof(wvtypeof<void>()));
    assert(! wvtypeof<void>().subtypeof(wvtypeof<A>()));

    A a;
    B b;
    C c;
    D d;

    assert(wvinstanceof<A*>(& a));

    assert(wvdynamic_cast<A*>(& a));
    assert(wvdynamic_cast<B*>(& b));
    assert(wvdynamic_cast<C*>(& c));
    assert(wvdynamic_cast<D*>(& d));
    assert(wvdynamic_cast<A*>(& b));
    assert(wvdynamic_cast<A*>(& d));
    assert(wvdynamic_cast<B*>(& b));
    assert(wvdynamic_cast<B*>(& d));
    assert(wvdynamic_cast<D*>((A*)& d));
    assert(wvdynamic_cast<void*>(& d));
    assert(wvdynamic_cast<void*>(& c));
    
    assert(! wvdynamic_cast<D*>(& a));
//    assert(! wvdynamic_cast<D*>(& c)); // fails on compilation as it should

    puts("All tests passed!");
    return 0;
}
