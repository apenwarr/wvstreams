#ifndef __UNIGREMLIN_H
#define __UNIGREMLIN_H

#include "wvstring.h"
#include "wvhashtable.h"
#include "uniconfroot.h"
#include "uniconfkey.h"

#define MAX_DEPTH       5
#define TYPE_STRING     0
#define TYPE_IP         1
#define TYPE_IP_NETWORK 2
#define TYPE_HOSTNAME   3
#define TYPE_BOOL       4
#define TYPE_INT        5
#define TYPE_FLOAT      6
#define NUM_TYPES       7
// Options for data.
enum Data {
    Create = (1 << 0),      // 00000001
    Delete = (1 << 1),      // 00000010
    Modify = (1 << 2),      // 00000100
    Valid = (1 << 3),       // 00001000
    Invalid = (1 << 4),     // 00010000
    Correct = (1 << 5),     // 00100000
    Incorrect = (1 << 6)    // 01000000
};
 
struct Victim
{
    int i;     // index
    int type;  // type of data
    WvString value; // value of data
    Victim(int _i, int _type, WvStringParm _value) : i(_i), type(_type), value(_value)
    {
    }
};

// correct data harvested off of the existing UniConf
struct Harvested
{
    int i;
    WvString value;
    Harvested(int _i, WvStringParm _value) : i(_i), value(_value) 
    {
    }
};

DeclareWvDict(Victim, int, i);
DeclareWvDict(Harvested, int, i);

class UniConfGremlin
{
public:
    // runlevels are defined in curr_runlevel()
    UniConfGremlin(WvString moniker, const UniConfKey key);
    ~UniConfGremlin();
    
    // FIXME: use_valid_data does not work when set to true yet
    void start(unsigned int seed = 0, int flags =
            Create|Modify|Valid|Invalid|Correct|Incorrect, 
            int _ratio_create = 60, int _ratio_valid = 90, 
            int _ratio_correct = 70);
    void test();
    WvString status();
private:
    // creates a list of the keys and expected values in the UniConf subtree
    void find_victims();

    void change_value(bool use_valid_data = false);
    void add_value();
    
    // begin randomly changing key values, using only valid data if
    // use_valid_data is true
    void start_trouble(int howmany);
    
    // returns what it thinks is the expected data for this element
    int inspect(WvString element);
    bool is_bool(char *elem);
    
    // returns "" if type was not a known string type
    WvString rand_str(int type);
    WvString type_value(int type); 
    WvString curr_runlevel();
    // commits for n iterations
    void spin_commit(int n);
    int randInt(int max, int min = 0);
    
    UniConfRoot root;
    UniConf cfg;
    VictimDict victims;
    HarvestedDict *harvested[NUM_TYPES];
    int num_victims, num_harvested[NUM_TYPES], ratio_create, ratio_valid, 
        ratio_correct;
    char flags;
    WvString last_change;
    bool modify_valid, modify_invalid, add_values, delete_values;
};
#endif
