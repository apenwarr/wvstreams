#include "wvstreamclone.h"
#include "wvlog.h"
#include <ctype.h>
#include <assert.h>

class WvProtoStream : public WvStreamClone
{
public:
    WvProtoStream(WvStream **_cloned, int start_state = 0,
		  WvLog *_debuglog = NULL);
    virtual ~WvProtoStream();
    
    class Token;
    class TokenList;
    
    // override uwrite() so we can log all output
    virtual size_t uwrite(const void *buffer, size_t size);

    virtual void execute();
    
    // Routines to convert an input line into a list of Tokens.
    virtual TokenList *tokenize(const char *line);
    size_t list_to_array(TokenList *tl, Token **array);
    size_t tokline(Token **array);
    
    // Convert token strings to enum values
    int tokanal(const Token *t, char **lookup,
		bool case_sensitive = false);
    
    // finite state machine
    int state;
    virtual void do_state(Token *t, size_t nt);
    virtual void switch_state(int newstate);
    
protected:
    WvLog *log;
    bool log_enable;
    
public:
    class Token
    {
    public:
	WvString data;
	size_t length;
	int extra;
	
	Token();
	Token(const char *_data, size_t _length, int _extra = 0);
	void fill(const char *_data, size_t _length, int _extra);
	~Token();
    };
    
    DeclareWvList(Token);
};


WvProtoStream::WvProtoStream(WvStream **_cloned, int start_state,
			     WvLog *_debuglog)
		: WvStreamClone(_cloned)
{
    if (_debuglog)
	log = new WvLog(_debuglog->split(WvLog::Debug2));
    else
	log = NULL;
    
    log_enable = true;
    state = 0;
    
    switch_state(start_state);
}


WvProtoStream::~WvProtoStream()
{
    if (log) delete log;
}


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


enum States {
    None = 0,
    One,
    Two,
    Three,
    Four,
};

char *toks[] = {
    "quit",
    "one",
    "Two",
    "THREE",
    "four",
    NULL
};


void WvProtoStream::do_state(Token *t, size_t nt)
{
    enum States tok = (States)tokanal(t, toks, false);

    if (tok < 0)
	print("ERROR Unknown command (%s)\n", t->data);
    else
	switch_state(tok);
}


void WvProtoStream::switch_state(int newstate)
{
    switch ((States)newstate)
    {
    case Three:
	if (state != Two)
	{
	    print("ERROR Only from state two!\n");
	    break;
	}
	
	// else fall through!

    case One:
    case Two:
    case Four:
	print("GO %s (%s)\n", newstate, toks[newstate]);
	break;
	
    case None:
	close();
	break;
    }

    state = newstate;
}


void WvProtoStream::execute()
{
    Token *t;
    size_t nt = tokline(&t);
    
    if (nt > 0)
	do_state(t, nt);
    
    if (nt) delete[] t;
}


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


int main()
{
    WvLog log("prototest");
    WvProtoStream ps(&wvcon, 1, &log);
    
    log("Starting...\n");
    
    while (ps.isok())
    {
	if (ps.select(100))
	    ps.callback();
    }
}
