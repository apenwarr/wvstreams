/*
 * Forces all of the templates to be expanded in place and tested.
 */

#include <iostream>
#include <cmath>
#include <cassert>
#include "wvfunctor.h"

static void sfvoid0() {
    cout << "static: sfvoid0()" << endl;
}

class Test {
    int id;
public:
    Test(int id) : id(id) { }
        
    static void fvoid0() {
        cout << "static: fvoid0()" << endl;
    }
    static int fint0() {
        cout << "static: fint0()" << endl;
        return 42;
    }
    static void fvoid1(int a1) {
        cout << "static: fvoid1(" << a1 << ")" << endl;
    }
    static int fint1(int a1) {
        cout << "static: fint1(" << a1 << ")" << endl;
        return 42;
    }
    static void fvoid2(int a1, double a2) {
        cout << "static: fvoid2(" << a1 << ", " << a2 << ")" << endl;
    }
    static int fint2(int a1, double a2) {
        cout << "static: fint2(" << a1 << ", " << a2 << ")" << endl;
        return 42;
    }
    static void fvoid3(int a1, double a2, const char* a3) {
        cout << "static: fvoid3(" << a1 << ", " << a2 << ", " << a3 << ")" << endl;
    }
    static int fint3(int a1, double a2, const char* a3) {
        cout << "static: fint3(" << a1 << ", " << a2 << ", " << a3 << ")" << endl;
        return 42;
    }

    void mfvoid0() {
        cout << id << ": mfvoid0()" << endl;
    }
    int mfint0() {
        cout << id << ": mfint0()" << endl;
        return 42;
    }
    void mfvoid1(int a1) {
        cout << id << ": mfvoid1(" << a1 << ")" << endl;
    }
    int mfint1(int a1) {
        cout << id << ": mfint1(" << a1 << ")" << endl;
        return 42;
    }
    void mfvoid2(int a1, double a2) {
        cout << id << ": mfvoid2(" << a1 << ", " << a2 << ")" << endl;
    }
    int mfint2(int a1, double a2) {
        cout << id << ": mfint2(" << a1 << ", " << a2 << ")" << endl;
        return 42;
    }

    void cmfvoid0() const {
        cout << id << ": cmfvoid0()" << endl;
    }
    int cmfint0() const {
        cout << id << ": cmfint0()" << endl;
        return 42;
    }
    void cmfvoid1(int a1) const {
        cout << id << ": cmfvoid1(" << a1 << ")" << endl;
    }
    int cmfint1(int a1) const {
        cout << id << ": cmfint1(" << a1 << ")" << endl;
        return 42;
    }
    void cmfvoid2(int a1, double a2) const {
        cout << id << ": cmfvoid2(" << a1 << ", " << a2 << ")" << endl;
    }
    int cmfint2(int a1, double a2) const {
        cout << id << ": cmfint2(" << a1 << ", " << a2 << ")" << endl;
        return 42;
    }
};

int main(int argc, char** argv) {
    Test* test = new Test(12345);

    // test pointers to static functions
    cout << "=== static ===" << endl;
    
    WvFunctor<void> fvoid0(Test::fvoid0);
    WvFunctor<int> fint0(Test::fint0);
    fvoid0();
    assert(fint0() == 42);
    
    WvFunctor<void, int> fvoid1(Test::fvoid1);
    WvFunctor<int, int> fint1(Test::fint1);
    WvFunctor<void> bfvoid1(Test::fvoid1, 13);
    WvFunctor<int> bfint1(Test::fint1, 13);
    fvoid1(13);
    assert(fint1(13) == 42);
    bfvoid1();
    assert(bfint1() == 42);

    WvFunctor<void, int, double> fvoid2(Test::fvoid2);
    WvFunctor<int, int, double> fint2(Test::fint2);
    WvFunctor<void, double> bfvoid2(Test::fvoid2, 13);
    WvFunctor<int, double> bfint2(Test::fint2, 13);
    WvFunctor<void> bbfvoid2(Test::fvoid2, 13, M_PI);
    WvFunctor<int> bbfint2(Test::fint2, 13, M_PI);
    fvoid2(13, M_PI);
    assert(fint2(13, M_PI) == 42);
    bfvoid2(M_PI);
    assert(bfint2(M_PI) == 42);
    bbfvoid2();
    assert(bbfint2() == 42);

    WvFunctor<void, int, double, const char*> fvoid3(Test::fvoid3);
    WvFunctor<int, int, double, const char*> fint3(Test::fint3);
    WvFunctor<void, double, const char*> bfvoid3(Test::fvoid3, 13);
    WvFunctor<int, double, const char*> bfint3(Test::fint3, 13);
    WvFunctor<void, const char*> bbfvoid3(Test::fvoid3, 13, M_PI);
    WvFunctor<int, const char*> bbfint3(Test::fint3, 13, M_PI);
    WvFunctor<void> bbbfvoid3(Test::fvoid3, 13, M_PI, "foobar");
    WvFunctor<int> bbbfint3(Test::fint3, 13, M_PI, "foobar");
    fvoid3(13, M_PI, "foobar");
    assert(fint3(13, M_PI, "foobar") == 42);
    bfvoid3(M_PI, "foobar");
    assert(bfint3(M_PI, "foobar") == 42);
    bbfvoid3("foobar");
    assert(bbfint3("foobar") == 42);
    bbbfvoid3();
    assert(bbbfint3() == 42);
    
    cout << endl;
    
    // test pointers to member funcptrtions
    cout << "=== member ===" << endl;
    
    WvFunctor<void, Test*> mfvoid0(&Test::mfvoid0);
    WvFunctor<int, Test*> mfint0(&Test::mfint0);
    WvFunctor<void> bmfvoid0(&Test::mfvoid0, test);
    WvFunctor<int> bmfint0(&Test::mfint0, test);
    mfvoid0(test);
    assert(mfint0(test) == 42);
    bmfvoid0();
    assert(bmfint0() == 42);

    WvFunctor<void, Test*, int> mfvoid1(&Test::mfvoid1);
    WvFunctor<int, Test*, int> mfint1(&Test::mfint1);
    WvFunctor<void, int> bmfvoid1(&Test::mfvoid1, test);
    WvFunctor<int, int> bmfint1(&Test::mfint1, test);
    WvFunctor<void> bbmfvoid1(&Test::mfvoid1, test, 13);
    WvFunctor<int> bbmfint1(&Test::mfint1, test, 13);
    mfvoid1(test, 13);
    assert(mfint1(test, 13) == 42);
    bmfvoid1(13);
    assert(bmfint1(13) == 42);
    bbmfvoid1();
    assert(bbmfint1() == 42);
    
    WvFunctor<void, Test*, int, double> mfvoid2(&Test::mfvoid2);
    WvFunctor<int, Test*, int, double> mfint2(&Test::mfint2);
    WvFunctor<void, int, double> bmfvoid2(&Test::mfvoid2, test);
    WvFunctor<int, int, double> bmfint2(&Test::mfint2, test);
    WvFunctor<void, double> bbmfvoid2(&Test::mfvoid2, test, 13);
    WvFunctor<int, double> bbmfint2(&Test::mfint2, test, 13);
    WvFunctor<void> bbbmfvoid2(&Test::mfvoid2, test, 13, M_PI);
    WvFunctor<int> bbbmfint2(&Test::mfint2, test, 13, M_PI);
    mfvoid2(test, 13, M_PI);
    assert(mfint2(test, 13, M_PI) == 42);
    bmfvoid2(13, M_PI);
    assert(bmfint2(13, M_PI) == 42);
    bbmfvoid2(M_PI);
    assert(bbmfint2(M_PI) == 42);
    bbbmfvoid2();
    assert(bbbmfint2() == 42);

    cout << endl;

    // test bound pointers to constant member funcptrtions
    cout << "=== constant member ===" << endl;
    
    WvFunctor<void, Test*> cmfvoid0(&Test::cmfvoid0);
    WvFunctor<int, Test*> cmfint0(&Test::cmfint0);
    WvFunctor<void> bcmfvoid0(&Test::cmfvoid0, test);
    WvFunctor<int> bcmfint0(&Test::cmfint0, test);
    cmfvoid0(test);
    assert(cmfint0(test) == 42);
    bcmfvoid0();
    assert(bcmfint0() == 42);
    
    WvFunctor<void, Test*, int> cmfvoid1(&Test::cmfvoid1);
    WvFunctor<int, Test*, int> cmfint1(&Test::cmfint1);
    WvFunctor<void, int> bcmfvoid1(&Test::cmfvoid1, test);
    WvFunctor<int, int> bcmfint1(&Test::cmfint1, test);
    WvFunctor<void> bbcmfvoid1(&Test::cmfvoid1, test, 13);
    WvFunctor<int> bbcmfint1(&Test::cmfint1, test, 13);
    cmfvoid1(test, 13);
    assert(cmfint1(test, 13) == 42);
    bcmfvoid1(13);
    assert(bcmfint1(13) == 42);
    bbcmfvoid1();
    assert(bbcmfint1() == 42);
    
    WvFunctor<void, Test*, int, double> cmfvoid2(&Test::cmfvoid2);
    WvFunctor<int, Test*, int, double> cmfint2(&Test::cmfint2);
    WvFunctor<void, int, double> bcmfvoid2(&Test::cmfvoid2, test);
    WvFunctor<int, int, double> bcmfint2(&Test::cmfint2, test);
    WvFunctor<void, double> bbcmfvoid2(&Test::cmfvoid2, test, 13);
    WvFunctor<int, double> bbcmfint2(&Test::cmfint2, test, 13);
    WvFunctor<void> bbbcmfvoid2(&Test::cmfvoid2, test, 13, M_PI);
    WvFunctor<int> bbbcmfint2(&Test::cmfint2, test, 13, M_PI);
    cmfvoid2(test, 13, M_PI);
    assert(cmfint2(test, 13, M_PI) == 42);
    bcmfvoid2(13, M_PI);
    assert(bcmfint2(13, M_PI) == 42);
    bbcmfvoid2(M_PI);
    assert(bbcmfint2(M_PI) == 42);
    bbbcmfvoid2();
    assert(bbbcmfint2() == 42);

    cout << endl;

    // test pointer assignment and nullity
    cout << "=== pointer assignment and nullity ===" << endl;

    WvFunctor<void> test0(Test::fvoid0);
    test0();
    assert(!! test0);
    
    test0 = sfvoid0;
    test0();
    assert(!! test0);

    test0 = Test::fvoid0;
    test0();
    assert(!! test0);

    WvFunctor<void> bound0(&Test::mfvoid0, test);
    bound0();
    assert(!! bound0);
    
    test0 = bound0;
    test0();
    assert(!! test0);
    assert(!! bound0);

    WvFunctor<void> null0;
    WvFunctor<void> anull0(0);
    assert(! null0);
    assert(! anull0);

    test0 = null0;
    assert(! test0);
    assert(! null0);

    test0 = 0;
    assert(! test0);

    cout << endl;

    // test pointer equality
    cout << "=== pointer equality ===" << endl;
    
    WvFunctor<void> eqtest0a(Test::fvoid0);
    WvFunctor<void> eqtest0b(Test::fvoid0);
    WvFunctor<void> eqtest0c(&Test::mfvoid0, test);
    WvFunctor<void> eqtest0d(0);
    WvFunctor<void> eqtest0e(0);
    WvFunctor<void, Test*> eqtest1a(&Test::mfvoid0);
    WvFunctor<void, Test*> eqtest1b;
    WvFunctor<void, Test*> eqtest1c(0);
    WvFunctor<void, Test*> eqtest1d((void (*)(Test*))0);
    assert(eqtest0a == eqtest0b);
    assert(eqtest0a != eqtest0c);
    assert(eqtest0b != eqtest0c);
    assert(eqtest0a != eqtest0d);
    assert(eqtest0d == eqtest0e);
    assert(eqtest1a != eqtest1b);
    assert(eqtest1b == eqtest1b);
    assert(eqtest1b == eqtest1c);
    assert(eqtest1c == eqtest1d);

    cout << endl;

    // done!
    cout << "=== all tests passed ===" << endl;
    return 0;
}
