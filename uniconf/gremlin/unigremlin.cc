#include <ctime>
#include <stdio.h>
#include <regex.h>
#include <ctype.h>
#include "unigremlin.h"


UniConfGremlin::UniConfGremlin(WvString moniker, const UniConfKey _key)
    : root(moniker), victims(500), num_victims(0)
{
    cfg = UniConf(root[_key]);
    for (int i = 0; i < NUM_TYPES; i++)
        harvested[i] = new HarvestedDict(500);
}

UniConfGremlin::~UniConfGremlin()
{
    for (int i = 0; i < NUM_TYPES; i++)
        delete harvested[i];
}

/* start(unsigned int seed)
 * Starts the gremlin out using the specified seed to seed the random numbers,
 * or if no seed is specified, uses the current time to seed them.
 */
void UniConfGremlin::start(unsigned int seed, int _flags, int _ratio_create,
        int _ratio_valid, int _ratio_correct)
{
    ratio_create = _ratio_create;
    ratio_valid = _ratio_valid;
    ratio_correct = _ratio_correct;

    //harvest out bit flags
    if (_flags & Create)
    {
        modify_valid = _flags & Valid;
        modify_invalid = _flags & Invalid;
    }
    add_values = _flags & Create;
    delete_values = _flags & Delete;
    
    if (seed == 0)
        seed = time(0);
    
    srand(seed);
    printf("Using seed %i\n", seed); //perhaps save this to a file?
//    printf("Finding Victims\n");
    find_victims();
//    printf("Starting Trouble\n");
    start_trouble(1000);
}

/* find_victims(UniConfKey _key)
 * Iterates through each element in the tree and adds a new victim to the list
 * victims with the element's key and a guess at what type of data should be
 * stored there.
 */
void UniConfGremlin::find_victims()
{
    UniConf::RecursiveIter i(cfg);
    Victim *victim;
    for (i.rewind(); i.next(); )
    {   
        if (i().getme() != "")
        {
            victim = new Victim(num_victims++, inspect(i().getme()), 
                    i().fullkey(cfg.fullkey()));
            victims.add(victim, true);
        }
    }
}

/* inspect(WvString element)
 * Inspects the string element in an attempt to guess what type of data should
 * be stored there.  Returns an integer representing what type the gremlin
 * thinks it is, and stores the value as an example of that type.
 */
int UniConfGremlin::inspect(WvString element)
{
    char *elem = element.edit();
    Harvested * harvest;
    // rough expression for an ip network
    char *ipNetExp = "[0-9].*\\.[0-9].*\\.[0-9].*\\.[0-9].*";
    if (is_bool(elem))
    {
        harvest = new Harvested(num_harvested[TYPE_BOOL]++, element);
        harvested[TYPE_BOOL]->add(harvest, true);
        return TYPE_BOOL;
    }
    
    if (isdigit(elem[0]))
    {
        regex_t preg1;
        regmatch_t pmatch1[1];
        regcomp(&preg1, ipNetExp, REG_EXTENDED | REG_NOSUB);
        
        if (!regexec(&preg1, elem, 1, pmatch1, 0))
        {
            if (elem[element.len() - 2] == '/' || 
                elem[element.len() - 3] == '/')
            {
                harvest = new Harvested(num_harvested[TYPE_IP_NETWORK]++, element);
                harvested[TYPE_IP_NETWORK]->add(harvest, true);
                return TYPE_IP_NETWORK;
            }
            else
            {
                harvest = new Harvested(num_harvested[TYPE_IP]++, element);
                harvested[TYPE_IP]->add(harvest, true);
                return TYPE_IP;
            }
        }
    }
    // now we just brute force the rest of the string
    for (size_t i = 1; i < element.len(); i++)
    {
        if (!isdigit(elem[i]))
        // either string or hostvalue, return string for now
        {
            harvest = new Harvested(num_harvested[TYPE_STRING]++, element);
            harvested[TYPE_STRING]->add(harvest, true);
            return TYPE_STRING;
        }
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
    if (num_victims > 0)
    {
        int r = num_victims + 1;
        while (r >= num_victims)
        {
            r = randInt(num_victims);
        }
        if (victims[r]->value == "")
            printf("Fuck, why are we changing the empty value?\n");
        last_change = WvString("Changed %s to be the value", victims[r]->value);
        WvString new_value;
        if (use_valid_data)
        {
            if (victims[r]->type == TYPE_INT)
            {
                int new_value = rand();
                cfg.xsetint(victims[r]->value, new_value);
                last_change = WvString("%s %s\n", last_change, new_value);
            }
            else 
            {
                WvString new_value = rand_str(victims[r]->type);
                cfg.xset(victims[r]->value, new_value);
                last_change = WvString("%s %s\n", last_change, new_value);
            }
        }
        else
        {
            int s = randInt(4);
            if (s)
            {
                WvString new_value = rand_str(victims[r]->type);
                int t = randInt(NUM_TYPES);
                cfg.xset(victims[r]->value, rand_str(t));
                last_change = WvString("%s %s\n", last_change, new_value);
            }
            else
            {
                int new_value = rand();
                cfg.xsetint(victims[r]->value, new_value);
                last_change = WvString("%s %s\n", last_change, new_value);
            }
        }
        cfg[victims[r]->value].commit();
    }
}

/* add_value()
 * Adds a random value into the UniConf also randomly selecting the type and 
 * contents.
 */
void UniConfGremlin::add_value()
{
    int depth = randInt(MAX_DEPTH);
    WvString key_value = "", elem;
    
    // generate a key
    for (int i = 0; i < depth || !key_value; i++)
    {
        elem = "";
        int length = randInt(8);
        for (int j = 0; j < length; j++)
        {
            // generate a valid written character (lower case right now)
            int num = randInt(122, 97);
            char c[2];
            c[0] = (char)num;
            c[1] = '\0';
            elem = WvString("%s%s", elem, c);
        }
        if (i > 0)
            key_value = WvString("%s/%s", key_value, elem);
    }
    key_value = WvString("/%s", key_value);
    last_change = WvString("Added the key %s with the new value", key_value);

    
    // generate a value
    int num = randInt(1);
    Victim *victim;
    if (num)
    // generate number
    {
        int new_value = rand();
        cfg.xsetint(key_value, new_value);
        victim = new Victim(num_victims, TYPE_INT, key_value);
        last_change = WvString("%s %s\n", last_change, new_value);
    }
    else
    // generate string
    {
        int type = randInt(5 + TYPE_STRING, TYPE_STRING);
        WvString new_value = rand_str(type);
        cfg.xset(key_value, new_value);
        victim = new Victim(num_victims, TYPE_STRING, key_value);
        last_change = WvString("%s %s\n", last_change, new_value);
    }
    victims.add(victim, true);
    num_victims ++;
    cfg.commit();
}

/* start_trouble()
 * Performs howmany actions, range of action controlled by initial bitflags.
 */
void UniConfGremlin::start_trouble(int howmany)
{
    int r = randInt(5);
    for (int i = 0; i < howmany; i ++)
    {
        r = randInt(5);
        
        if (r < 1)
            spin_commit(500);
        else if (r == 1 && modify_valid)
            change_value(true);
        else if (r == 2 && modify_invalid)
            change_value(false);
        else if (r == 3 && add_values)
            add_value();
        else if (r >= 4  && num_victims > 0 && delete_values)
        // randomly delete value, but always remember as a victim for fun
        {
            r = randInt(num_victims);
            cfg[victims[r]->value].remove();
            cfg[victims[r]->value].commit();
            last_change = WvString("Deleted the key %s\n", victims[r]->value);
        }
        //printf("%s", status().cstr());
    }
}

/* spin_commit(n)
 * commits for n iterations, to see what effect it has
 */
void UniConfGremlin::spin_commit(int n)
{
    for (int i = 0; i < n; i++)
    {
        cfg.commit();
    }
}

/* rand_str(int type)
 * Returns a random string of the specified format(by type)
 * - not great random numbers since there's a very low probability of
 *   getting rand() = RAND_MAX but it will happen from time to time.  
 *   had to increase the range to 1 above the max range I wanted, 
 *   just since when casting back to int it chops off the fractional part
 */
WvString UniConfGremlin::rand_str(int type)
{
    if (type == TYPE_STRING || type == TYPE_HOSTNAME)
    {
        int a = randInt(50);
        WvString result = "";
        for (int i = 0; i < a; i++)
        {
            int b = randInt(124, 97); 
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
        int a = randInt(10);
        return WvString("%s", strs[a]);
    }
    else if (type == TYPE_IP || type == TYPE_IP_NETWORK)
    {
        int a = randInt(256),
            b = randInt(256), 
            c = randInt(256),
            d = randInt(256);
        if (type == TYPE_IP_NETWORK)
        {
            int e = randInt(33);
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

/* type_value(int type)
 * Returns a string representation of the type passed to it.
 */
WvString UniConfGremlin::type_value(int type)
{
    if (type == TYPE_STRING)
        return "String";
    else if (type == TYPE_IP)
        return "IP";
    else if (type == TYPE_IP_NETWORK)
        return "IP Network";
    else if (type == TYPE_HOSTNAME)
        return "Hostvalue";
    else if (type == TYPE_BOOL)
        return "Boolean";
    else if (type == TYPE_INT)
        return "Integer";
    else if (type == TYPE_FLOAT)
        return "Float";
    else 
        return "Unknown";
}

/* randInt(int max, int min)
 * generate a random integer between max and min
 */
int UniConfGremlin::randInt(int max, int min = 0)
{
    return (int)(((double)rand() / (double)RAND_MAX) * (max - min)) + min;
}

/* curr_runlevel()
 * Returns a string describing the current runlevel.
 */
/*WvString UniConfGremlin::curr_runlevel()
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
}*/

/* status()
 * Displays the most recent action.
 */
WvString UniConfGremlin::status()
{
    return WvString("Last Action was : %s", last_change);
}

/* test()
 * Code can be put here to test and tweak the functionality of the gremlin
 */
void UniConfGremlin::test()
{
    printf("Testing Random Generators\n");
    printf("STRING %s\n", UniConfGremlin::rand_str(TYPE_STRING).cstr());
    printf("IP %s\n", UniConfGremlin::rand_str(TYPE_IP).cstr());
    printf("IP_NETWORK %s\n", UniConfGremlin::rand_str(TYPE_IP_NETWORK).cstr());
    printf("HOSTNAME %s\n", UniConfGremlin::rand_str(TYPE_HOSTNAME).cstr());
    printf("BOOL %s\n", UniConfGremlin::rand_str(TYPE_BOOL).cstr());   
    printf("INT %s\n", UniConfGremlin::rand_str(TYPE_INT).cstr());
    printf("FLOAT %s\n", UniConfGremlin::rand_str(TYPE_FLOAT).cstr());
    printf("Testing adding things to the UniConf\n"); 
    int count = 0;
    for (int i = 0; i < 5; i ++)
    {
        printf("Adding 5 random strings of type %s\n", type_value(i).cstr());
        for (int j = 0; j < 5; j ++)
        {
            WvString rand = rand_str(i);
            printf("Adding %s\n", rand.cstr());
            cfg.xset(count, rand);
            cfg.commit();
            count ++;
        }
    }
    printf("Adding 5 random ints\n");
    for (int i = 5; i < 10; i ++)
    {
        int randint = rand();
        printf("Adding %i\n", randint);
        cfg.xsetint(count, randint);
        cfg.commit();
        count ++;
    }
    printf("Testing Find Victims\n");
    find_victims();
    for (int i = 0; i < num_victims; i ++)
    {
       printf("%s:", victims[i]->value.cstr());
       printf("%s:", type_value(victims[i]->type).cstr());
       printf("%s\n", cfg[victims[i]->value].getme().cstr());
    }
    
    printf("Testing Start Trouble\n");
    start_trouble(1000);
}
