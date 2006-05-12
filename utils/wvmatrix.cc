#include "wvmatrix.h"

WvMatrix::WvMatrix(const int _m, const int _n, const int *_data)
    : m(_m), n(_n)
{
    if (!m || !n)
    {
	data = NULL;
	return;
    }

    data = new int[m * n];
    
    if (_data)
	memcpy(data, _data, m * n * sizeof(int));
    else
	for (int i = 0; i < m * n; i++)
	    data[i] = 0;
}


WvMatrix::~WvMatrix()
{
    delete[] data;
}


WvMatrix::WvMatrix(const WvMatrix& mx)
    : m(mx.m), n(mx.n)
{
    data = new int[m * n];
    memcpy(data, mx.data, m * n * sizeof(int));
}


WvMatrix& WvMatrix::operator= (const WvMatrix& mx)
{
    delete[] data;
    data = new int[m * n];

    m = mx.m;
    n = mx.n;
    memcpy(data, mx.data, m * n * sizeof(int));

    return *this;
}


WvMatrix WvMatrix::operator+ (const WvMatrix &rhs) const
{
    WvMatrix res(rhs);

    if (m != rhs.m || n != rhs.n)
	return res;

    for (int i = 0; i < m * n; i++)
	res.data[i] += data[i];

    return res;
}


WvMatrix WvMatrix::operator* (const WvMatrix &rhs) const
{
    WvMatrix res(m, rhs.n);

    if (n != rhs.m)
	return res;

    int c;

    for (int i = 0; i < res.m; i++)
	for (int j = 0; j < res.n; j++)
	{
	    c = 0;
	    for (int k = 0; k < n; k++)
		c += (*this)(i, k) * rhs(k, j);
	    res(i, j) = c;
	}

    return res;
}


WvString WvMatrix::printable()
{
    WvString res("{%s", data[0]);
    for (int i = 1; i < m * n; i++)
	res.append(", %s", data[i]);
    return res;
}
