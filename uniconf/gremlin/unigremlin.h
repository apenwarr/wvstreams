#ifndef __UNIGREMLIN_H
#define __UNIGREMLIN_H

#include "wvstring.h"
#include "wvhashtable.h"
#include "uniconfroot.h"
#include "uniconfkey.h"

struct Victim
{
    int i;
    int type;
    WvString name;
    Victim(int _i, int _type, WvStringParm _name)
    { i = _i; type = _type; name = _name; }
};
DeclareWvDict(Victim, int, i);

class UniConfGremlin
{
public:
    // runlevels are defined in curr_runlevel()
    UniConfGremlin(WvString moniker, const UniConfKey key = "", 
            int max_runlevel = 5);
    
    // FIXME: use_valid_data does not work when set to true yet
    void start(unsigned int seed = 0);
    void test(); 
    WvString status();
private:
    // creates a list of the keys and expected values in the UniConf subtree
    void find_victims(UniConfKey _key);

    void change_value(bool use_valid_data = false);
    void add_value();
    
    // begin randomly changing key values, using only valid data if
    // use_valid_data is true
    void start_trouble(int curr_runlevel);
    
    // returns what it thinks is the expected data for this element
    int inspect(WvString element);
    bool is_bool(char *elem);
    
    // returns "" if type was not a known string type
    WvString rand_str(int type);
    WvString type_name(int type); 
    WvString curr_runlevel();
    // commits for n iterations
    void spin_commit(int n); 
    
    UniConfRoot cfg;
    UniConfKey key;
    static VictimDict victims;
    int num_victims, max_runlevel, runlevel;
    WvString last_change;
};
#endif
