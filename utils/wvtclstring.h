/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Functions to handle "tcl-style" strings and lists.
 * 
 * Using wvtcl_encode(), you can encode _any_ list of strings into a single
 * string, then reliably split the single string back into the list using
 * wvtcl_decode().
 * 
 * You can create recursive lists of lists by simply running wvtcl_encode()
 * on a list of strings returned from wvtcl_encode().
 * 
 * Example list encodings (all of the following lists have exactly 3 elements):
 *     foo blah weasels
 *     e1 elem2 {element 3}
 *     x1 {} "element 3"
 *     w x y\ z
 * 
 * Example list of lists:
 *     foo\ blah\ weasels {e1 elem2 {element 3}} {w x y\ z}
 * 
 * FIXME:
 *   It would be possible to represent arbitrary binary blobs using this
 *   technique, but we'd have to avoid using null-terminated strings in a few
 *   places, particularly in the input to wvtcl_escape().
 * 
 *   We could even make encoded binary blobs printable (although that's not
 *   _strictly_ necessary in all cases) by encoding non-printable characters
 *   using \x## notation, if wvtcl_escape() or wvtcl_unescape() supported it.
 */
#ifndef __WVTCLSTRING_H
#define __WVTCLSTRING_H

#include "wvstringlist.h"

// the default set of "nasties", ie. characters that need to be escaped if
// they occur somewhere in a string.
#define WVTCL_NASTIES    " \t\n\r"


// {, }, \, and " are always considered "nasty."
#define WVTCL_ALWAYS_NASTY "{}\\\""


// the default set of split characters, ie. characters that separate elements
// in a list.  If these characters appear unescaped and not between {} or ""
// in a list, they signify the end of the current element.
#define WVTCL_SPLITCHARS " \t\n\r"


// tcl-escape a string.  There are three ways to do this:
//   1) Strings that need no escaping are unchanged.
//   2) Strings containing characters in 'nasties' are usually encoded just
//         by enclosing the unmodified string in braces.
//         (For example, "foo blah" becomes "{foo blah}")
//   3) Strings containing nasties _and_ unmatched braces are encoded using
//         backslash notation.  (For example, " foo} " becomes "\ foo\}\ "
WvFastString wvtcl_escape(WvStringParm s, const char *nasties = WVTCL_NASTIES);


// tcl-unescape a string.  This is generally the reverse of wvtcl_escape,
// except we can reverse any backslashified or embraced string, even if it
// doesn't follow the "simplest encoding" rules used by wvtcl_escape.  We
// can also handle strings in double-quotes, ie. '"foo"' becomes 'foo'.
WvFastString wvtcl_unescape(WvStringParm s);


// encode a tcl-style list.  This is easily done by tcl-escaping each
// string in 'l', then appending the escaped strings together, separated by
// the first char in splitchars.
WvString wvtcl_encode(WvStringList &l, const char *nasties = WVTCL_NASTIES,
		      const char *splitchars = WVTCL_SPLITCHARS);


// split a tcl-style list.  There are some special "convenience" features
// here, which allow users to create lists more flexibly than wvtcl_encode
// would do.
// 
// Elements of the list are separated by any number of any characters from
// the 'splitchars' list.
// 
// Quotes are allowed around elements: '"foo"' becomes 'foo'.  These work
// mostly like braces, except the string is assumed to be backslashified.
// That is, '"\ "' becomes ' ', whereas '{\ }' becomes '\ ' (ie. the backslash
// wouldn't be removed).
// 
// Zero-length elements must be represented by 
// 
void wvtcl_decode(WvStringList &l, WvStringParm _s,
		  const char *splitchars = WVTCL_SPLITCHARS,
		  bool do_unescape = true);


#endif // __WVTCLSTRING_H
