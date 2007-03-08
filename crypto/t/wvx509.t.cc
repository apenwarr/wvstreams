/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvx509.h"
#include "wvrsa.h"

void basic_test(WvX509Mgr *t509, WvStringParm dname)
{
    WVPASS(t509->test());
    fprintf(stderr, "Error: %s\n", t509->isok() ? "OK" : t509->errstr().cstr());
    WVPASS(t509->isok());
    WVPASS(t509->get_subject() == dname);
    WVPASS(t509->get_issuer() == dname);
}


WVTEST_MAIN("X509")
{
    static const char rsakeytext[] = 
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIICXgIBAAKBgQDVQrw1PHhLm2qSGl24SAb4+mJb2jPW0UwsbodDG5Da0h405zUp\n"
"0U5yL7zKNQG6BYvuV5bCUeo6Q0uvd7D6eKE7VqzcVHVJfM2KvfKOI5eMiu76pz6V\n"
"XxnW53kEdw1ZiCnCQTK1JntJn41ZsWxY7mQ3PLBpcobNIdoyUDwXIOFwOQIDAQAB\n"
"AoGAVcyuqgB1KX4Sx0tCT4TzATLDZc8JMjEso2eoldA+XDtTGde3pOZn2DrqirP+\n"
"yNe4b6Dfr7iDMwOmLKdMFcl4nAmE8UnOQWFtW52SCWFGLdiSqD8hn0gbKoYvaGDu\n"
"vlqN/KvKen0AxiveYS6T24GFzHejr2RVcmT4m5UogNGFR90CQQD7VcPzfcPJZ3c1\n"
"jWiJvoUrzdLqYactRoKidm+O7AVjMU7xkXCPLGgXbzVoyuzx11z1D/nFUE615t/v\n"
"vkKVsnTDAkEA2TgOnSX6gSClZLUy7bWoSHQwcuj3Ogt2C0zA9p/sRgRaAuNCDWNC\n"
"OmkRAHjbOtJK0tChxoG0BYdyv9WIZ5bHUwJBANf+0xH06UezNY2+YzLNmyEUF8j5\n"
"93Q/fpEke6c2S0L94zxTo4pHvYU2O449pvgH/4lUG3FpHNvS+GzO8+Y2oYUCQQC8\n"
"Nf8rmOmqEvBcB0jegQUT6mDEYCk+yQl6FwInb0AZFtIrKGBmGzgaRkkuAInsOKQO\n"
"cCmMR3wFQmxh3ZI4N4PzAkEA4YbOing3yMDXxUFByZMKSX6oqwNePuVfK1IHmoUP\n"
"oZxEIiwTN7rE9kRV1OFDUwvGiqcegSxjf6SUgdpBC7Du9A==\n"
"-----END RSA PRIVATE KEY-----\n";
    
    static const char x509certtext[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIICXDCCAcWgAwIBAgIEM7WjbjANBgkqhkiG9w0BAQUFADBBMRUwEwYDVQQDEwx0\n"
"ZXN0LmZvby5jb20xEzARBgoJkiaJk/IsZAEZEwNmb28xEzARBgoJkiaJk/IsZAEZ\n"
"EwNjb20wHhcNMDQwMzI2MjEwNTI5WhcNMTQwMzI0MjEwNTI5WjBBMRUwEwYDVQQD\n"
"Ewx0ZXN0LmZvby5jb20xEzARBgoJkiaJk/IsZAEZEwNmb28xEzARBgoJkiaJk/Is\n"
"ZAEZEwNjb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBANVCvDU8eEubapIa\n"
"XbhIBvj6YlvaM9bRTCxuh0MbkNrSHjTnNSnRTnIvvMo1AboFi+5XlsJR6jpDS693\n"
"sPp4oTtWrNxUdUl8zYq98o4jl4yK7vqnPpVfGdbneQR3DVmIKcJBMrUme0mfjVmx\n"
"bFjuZDc8sGlyhs0h2jJQPBcg4XA5AgMBAAGjYTBfMBEGCWCGSAGG+EIBAQQEAwIG\n"
"QDAbBglghkgBhvhCAQwEDhYMdGVzdC5mb28uY29tMA4GA1UdDwEB/wQEAwICpDAd\n"
"BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDQYJKoZIhvcNAQEFBQADgYEA\n"
"D8WtaiTyx3DkHiJwC7EHVCOTqRctuS+3x1y0b2CO/3ijWASg+UDvGFKpAT4i6EF4\n"
"/e1aTgQEJ3a29XQEKamk5Py2AAb7acBbuKIUzEzcFx4QzzY+fdz1PgKz4DiUypZj\n"
"UFCAL0f74uuFTQ7ylt6F0m554LSniOHDOEzyBZZq/bk=\n"
"-----END CERTIFICATE-----\n";
    
    static const char certreqtext[] =
"-----BEGIN CERTIFICATE REQUEST-----\n"
"MIIBazCB1QIBADAsMSowKAYKCZImiZPyLGQBGRMadGVzdC5mb28uY29tL0RDPWZv\n"
"by9EQz1jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBANVCvDU8eEubapIa\n"
"XbhIBvj6YlvaM9bRTCxuh0MbkNrSHjTnNSnRTnIvvMo1AboFi+5XlsJR6jpDS693\n"
"sPp4oTtWrNxUdUl8zYq98o4jl4yK7vqnPpVfGdbneQR3DVmIKcJBMrUme0mfjVmx\n"
"bFjuZDc8sGlyhs0h2jJQPBcg4XA5AgMBAAGgADANBgkqhkiG9w0BAQUFAAOBgQA3\n"
"+zj1ZCRGOqCyWJFT0J6P+LJxn57My7GKkUihXQFo6xym98kKfYwc7HTIVq+clnYA\n"
"rqmoQu1THFu6mVRdM9zp2zmRBtsmOxKkhx89RtNDhKu75HQE8S5xGmCPyZhFA0eX\n"
"elfqYQM30sg+MeZVTccV0wCd9augzYa2qA8MExu7SQ==\n"
"-----END CERTIFICATE REQUEST-----\n";
	
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
    
    const WvString strcert = "3082030030820269a003020102020460ad1ad"
"5300d06092a864886f70d010105050030818e310b3009060355040613024341310"
"f300d060355040813065175656265633111300f060355040713084d6f6e7472656"
"16c31123010060355040a13095465737479204f6e65311d301b060355040313146"
"1726961322e7765617665726e65742e6e756c6c31283026060a0992268993f22c6"
"4011916187765626d6173746572407765617665726e65742e6e756c6c301e170d3"
"034303333313230343235315a170d3134303332393230343235315a30818e310b3"
"009060355040613024341310f300d060355040813065175656265633111300f060"
"355040713084d6f6e747265616c31123010060355040a13095465737479204f6e6"
"5311d301b0603550403131461726961322e7765617665726e65742e6e756c6c312"
"83026060a0992268993f22c64011916187765626d6173746572407765617665726"
"e65742e6e756c6c30819f300d06092a864886f70d010101050003818d003081890"
"2818100b507269be849a8723ec72d6f769c9a4e228f3a4e7c4c948776d8f525364"
"b98ced92031592a054042f5fe63757cc270ac2a71785347d6d784f4b871161e13e"
"3ac2abe5c1830d8d2401a72976435fd4170c72aa063eedb45fa04d7ced92974162"
"363d78715fb044d66e6ee382a2f1ea70e8d8c50b9a2a575ab14cd7d56466046350"
"203010001a3693067301106096086480186f842010104040302064030230609608"
"6480186f842010c0416161461726961322e7765617665726e65742e6e756c6c300"
"e0603551d0f0101ff0404030202a4301d0603551d250416301406082b060105050"
"7030106082b06010505070302300d06092a864886f70d010105050003818100b2b"
"fe7b80feec0bfe23250f5d9eb3d409364628c001fee50185eb755ffb863093fe23"
"973c939fdd03bc90b32eb697f1eb9b4a7a0134ee78a509992da0d803af22b5148a"
"76b197a54126be4a3d03897ed6cc98e77e65b797aee3f3b66c17afb4a2fd6dc498"
"cd86f4d7952cfde7d1e044a38373f80b9d1da51461267a83d967f24"; 

    const WvString strrsa = "3082025e02010002818100b507269be849a872"
"3ec72d6f769c9a4e228f3a4e7c4c948776d8f525364b98ced92031592a054042f5"
"fe63757cc270ac2a71785347d6d784f4b871161e13e3ac2abe5c1830d8d2401a72"
"976435fd4170c72aa063eedb45fa04d7ced92974162363d78715fb044d66e6ee38"
"2a2f1ea70e8d8c50b9a2a575ab14cd7d5646604635020301000102818100b277a4"
"469c10d1f21f95f963240a6bcd9020a818ec4e0b3829a0e6bd92f3a0687c825264"
"571aea29999efbaabe1e6b3a3075c16c492cb338ae928f5a80b897005fdeca7966"
"14f58329552bfd117755fae42ba39aa3266bb2560377d5d747e31ddfd2ae1bbd9b"
"8979e8748d4d47573588a4140715fbc14ea373f2461517749641024100f0b0ddee"
"376d6e08c9dc0bce540e9b464bf23121241cf6dd69fd67c9e195c6dce0ddb47012"
"56761567a8c70fff3e12cc412e8cb74f120eb620d7d129624c359d024100c08acd"
"153fe5f801c81a85a62b63f50b9346ebe350a18c3aecd11379a17093f52fdea874"
"df97b22189dd4ca479723422c0a5b5124873e087f316cbf681d2fb79024100810c"
"33517fc25a56b7f4151861151bc78afca5bec1200e741459db85f03f5fca197e85"
"39f97b0600dffd2c0db5aa5065d724e02980698c1db66a4028d21d4e3902405e57"
"a48574f9c9bb95c0e91bb2c7179ac45f4bd5e5fc4229dd3fd4bb144f852fee74bb"
"360918db3f73bdeb7febc1f9a9cd9b644dc112864216ea64a634969c81024100e5"
"453ec9a3cdbe2fe17e86b32e998fe8713066ed254a60390f7898e4e536dea92af7"
"743d55a35fad75c14fe1e239251c471e133ca8e85ef3a1d3c5b6b288bfda";
    
    const WvString signature = "jmvlFrrXiXOkDzhlPQMvASEMc+ImuJFUmc/"
    "6X8vXujNqbbjerQVZvwOHx35lBcOx9gCcB089gDhPOD3Wi4RhHWZ0vWysdUimn"
    "Dl65EKEl3OS2anc8Pqi43+Tj7HVIxWB+nLI1m+wDTKqkx+3e1a4q0YjDqWTAT5"
    "4zTP1SHxKG5E=";

    // Setup a new DN entry, like a server would set.
    const WvString dName("cn=test.foo.com,dc=foo,dc=com");
    const WvString dName1("/CN=test.foo.com/DC=foo/DC=com");
    const WvString dName2("/C=CA/ST=Quebec/L=Montreal/O=Testy One/CN=aria2.weavernet.null/DC=webmaster@weavernet.null");
    
    {
	WvX509Mgr t509;
	WVFAIL(t509.isok());
	WVFAIL(t509.test());
    }
    
    // WIN32 people!  If your unit tests are crashing here, you probably
    // linked openssl with a different version of msvcr*.dll than the
    // unit test.  We pass a FILE* from one to the other here, and it
    // won't work if there's a library mismatch.
    {
        WvX509Mgr t509(NULL);
	t509.decode(WvX509Mgr::RsaPEM, rsakeytext);
	t509.decode(WvX509Mgr::CertPEM, x509certtext);
	basic_test(&t509, dName1);
	WvString certencode = t509.encode(WvX509Mgr::CertPEM);
	WVPASS(certencode == x509certtext);
	WvString rsaencode = t509.encode(WvX509Mgr::RsaPEM);
	WVPASS(rsaencode == rsakeytext);
	WVPASS(t509.certreq() == certreqtext);
    }
    {
        WvX509Mgr t509(strcert, strrsa);
	WVPASS(t509.get_serial() == "1621957333");
	basic_test(&t509, dName2);
	WVPASS(t509.hexify() == strcert);
    }
    {
	WvX509Mgr t509(dName, 1024);
	basic_test(&t509, dName1);
    }
    {
	WvRSAKey *rsa = new WvRSAKey(strrsa, true);
	WvX509Mgr t509(dName, rsa);
	basic_test(&t509, dName1);
	WVFAIL(t509.hexify() == strcert); // This should fail since the times are different in the certificates.
	t509.decode(WvX509Mgr::CertPEM, signedcerttext);
	WVPASS(t509.test());
	WVPASS(t509.isok());
	WVPASS(t509.get_subject() == "/C=CA/ST=Quebec/L=Montreal/O=Testy One/CN=aria2.weavernet.null");
	WVPASS(t509.get_issuer() == "/O=NITI-TEST");
	// The certificate in x509certtext has a different public/private key
	// and so, this SHOULD fail.
	t509.decode(WvX509Mgr::CertPEM, x509certtext);
	WVFAIL(t509.test());
	WVFAIL(t509.isok());
    }
    {
	// Now we stress test it to make sure that it fails predictably...
	WvX509Mgr t509(NULL);
	t509.decode(WvX509Mgr::RsaPEM, WvString::null);
	t509.decode(WvX509Mgr::CertPEM, "");
	WVFAIL(t509.test());
	WVFAIL(t509.isok());
	WVFAIL(t509.get_subject() == "/CN=test.foo.com/DC=foo/DC=com");
	WVFAIL(t509.get_issuer() == "/CN=test.foo.com/DC=foo/DC=com");
    }
    {
        WvX509Mgr t509("", strrsa);
	WVFAIL(t509.test());
	WVFAIL(t509.isok());
	WVFAIL(t509.get_subject() == "/CN=test.foo.com/DC=foo/DC=com");
	WVFAIL(t509.get_issuer() == "/CN=test.foo.com/DC=foo/DC=com");
    }
    {
	WvX509Mgr t509(strcert, "");
	WVFAIL(t509.get_rsa().isok());
    }
    
    {
	WvX509Mgr t509(strcert, strrsa);
	WVPASS(t509.sign("foo") == signature);
    }
	
    {
	WvX509Mgr t509(strcert, strrsa);
	WVPASS(t509.verify("foo", signature));
    }
    {
	// This test is currently invalid, since 
	// strcert doesn't have the right keyUsage set, and we
	// check for that now.
	// I'll fix it when I fix niti BUG:9969 (not publically
        // available...)
#if 0	
	WvX509Mgr t509(strcert, strrsa);
	
	WvX509Mgr n509(dName1, 1024);
	WvString request(n509.certreq());
	WvString cert(t509.signcert(request));
	WVFAIL(t509.isok());
	n509.decode(WvX509Mgr::CertPEM, cert);
	WVPASS(n509.get_issuer() == dName2);
	fprintf(stderr,"\n\n%s\n\n", n509.get_issuer());
	WVPASS(n509.validate(&t509));
#endif
    }
}


WVTEST_MAIN("Get extensions memory corruption")
{
    // this test validates a workaround for a bug
    // in openssl 0.9.8 -- where getting certain attributes using
    // ASN1_item_d2i could cause openssl to shuffle around
    // const pointers, leading to weird memory behaviour on
    // destroys

    static const char triggercert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFSzCCBDOgAwIBAgIBAzANBgkqhkiG9w0BAQUFADCBgDELMAkGA1UEBhMCQ0Ex\n"
    "KzApBgNVBAoTIkNhcmlsbG9uIEluZm9ybWF0aW9uIFNlY3VyaXR5IEluYy4xHzAd\n"
    "BgNVBAsTFkNlcnRpZmljYXRpb24gU2VydmljZXMxIzAhBgNVBAMTGlRlc3QgQ2Vy\n"
    "dGlmaWNhdGUgQXV0aG9yaXR5MB4XDTA2MTIyNDIyMzgzNVoXDTA5MTIyMzIyMzgz\n"
    "NVowgYIxCzAJBgNVBAYTAkNBMSswKQYDVQQKDCJDYXJpbGxvbiBJbmZvcm1hdGlv\n"
    "biBTZWN1cml0eSBJbmMuMQ4wDAYDVQQLDAVVc2VyczEVMBMGA1UEAwwMVGVzdGlu\n"
    "ZyBVc2VyMR8wHQYJKoZIhvcNAQkBFhB0ZXN0QGNhcmlsbG9uLmNhMIGfMA0GCSqG\n"
    "SIb3DQEBAQUAA4GNADCBiQKBgQCwYnEgxl9uly9EeXg6N/SLzx6ZB5iU/REgB1bi\n"
    "9u/4O7S7ebAh5VKO8JIo+0JaHW7pBFM0ywe3KvHwpvdsHvmRCQ4+Qze8itFuhDSS\n"
    "TFJ7eP9bxiyroYIaRuS9G3xrM6jGdkw1IhmN2FDpWXBTWtfF/8Lor4p9TemGUARl\n"
    "prfDjQIDAQABo4ICTjCCAkowCQYDVR0TBAIwADAOBgNVHQ8BAf8EBAMCB4AwUAYJ\n"
    "YIZIAYb4QgENBEMWQURvIE5vdCB0cnVzdCAtIENlcnRpUGF0aCBjb21wbGlhbnQg\n"
    "SUQgQ2VydCBmb3IgVEVTVCBwdXJwb3NlcyBvbmx5MB0GA1UdDgQWBBRI8EWCS88U\n"
    "Hug1TLLei5iiJGJSXTAfBgNVHSMEGDAWgBQZ8G5V0iRoGnd6l8LUaI01I0M3+jAb\n"
    "BgNVHREEFDASgRB0ZXN0QGNhcmlsbG9uLmNhMEsGCCsGAQUFBwEBBD8wPTA7Bggr\n"
    "BgEFBQcwAoYvaHR0cDovL3d3dy5jYXJpbGxvbi5jYS9jYW9wcy9tZWRpdW0tdGVz\n"
    "dC1jYS5jcnQwPQYDVR0fBDYwNDAyoDCgLoYsaHR0cDovL3d3dy5jYXJpbGxvbi5j\n"
    "YS9jYW9wcy9tZWRpdW0tY3JsMS5jcmwwgfEGA1UdIASB6TCB5jBuBgsrBgEEAYHD\n"
    "XgEBZTBfMF0GCCsGAQUFBwICMFEwKRoiQ2FyaWxsb24gSW5mb3JtYXRpb24gU2Vj\n"
    "dXJpdHkgSW5jLjADAgEBGiRURVNUIG1lZGl1bSBzdyBwb2xpY3kgLSBkbyBub3Qg\n"
    "dHJ1c3QwdAYLKwYBBAGBw14BAWYwZTBjBggrBgEFBQcCAjBXMCkaIkNhcmlsbG9u\n"
    "IEluZm9ybWF0aW9uIFNlY3VyaXR5IEluYy4wAwIBARoqVEVTVCBtZWRpdW0gaGFy\n"
    "ZHdhcmUgcG9saWN5IC0gZG8gbm90IHRydXN0MA0GCSqGSIb3DQEBBQUAA4IBAQAH\n"
    "NnhF+cbDe2jCPZ8J36Vb9p7QBFhGHiPQKpWVgEISjfYDqyilDUMvqo1RshBnxdTt\n"
    "cPMu1MxcQOOvU1f3jRCJPngzzZXW91zOZKQREPanekUcil7ZrY9MPgpIPqMgGw5h\n"
    "rX5R/RyAG/vkfJXe5SMd4GeUOuPuDVCxM3y2cylT0TS5zaWSYNdmERDm5tp2fHlj\n"
    "rqE1w9PZDeLz9ZhKfo/pcxCvE7RdS1Zhpi3KMUEtDXq9RN2D81mvo1TyzHvm84QB\n"
    "R7Z/1y/HH8vZj7q//xJrJt/IqPuXcYUacaS520ouzPhXNRkMxl4VZ8fCGnbrqPig\n"
    "0KUUwRu2l4LDN9drx+L5\n"
    "-----END CERTIFICATE-----\n";

    WvX509Mgr t509(NULL);
    t509.decode(WvX509Mgr::CertPEM, triggercert);

    WVPASS(t509.isok());
    printf("%s", t509.get_aia().cstr());
}
