/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Version number and string manipulations.  Version numbers are 32-bit
 * hexadecimal numbers such as 0x00012a00.  The first 16 bits are the major
 * version, and the second 16 bits are the (fractional) minor version.  For
 * example, the above example corresponds to version "1.2a" (which is the
 * version string).
 */
#include "wvtest.h"
#include "wvverstring.h"
#include <stdio.h>
#include <string.h>

/** Tests ver_to_string() and string_to_ver().
 * Given a hex number 0xabcdefgh, where each letter represents a hex
 * digit, ver_to_string should return a string of the form "abcd.efgh",
 * with any meaningless zeros removed.
 *
 * Similarly, given a string of the form "abcd.efgh", string_to_ver
 * should return a hex number of the form 0xabcdefgh, with implicit
 * meaningless zeros inserted.
 */
WVTEST_MAIN("wvvertest.cc")
{
    if (!WVFAIL(strcmp(ver_to_string(0x99998888), "9999.8888")))
        printf("   because [%s] != [9999.8888]\n", ver_to_string(0x99998888));
    if (!WVPASS(string_to_ver("1.0") == 0x00010000))
        printf("   because [%08x] != [00010000]\n", string_to_ver("1.0"));
    
    if (!WVFAIL(strcmp(ver_to_string(0x01a00200), "1a0.02")))
        printf("   because [%s] != [1a0.02]\n", ver_to_string(0x01a00200));
    if (!WVPASS(string_to_ver(".02a") == 0x000002a0))
        printf("   because [%08x] != [000002a0]\n", string_to_ver(".02a"));    
    
    if (!WVFAIL(strcmp(ver_to_string(0x00001000), "0.1")))
        printf("   because [%s] != [0.1]\n", ver_to_string(0x00001000));
    if (!WVPASS(string_to_ver("1b") == 0x001b0000))
        printf("   because [%08x] != [001b0000]\n", string_to_ver("1b"));    
    
    if (!WVFAIL(strcmp(ver_to_string(0x00000000), "0.0")))
        printf("   because [%s] != [0.0]\n", ver_to_string(0x00000000));
    if (!WVPASS(string_to_ver("1A.") == 0x001a0000))
        printf("   because [%08x] != [001a0000]\n", string_to_ver("1A."));

    if (!WVFAIL(strcmp(new_ver_to_string(0x99887777), "99.88.7777")))
        printf("   because [%s] != [99.88.7777]\n",
	       new_ver_to_string(0x99887777));
    if (!WVPASS(string_to_new_ver("1.00.0") == 0x01000000))
        printf("   because [%08x] != [01000000]\n",
	       string_to_new_ver("1.00.0"));
    
    if (!WVFAIL(strcmp(new_ver_to_string(0x01a00200), "1.a0.02")))
        printf("   because [%s] != [1.a0.02]\n",
	       new_ver_to_string(0x01a00200));
    if (!WVPASS(string_to_new_ver(".02.a") == 0x0002a000))
        printf("   because [%08x] != [0002a000]\n",
	       string_to_new_ver(".02.a"));    
    if (!WVPASS(string_to_new_ver(".5.37ab") == 0x000537ab))
        printf("   because [%08x] != [000537ab]\n",
	       string_to_new_ver(".5.37a"));    
    
    if (!WVFAIL(strcmp(new_ver_to_string(0x00001000), "0.00.1")))
        printf("   because [%s] != [0.00.1]\n", new_ver_to_string(0x00001000));
    if (!WVPASS(string_to_new_ver("1b") == 0x1b000000))
        printf("   because [%08x] != [1b000000]\n", string_to_new_ver("1b"));

    if (!WVFAIL(strcmp(new_ver_to_string(0x00001000, true), "0.1")))
	printf("   because [%s] != [0.1]\n", new_ver_to_string(0x00001000,
							       true));
    
    if (!WVFAIL(strcmp(new_ver_to_string(0x00000000), "0.00.0")))
        printf("   because [%s] != [0.00.0]\n", new_ver_to_string(0x00000000));
    if (!WVPASS(string_to_new_ver("1A.") == 0x1a000000))
        printf("   because [%08x] != [1a000000]\n", string_to_new_ver("1A."));

    if (!WVFAIL(strcmp(new_ver_to_string(0x00000000, true), "0.0")))
        printf("   because [%s] != [0.0]\n", new_ver_to_string(0x00000000,
							       true));
}
