
#include "wvcallback.h"
#include <stdio.h>

//Declare a new type of WvCallback called WvMath
//This WvCallbak can point to functions that take 2 input parameters, both of type 
//integer, and returns an integer value.
DeclareWvCallback(2, int, WvMath, int, int);

int addition(int a, int b)
{
    return a+b;
}

    
int main()
{
    WvMath callback(NULL); //Declare a WvCallback of type WvMath
    //callback = wvcallback(WvMath, *this, Math::addition);
    callback = addition; // Now callback becomes a function pointer to the addition function

    int answer = callback(5, 6); //Bind input parameter values to callback, same 
    //way as we bind values to the addition function.

    printf("answer = %d\n", answer);
    
    
}

