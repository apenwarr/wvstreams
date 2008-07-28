#include "wvattrs.h"

WvAttrs::WvAttrs() : attrlist(NULL), attrlen(0)
{
}

WvAttrs::WvAttrs(const WvAttrs &copy) : attrlist(NULL), attrlen(copy.attrlen)
{
    if (copy.attrlen) {
	attrlist = (char *)malloc((copy.attrlen + 1) * sizeof(char));
	memcpy(attrlist, copy.attrlist, copy.attrlen + 1);
    }
}

WvAttrs::~WvAttrs()
{
    free(attrlist);
}

char *WvAttrs::_get(WvStringParm name) const
{
    if (!attrlist)
	return NULL;

    const char *curpos = attrlist;
    while (*curpos)
    {
	const char *const valoffset = curpos + strlen(curpos) + 1;
	if (!strcmp(curpos, name.cstr()))
	    return (char *)valoffset; //value

	curpos = valoffset + strlen(valoffset) + 1;
    }

    return NULL;
}

void WvAttrs::set(WvStringParm name, WvStringParm value)
{
    if (!name)
	return;

    const int namelen = name.len();
    char *exists = _get(name);
    if (exists)
    {
	//We're trying to readd a key.  Sigh.  Oh well, delete and readd!
	const int toremove = namelen + strlen(exists) + 2;
	exists -= namelen + 1; //index of name, rather than value

	/* Length of part after what we want to remove */
	const int endpart = attrlen - (exists - attrlist) - toremove + 1;
	memmove(exists, exists + toremove, endpart);
	attrlen -= toremove;
	attrlist = (char *)realloc(attrlist, (attrlen + 1)
						* sizeof(char));
    }

    if (!value) /* Make a null or empty value a delete */
	return;

    const unsigned int totallen = namelen + value.len() + 2;
    attrlist = (char *)realloc(attrlist, (attrlen + totallen + 1)*sizeof(char));

    char *const beginloc = attrlist + attrlen;
    strcpy(beginloc, name.cstr());
    strcpy(beginloc + namelen + 1, value.cstr());

    attrlen += totallen;
    attrlist[attrlen] = 0;
}

WvString WvAttrs::get(WvStringParm name) const
{
    const char *const ret = _get(name);
    return ret ? ret : "";
}
