#ifndef __WVLINKERHACK_H
#define __WVLINKERHACK_H

/**
 * Create a target that WV_LINK_TO can point at when it wants to make sure
 * your object file is linked into the final binary.
 * 
 * Do not put the name in quotes.
 * 
 * Example (in uniinigen.cc): WV_LINK(UniIniGen);
 */
#define WV_LINK(name) int *__wv_link_##name

/**
 * Link to a target created by WV_LINK, ensuring that the file which created
 * the WV_LINK is included in your final binary.  This is useful for ensuring
 * particular monikers will always be available when you link statically,
 * even though you don't refer to any particular classes implementing that
 * moniker directly.
 * 
 * Do not put the name in quotes.
 * 
 * Example (somewhere other than uniinigen.cc): WV_LINK_TO(UniIniGen);
 */
#define WV_LINK_TO(name) \
	extern int *__wv_link_##name; \
	static int **__wv_link_func_##name () __attribute((unused)); \
	static int **__wv_link_func_##name () { return &__wv_link_##name; }

#endif // __WVLINKERHACK_H
