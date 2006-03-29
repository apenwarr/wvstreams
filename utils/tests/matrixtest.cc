#include "wvmatrix.h"
#include <stdio.h>

void print_matrix(WvMatrix &mx)
{
    for (int i = 0; i < mx.m; i++)
    {
	for (int j = 0; j < mx.n; j++)
	    printf("%3d ", mx(i, j));
	printf("\n");
    }
}


int main()
{
    int dataa[] = {2, 4, -6, 7, 1, 3, 2, 1, -4, 3, -5, 5};
    int datab[] = {0, 1, 6, -2, 2, 3, 4, 3, -2, 1, 4, 4};
    WvMatrix a(3, 4, dataa);
    printf("WvMatrix a:\n");
    print_matrix(a);
    WvMatrix b(3, 4, datab);
    printf("WvMatrix b:\n");
    print_matrix(b);

    WvMatrix c = a + b;
    
    printf("WvMatrix a + b = \n");
    print_matrix(c);

    int datad[] = {2, 0, -3, 4, 1, 5};
    int datae[] = {7, -1, 4, 7, 2, 5, 0, -4, -3, 1, 2, 3};
    WvMatrix d(2, 3, datad);
    printf("WvMatrix d:\n");
    print_matrix(d);
    WvMatrix e(3, 4, datae);
    printf("WvMatrix e:\n");
    print_matrix(e);

    WvMatrix f = d * e;

    printf("WvMatrix d * e = \n");
    print_matrix(f);

    printf("done\n");

    return 0;
}
