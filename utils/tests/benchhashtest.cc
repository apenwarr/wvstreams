#include <map>
#include <stdio.h>
#include <string>
#include <tr1/unordered_map>
#include "wvhashtable.h"
#include "wvscatterhash.h"
#include "wvtimeutils.h"

using std::make_pair;
using std::map;
using std::string;
using std::tr1::unordered_map;


#define NUMITEMS   1000000
#define NUMLOOKUPS  100000

class Timer
{
public:
    Timer(const string &_name):
	starttime(wvtime()),
	name(_name)
    {
    }
    ~Timer()
    {
	WvTime stoptime(wvtime());

	fprintf(stderr, "timer %s: %li ms\n", name.c_str(),
		msecdiff(stoptime, starttime));
    }

private:
    const WvTime starttime;
    const string name;
};


template<class Map>
static void do_bench_wvhash()
{
    Timer timer1("overall");
    Map mymap(NUMLOOKUPS);
    {
	Timer timer2("without destructor");

	{
	    Timer timer3("initialization");
	    for (int i = 0; i < NUMITEMS; ++i)
		mymap.set(rand(), rand());
	}

	{
	    Timer timer4("iterate");
	    size_t count = 0;
	    typename Map::Iter it(mymap);

	    for (it.rewind(); it.next(); )
	    {
		++(it.ptr()->data);
		++count;
	    }

	    fprintf(stderr, "count = %i\n", count);
	}

	{
	    Timer timer5("random lookups");
	    size_t found = 0;
	    size_t sum = 0;

	    for (int i = 0; i < NUMLOOKUPS; ++i)
	    {
		WvMapPair<int, int> *it = mymap.find_pair(rand());
		if (it)
		{
		    sum += it->data;
		    ++found;
		}
	    }

	    fprintf(stderr, "found = %i, sum = %i\n", found, sum);
	}

	{
	    Timer timer6("zap/clear");

	    mymap.zap();
	}
    }
}


template<class Map>
static void do_bench_std()
{
    Timer timer1("overall");
    Map mymap;
    {
	Timer timer2("without destructor");

	{
	    Timer timer3("initialization");
	    for (int i = 0; i < NUMITEMS; ++i)
		mymap.insert(make_pair(rand(), rand()));
	}

	{
	    Timer timer4("iterate");
	    size_t count = 0;
	    typename Map::iterator it;

	    for (it = mymap.begin(); it != mymap.end(); ++it)
	    {
		++it->second;
		++count;
	    }

	    fprintf(stderr, "count = %i\n", count);
	}

	{
	    Timer timer5("random lookups");
	    size_t found = 0;
	    size_t sum = 0;

	    for (int i = 0; i < NUMLOOKUPS; ++i)
	    {
		typename Map::iterator it = mymap.find(rand());
		
		if (it != mymap.end())
		{
		    sum += it->second;
		    ++found;
		}
	    }

	    fprintf(stderr, "found = %i, sum = %i\n", found, sum);
	}

	{
	    Timer timer6("zap/clear");

	    mymap.clear();
	}
    }
}


int main()
{
    fprintf(stderr, "--- WvHashTable\n");
    srand(42);
    do_bench_wvhash<WvMap<int, int, OpEqComp, WvHashTable> >();
    fprintf(stderr, "\n");

    fprintf(stderr, "--- WvScatterHash\n");
    srand(42);
    do_bench_wvhash<WvMap<int, int, OpEqComp, WvScatterHash> >();
    fprintf(stderr, "\n");

    fprintf(stderr, "--- map\n");
    srand(42);
    do_bench_std<map<int, int> >();
    fprintf(stderr, "\n");

    fprintf(stderr, "--- unordered_map\n");
    srand(42);
    do_bench_std<unordered_map<int, int> >();
    fprintf(stderr, "\n");
}

