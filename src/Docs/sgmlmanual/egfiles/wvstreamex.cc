#include <wvstream.h>

int main()
{
    WvStream wvstdin(0), wvstdout(1);
    
    while (wvstdin.isok())
	wvstdout.print("You said: %s\n", wvstdin.getline(-1));
}
