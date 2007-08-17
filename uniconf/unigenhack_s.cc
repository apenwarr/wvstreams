/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A file that, if you link to it, will ensure that you get the various
 * standard UniConf generators linked into your application.  This is
 * important if you use static linking and construction via monikers, as
 * otherwise the linker will try to be smart and leave out the not-
 * directly-referenced files.
 * 
 * See also unigenhack.cc.
 */
#include "wvlinkerhack.h"

WV_LINK(UniGenHack);

WV_LINK_TO(UniIniGen);
WV_LINK_TO(UniListGen);
WV_LINK_TO(UniDefGen);
WV_LINK_TO(UniClientGen);
WV_LINK_TO(UniAutoGen);
WV_LINK_TO(UniCacheGen);
WV_LINK_TO(UniNullGen);
WV_LINK_TO(UniPermGen);
WV_LINK_TO(UniReadOnlyGen);
WV_LINK_TO(UniReplicateGen);
WV_LINK_TO(UniRetryGen);
WV_LINK_TO(UniSecureGen);
WV_LINK_TO(UniSubtreeGen);
WV_LINK_TO(UniUnwrapGen);

#ifdef _WIN32
WV_LINK_TO(UniPStoreGen);
WV_LINK_TO(UniRegistryGen);
#endif

