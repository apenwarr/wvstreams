#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <regex.h>
#include <ctype.h>
#include "unigremlin.h"

#define MAX_DEPTH       5
#define TYPE_STRING     0
#define TYPE_IP         1
#define TYPE_IP_NETWORK 2
#define TYPE_HOSTNAME   3
#define TYPE_BOOL       4
#define TYPE_INT        5
#define TYPE_FLOAT      6
#define NUM_TYPES       7

VictimDict UniConfGremlin::victims(500);

UniConfGremlin::UniConfGremlin(WvString moniker, const UniConfKey _key,
        int _max_runlevel)
    : cfg(moniker), key(_key), max_runlevel(_max_runlevel)
{
    runlevel = 1;
    num_victims = 0;
    if (max_runlevel > 5 || max_runlevel < 1)
        max_runlevel = 4;
}

/* start(unsigned int seed)
 * Starts the gremlin out using the specified seed to seed the random numbers,
 * or if no seed is specified, uses the current time to seed them.
 */
void UniConfGremlin::start(unsigned int seed)
{
    if (seed == 0)
        seed = time(0);
    
    srand(seed);
    cout << "Using seed " << seed << "\n";
    find_victims(key);
    for (runlevel = 1; runlevel <= max_runlevel; runlevel ++)
        start_trouble(runlevel);
    runlevel = max_runlevel - 1;
}

/* find_victims(UniConfKey _key)
 * Iterates through each element in the tree and adds a new victim to the list
 * victims with the element's key and a guess at what type of data should be
 * stored there.
 */
void UniConfGremlin::find_victims(UniConfKey _key)
{
    UniConf::Iter i(cfg[_key]);
    Victim *victim;
    for (i.rewind(); i.next(); )
    {   
        if (i().get() == "")
            find_victims(i().fullkey());
        else
        {
            victim = new Victim(num_victims, inspect(i().get()), i().fullkey());
            victims.add(victim, true);
            num_victims ++;
        }
    }
}

/* inspect(WvString element)
 * Inspects the string element in an attempt to guess what type of data should
 * be stored there.  Returns an integer representing what type the gremlin
 * thinks it is.
 */
int UniConfGremlin::inspect(WvString element)
{
    char *elem = element.edit();
    // rough expression for an ip network
    char *ipNetExp = "[0-9].*\\.[0-9].*\\.[0-9].*\\.[0-9].*";
    if (is_bool(elem))
        return TYPE_BOOL;
    
    if (isdigit(elem[0]))
    {
        regex_t preg1;
        regmatch_t pmatch1[1];
        regcomp(&preg1, ipNetExp, REG_EXTENDED | REG_NOSUB);
        
        if (!regexec(&preg1, elem, 1, pmatch1, 0))
        {
            if (elem[element.len() - 2] == '/' || 
                elem[element.len() - 3] == '/')
                return TYPE_IP_NETWORK;
            else
                return TYPE_IP;
        }
    }
    // now we just brute force the rest of the string
    for (size_t i = 1; i < element.len(); i++)
    {
        if (!isdigit(elem[i]))
        // either string or hostname, return string for now
            return TYPE_STRING;
    }
    // either int or float, return int for now
    return TYPE_INT;
}

/* is_bool(char *elem)
 * Returns true if the string stored in elem represents a boolean valuation
 */
bool UniConfGremlin::is_bool(char *elem)
{
    const char *strs[] = {
        "true", "yes", "on", "enabled", "1",
        "false", "no", "off", "disabled", "0"
    };
    
    for (size_t i = 0; i < sizeof(strs) / sizeof(const char*); ++i)
        if (strcasecmp(elem, strs[i]) == 0)
            return true;
    
    return false;
}

/* change_value(bool use_valid_data)
 * Changes a random value from the list of victims, the victim list includes
 * previously deleted victims as well, so this could potentially add previously
 * deleted values back in as well.  Changes it with new random data.
 */
void UniConfGremlin::change_value(bool use_valid_data)
{
    int r = num_victims + 1;
    while (r >= num_victims)
    {
        r = (int)(((double)rand() / (double)RAND_MAX) * (num_victims));
    }
    last_change = WvString("Changed %s to be the value", victims[r]->name);
    WvString new_value;
    if (use_valid_data)
    {
        if (victims[r]->type == TYPE_INT)
        {
            int new_value = rand();
            cfg[victims[r]->name].setint(new_value);
            last_change = WvString("%s %s\n", last_change, new_value);
        }
        else 
        {
            WvString new_value = rand_str(victims[r]->type);
            cfg[victims[r]->name].set(new_value);
            last_change = WvString("%s %s\n", last_change, new_value);
        }
    }
    else
    {
        int s = (int)(((double)rand() / (double)RAND_MAX) * 2);
        if (s)
        {
            int new_value = rand();
            cfg[victims[r]->name].setint(rand());
            last_change = WvString("%s %s\n", last_change, new_value);
        }
        else
        {
            WvString new_value = rand_str(victims[r]->type);
            int t = (int)(((double)rand() / (double)RAND_MAX) * NUM_TYPES);
            cfg[victims[r]->name].set(rand_str(t));
            last_change = WvString("%s %s\n", last_change, new_value);
        }
            
    }
    cfg[victims[r]->name].commit();
            
}

/* add_value()
 * Adds a random value into the UniConf also randomly selecting the type and 
 * contents.
 */
void UniConfGremlin::add_value()
{
    int depth = (int)(((double)rand() / (double)RAND_MAX) * MAX_DEPTH);
    WvString key_name = "", elem;
    
    // generate a key name 32 characters or less
    for (int i = 0; i < depth; i++)
    {
        elem = "";
        int length = (int)(((double)rand() / (double)RAND_MAX) * 8);
        for (int j = 0; j < length; j++)
        {
            // generate a valid written character (lower case right now)
            int num = (int)((((double)rand() / (double)RAND_MAX) * 25)
                     + 97);
            char c[2];
            c[0] = (char)num;
            c[1] = '\0';
            elem = WvString("%s%s", elem, c);
        }
        if (i > 0)
            key_name = WvString("%s/%s", key_name, elem);
        else
            key_name = elem;
    }
    key_name = WvString("%s/%s", key, key_name);
    
    last_change = WvString("Added the key %s with the new value", key_name);
    // generate a value
    int num = (int)((double)rand() / (double)RAND_MAX);
    Victim *victim;
    if (num)
    // generate number
    {
        int new_value = rand();
        cfg[key_name].setint(new_value);
        victim = new Victim(num_victims, TYPE_INT, key_name);
        last_change = WvString("%s %s\n", last_change, new_value);
    }
    else
    // generate string
    {
        int type = (int)(((double)rand() / (double)RAND_MAX) * 5 
                   + TYPE_STRING);
        WvString new_value = rand_str(type);
        cfg[key_name].set(new_value);
        victim = new Victim(num_victims, TYPE_STRING, key_name);
        last_change = WvString("%s %s\n", last_change, new_value);
    }
    victims.add(victim, true);
    num_victims ++;
    cfg[key_name].commit();
}

/* start_trouble(int curr_runlevel)
 * This is where it makes the calls for 1000 actions on the specified runlevel
 * and calls the appropriate method.
 */
void UniConfGremlin::start_trouble(int curr_runlevel)
{
    cout << UniConfGremlin::curr_runlevel() << "\n";
    int r = curr_runlevel;
    for (int i = 0; i < 1000; i ++)
    {
        if (curr_runlevel == 5)
            r = (int)(((double)rand() / (double)RAND_MAX) * 4) + 1;
        
        if (r == 1)
            change_value(true);
        else if (r == 2)
            change_value(false);
        else if (r == 3)
            add_value();
        else if (r >= 4)
        // randomly delete value, but always remember as a victim for fun
        {
            int r = (int)(((double)rand() / (double)RAND_MAX) * (num_victims));
            cfg[victims[r]->name].remove();
            cfg[victims[r]->name].commit();
            last_change = WvString("Deleted the key %s\n", victims[r]->name);
        }
    }
}

/* rand_str(int type)
 * Returns a random string of the specified format(by type)
 * - not great random numbers, had to increase the range to 1 above the max 
 *  range I wanted, just since when casting back to int it chops off the 
 *  fractional part, and there is very low probability of getting 
 *  rand() = RAND_MAX but it will happen from time to time.
 */
WvString UniConfGremlin::rand_str(int type)
{
    if (type == TYPE_STRING || type == TYPE_HOSTNAME)
    {
        int a = (int)(((double)rand() / (double)RAND_MAX) * 200);
        WvString result = "";
        for (int i = 0; i < a; i++)
        {
            int b = (int)((((double)rand() / (double)RAND_MAX) * 26) + 97); 
            char c[2];
            c[0] = (char)b;
            c[1] = '\0';
            result = WvString("%s%s", result, c);
        }
        return WvString("%s", result);
    }  
    else if (type == TYPE_BOOL)
    {
        const char *strs[] = {
            "true", "yes", "on", "enabled", "1",
            "false", "no", "off", "disabled", "0"
        };
        int a = (int)(((double)rand() / (double)RAND_MAX) * 10);
        return WvString("%s", strs[a]);
    }
    else if (type == TYPE_IP || type == TYPE_IP_NETWORK)
    {
        int a = (int)(((double)rand() / (double)RAND_MAX) * 256), 
            b = (int)(((double)rand() / (double)RAND_MAX) * 256), 
            c = (int)(((double)rand() / (double)RAND_MAX) * 256),
            d = (int)(((double)rand() / (double)RAND_MAX) * 256);
        if (type == TYPE_IP_NETWORK)
        {
            int e = (int)(((double)rand() / (double)RAND_MAX) * 33);
            return WvString("%s.%s.%s.%s/%s", a, b, c, d, e);
        }
        else
            return WvString("%s.%s.%s.%s", a, b, c, d);
    }
    else if (type == TYPE_INT)
        return rand();
    else if (type == TYPE_FLOAT)
        return rand();
    else 
        return "";
}

/* type_name(int type)
 * Returns a string representation of the type passed to it.
 */
WvString UniConfGremlin::type_name(int type)
{
    if (type == TYPE_STRING)
        return "String";
    else if (type == TYPE_IP)
        return "IP";
    else if (type == TYPE_IP_NETWORK)
        return "IP Network";
    else if (type == TYPE_HOSTNAME)
        return "Hostname";
    else if (type == TYPE_BOOL)
        return "Boolean";
    else if (type == TYPE_INT)
        return "Integer";
    else if (type == TYPE_FLOAT)
        return "Float";
    else 
        return "Unknown";
}

/* curr_runlevel()
 * Returns a string describing the current runlevel.
 */
WvString UniConfGremlin::curr_runlevel()
{
    if (runlevel == 1)
        return "1 - Change keys with valid data.";
    else if (runlevel == 2)
        return "2 - Change keys with invalid data.";
    else if (runlevel == 3)
        return "3 - Add new keys.";
    else if (runlevel == 4)
        return "4 - Remove keys.";
    else if (runlevel == 5)
        return "5 - Add/Remove keys, change keys with valid/invalid data.";
    else
        return WvString("Apparently I'm on runlevel %s", runlevel);
}

/* status()
 * Displays the most recent runlevel and action.
 */
WvString UniConfGremlin::status()
{
    return WvString("Last Runlevel was : %s\nLast Action was : %s", 
            curr_runlevel(), last_change);
}

/* test()
 * Code can be put here to test and tweak the functionality of the gremlin
 */
void UniConfGremlin::test()
{
    /*
    cout << "Testing Random Generators\n";
    cout << "STRING " << UniConfGremlin::rand_str(TYPE_STRING) << "\n";
    cout << "IP " << UniConfGremlin::rand_str(TYPE_IP) << "\n";
    cout << "IP_NETWORK " << UniConfGremlin::rand_str(TYPE_IP_NETWORK) 
         << "\n";
    cout << "HOSTNAME " << UniConfGremlin::rand_str(TYPE_HOSTNAME) << "\n";
    cout << "BOOL " << UniConfGremlin::rand_str(TYPE_BOOL) << "\n";   
    cout << "INT " << UniConfGremlin::rand_str(TYPE_INT) << "\n";
    cout << "FLOAT " << UniConfGremlin::rand_str(TYPE_FLOAT) << "\n";
    */
/*
    cout << "Testing adding things to the UniConf\n"; 
    int count = 0;
    for (int i = 0; i < 5; i ++)
    {
        cout << "Adding 5 random strings of type " << type_name(i) << "\n";
        for (int j = 0; j < 5; j ++)
        {
            WvString rand = rand_str(i);
            cout << "Adding " << rand << "\n";
            cfg[count].set(rand);
            cfg[count].commit();
            count ++;
        }
    }
    cout << "Adding 5 random ints\n";
    for (int i = 5; i < 10; i ++)
    {
        int rand = rand_str(TYPE_INT);
        cout << "Adding " << rand << "\n";
        cfg[count].setint(rand);
        cfg[count].commit();
        count ++;
    }
  */  
    cout << "Testing Find Victims\n";
    find_victims(key);
    for (int i = 0; i < num_victims; i ++)
    {
       cout << victims[i]->name << ":";
       cout << type_name(victims[i]->type) << ":";
       cout << cfg[victims[i]->name].get() << "\n";
    }
    
    cout << "Testing Start Trouble\n";
    start_trouble(1);
}

/* main()
 * Simply used to test the gremlin
 */
int main()
{
    UniConfGremlin g("ini:/tmp/test.ini", "", 5);
    cout << "Gremlin created\n";
    g.start();
    cout << g.status();
    return 0;
}
