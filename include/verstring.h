/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
/** \file
 * Version number and string manipulations.  Version numbers are 32-bit
 * hexadecimal numbers such as 0x00012a00.  The first 16 bits are the major
 * version, and the second 16 bits are the (fractional) minor version.  For
 * example, the above example corresponds to version "1.2a" (which is the
 * version string).
 */

#ifndef __VERSTRING_H
#define __VERSTRING_H


/// converts a version number to a c string.
const char *ver_to_string(unsigned int ver);

/// converts a c string to a version number
unsigned int string_to_ver(const char *str);

#endif // __VERSTRING_H
