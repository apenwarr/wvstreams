#include <wvstream.h>
#include <wvistreamlist.h>

class Silly : public WvStream
{
public:
    static int counter;
    Silly()
        {
            alarm(0);
            uses_continue_select = true;
	    //personal_stack_size = 16384;
            WvIStreamList::globallist.append(this, true);
        }
    virtual void execute()
        {
            fprintf(stderr, "#%d\t going into continue select\n", counter++);
            continue_select(-1);
        }
};

int Silly::counter = 1;

int main()
{
    for (int i = 0; i < 20; i++)
        new Silly();

    while (1)
         WvIStreamList::globallist.select(-1);
}
