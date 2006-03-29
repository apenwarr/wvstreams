#include "wvcallback.h"

typedef WvCallback<void> Cb;

void f()
{
    // nothing
}


class Derived : public Cb
{
public:
    Derived(const Cb &cb) : Cb(cb)
        { }
};


int main()
{
    Cb cb1(f);
    Cb cb2(cb1);
    Derived cb3(f);
    Derived cb4(cb1);
    Derived cb5(cb3);
    return 0;
}
