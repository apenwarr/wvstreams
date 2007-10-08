#include "wvstreamsdebugger.h"
#include "wvlinklist.h"

using std::make_pair;


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
	CommandMap::iterator it = commands->find(cmd);
	if (it == commands->end())
	    return NULL;
	command = &it->second;
    }

    void *cd;
    CommandDataMap::iterator it = command_data.find(cmd);
    if (it == command_data.end())
    {
        // In case the command has been added since our constructor
        // was executed...
        if (command->init_cb)
            cd = command->init_cb(cmd);
        else
            cd = NULL;
            
        command_data[cmd] = cd;
    }
    else
	cd = it->second;

    return cd;
}


WvStreamsDebugger::WvStreamsDebugger()
{
    if (!debuggers)
        debuggers = new WvStreamsDebuggerList;
    debuggers->append(this, false);
    
    // Add command data for existing commands
    CommandMap::iterator it;
    for (it = commands->begin(); it != commands->end(); ++it)
        get_command_data(it->first, &it->second);
}


WvStreamsDebugger::~WvStreamsDebugger()
{
    // Remove command data
    CommandDataMap::iterator it;
    for (it = command_data.begin(); it != command_data.end(); ++it)
    {
	CommandMap::iterator it2 = commands->find(it->first);
	if (it2 != commands->end() && it2->second.cleanup_cb)
            it2->second.cleanup_cb(it->first, it->second);
    }
    command_data.clear();
    
    debuggers->unlink(this);
}


WvString WvStreamsDebugger::run(WvStringParm cmd, WvStringList &args,
				ResultCallback result_cb)
{
    CommandMap::iterator it = commands->find(cmd);
    if (it == commands->end())
        return "No such command";
    Command *command = &it->second;
   
    return command->run_cb(cmd, args, result_cb,
			   get_command_data(cmd, command));
}


bool WvStreamsDebugger::add_command(WvStringParm cmd,
        InitCallback init_cb,
        RunCallback run_cb,
        CleanupCallback cleanup_cb)
{
    if (!commands)
        commands = new CommandMap;

    return commands->insert(
	make_pair(cmd, Command(init_cb, run_cb, cleanup_cb))).second;
}


bool WvStreamsDebugger::foreach(WvStringParm cmd, ForeachCallback foreach_cb)
{
    CommandMap::iterator it = commands->find(cmd);

    if (it == commands->end())
        return false;
    
    if (debuggers)
    {
        WvStreamsDebuggerList::Iter i(*debuggers);
        for (i.rewind(); i.next(); )
        {
            void *cd = i->get_command_data(cmd, &it->second);
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
    cmd_list.append("Commands available:");
    CommandMap::iterator it;
    for (it = commands->begin(); it != commands->end(); ++it)
	cmd_list.append(it->first);
    result_cb(cmd, cmd_list);
    return WvString::null;
}
