/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 * 
 * Implementation of globbing support through WvRegex
 */
#include "wvglob.h"

WvGlob::WvGlob() : WvRegex()
{
}

WvGlob::WvGlob(WvStringParm glob) : WvRegex()
{
    set(glob);
}

bool WvGlob::set(WvStringParm glob)
{
    WvString errstr;
    WvString regex = glob_to_regex(glob, &errstr);
    if (!!errstr)
        WvErrorBase::seterr(errstr);
    else if (!!regex)
        WvRegex::set(regex);
    else WvErrorBase::seterr("Failed to convert glob pattern to regex");
    return isok();
}

const bool WvGlob::normal_quit_chars[256] = {
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false
};
const bool WvGlob::brace_quit_chars[256] = {
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, true /* , */, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, true /* } */, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false
};

//
// Known bugs:
//
// - If / is part of a range it will not be excluded in the resulting regex
//   eg. fred[.-0]joe will match fred/joe (this violates glob(7))
//   However, explcit / in bracket expression results in error.
//
WvString WvGlob::glob_to_regex(const char *src, size_t &src_used,
        char *dst, size_t &dst_used, const bool quit_chars[256])
{
    enum { NORMAL, BACKSLASH, BRACKET, BRACKET_FIRST } state = NORMAL;
    src_used = 0;
    dst_used = 0;
    bool quit_now = false;
    while (!quit_now && src[src_used])
    {
        switch (state)
        {
            case NORMAL:
                if (quit_chars[(unsigned char)src[src_used]])
                {
                    quit_now = true;
                    break;
                }

                switch (src[src_used])
                {
                    case '\\':
                        state = BACKSLASH;
                        break;

                    case '[':
                        if (src[src_used+1] == '^' && src[src_used+2] == ']')
                        {
                            // Get rid of degenerate case:
                            src_used += 2;
                            if (dst) dst[dst_used] = '\\';
                            ++dst_used;
                            if (dst) dst[dst_used] = '^';
                            ++dst_used;
                        }
                        else
                        {
                            if (dst) dst[dst_used] = '(';
                            ++dst_used;
                            state = BRACKET_FIRST;
                        }
                        break;

                    case '*':
                        if (dst) dst[dst_used] = '(';
                        ++dst_used;
                        if (dst) dst[dst_used] = '[';
                        ++dst_used;
                        if (dst) dst[dst_used] = '^';
                        ++dst_used;
                        if (dst) dst[dst_used] = '/';
                        ++dst_used;
                        if (dst) dst[dst_used] = ']';
                        ++dst_used;
                        if (dst) dst[dst_used] = '*';
                        ++dst_used;
                        if (dst) dst[dst_used] = ')';
                        ++dst_used;
                        break;

                    case '?':
                        if (dst) dst[dst_used] = '(';
                        ++dst_used;
                        if (dst) dst[dst_used] = '[';
                        ++dst_used;
                        if (dst) dst[dst_used] = '^';
                        ++dst_used;
                        if (dst) dst[dst_used] = '/';
                        ++dst_used;
                        if (dst) dst[dst_used] = ']';
                        ++dst_used;
                        if (dst) dst[dst_used] = ')';
                        ++dst_used;
                        break;

                    case '{':
                        if (dst) dst[dst_used] = '(';
                        ++dst_used;
                        ++src_used;
                        while (true)
                        {
                            size_t sub_src_used, sub_dst_used;

                            WvString errstr =
                                glob_to_regex(&src[src_used], sub_src_used,
                                    dst? &dst[dst_used]: NULL, sub_dst_used,
                                    brace_quit_chars);
                            if (errstr) return errstr;

                            src_used += sub_src_used;
                            dst_used += sub_dst_used;

                            if (src[src_used] == '}')
                                break;
                            else if (src[src_used] != ',')
                                return WvString("Unfinished brace expression (index %s)", src_used);
                            if (dst) dst[dst_used] = '|';
                            ++dst_used;
                            ++src_used;
                        }
                        if (dst) dst[dst_used] = ')';
                        ++dst_used;
                        break;

                    case '^':
                    case '.':
                    case '$':
                    case '(':
                    case ')':
                    case '|':
                    case '+':
                        if (dst) dst[dst_used] = '\\';
                        ++dst_used;
                        if (dst) dst[dst_used] = src[src_used];
                        ++dst_used;
                        break;

                    default:
                        if (dst) dst[dst_used] = src[src_used];
                        ++dst_used;
                        break;
                }
                break;

            case BACKSLASH:
                switch (src[src_used])
                {
                    case '^':
                    case '.':
                    case '$':
                    case '(':
                    case ')':
                    case '|':
                    case '+':
                    case '[':
                    case '{':
                    case '*':
                    case '?':
                    case '\\':
                        if (dst) dst[dst_used] = '\\';
                        ++dst_used;
                        // fall through..
                    default:
                        if (dst) dst[dst_used] = src[src_used];
                        ++dst_used;
                        break;

                }
                state = NORMAL;
                break;

            case BRACKET_FIRST:
                switch (src[src_used])
                {
                    case '!':
                        if (dst) dst[dst_used] = '[';
                        ++dst_used;
                        if (dst) dst[dst_used] = '^';
                        ++dst_used;
                        break;

                    case '^':
                        if (dst) dst[dst_used] = '\\';
                        ++dst_used;
                        if (dst) dst[dst_used] = '^';
                        ++dst_used;
                        if (dst) dst[dst_used] = '|';
                        ++dst_used;
                        if (dst) dst[dst_used] = '[';
                        ++dst_used;
                        break;

                    case '/':
                        return WvString("Slash not allowed in bracket expression (index %s)", src_used);

                    default:
                        if (dst) dst[dst_used] = '[';
                        ++dst_used;
                        if (dst) dst[dst_used] = src[src_used];
                        ++dst_used;
                        break;
                }
                state = BRACKET;
                break;

            case BRACKET:
                switch (src[src_used])
                {
                    case ']':
                        if (dst) dst[dst_used] = ']';
                        ++dst_used;
                        if (dst) dst[dst_used] = ')';
                        ++dst_used;
                        state = NORMAL;
                        break;

                    case '/':
                        return WvString("Slash not allowed in bracket expression (index %s)", src_used);

                    default:
                        if (dst) dst[dst_used] = src[src_used];
                        ++dst_used;
                        break;
                }
                break;
        }

        if (!quit_now) ++src_used;
    }

    if (state == BRACKET || state == BRACKET_FIRST)
        return WvString("Unfinished bracket expression (index %s)", src_used);
    else if (state == BACKSLASH)
        return WvString("Unfinished backslash expression (index %s)", src_used);
    else return WvString::null;
}

WvString WvGlob::glob_to_regex(WvStringParm glob, WvString *errstr)
{
    if (glob.isnull())
    {
        if (errstr) *errstr = WvString("Glob is NULL");
        return WvString::null;
    }

    size_t src_used, dst_used;
    WvString local_errstr = glob_to_regex(glob, src_used, NULL, dst_used, normal_quit_chars);
    if (!!local_errstr)
    {
        if (errstr) *errstr = local_errstr;
        return WvString::null;
    }
    
    WvString result;
    result.setsize(1+dst_used+1+1);
    char *dst = result.edit();
    *dst++ = '^';
    local_errstr = glob_to_regex(glob, src_used, dst, dst_used, normal_quit_chars);
    if (!!local_errstr)
    {
        if (errstr) *errstr = local_errstr;
        return WvString::null;
    }
    dst += dst_used;
    *dst++ = '$';
    *dst++ = '\0';

    return result;
}
