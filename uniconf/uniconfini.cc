/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * A generator for .ini files.
 */
#include "uniconfini.h"
#include "wvtclstring.h"
#include "strutils.h"
#include "wvfile.h"

static void printsection(WvStream &file, const UniConfKey &key)
{
    file.print("[%s]\n", wvtcl_escape(key.strip(), "\t\r\n[]"));
}

static void printkey(WvStream &file, const UniConfKey &key,
    WvStringParm value)
{
    // need to escape []#= in key only to distinguish a key/value
    // pair from a section name or comment and to delimit the value
    file.print("%s = %s\n",
        wvtcl_escape(key.strip(), " \t\r\n[]=#"),
        wvtcl_escape(value, " \t\r\n"));
}


/***** UniConfIniFileGen *****/

UniConfIniFileGen::UniConfIniFileGen(
    WvStringParm _filename) :
    filename(_filename), log(filename)
{
    log(WvLog::Debug1, "Using IniFile \"%s\"\n", filename);
    refresh(UniConfKey::EMPTY, UniConf::INFINITE);
}


UniConfIniFileGen::~UniConfIniFileGen()
{
}


UniConfLocation UniConfIniFileGen::location() const
{
    return UniConfLocation(WvString("ini://%s", filename));
}


bool UniConfIniFileGen::refresh(const UniConfKey &key,
    UniConf::Depth depth)
{
    /** open the file **/
    WvFile file(filename, O_RDONLY);
    if (! file.isok())
    {
        log("Cannot open config file for reading: \"%s\"\n",
            file.errstr());
        file.close();
        return false;
    }

    /** wipe out all known data **/
    UniConfTempGen::remove(UniConfKey::EMPTY);
    
    /** loop over all Tcl words in the file **/
    UniConfKey section;
    WvDynamicBuffer buf;
    for (bool eof = false; ! eof; )
    {
        if (file.isok())
        {
            // read entire lines to ensure that we get whole values
            char *line = file.getline(-1);
            if (line)
                buf.putstr(line);
            else
                eof = true;
        }
        else
            eof = true;

        if (eof)
        {
            // detect missing newline at end of file
            size_t avail = buf.used();
            if (avail == 0)
                break;
            if (buf.peek(avail - 1) == '\n')
                break;
            // run the loop one more time to compensate
        }
        buf.put('\n');
        
        for (WvString word;
            ! (word = wvtcl_getword(buf, "\r\n", false)).isnull(); )
        {
            char *str = trim_string(word.edit());
            int len = strlen(str);
            if (len == 0)
            {
                // we have an empty line
                continue;
            }
            if (str[0] == '#')
            {
                // we have a comment line
                log(WvLog::Debug5, "Comment: \"%s\"\n", str + 1);
                continue;
            }
            if (str[0] == '[' && str[len - 1] == ']')
            {
                // we have a section name line
                str[len - 1] = '\0';
                WvString name(wvtcl_unescape(trim_string(str + 1)));
                section = UniConfKey(name.unique());
                log(WvLog::Debug5, "Section: \"%s\"\n", section);
                continue;
            }
            // we possibly have a key = value line
            WvConstStringBuffer line(word);
            WvString temp = wvtcl_getword(line, "=", false);
            if (! temp.isnull() && line.peek(-1) == '=')
            {
                WvString name(wvtcl_unescape(trim_string(temp.edit())));
                UniConfKey key(name.unique());
                if (! key.isempty())
                {
                    key.prepend(section);
                    temp = line.getstr();
                    WvString value(wvtcl_unescape(trim_string(temp.edit())));
                    UniConfTempGen::set(key, value.unique());
                    log(WvLog::Debug5, "Set: (\"%s\", \"%s\")\n",
                        key, value);
                    continue;
                }
            }
            log("Ignoring malformed input line: \"%s\"\n", word);
        }
    }
    dirty = false;

    /** close the file **/
    file.close();
    if (file.geterr())
    {
        log("Error reading from config file: \"%s\"\n", file.errstr());
        return false;
    }

    /** handle unparsed input **/
    size_t avail = buf.used();
    while (avail > 0 && buf.peek(avail - 1) == '\n')
    {
        buf.unalloc(1); // strip off uninteresting trailing newlines
        avail -= 1;
    }

    if (avail > 0)
    {
        // last line must have contained junk
        log("XXX Ignoring malformed input line: \"%s\"\n", buf.getstr());
    }

    /** done **/
    return true;
}


bool UniConfIniFileGen::commit(const UniConfKey &key,
    UniConf::Depth depth)
{
    /** check dirtiness **/
    if (! dirty)
        return true;
    dirty = false;

    /** open the file **/
    WvFile file(filename, O_WRONLY | O_TRUNC | O_CREAT);
    if (! file.isok())
    {
        log("Cannot open config file for writing: \"%s\"\n",
            file.errstr());
        return false;
    }

    /** iterate over all keys **/
    save(file, root);

    /** close the file **/
    file.close();
    if (file.geterr())
    {
        log("Error writing to config file: \"%s\"\n", file.errstr());
        return false;
    }

    /** done **/
    return true;
}


void UniConfIniFileGen::save(WvStream &file, UniConfTree &parent)
{
    UniConfTree::Iter it(parent);
    
    /** output values for non-empty direct or barren nodes **/
    // we want to ensure that a key with an empty value will
    // get created either by writing its value or by ensuring
    // that some subkey will create it implictly with empty value
    bool printedsection = false;
    for (it.rewind(); it.next() && file.isok(); )
    {
        UniConfTree *node = it.ptr();
        if (!! node->value() || ! node->haschildren())
        {
            if (! printedsection)
            {
                printsection(file, parent.fullkey());
                printedsection = true;
            }
            printkey(file, node->key(), node->value());
        }
    }
    if (printedsection)
        file.print("\n");

    /** output child sections **/
    for (it.rewind(); it.next() && file.isok(); )
    {
        save(file, *it);
    }
}



/***** UniConfIniFileGenFactory *****/

UniConfIniFileGen *UniConfIniFileGenFactory::newgen(
    const UniConfLocation &location)
{
    return new UniConfIniFileGen(location.payload());
}
