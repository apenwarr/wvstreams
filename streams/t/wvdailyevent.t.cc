
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

class WvDailyEventTest : public WvDailyEvent
{
public:
    bool ran;

    WvDailyEventTest(int first_hour, int num_per_day = 0,
		     bool skip_first = false)
	: WvDailyEvent(first_hour, num_per_day, skip_first), ran(false)
    {
    }

    virtual void execute()
    {
	ran = true;
    }
};

const int NUM_MINUTES_IN_DAY = 1440;
    
WVTEST_MAIN("WvDailyEvent-waits-for-one-time-period Test")
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEventTest devent(tnow->tm_hour, NUM_MINUTES_IN_DAY);
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 12); // every 5 seconds

    int seconds_passed = 0;
    while (!devent.ran)
	devent.runonce();
    {
        if (now != time(NULL))
            seconds_passed += time(NULL) - now;
        now = time(NULL);
    }
    WVPASS(within_range(seconds_passed, 5, 2));
}

WVTEST_MAIN("configure()-causes-wait-for-one-time-period test")
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEventTest devent(tnow->tm_hour, NUM_MINUTES_IN_DAY);
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 12); // every 5 seconds
    
    wait(1);
    
    printf("Reconfiguring granularity to once every 10 seconds\n");
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 6);

    int seconds_passed = 0;
    while (!devent.ran)
	devent.runonce();
    {
        if (now != time(NULL))
            seconds_passed += time(NULL) - now;
        now = time(NULL);
    }
    WVPASS(within_range(seconds_passed, 10, 2));
}


WVTEST_MAIN("Ridiculous values (num_per_day) test") 
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEventTest devent(tnow->tm_hour + 24, NUM_MINUTES_IN_DAY * 10000);
    devent.set_num_per_day(NUM_MINUTES_IN_DAY * 4933);
    
    int seconds_passed = 0;
    while (!devent.ran)
	devent.runonce();
    {
        if (now != time(NULL))
            seconds_passed += time(NULL) - now;
        now = time(NULL);
    }
    WVPASS(within_range(seconds_passed, 1, 2));
} 
