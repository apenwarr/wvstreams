/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Version number and string manipulations.
 */
#ifndef __WVVERSTRING_H
#define __WVVERSTRING_H

/** Converts an old-style version number to a c string.
 * The version number is a 32-bit hexadecimal number, split into a 16-bit
 * major version and a 16-bit minor version.  The string equivalent of 
 * 0x00012a00 is "1.2a".
 */
const char *ver_to_string(unsigned int ver);

/** Converts a new-style version number to a c string.
 * The new-style version number is a 32-bit hexadecimal number, split into
 * an 8-bit major version, an 8-bit minor version, and a 16-bit revision
 * number.  The string equivalent of 0x01020150 would be "1.02.0150".
 * If 'convert_old' is true, any version number starting with 00 will be
 * converted to an old-style version string (see ver_to_string()).
 */
const char *new_ver_to_string(unsigned int ver, bool convert_old = false);

/// Converts a c string to an old-style version number.
unsigned int string_to_ver(const char *str);

/// Converts a c string to a new-style version number.
unsigned int string_to_new_ver(const char *str);

#endif // __WVVERSTRING_H
