/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A very simple word wrapping encoder.
 */
#include "wvwordwrap.h"

WvWordWrapEncoder::WvWordWrapEncoder(int _maxwidth) :
    maxwidth(_maxwidth)
{
    line = new char[maxwidth];
    _reset();
}


WvWordWrapEncoder::~WvWordWrapEncoder()
{
    delete[] line;
}


bool WvWordWrapEncoder::_reset()
{
    width = 0;
    curindex = wordindex = 0;
    inword = false;
    return true;
}


bool WvWordWrapEncoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    while (inbuf.used() != 0)
    {
        int ch = inbuf.getch();
        switch (ch)
        {
            case '\n':
                if (! inword)
                    curindex = 0;
                flushline(outbuf);
                width = 0;
                outbuf.putch('\n');
                break;
            
            case ' ':
                if (inword)
                    flushline(outbuf);
                width += 1;
                if (width <= maxwidth)
                    line[curindex++] = ch;
                break;
            
            case '\t':
                if (inword)
                    flushline(outbuf);
                width = (width + 8) & ~7;
                if (width <= maxwidth)
                    line[curindex++] = ch;
                break;

            default:
                if (width >= maxwidth)
                {
                    if (! inword)
                    {
                        // discard trailing whitespace
                        curindex = wordindex = 0;
                        width = 0;
                    }
                    else if (wordindex == 0)
                    {
                        // insert hard line break
                        flushline(outbuf);
                        width = 0;
                    }
                    else
                    {
                        // insert soft line break
                        curindex -= wordindex;
                        memmove(line, line + wordindex, curindex);
                        wordindex = 0;
                        width = curindex;
                    }
                    outbuf.putch('\n');
                }
                if (! inword)
                {
                    inword = true;
                    wordindex = curindex;
                }
                width += 1;
                line[curindex++] = ch;
                break;
                    
        }
    }
    if (flush)
        flushline(outbuf);
    return true;
}


void WvWordWrapEncoder::flushline(WvBuffer &outbuf)
{
    outbuf.put(line, curindex);
    curindex = wordindex = 0;
    inword = false;
}
