/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 */
#include "wvtest.h"
#include "wvverstring.h"
#include <stdio.h>
#include <string.h>

/** Tests ver_to_string(), string_to_ver(), new_ver_to_string(), and
 * string_to_new_ver().
 * 
 * Given a hex number 0xabcdefgh, where each letter represents a hex
 * digit, ver_to_string should return a string of the form "abcd.efgh",
 * with any meaningless zeros removed.  new_ver_to_string should return
 * a string of the form "ab.cd.efgh", with zeroes at the beginning
 * and end removed (but 'c' and 'd' are never removed to preserve
 * sorting order).
 *
 * Similarly, given a string of the form "abcd.efgh", string_to_ver
 * should return a hex number of the form 0xabcdefgh, with implicit
 * meaningless zeros inserted.  string_to_new_ver would translate
 * "ab.cd.efgh" as 0xabcdefgh.  string_to_new_ver should also translate
 * "ab.c.def" as 0xab0cdef0.
 */

WVTEST_MAIN("version functions")
{
    if (!WVFAIL(strcmp(old_ver_to_string(0x99998888), "9999.8888")))
        printf("   because [%s] != [9999.8888]\n", old_ver_to_string(0x99998888));
    if (!WVPASS(string_to_old_ver("1.0") == 0x00010000))
        printf("   because [%08x] != [00010000]\n", string_to_old_ver("1.0"));
    
    if (!WVFAIL(strcmp(old_ver_to_string(0x01a00200), "1a0.02")))
        printf("   because [%s] != [1a0.02]\n", old_ver_to_string(0x01a00200));
    if (!WVPASS(string_to_old_ver(".02a") == 0x000002a0))
        printf("   because [%08x] != [000002a0]\n", string_to_old_ver(".02a"));    
    
    if (!WVFAIL(strcmp(old_ver_to_string(0x00001000), "0.1")))
        printf("   because [%s] != [0.1]\n", old_ver_to_string(0x00001000));
    if (!WVPASS(string_to_old_ver("1b") == 0x001b0000))
        printf("   because [%08x] != [001b0000]\n", string_to_old_ver("1b"));    
    
    if (!WVFAIL(strcmp(old_ver_to_string(0x00000000), "0.0")))
        printf("   because [%s] != [0.0]\n", old_ver_to_string(0x00000000));
    if (!WVPASS(string_to_old_ver("1A.") == 0x001a0000))
        printf("   because [%08x] != [001a0000]\n", string_to_old_ver("1A."));

    // new-style versions
    if (!WVFAIL(strcmp(ver_to_string(0x99887777), "99.88.7777")))
        printf("   because [%s] != [99.88.7777]\n", ver_to_string(0x99887777));
    if (!WVFAIL(strcmp(ver_to_string(0x01a00200), "1.a0.02")))
        printf("   because [%s] != [1.a0.02]\n", ver_to_string(0x01a00200));
    if (!WVFAIL(strcmp(ver_to_string(0x00001000), "0.00.1")))
        printf("   because [%s] != [0.00.1]\n", ver_to_string(0x00001000));
    if (!WVFAIL(strcmp(ver_to_string(0x00000000), "0.00.0")))
        printf("   because [%s] != [0.00.0]\n", ver_to_string(0x00000000));
    if (!WVFAIL(strcmp(ver_to_string(0x04010000), "4.01.0")))
        printf("   because [%s] != [4.01.0]\n", ver_to_string(0x04010000));

    if (!WVPASS(string_to_ver("1.00.0") == 0x01000000))
        printf("   because [%08x] != [01000000]\n", string_to_ver("1.00.0"));
    if (!WVPASS(string_to_ver("3.66c") == 0x0366c000))
        printf("   because [%08x] != [0366c000]\n", string_to_ver("3.66c"));
    if (!WVPASS(string_to_ver(".02.a") == 0x0002a000))
        printf("   because [%08x] != [0002a000]\n", string_to_ver(".02.a"));
    if (!WVPASS(string_to_ver("4.1.2a6") == 0x04012a60))
        printf("   because [%08x] != [04012a60]\n", string_to_ver("4.1.2a6"));
    if (!WVPASS(string_to_ver(".5.37ab") == 0x000537ab))
        printf("   because [%08x] != [000537ab]\n", string_to_ver(".5.37a"));
    if (!WVPASS(string_to_ver("1b") == 0x1b000000))
        printf("   because [%08x] != [1b000000]\n", string_to_ver("1b"));
    if (!WVPASS(string_to_ver("1A.") == 0x1a000000))
        printf("   because [%08x] != [1a000000]\n", string_to_ver("1A."));
    if (!WVPASS(string_to_ver("4.01") == 0x04010000))
        printf("   because [%08x] != [04010000]\n", string_to_ver("4.01"));

    wvversion_watershed = 0x01000000;
    if (!WVFAIL(strcmp(ver_to_string(0x01000001), "1.00.0001")))
        printf("   because [%s] != [1.00.0001]\n", ver_to_string(0x01000001));
    if (!WVFAIL(strcmp(ver_to_string(0x00999999), "99.9999")))
        printf("   because [%s] != [99.9999]\n", ver_to_string(0x00999999));
    if (!WVPASS(string_to_ver("1.1") == 0x01010000))
        printf("   because [%08x] != [01010000]\n", string_to_ver("1.1"));
    if (!WVPASS(string_to_ver("0.9") == 0x00009000))
        printf("   because [%08x] != [00009000]\n", string_to_ver("0.9"));

    wvversion_watershed = 0x04010000;
    if (!WVPASS(string_to_ver("4.01.0001") == 0x04010001))
        printf("   because [%08x] != [04010001]\n", string_to_ver("4.01.0001"));
}
