/*
 * A minimal program the uses functors.
 * Useful for examining the generated assembly language code.
 */
#include "wvfunctor.h"
 
struct Test
{
    static void fvoid0()
    {
    }
    void mfvoid0()
    {
    }
};
 
 
void ftest()
{
    WvFunctor<void> fvoid0(& Test::fvoid0);
    fvoid0();
}


void mftest()
{
    Test test;
    WvFunctor<void, Test*> mfvoid0(& Test::mfvoid0);
    mfvoid0(& test);
}


void bmftest()
{
    Test test;
    WvFunctor<void> bmfvoid0(& Test::mfvoid0, & test);
    bmfvoid0();
}


int main()
{
    ftest();
    mftest();
    bmftest();
    return 0;
}

