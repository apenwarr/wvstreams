/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * C-style backslash escaping and unescaping of strings.
 */
#ifndef __WVBACKSLASH_H
#define __WVBACKSLASH_H

#include "wvencoder.h"

/**
 * An encoder that performs C-style backslash escaping of strings.
 * <p>
 * Use this to escape control characters, unprintable characters,
 * and optionally quotes or any other special printable characters
 * into sequences of the form \\n, \\xFF, \\", etc...
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvBackslashEncoder : public WvEncoder
{
    WvString nasties;

public:
    /**
     * Creates a C-style backslash encoder.
     *   nasties - the set of printable characters to escape
     *             in addition to the non-printable ones
     *             (should always contain at least backslash)
     */
    WvBackslashEncoder(WvStringParm _nasties = "\\\"");
    virtual ~WvBackslashEncoder() { }

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _reset();
};


/**
 * An encoder that performs C-style backslash unescaping of strings.
 * <p>
 * Recognizes the following sequences preceeded by backslash:
 * <ul>
 * <li>a: substitutes alarm bell (ascii 7)</li>
 * <li>b: substitutes backspace (ascii 8)</li>
 * <li>f: substitutes formfeed (ascii 12)</li>
 * <li>n: substitutes newline (ascii 10)</li>
 * <li>r: substitutes carriage return (ascii 13)</li>
 * <li>t: substitutes tab (ascii 9)</li>
 * <li>v: substitutes vertical tab (ascii 11)</li>
 * <li>0: substitutes null (ascii 0)</li>
 * <li>0xyz: substitutes character with octal encoding xyz</li>
 * <li>xxy: substitutes character with hex encoding xy</li>
 * <li>\\: substitutes backslash</li>
 * <li>otherwise substitutes the next character (strips the backslash)</li>
 * </ul>
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvBackslashDecoder : public WvEncoder
{
    enum State
        { Initial, Escape, Hex1, Hex2, Octal1, Octal2, Octal3 };
    State state;
    WvInPlaceBuffer tmpbuf;
    int value;

public:
    /**
     * Creates a C-style backslash decoder.
     */
    WvBackslashDecoder();
    virtual ~WvBackslashDecoder() { }

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _reset();

private:
    bool flushtmpbuf(WvBuffer &outbuf);
};

#endif // __WVBACKSLASH_H
