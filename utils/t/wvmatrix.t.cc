#include "wvtest.h"
#include "wvmatrix.h"

// BEGIN first old-style definition
// test that creation matches intended data
bool matrix_matches(WvMatrix &in, int data[])
{
    int k = 0;
    bool matches = true;
    for (int i = 0; i < in.m; i++)
    {
        for (int j = 0; j < in.n; j++)
        {
            if (!WVPASS(in(i, j) == data[k]))
            {
                printf("   because [%d] != [%d]\n", in(i, j), data[k]);
                matches = false;
            }
            k++;
        }
    }
    return matches;
}
// returns true iff in1 + in2 equals out
// in1, in2 and out *must* have appropriately matching dimensions
bool matrix_sum(WvMatrix &in1, WvMatrix &in2, WvMatrix &out)
{
    bool matches = true;
    for (int i = 0; i < in1.m; i++)
    {
	for (int j = 0; j < in1.n; j++)
        {
            if (!WVPASS(out(i, j) == in1(i, j) + in2(i, j)))
            {
                printf("   because [%d] != [%d]\n", out(i, j), 
                        in1(i, j) + in2(i, j));
                matches = false;
            }
        }
    }
    return matches;
}
// returns true iff in1 * in2 equals out
// in1, in2 and out *must* have appropriately matching dimensions
bool matrix_product(WvMatrix &in1, WvMatrix &in2, WvMatrix &out)
{
    int result[in1.m * in2.n], c, n = 0;
    bool matches = true;
    // calculate our own results
    for (int i = 0; i < in1.m; i++)
    {
        for (int j = 0; j < in2.n; j++)
        {
            c = 0;
            for (int k = 0; k < in1.n; k++)
                c += in1(i, k) * in2(k, j);
            result[n] = c;
            n++;
        }
    }

    n = 0;
    for (int i = 0; i < out.m; i++)
     {
         for (int j = 0; j < out.n; j++)
         {
             if (!WVPASS(out(i, j) == result[n]))
             {
                 printf("   because [%d] != [%d]\n", out(i, j), result[n]);
                 matches = false;
             }
             n++;
         }
     }
    return matches;
}
// END first old-style definition
WVTEST_MAIN("old-style") 
{
    // test sum
    {
        int dataa[] = {2, 4, -6, 7, 1, 3, 2, 1, -4, 3, -5, 5};
        int datab[] = {0, 1, 6, -2, 2, 3, 4, 3, -2, 1, 4, 4};
        WvMatrix a(3, 4, dataa);
        matrix_matches(a, dataa);
        WvMatrix b(3, 4, datab);
        matrix_matches(b, datab);

        WvMatrix c = a + b;
    
        matrix_sum(a, b, c);
    }

    // test product
    {
        int datad[] = {2, 0, -3, 4, 1, 5};
        int datae[] = {7, -1, 4, 7, 2, 5, 0, -4, -3, 1, 2, 3};
        WvMatrix d(2, 3, datad);
        matrix_matches(d, datad);
        WvMatrix e(3, 4, datae);
        matrix_matches(e, datae);

        WvMatrix f = d * e;

        matrix_product(d, e, f);
    }
}
