#include <map>
#include <assert.h>
#include "wvstring.h"

using std::map;


int main ()
{
    map<WvString, WvString> mymap;
    fprintf(stderr, "added foo = bar\n");
    mymap["foo"] = "bar";
    fprintf(stderr, "looked up foo = %s\n", mymap.find("foo")->second.cstr());
    mymap.erase("foo");
    assert(mymap.find("foo") == mymap.end());

    mymap["meaw"] = "death";
    mymap["dog"] = "cow";
    mymap["star"] = "trek";
    mymap["star"] = "office";

    // Iterator test
    map<WvString, WvString>::iterator it;

    for (it = mymap.begin(); it != mymap.end(); ++it)
        fprintf(stderr, "Iter test: %s = %s\n",
		it->first.cstr(), it->second.cstr());
}


