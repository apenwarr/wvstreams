#include "wvxplc.h"
#include <xplc/IServiceManager.h>
#include <xplc/IStaticServiceHandler.h>
#include <xplc/IMonikerService.h>
#include <xplc/IMoniker.h>
#include <assert.h>
#include <stdio.h>

class Hello : public IObject
{
    IMPLEMENT_IOBJECT(Hello);
public:
    Hello()
        { printf("Hello!\n"); }
    virtual ~Hello()
        { printf("Goodbye!\n"); }
};

UUID_MAP_BEGIN(Hello)
  UUID_MAP_ENTRY(IObject)
UUID_MAP_END


class HelloFactory : public IMoniker
{
    IMPLEMENT_IOBJECT(HelloFactory);
public:
    virtual IObject *resolve(const char *s)
        { return new Hello; }
};

UUID_MAP_BEGIN(HelloFactory)
  UUID_MAP_ENTRY(IObject)
  UUID_MAP_ENTRY(IMoniker)
UUID_MAP_END



static const UUID _hellouuid =
  {0x414a69c6, 0x3c9e, 0x49f7,
      {0xab, 0x08, 0xe5, 0x5c,
	      0x7b, 0x6c, 0x23, 0x99}};


int main()
{
    fprintf(stderr, "Starting...\n");
    
    XPLC xplc;
    
    IServiceManager *servmgr = XPLC_getServiceManager();
    assert(servmgr);
    
    IStaticServiceHandler *handler = mutate<IStaticServiceHandler>(
		       servmgr->getObject(XPLC_staticServiceHandler));
    assert(handler);
    
    handler->addObject(_hellouuid, new HelloFactory);
    
    IMonikerService *monikers = mutate<IMonikerService>(
					servmgr->getObject(XPLC_monikers));
    assert(monikers);
    
    monikers->registerObject("hello", _hellouuid);
    
    fprintf(stderr, "About to create...\n");
    IObject *obj = xplc.create<IObject>("hello:");
    assert(obj);
    obj->release();
    
    fprintf(stderr, "Done.\n");
    return 0;
}

