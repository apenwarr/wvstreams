#include "wvconf.h"
#include "wvx509.h"
#include "wvrsa.h"
#include <assert.h>

int main()
{
    WvRSAKey *rsa1 = new WvRSAKey(1024);
    WvX509Mgr cert1("O=foo.com", rsa1);
    
    assert(cert1.test());
    
    WvString rsastr(rsa1->private_str()), rsaxstr(rsa1->public_str());
    WvString certstr(cert1.hexify()); 
    
    printf("string lengths: %d %d %d\n", rsastr.len(), rsaxstr.len(),
	   certstr.len());
    
    WvRSAKey rsa2(rsastr, rsaxstr);
    WvX509Mgr cert2(certstr, rsastr);
    
    assert(rsa2.isok());
    assert(cert2.test());

    return 0;
}
