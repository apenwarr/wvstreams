#include "wvfdstream.h"
#include "wvistreamlist.h"
#include "wvstrutils.h"
#include "wvunixsocket.h"
#include <readline/readline.h>
#include <readline/history.h>


class WvReadLineStream : public WvStream
{
    static WvReadLineStream *me;
    WvStream *base;
    WvString prompt;
    WvDynBuf line_buf;
    WvStringList commands;

    virtual size_t uread(void *_buf, size_t count)
    {
        size_t result = 0;
        char *buf = (char *)_buf;
        while (count > 0 && line_buf.used() > 0)
        {
            size_t chunk = line_buf.optgettable();
            if (chunk > count)
                chunk = count;
            memcpy(buf, line_buf.get(chunk), chunk);
            count -= chunk;
            buf += chunk;
            result += chunk;
        }
        return result;
    }

    virtual size_t uwrite(const void *_buf, size_t count)
    {
        const char *buf = (const char *)_buf;
        for (size_t i=0; i<count; ++i)
        {
            if (buf[i] == '\n')
                rl_crlf();
            else
                rl_show_char(buf[i]);
        }
        return count;        
    }

    static void readline_callback(char *str)
    {
        if (str == NULL)
            return;
        size_t len = strlen(str);
        if (len == 0)
            return;
        me->line_buf.put(str, len);
        me->line_buf.putch('\n');
        add_history(str);
    }

    static int readline_getc(FILE *)
    {
        char ch;
        assert(me->base->read(&ch, 1) == 1);
        return ch;
    }

    static char *readline_command_completion_function(const char *text, int state)
    {
        static int skip = 0;
        if (state == 0)
            skip = 0;
        int my_skip = skip;
        size_t len = strlen(text);
        WvStringList::Iter i(me->commands);
        for (i.rewind(); i.next(); )
        {
            if (my_skip-- > 0)
                continue;
            ++skip;
            if (i->len() >= len && strncmp(*i, text, len) == 0)
                return strdup(*i);
        }
        return NULL;       
    }

    virtual void pre_select(SelectInfo &si)
    {
        if (si.wants.readable && line_buf.used() > 0)
            si.msec_timeout = 0;
        
        base->pre_select(si);
    }

    virtual bool post_select(SelectInfo &si)
    {
        bool now = false;
        if (si.wants.readable && line_buf.used() > 0)
            now = true;

        while (base->isreadable())
            rl_callback_read_char();
        return base->post_select(si) || now;
    }

public:

    WvReadLineStream(WvStream *_base, WvStringParm _prompt)
    {
        base = _base;
        prompt = _prompt;

        assert(!me);
        me = this;
        set_wsname("readline on %s", base->wsname());
        rl_already_prompted = 1;
        rl_completion_entry_function = readline_command_completion_function;
        rl_callback_handler_install(prompt, readline_callback);
        rl_getc_function = readline_getc;
    }

    ~WvReadLineStream()
    {
        rl_getc_function = NULL;
        rl_callback_handler_remove();
        me = NULL;
    }

    virtual bool isok() const
    {
        return WvStream::isok() && base->isok();
    }

    void display_prompt()
    {
        base->print("%s", prompt);
        rl_already_prompted = 1;
    }

    void set_commands(const WvStringList &_commands)
    {
        commands.zap();
        WvStringList::Iter i(_commands);
        for (i.rewind(); i.next(); )
            commands.append(*i);
    }

    const char *wstype() const { return "WvReadLineStream"; }
};


WvReadLineStream *WvReadLineStream::me = NULL;


void remote_cb(WvStream &remote, void *_local)
{
    WvReadLineStream &local = *(WvReadLineStream *)_local;

    const char *line = remote.getline();
    if (line == NULL)
        return;

    WvStringList words;
    wvtcl_decode(words, line);

    WvString first = words.popstr();
    bool last_line = !!first && first != "-";
    if (last_line)
        local.print("%s ", first);
    local.print("%s\n", words.join(" "));
    if (last_line)
        local.display_prompt();

    if (words.popstr() == "Commands availible:")
        local.set_commands(words);
}


void local_cb(WvStream &_local, void  *_remote)
{
    WvReadLineStream &local = (WvReadLineStream &)_local;
    WvStream &remote = *(WvStream *)_remote;

    const char *line = local.getline();
    if (line == NULL)
        return;

    if (strcmp(line, "quit") == 0)
        remote.close();

    remote.print("%s\n", line);
}


int main(int argc, char **argv)
{
    WvReadLineStream readlinestream(wvcon, "> ");

    const char *sockname = "/tmp/weaver.wsd";
    if (argc >= 2)
        sockname = argv[1];

    WvUnixConn *s = new WvUnixConn(sockname);
    if (!s->isok())
    {
        wverr->print("Failed to connect to %s: %s\n",
                sockname, s->errstr());
        return 1;
    }
    s->set_wsname("%s", sockname);
    s->print("help\n");

    s->setcallback(remote_cb, &readlinestream);
    WvIStreamList::globallist.append(s, true);

    readlinestream.setcallback(local_cb, s);
    WvIStreamList::globallist.append(&readlinestream, false);

    while (s->isok() && readlinestream.isok())
        WvIStreamList::globallist.runonce();

    return 0;
}

