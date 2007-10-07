#include "wvstreamsdebugger.h"
#include "wvlinklist.h"


static const int command_map_size = 16;

DeclareWvList(WvStreamsDebugger);
static WvStreamsDebuggerList *debuggers;


WvStreamsDebugger::CommandMap *WvStreamsDebugger::commands;


class WvStreamsDebuggerStaticInitCleanup
{
public:
    WvStreamsDebuggerStaticInitCleanup()
    {
        WvStreamsDebugger::add_command("help",
                0, &WvStreamsDebugger::help_run_cb, 0);
    }
    ~WvStreamsDebuggerStaticInitCleanup()
    {
        assert(!debuggers || debuggers->isempty());
        
        if (WvStreamsDebugger::commands)
        {
            delete WvStreamsDebugger::commands;
            WvStreamsDebugger::commands = NULL;
        }
        
        if (debuggers)
        {
            delete debuggers;
            debuggers = NULL;
        }
    }
};
static WvStreamsDebuggerStaticInitCleanup ___;


void *WvStreamsDebugger::get_command_data(WvStringParm cmd, Command *command)
{
    if (command == NULL)
    {
        Command *pcommand = commands->find(cmd);
        if (!pcommand)
            return NULL;
        command = pcommand;
    }
    
    void **pcd = command_data.find(cmd);
    void *cd;
    if (pcd == NULL)
    {
        // In case the command has been added since our constructor
        // was executed...
        
        if (!!command->init_cb)
            cd = command->init_cb(cmd);
        else
            cd = NULL;
            
        command_data.add(cmd, cd);
    }
    else
        cd = *pcd;
    
    return cd;
}


WvStreamsDebugger::WvStreamsDebugger() :
    command_data(command_map_size)
{
    if (!debuggers)
        debuggers = new WvStreamsDebuggerList;
    debuggers->append(this, false);
    
    // Add command data for existing commands
    CommandMap::Iter i(*commands);
    for (i.rewind(); i.next(); )
        get_command_data(i->key, &i->data);
}


WvStreamsDebugger::~WvStreamsDebugger()
{
    // Remove command data
    CommandDataMap::Iter i(command_data);
    for (i.rewind(); i.next(); )
    {
        WvString cmd = i->key;
        void *cd = i->data;
        
        Command &command = (*commands)[cmd];
        if (command.cleanup_cb)
            command.cleanup_cb(cmd, cd);
    }
    command_data.zap();
    
    debuggers->unlink(this);
}


WvString WvStreamsDebugger::run(WvStringParm cmd, WvStringList &args,
        ResultCallback result_cb)
{
    Command *pcommand = commands->find(cmd);
    if (!pcommand)
        return "No such command";
    Command *command = pcommand;
   
    return command->run_cb(cmd, args, result_cb, get_command_data(cmd, command));
}


bool WvStreamsDebugger::add_command(WvStringParm cmd,
        InitCallback init_cb,
        RunCallback run_cb,
        CleanupCallback cleanup_cb)
{
    if (!commands)
        commands = new CommandMap(command_map_size);
        
    if (commands->exists(cmd))
        return false;
    commands->add(cmd, Command(init_cb, run_cb, cleanup_cb));
    return true;
}


bool WvStreamsDebugger::foreach(WvStringParm cmd, ForeachCallback foreach_cb)
{
    Command *command = commands->find(cmd);
    if (!command)
        return false;
    
    if (debuggers)
    {
        WvStreamsDebuggerList::Iter i(*debuggers);
        for (i.rewind(); i.next(); )
        {
            void *cd = i->get_command_data(cmd, command);
            foreach_cb(cmd, cd); 
        }
    }
    
    return true;
}


WvString WvStreamsDebugger::help_run_cb(WvStringParm cmd,
        WvStringList &args,
        ResultCallback result_cb, void *)
{
    WvStringList cmd_list;
    cmd_list.append("Commands availible:");
    CommandMap::Iter i(*commands);
    for (i.rewind(); i.next(); )
        cmd_list.append(i->key);
    result_cb(cmd, cmd_list);
    return WvString::null;
}
