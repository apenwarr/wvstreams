#include "wvlog.h"
#include "wvtcp.h"
#include "wvsslstream.h"
#include "wvx509.h"
#include "wvrsa.h"
#include "wvistreamlist.h"
#include "strutils.h"
#include "wvcrash.h"
#include <signal.h>

volatile bool want_to_die = false;
WvX509Mgr *x509cert = NULL;

static const char signedcerttext[] =
"-----BEGIN CERTIFICATE-----\n"
"MIICiDCCAfGgAwIBAgIBATANBgkqhkiG9w0BAQQFADAUMRIwEAYDVQQKEwlOSVRJ\n"
"LVRFU1QwHhcNMDQwMzMwMTgzOTM4WhcNMDUwMzMwMTgzOTM4WjBkMQswCQYDVQQG\n"
"EwJDQTEPMA0GA1UECBMGUXVlYmVjMREwDwYDVQQHEwhNb250cmVhbDESMBAGA1UE\n"
"ChMJVGVzdHkgT25lMR0wGwYDVQQDExRhcmlhMi53ZWF2ZXJuZXQubnVsbDCBnzAN\n"
"BgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAtQcmm+hJqHI+xy1vdpyaTiKPOk58TJSH\n"
"dtj1JTZLmM7ZIDFZKgVAQvX+Y3V8wnCsKnF4U0fW14T0uHEWHhPjrCq+XBgw2NJA\n"
"GnKXZDX9QXDHKqBj7ttF+gTXztkpdBYjY9eHFfsETWbm7jgqLx6nDo2MULmipXWr\n"
"FM19VkZgRjUCAwEAAaOBmTCBljAJBgNVHRMEAjAAMCwGCWCGSAGG+EIBDQQfFh1P\n"
"cGVuU1NMIEdlbmVyYXRlZCBDZXJ0aWZpY2F0ZTAdBgNVHQ4EFgQUzpHzxgB4JiHt\n"
"/EbtqgfEbSpsThUwPAYDVR0jBDUwM4AUx1ECvcgeUvpwQJNQs2g3qq0CPzWhGKQW\n"
"MBQxEjAQBgNVBAoTCU5JVEktVEVTVIIBADANBgkqhkiG9w0BAQQFAAOBgQCLUpzC\n"
"nPlIYTDK7rq1Mh/Bqcw0RYamxUOCMsNmkMBovb37yfmo4k+HfhiHhiM6Ao6H5GIF\n"
"+p09tcn1Iotrek1IGrXsH7Hjkpzf8lkqwEjygIJ0iAWMqAI4AVoRx/5JrG+8ePPU\n"
"rKZf8WzBwn45ibb+xwtjdQXVro1l+K4UATTHLQ==\n"
"-----END CERTIFICATE-----\n";

const WvString strrsa = "3082025e02010002818100b507269be849a8723ec72d6f769c9a4e228f3a4e7c4c948776d8f525364b98ced92031592a054042f5fe63757cc270ac2a71785347d6d784f4b871161e13e3ac2abe5c1830d8d2401a72976435fd4170c72aa063eedb45fa04d7ced92974162363d78715fb044d66e6ee382a2f1ea70e8d8c50b9a2a575ab14cd7d5646604635020301000102818100b277a4469c10d1f21f95f963240a6bcd9020a818ec4e0b3829a0e6bd92f3a0687c825264571aea29999efbaabe1e6b3a3075c16c492cb338ae928f5a80b897005fdeca796614f58329552bfd117755fae42ba39aa3266bb2560377d5d747e31ddfd2ae1bbd9b8979e8748d4d47573588a4140715fbc14ea373f2461517749641024100f0b0ddee376d6e08c9dc0bce540e9b464bf23121241cf6dd69fd67c9e195c6dce0ddb4701256761567a8c70fff3e12cc412e8cb74f120eb620d7d129624c359d024100c08acd153fe5f801c81a85a62b63f50b9346ebe350a18c3aecd11379a17093f52fdea874df97b22189dd4ca479723422c0a5b5124873e087f316cbf681d2fb79024100810c33517fc25a56b7f4151861151bc78afca5bec1200e741459db85f03f5fca197e8539f97b0600dffd2c0db5aa5065d724e02980698c1db66a4028d21d4e3902405e57a48574f9c9bb95c0e91bb2c7179ac45f4bd5e5fc4229dd3fd4bb144f852fee74bb360918db3f73bdeb7febc1f9a9cd9b644dc112864216ea64a634969c81024100e5453ec9a3cdbe2fe17e86b32e998fe8713066ed254a60390f7898e4e536dea92af7743d55a35fad75c14fe1e239251c471e133ca8e85ef3a1d3c5b6b288bfda";

bool validateme(WvX509Mgr *x509)
{
    if (x509 && x509->isok())
    {
    	wvcon->print("X509 Subject: %s\n", x509->get_subject());
    	return true;
    }
    else
    {    
	wvcon->print("X509 Error: %s\n", x509->errstr());
        return false;
    }
}

void sighandler_die(int sig)
{
    fprintf(stderr, "Exiting on signal %d.\n", sig);
    want_to_die = true;
    signal(sig, SIG_DFL);
}


void bounce_to_list(WvStream &s, void *_list)
{
    char buf[1024];
    size_t len;
    
    len = s.read(buf, sizeof(buf));
    
    WvIStreamList &list = *(WvIStreamList *)_list;
    WvIStreamList::Iter i(list);
    for (i.rewind(); i.next(); )
    {
	if (i.ptr() != &s) i->write(buf, len);
    }
}
 
 
void tcp_incoming(WvStream &_listener, void *userdata)
{
    WvTCPListener *listener = (WvTCPListener *)&_listener;
    WvIStreamList *l = (WvIStreamList *)userdata;
    WvTCPConn *s = listener->accept();
    
    if (s)
    {
	assert(x509cert);
	WvSSLStream *sslsrvr = new WvSSLStream(s, x509cert, validateme,
					       true);
	l->append(sslsrvr, true, "ss tcp");
	sslsrvr->setcallback(bounce_to_list, l);
    }
}

 
void setupcert()
{
    WvString dName = encode_hostname_as_DN(fqdomainname());
    WvRSAKey *rsa = new WvRSAKey(strrsa, true);
    x509cert = new WvX509Mgr(dName, rsa);
    if (!x509cert->isok())
    {
	wverr->print("Error: %s\n", x509cert->errstr());
	want_to_die = true;
    }
}


int main(int argc, char **argv)
{
    // Set up WvCrash
    wvcrash_setup(argv[0]);
    
    // make sure electric fence works
    free(malloc(1));

    WvLog log("SSL-Server", WvLog::Info);
    WvIStreamList l;
    
    signal(SIGINT,  sighandler_die);
    signal(SIGTERM, sighandler_die);
    signal(SIGPIPE, SIG_IGN);
    
    if (argc >= 2)
    {
	WvString dName = encode_hostname_as_DN(argv[1]);
	WvRSAKey *rsa = new WvRSAKey(strrsa, true);
    	x509cert = new WvX509Mgr(dName, rsa);
    	if (!x509cert->isok())
    	{
	    wverr->print("Error: %s\n", x509cert->errstr());
	    want_to_die = true;
    	}	
    } 
    else
    {
	setupcert();
    }

    //x509cert->decode(WvX509Mgr::CertPEM, signedcerttext);
    WvTCPListener tcplisten("0.0.0.0:5238");
    tcplisten.setcallback(tcp_incoming, &l);
    
    if (!tcplisten.isok())
    {
	log("Can't listen: %s\n", tcplisten.errstr());
	return 1;
    }
    
    log("Listening on %s.\n", *tcplisten.src());
    l.append(&tcplisten, false, "ss tcp listener"); 
    l.append(wvcon, false, "wvcon");
    
    wvcon->setcallback(bounce_to_list, &l);
    
    while (!want_to_die && wvcon->isok() && tcplisten.isok())
    {
	if (l.select(-1))
	    l.callback(); 
    }
    if (!tcplisten.isok())
	wvcon->print("Exited with error: %s\n", tcplisten.errstr());
    

    return 0;
}

