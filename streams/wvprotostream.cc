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
#include <ctype.h>
#include <assert.h>


WvProtoStream::WvProtoStream(WvStream **_cloned, WvLog *_debuglog)
		: WvStreamClone(_cloned)
{
    if (_debuglog)
	log = new WvLog(_debuglog->split(WvLog::Debug2));
    else
	log = NULL;
    
    log_enable = true;
    state = 0;
}


WvProtoStream::~WvProtoStream()
{
    if (log) delete log;
}


/* Just like a WvStream::uwrite(), but it copies all output to WvLog if
 * log_enable==true.
 */
size_t WvProtoStream::uwrite(const void *buf, size_t size)
{
    if (log && log_enable)
    {
	(*log)("Send: ");
	log->write(buf, size);
	(*log)("\n");
    }
    
    return WvStreamClone::uwrite(buf, size);
}


/* Default input tokenizer.  "line" is NULL-terminated, and individual string
 * tokens are separated by any amount of whitespace.
 */
WvProtoStream::TokenList *WvProtoStream::tokenize(const char *line)
{
    TokenList *tl = new TokenList;
    const char *sptr, *cptr;
    
    for (sptr = cptr = line; *cptr; cptr++)
    {
	if (isblank(*(unsigned char *)cptr))
	{
	    if (cptr > sptr)
		tl->append(new Token(sptr, cptr-sptr), true);
	    sptr = cptr + 1; // skip whitespace
	}
    }
    
    if (cptr > sptr)
	tl->append(new Token(sptr, cptr-sptr), true);
    
    if (log && log_enable)
    {
	(*log)(" Got: ");
	TokenList::Iter i(*tl);
	for (i.rewind(); i.next(); )
	    (*log)("(%s) ", i.data()->data);
	(*log)("\n");
    }
    
    return tl;
}


/* convert a TokenList to an array of Token.
 * The TokenList becomes invalid after this operation!
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
	(*array)[count].fill(t.data.str, t.length, t.extra);
    }
    
    delete tl;
    return count;
}


/* Retrieve an input line and convert it to an array of tokens.  This is the
 * usual high-level interface to the input tokenizer.
 */
size_t WvProtoStream::tokline(Token **array)
{ 
    char *line = getline(0);
    
    if (!line) return 0;
    return list_to_array(tokenize(line), array);
}


/* returns -1 if t is not in lookup[], or else the index into lookup where
 * the token was found.
 */
int WvProtoStream::tokanal(const Token *t, char **lookup,
			   bool case_sensitive)
{
    assert(t);
    assert(lookup);
    
    char **i;
    
    for (i = lookup; *i; i++)
    {
	if ( (!case_sensitive && !strcasecmp(t->data.str, *i))
	  || ( case_sensitive && !strcmp(t->data.str, *i)) )
	    return i - lookup;
    }
    
    return -1;
}


void WvProtoStream::do_state(Token *, size_t)
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
    Token *t;
    size_t nt = tokline(&t);
    
    if (nt > 0)
	do_state(t, nt);
    
    if (nt) delete[] t;
}



//////////////////////////////////// WvProtoStream::Token



WvProtoStream::Token::Token()
{
    // leave empty -- you should call fill() manually later!
}


WvProtoStream::Token::Token(const char *_data, size_t _length, int _extra)
{
    fill(_data, _length, _extra);
}


void WvProtoStream::Token::fill(const char *_data, size_t _length, int _extra)
{
    length = _length;
    
    data.setsize(length + 1);
    memcpy(data.str, _data, length);
    data.str[length] = 0;
    
    extra = _extra;
}


WvProtoStream::Token::~Token()
{
    // 'data' member is freed automatically
}


