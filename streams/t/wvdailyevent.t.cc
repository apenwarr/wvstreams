
#include <time.h>

#include "wvtest.h"
#include "wvdailyevent.h"

// Wait a number of seconds
void wait(int num_seconds)
{
    time_t start_time = time(NULL);
    time_t end_time = start_time + num_seconds;

    while(end_time > time(NULL)) 
    {
        // wait!
    }
}

bool within_range(int numbertocheck, int numberneeded, int range) 
{
    if (numbertocheck - numberneeded < range / 2 || numberneeded - numbertocheck < range / 2)
        return true;
    else
        return false;
}

const int NUM_MINUTES_IN_DAY = 1440;
    
WVTEST_MAIN("WvDailyEvent-waits-for-one-time-period Test")
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour, NUM_MINUTES_IN_DAY);
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 20); // every 3 seconds

    int seconds_passed = 0;
    while(!devent.select(0))
    {
        if (now != time(NULL))
            seconds_passed += time(NULL) - now;
        now = time(NULL);
    }
    WVPASS(within_range(seconds_passed, 3, 2));
}

WVTEST_MAIN("configure()-causes-wait-for-one-time-period test")
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour, NUM_MINUTES_IN_DAY);
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 20); // every 3 seconds
    
    wait(1);
    
    printf("\nReconfiguring granularity to once every 5 seconds");
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 12);

    int seconds_passed = 0;
    while(!devent.select(0))
    {
        if (now != time(NULL))
            seconds_passed += time(NULL) - now;
        now = time(NULL);
    }
    WVPASS(within_range(seconds_passed, 6, 2));
}


WVTEST_MAIN("Ridiculous values (num_per_day) test") 
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour + 24, NUM_MINUTES_IN_DAY * 10000);
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 4933);
    
    int seconds_passed = 0;
    while(!devent.select(0))
    {
        if (now != time(NULL))
            seconds_passed += time(NULL) - now;
        now = time(NULL);
    }
    WVPASS(within_range(seconds_passed, 1, 2));
} 
