#if 0
#include "wvtest.h"
#include "wvdailyevent.h"

#include <time.h>
#include <iostream.h>

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

const int NUM_MINUTES_IN_DAY = 1440;
    
WVTEST_MAIN("WvDailyEvent-waits-for-one-time-period Test")
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour, NUM_MINUTES_IN_DAY);
    
    printf("immediately after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));

    wait(30);
    
    printf("\n30 seconds after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));
    
    wait(30);
    
    printf("\n1 minute after object is created\n");
    WVPASS(devent.isok());
    WVPASS(devent.select(1));
    devent.runonce(1);
    WVFAIL(devent.select(1));
}

WVTEST_MAIN("configure()-causes-wait-for-one-time-period test")
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour, NUM_MINUTES_IN_DAY);
    
    printf("immediately after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));

    wait(30);
    
    printf("\n30 seconds after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));
    
    printf("\nReconfiguring granularity to once every 2 mins\n");
    devent.configure(tnow->tm_hour, NUM_MINUTES_IN_DAY/2);
    
    wait(30);

    printf("\n30 seconds after reconfiguration\n");
    WVFAIL(devent.select(1));

    wait(60);

    printf("\n90 seconds after reconfiguration\n");
    WVFAIL(devent.select(1));

    wait(30);
    
    printf("\n120 seconds after reconfiguration\n");
    WVPASS(devent.select(1));
    devent.runonce(1);
    WVFAIL(devent.select(1));
}

WVTEST_MAIN("Ridiculous values (num_per_day) test") 
{
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour + 24, NUM_MINUTES_IN_DAY + 10000);
    
    printf("immediately after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));

    wait(30);
    
    printf("\n30 seconds after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));
    
    wait(30);
    
    printf("\n1 minute after object is created\n");
    WVPASS(devent.isok());
    devent.runonce(1);
    WVFAIL(devent.select(1));
}

WVTEST_MAIN("Ridiculous values (first_hour) test") 
{
    printf("At the moment, this test will take one day so it has been commented out\n");

/*
    // Get localtime
    time_t now;
    struct tm *tnow;

    now = time(NULL);
    tnow = localtime(&now);
    
    WvDailyEvent devent(tnow->tm_hour + 24, 0);
    
    printf("immediately after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));

    wait(30);
    
    printf("\n30 seconds after object is created\n");
    WVPASS(devent.isok());
    WVFAIL(devent.select(1));
    
    wait(30);
    
    printf("\n1 minute after object is created\n");
    WVPASS(devent.isok());
*/
}

#endif
