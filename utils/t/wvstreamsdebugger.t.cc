#include "wvtest.h"
#include "wvstreamsdebugger.h"
#include "wvstream.h"

void *cmd_init(WvStringParm cmd)
{
    wvcon->print("%s: %s\n", __FUNCTION__, cmd);
    int *var = new int;
    *var = 0;
    return var;
}
WvString cmd_run(WvStringParm cmd,
        WvStringList &args,
        WvStreamsDebugger::ResultCallback result_cb, void *ud)
{
    wvcon->print("%s: %s\n", __FUNCTION__, cmd);
    WvStringList result_list;
    result_list.append("value is %s", *(const int *)ud);
    result_cb(cmd, result_list);
    return WvString::null;
}
void cmd_cleanup(WvStringParm cmd, void *ud)
{
    wvcon->print("%s: %s\n", __FUNCTION__, cmd);
    delete (int *)ud;
} 

void foreach_cb(WvStringParm cmd, void *ud)
{
    wvcon->print("%s: %s\n", __FUNCTION__, cmd);
    ++*(int *)ud;
}

static WvString result;
void result_cb(WvStringParm cmd, WvStringList &result_list)
{
    wvcon->print("%s: %s\n", __FUNCTION__, cmd);
    result = result_list.join(" ");
}   

WVTEST_MAIN("add command")
{
    WvStreamsDebugger::add_command("foo", cmd_init, cmd_run, cmd_cleanup);
    
    WvStreamsDebugger debugger;
    WvStringList args;
    WVPASS(result.isnull());
    debugger.run("foo", args, result_cb);
    WVPASSEQ(result, "value is 0");
    WvStreamsDebugger::foreach("foo", foreach_cb);
    WVPASSEQ(result, "value is 0");
    debugger.run("foo", args, result_cb);
    WVPASSEQ(result, "value is 1");    
    WvStreamsDebugger::foreach("foo", foreach_cb);
    WVPASSEQ(result, "value is 1");    
    debugger.run("foo", args, result_cb);
    WVPASSEQ(result, "value is 2");    
}
