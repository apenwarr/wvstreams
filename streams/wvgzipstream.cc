#include "wvgzipstream.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"

WV_LINK(WvGzipStream);

static IWvStream *creator(WvStringParm s, IObject *_obj)
{
    return new WvGzipStream(new WvStreamClone(wvcreate<IWvStream>(s, _obj)));
}

static WvMoniker<IWvStream> reg("gzip", creator);


