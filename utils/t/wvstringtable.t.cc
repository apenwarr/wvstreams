#include "wvtest.h"
#include "wvstringtable.h"

// BEGIN stringtabletest.cc definition

WVTEST_MAIN("stringtabletest.cc")
{   WvString parm1("Luke\tis one\t\tsexy\tman.");
    
    {
	// Basic test
	WvStringTable testTable;

	WVPASS(testTable["Luke"] == NULL);
	WVPASS(testTable.join(" ") == "");
	testTable.split(WvString("\t"));
	WVPASS(testTable.count() == 2);
	WVPASS(testTable.join("\r\n") == "\r\n");
	testTable.split(parm1);
	WVPASS(testTable.count() == 7);
	WVFAIL(testTable.isempty());
	WVPASS(testTable["sexy"] != NULL);
	testTable.zap();
	testTable.split("", "\n ");
	testTable.splitstrict("");
	testTable.splitstrict(WvString(), "\n\r ");
	testTable.split(WvString());
	WVPASS(testTable.count() == 2);
    }

    {
	// More complicated test
	WvStringTable testTable2;
	WVPASS(testTable2.isempty());
	testTable2.split(WvString("\t\tTesting"), "\t");
	WVFAIL(testTable2.join("\n") == "Testing");
	WVPASS(testTable2.count() == 2);
	testTable2.zap();
	WVPASS(testTable2.isempty());
	testTable2.split(WvString("Testing\t\n"), "\n\t");
	WVFAIL(testTable2.join("\r\n") == "Testing");
	WVPASS(testTable2.count() == 2);
	testTable2.splitstrict("\r\n\t ");
	WVPASS(testTable2.count() == 7);
	testTable2.zap();

	testTable2.splitstrict("\t\rTesting2");
	WVPASS(testTable2.count() == 3);
	testTable2.splitstrict("Testing2\t\r\n", " \t\r\n");
	testTable2.split("\n", "\r");
	WVPASS(testTable2.count() == 8);

	testTable2.splitstrict(parm1, " \t");
	testTable2.split(parm1, " \t\r\n", 9);
	WVPASS(testTable2.count() == 19);
	testTable2.split(parm1, "\t", 2);
	WVPASS(testTable2.count() == 21);
	WVFAIL(testTable2.join() == "");
    }

    {
	// Different parameters on invoking
	WvStringTable testTable3(19);
	WVPASS(testTable3.isempty());
	testTable3.split(parm1, " \t", 11);
	WVPASS(testTable3.count() == 5);
	testTable3.splitstrict(parm1, "\t");
	WVPASS(testTable3.count() == 10);
	testTable3.splitstrict(WvString(), "\r\n", 3);
	testTable3.split(WvString(), "\t", 11);
	WVPASS(testTable3.count() == 10);
	for (int i = 0; i < 256; ++i)
	    testTable3.split(parm1, " \t\r\n");
	WVPASS(testTable3.join(" ").len() == 5676);
	WVPASS(testTable3.count() == 1290);
	testTable3.split("abdagag \tgaag", "\n");
	testTable3.split("\tagagagaga\t", " \t\r\n", 2);
	WVPASS(testTable3.count() == 1293);
	testTable3.zap();
    }

    {
	// Heap behaviour.
	WvString *heapString = new WvString("This is\t\talmost\ndone!\t");
	WvStringTable *testTable4 = new WvStringTable(2101);
	testTable4->split(*heapString);
	WVPASS(testTable4->count() == 5);
	testTable4->splitstrict(*heapString);
	WVPASS(testTable4->count() == 11);
	/*testTable4->zap();
	testTable4->splitstrict(WvString("I rock hard"), " ", 2);
	WVFAIL((*testTable4)["rock hard"] != NULL);*/

	delete testTable4;
	WVPASS(!strcmp(heapString->cstr(), "This is\t\talmost\ndone!\t"));
	delete heapString;
    }
}

// END stringtabletest.cc definition
