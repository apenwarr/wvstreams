#include "wvmoniker.h"
#include "wvmonikerregistry.h"
#include <stdio.h>

class ITest : public IObject
{
public:
    virtual void f() = 0;
};

DEFINE_IID(ITest, {0xcd3239a7, 0x0ea1, 0x4e1a,
  {0xba, 0x08, 0xb8, 0x5e, 0xe4, 0xda, 0xad, 0x69}});


class Test : public GenericComponent<ITest>
{
public:
    WvString s;
    
    Test(WvStringParm _s) : s(_s)
	{ printf("%p(\"%s\"): creating!\n", this, s.cstr()); }
    virtual ~Test()
	{ printf("%p(\"%s\"): destroying!\n", this, s.cstr()); }
    
    virtual void f()
        { printf("%p(\"%s\"): f() called!\n", this, s.cstr()); }
};


static IObject *createfunc(WvStringParm s, IObject *obj, void *userdata)
{
    return new Test(s);
}


static IObject *createfunc2(WvStringParm s, IObject *obj, void *userdata)
{
    return new Test(WvString("bunk(%s)", s));
}


static ITest *createfunc3(WvStringParm s, IObject *obj, void *userdata)
{
    return new Test(WvString("stunk(%s)", s));
}


int main()
{
    WvMoniker<IObject> junk("obj", createfunc);
    WvMoniker<IObject> bunk("obj2", createfunc2);
    WvMoniker<ITest> stunk("test", createfunc3);
    
    WvMonikerRegistry *reg = WvMonikerRegistry::find_reg(IID<IObject>::get());
    IObject *a = reg->create("obj:obj-a");
    IObject *b = reg->create("obj2:obj2-b");
    reg->release();
    
    IObject *c = wvcreate<IObject>("obj2:obj2-c");
    
    ITest *d = wvcreate<ITest>("test:test-d");
    if (d)
	d->f();

    if (a)
	a->release();
    delete b;
    delete c;
    if (d)
	d->release();
}
