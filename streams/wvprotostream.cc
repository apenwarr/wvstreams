/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1998 Worldvisions Computer Technology, Inc.
 * 
 * WvProtoStream is a framework that makes it easy to communicate using
 * common command-response driven protocols.  This is supposed to be flexible
 * enough to handle FTP, HTTP, SMTP, tunnelv, Weaver rcmd, and many others.
 */
#include "wvprotostream.h"
#include "wvlog.h"
#include "strutils.h"
#include <ctype.h>
#include <assert.h>


WvProtoStream::WvProtoStream(WvStream **_cloned, WvLog *_debuglog)
		: WvStreamClone(_cloned)
{
    if (_debuglog)
	logp = new WvLog(_debuglog->split(WvLog::Debug2));
    else
	logp = NULL;
    
    log_enable = true;
    state = 0;
}


WvProtoStream::~WvProtoStream()
{
    if (logp) delete logp;
}


/* Just like a WvStream::uwrite(), but it copies all output to WvLog if
 * log_enable==true.
 */
size_t WvProtoStream::uwrite(const void *buf, size_t size)
{
    if (logp && log_enable)
    {
	(*logp)("Sent: ");
	logp->write(buf, size);
	(*logp)("\n");
    }
    
    return WvStreamClone::uwrite(buf, size);
}


WvProtoStream::Token *WvProtoStream::next_token()
{
    static unsigned char whitespace[] = " \t\r\n";
    size_t len;
    
    // find and remove up to first non-whitespace
    tokbuf.get(tokbuf.match(whitespace, sizeof(whitespace)));

    // return a token up to the first whitespace character
    len = tokbuf.match(whitespace, sizeof(whitespace), true);
    return len ? new Token(tokbuf.get(len), len) : NULL;
}


WvString WvProtoStream::next_token_str()
{
    Token *t = next_token();
    if (!t) return WvString("");
    
    WvString s(t->data);
    delete t;
    return s;
}


WvString WvProtoStream::token_remaining()
{
    tokbuf.put("", 1);
    return trim_string((char *)tokbuf.get(tokbuf.used()));
}


/* Default input tokenizer.  "line" is NULL-terminated, and individual string
 * tokens are separated by any amount of whitespace.
 */
WvProtoStream::TokenList *WvProtoStream::tokenize()
{
    TokenList *tl = new TokenList;
    Token *t;

    while ((t = next_token()) != NULL)
	tl->append(t, true);
#if 0 
    if (logp && log_enable && tl->count())
    {
	(*logp)("Read: ");
	TokenList::Iter i(*tl);
	for (i.rewind(); i.next(); )
	    (*logp)("(%s) ", i.data()->data);
	(*logp)("\n");
    }
#endif
    return tl;
}


/* convert a TokenList to an array of Token.
 * The TokenList becomes invalid after this operation!
 * Remember to free the array afterwards!
 */
size_t WvProtoStream::list_to_array(TokenList *tl, Token **array)
{
    size_t total = tl->count(), count;
    
    assert(array);
    *array = new Token[total];
    
    TokenList::Iter i(*tl);
    for (count = 0, i.rewind(); i.next(); count++)
    {
	Token &t = *i.data();
	(*array)[count].fill((unsigned char *)t.data.str, t.length);
    }
    
    delete tl;
    return count;
}


/* Retrieve an input line and parse its first token.
 * This is the usual high-level interface to the input tokenizer. Remember
 * to free the array afterwards!
 * Ths input line is specifically allowed to be a NULL pointer.  In that case,
 * the returned token will be NULL also.
 */
WvProtoStream::Token *WvProtoStream::tokline(const char *line)
{ 
    if (!line) return NULL;
    
    char *newline = strdup(line);
    
    tokbuf.zap();
    tokbuf.put(line, strlen(line));

    if (strlen(trim_string(newline)) > 0)
	(*logp)("Read: %s\n", trim_string(newline));
    
    free(newline);
    
    return next_token();
}


/* returns -1 if t is not in lookup[], or else the index into lookup where
 * the token was found.
 */
int WvProtoStream::tokanal(const Token &t, char **lookup,
			   bool case_sensitive)
{
    assert(lookup);
    
    char **i;
    
    for (i = lookup; *i; i++)
    {
	if ( (!case_sensitive && !strcasecmp(t.data.str, *i))
	  || ( case_sensitive && !strcmp(t.data.str, *i)) )
	    return i - lookup;
    }
    
    return -1;
}


void WvProtoStream::do_state(Token &)
{
}


void WvProtoStream::switch_state(int newstate)
{
    state = newstate;
}


/* Default execute() function -- process a line of input, and handle it
 * (based on the current system state) using do_state().
 */
void WvProtoStream::execute()
{
    Token *t1 = tokline(getline(0));
    
    if (t1)
    {
	do_state(*t1);
	delete t1;
    }
}



//////////////////////////////////// WvProtoStream::Token



WvProtoStream::Token::Token()
{
    // leave empty -- you should call fill() manually later!
}


WvProtoStream::Token::Token(const unsigned char *_data, size_t _length)
{
    fill(_data, _length);
}


void WvProtoStream::Token::fill(const unsigned char *_data,
				size_t _length)
{
    length = _length;
    
    data.setsize(length + 1);
    memcpy(data.str, _data, length);
    data.str[length] = 0;
}


WvProtoStream::Token::~Token()
{
    // 'data' member is freed automatically
}
