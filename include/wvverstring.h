/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Version number and string manipulations.
 *
 * The old version number was a 32-bit hexadecimal number, split into a 16-bit
 * major version and a 16-bit minor version.  For example, the old-style string
 * equivalent of 0x00012a00 would be "1.2a".
 *
 * The new-style version number is a 32-bit hexadecimal number, split into
 * an 8-bit major version, an 8-bit minor version, and a 16-bit revision
 * number.  The new-style string equivalent of 0x01020150 would be "1.02.0150".
 */
#ifndef __WVVERSTRING_H
#define __WVVERSTRING_H

/** Watershed for version style, given in new style.
 * Any version equal to or greater than 'wvversion_watershed' will be treated
 * as a new-style version (set to 0 to always use the new style).
 * Versions less than 'version_watershed' will use the old style.
 * The default value is 0.
 */
extern unsigned int wvversion_watershed;

/** Converts a version number to a c string.
 * Set 'my_watershed' to non-zero to override 'wvversion_watershed'.
 */
const char *ver_to_string(unsigned int ver, unsigned int my_watershed = 0);

/** Converts an old-style version number to a c string.
 * This is used automatically by ver_to_string() if ver < watershed.
 */
const char *old_ver_to_string(unsigned int ver);

/** Converts a c string to a version number.
 * Set 'my_watershed' to non-zero to override 'wvversion_watershed'.
 */
unsigned int string_to_ver(const char *str, unsigned int my_watershed = 0);

/** Converts a c string to a new-style version number.
 * This is used automatically by string_to_ver() if string_to_ver(str) <
 * watershed.
 * This function can also understand missing zeros, which are assumed to be
 * at the front for the major and minor versions and at the end for the
 * revision.  Thus "1.2.15" would be translated as 0x01021500.
 */
unsigned int string_to_old_ver(const char *str);

#endif // __WVVERSTRING_H
