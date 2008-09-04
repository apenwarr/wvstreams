#include "wvfile.h"
#include "wvfileutils.h"
#include "wvrsa.h"
#include "wvtest.h"
#include "wvx509.h"
#include "wvx509mgr.h"
#include "wvautoconf.h"

// default keylen for where we're not using pre-existing certs
const static int DEFAULT_KEYLEN = 512; 

// carillon cert: has a number of interesting characteristics (policy,
// aia information, ...) which we can test against
const static char carillon_cert[] =
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

const static char rsakeytext[] = 
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
    
const static char x509certtext[] = 
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
    
const static char certreqtext[] =
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
	
const static char signedcerttext[] =
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


static void basic_test(WvX509 &t509, WvStringParm dname)
{
    WVPASS(t509.isok());
    WVPASSEQ(t509.get_subject(), dname);
    WVPASSEQ(t509.get_issuer(), dname);
}


WVTEST_MAIN("X509")
{
    // Setup a new DN entry, like a server would set.
    const WvString dName("cn=test.foo.com,dc=foo,dc=com");
    const WvString dName1("/CN=test.foo.com/DC=foo/DC=com");
    const WvString dName2("/C=CA/ST=Quebec/L=Montreal/O=Testy One"
			  "/CN=aria2.weavernet.null"
			  "/DC=webmaster@weavernet.null");
    
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
        WvX509Mgr t509;
	t509.decode(WvRSAKey::RsaPEM, rsakeytext);
	t509.decode(WvX509::CertPEM, x509certtext);
	basic_test(t509, dName1);
	WvString certencode = t509.encode(WvX509::CertPEM);
	WVPASSEQ(certencode, x509certtext);
	WvString rsaencode = t509.encode(WvRSAKey::RsaPEM);
	WVPASSEQ(rsaencode, rsakeytext);
        WvRSAKey rsa;
        rsa.decode(WvRSAKey::RsaPEM, rsakeytext);
	WVPASSEQ(WvX509::certreq(t509.get_subject(), rsa),
                 certreqtext);
    }
    {
	WvX509Mgr t509(dName, 1024);
	basic_test(t509, dName1);
    }
    {        
        WvX509 t509;
	t509.decode(WvX509::CertPEM, signedcerttext);
	WVPASS(t509.isok()); 
        WVFAIL(t509.validate()); // this certificate has expired
	WVPASSEQ(t509.get_subject(),
		 "/C=CA/ST=Quebec/L=Montreal/O=Testy One"
		 "/CN=aria2.weavernet.null");
	WVPASSEQ(t509.get_issuer(), "/O=NITI-TEST");
        WVPASSEQ(t509.get_ski(), 
             "CE:91:F3:C6:00:78:26:21:ED:FC:46:ED:AA:07:C4:6D:2A:6C:4E:15");
        WVPASSEQ(t509.get_aki(), 
             "C7:51:02:BD:C8:1E:52:FA:70:40:93:50:B3:68:37:AA:AD:02:3F:35");
    }    
    {
	// Now we stress test it to make sure that it fails predictably...
	WvX509Mgr t509;
	t509.decode(WvRSAKey::RsaPEM, WvString::null);
	t509.decode(WvX509::CertPEM, "");
	WVFAIL(t509.test());
	WVFAIL(t509.isok());
	WVFAIL(t509.get_subject() == "/CN=test.foo.com/DC=foo/DC=com");
	WVFAIL(t509.get_issuer() == "/CN=test.foo.com/DC=foo/DC=com");
    }
}


WVTEST_MAIN("mismatched key / certificate")
{
    WvX509Mgr t509;
    t509.decode(WvX509::CertPEM, signedcerttext);
    t509.decode(WvRSAKey::RsaPEM, rsakeytext);
    WVFAIL(t509.isok());
    WVFAIL(t509.test());
}


WVTEST_MAIN("Hexified encoding/decoding")
{
    const char cert_hex[] ="3082030030820269a003020102020460ad1ad"
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

    const char rsa_hex[] = "3082025e02010002818100b507269be849a872"
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

    const WvString dName2("/C=CA/ST=Quebec/L=Montreal"
			  "/O=Testy One/CN=aria2.weavernet.null"
			  "/DC=webmaster@weavernet.null");

    WvX509Mgr t509;
    t509.decode(WvX509Mgr::CertHex, cert_hex);
    t509.decode(WvRSAKey::RsaHex, rsa_hex);
    basic_test(t509, dName2);
    
    WVPASSEQ(t509.get_serial(), "1621957333");
    WVPASSEQ(t509.encode(WvX509Mgr::CertHex), cert_hex);
    WVPASSEQ(t509.encode(WvRSAKey::RsaHex), rsa_hex);
}


WVTEST_MAIN("Replacing key failure")
{
    WvX509Mgr t509("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN);
    WVPASS(t509.test());
    WvRSAKey rsakey(DEFAULT_KEYLEN);
    t509.decode(WvRSAKey::RsaHex, rsakey.encode(WvRSAKey::RsaHex));
    WVFAIL(t509.test());
}


WVTEST_MAIN("Get extensions memory corruption")
{
    // this test validates a workaround for a bug
    // in openssl 0.9.8 -- where getting certain attributes using
    // ASN1_item_d2i could cause openssl to shuffle around
    // const pointers, leading to weird memory behaviour on
    // destroys

    WvX509 t509;
    t509.decode(WvX509Mgr::CertPEM, carillon_cert);

    WVPASS(t509.isok());
    WVPASSEQ("CA Issuers - "
	     "URI:http://www.carillon.ca/caops/medium-test-ca.crt", 
             t509.get_aia());
}


WVTEST_MAIN("set_aia")
{
    // yeah, the certificate generated doesn't make much sense here,
    // but we're only testing that the extension works properly

    WvX509Mgr x("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN, false);
    WvStringList ca_in;
    WvStringList ocsp_in;
    ca_in.append("http://localhost/~wlach/testca.pem");
    ca_in.append("http://localhost/~wlach/testca-alt.pem");
    ocsp_in.append("http://localhost/~wlach/blah.ocsp");
    ocsp_in.append("http://localhost/~wlach/blarg.ocsp");

    x.set_aia(ca_in, ocsp_in);
    WvStringList ca_out;
    x.get_ca_urls(ca_out);
    WvStringList ocsp_out;
    x.get_ocsp(ocsp_out);

    WVPASSEQ(ca_in.popstr(), ca_out.popstr());
    WVPASSEQ(ca_in.popstr(), ca_out.popstr());
    WVPASSEQ(ocsp_in.popstr(), ocsp_out.popstr());
    WVPASSEQ(ocsp_in.popstr(), ocsp_out.popstr());
    WVPASSEQ(ca_in.count(), 0);
    WVPASSEQ(ca_out.count(), 0);
    WVPASSEQ(ocsp_in.count(), 0);
    WVPASSEQ(ocsp_out.count(), 0);
}


WVTEST_MAIN("set_crl_dp")
{
    WvX509Mgr x("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN, false);
    WvStringList dp_in;
    dp_in.append("http://localhost/~wlach/testca.crl");
    dp_in.append("http://localhost/~wlach/testca-alt.crl");
    x.set_crl_urls(dp_in);

    WvStringList dp_out;
    x.get_crl_urls(dp_out);

    WVPASSEQ(dp_in.popstr(), dp_out.popstr());
    WVPASSEQ(dp_in.popstr(), dp_out.popstr());
    WVPASSEQ(dp_in.count(), 0);
    WVPASSEQ(dp_out.count(), 0);
}


WVTEST_MAIN("certreq / signreq / signcert")
{
    // certificate request
    WvRSAKey rsakey(DEFAULT_KEYLEN);
    WvString certreq 
	= WvX509Mgr::certreq("cn=test.signed.com,dc=signed,dc=com", rsakey);
    WvX509Mgr cacert("CN=test.foo.com,DC=foo,DC=com", DEFAULT_KEYLEN, true);
    WvString certpem = cacert.signreq(certreq);
    
    WvX509 cert;
    cert.decode(WvX509Mgr::CertPEM, certpem);

    // test that it initially checks out
    WVPASS(cert.issuedbyca(cacert));
    WVPASS(cert.signedbyca(cacert));
    
    // change some stuff, verify that tests fail
    WvStringList ca_in, ocsp_in;
    ca_in.append("http://localhost/~wlach/testca.pem");
    ca_in.append("http://localhost/~wlach/testca-alt.pem");
    cert.set_aia(ca_in, ocsp_in);
    WVPASS(cert.issuedbyca(cacert));
    WVFAIL(cert.signedbyca(cacert));

    // should pass again after re-signing
    cacert.signcert(cert);    
    WVPASSEQ(cert.get_issuer(), cacert.get_subject());
    WVPASS(cert.issuedbyca(cacert));
    WVPASS(cert.signedbyca(cacert));
}


WVTEST_MAIN("certificate policies")
{
    WvX509 t509;
    t509.decode(WvX509Mgr::CertPEM, carillon_cert);

    WvStringList policies;
    WVPASS(t509.get_policies(policies));
    WVPASSEQ(policies.popstr(), "1.3.6.1.4.1.25054.1.1.101");
    WVPASSEQ(policies.popstr(), "1.3.6.1.4.1.25054.1.1.102");
    WVPASSEQ(policies.count(), 0);

    policies.zap();
    WvX509Mgr cacert("CN=test.foo.com, DC=foo, DC=com", DEFAULT_KEYLEN, true);
    WVFAIL(cacert.get_policies(policies)); 
    WVPASSEQ(policies.count(), 0);
}


WVTEST_MAIN("certificate policies set and get")
{
    WvX509Mgr x("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN, false);
    WvStringList policies_in;
    policies_in.append("1.2.3.4");
    policies_in.append("1.2.3.5");
    x.set_policies(policies_in);

    WvStringList policies_out;
    x.get_policies(policies_out);

    WVPASSEQ(policies_in.popstr(), policies_out.popstr());
    WVPASSEQ(policies_in.popstr(), policies_out.popstr());
    WVPASSEQ(policies_in.count(), 0);
    WVPASSEQ(policies_out.count(), 0);
}


WVTEST_MAIN("basic constraints")
{
    bool is_ca;
    int pathlen;

    // the carillon cert is a user cert, so it should have no path length,
    // and the ca bit of basicConstraints set to false
    WvX509 t509;
    t509.decode(WvX509Mgr::CertPEM, carillon_cert);

    WVPASS(t509.get_basic_constraints(is_ca, pathlen));
    WVFAIL(is_ca);
    WVPASSEQ(pathlen, (-1)); // no path length restriction

    // by default, a CA certificate we create should have the ca bit set to 
    // true, and no path length restriction.
    WvX509Mgr cacert("CN=test.foo.com, DC=foo, DC=com", DEFAULT_KEYLEN, true);
    WVPASS(cacert.get_basic_constraints(is_ca, pathlen));
    WVPASS(is_ca);
    WVPASSEQ(pathlen, (-1)); // no path length restriction

    // now, let's try setting the path length, and see what the result is
    cacert.set_basic_constraints(true, 5);
    WVPASS(cacert.get_basic_constraints(is_ca, pathlen));
    WVPASS(is_ca);
    WVPASSEQ(pathlen, 5); 
    

}


#ifdef HAVE_OPENSSL_POLICY_MAPPING
WVTEST_MAIN("get/set certificate policy extensions")
{
    WvRSAKey rsakey(DEFAULT_KEYLEN);

    WvString certreq 
	= WvX509Mgr::certreq("cn=test.signed.com,dc=signed,dc=com", rsakey);
    WvX509Mgr cacert("CN=test.foo.com, DC=foo, DC=com", DEFAULT_KEYLEN, true);
    WvString certpem = cacert.signreq(certreq);
    
    WvX509Mgr cert;
    cert.decode(WvX509Mgr::CertPEM, certpem);

    int require_explicit_policy_in = 10;
    int inhibit_policy_mapping_in = 5;
    cert.set_policy_constraints(require_explicit_policy_in, 
                         inhibit_policy_mapping_in);

    WvString issuer_domain_in = "2.16.840.1.101.3.2.1.48.1";
    WvString subject_domain_in = "2.16.840.1.101.3.2.1.48.2";
    WvX509Mgr::PolicyMapList list;
    list.append(new WvX509Mgr::PolicyMap(issuer_domain_in, subject_domain_in), 
                true);
    cert.set_policy_mapping(list);
    list.zap();
    cacert.signcert(cert);    

    int require_explicit_policy_out = 0;
    int inhibit_policy_mapping_out = 0;
    cert.get_policy_constraints(require_explicit_policy_out, 
                                inhibit_policy_mapping_out);
    WVPASSEQ(require_explicit_policy_in, require_explicit_policy_out);
    WVPASSEQ(inhibit_policy_mapping_in, inhibit_policy_mapping_out);

    WVPASS(cert.get_policy_mapping(list));
    WVPASSEQ(list.count(), 1);
    if (list.count() > 0)
    {
        WvX509Mgr::PolicyMap *map = list.first();
        WVPASSEQ(map->issuer_domain, issuer_domain_in);
        WVPASSEQ(map->subject_domain, subject_domain_in);
    }
}


WVTEST_MAIN("ski / aki")
{
    WvRSAKey rsakey(DEFAULT_KEYLEN);
    WvString certreq 
	= WvX509Mgr::certreq("cn=test.signed.com,dc=signed,dc=com", rsakey);
    WvX509Mgr cacert("CN=test.foo.com, DC=foo, DC=com", DEFAULT_KEYLEN, true);

    WvString certpem = cacert.signreq(certreq);
    wvcon->print("\n%s\n", certpem);
    WvX509Mgr cert;
    cert.decode(WvX509Mgr::CertPEM, certpem);
    WVPASS(!!cert.get_ski());
    WVPASS(!!cert.get_aki());
    WVPASS(!!cacert.get_ski());
    WVFAIL(!!cacert.get_aki());
    WVPASSEQ(cert.get_aki(), cacert.get_ski());
}
#endif // HAVE_OPENSSL_POLICY_MAPPING

bool test_encode_decode_str(WvX509::DumpMode mode)
{
    WvX509Mgr cert1("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN, false);
    WvString s = cert1.encode(mode);
    WvX509Mgr cert2;
    cert2.decode(mode, s);

    WVPASSEQ(cert1.get_subject(), cert2.get_subject());
    return cert1.get_subject() == cert2.get_subject();
}


bool test_encode_decode_buf(WvX509::DumpMode mode)
{
    WvX509Mgr cert1("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN, false);
    WvDynBuf buf;
    cert1.encode(mode, buf);
    WvX509Mgr cert2;
    cert2.decode(mode, buf);

    WVPASSEQ(cert1.get_subject(), cert2.get_subject());
    return cert1.get_subject() == cert2.get_subject();
}


bool test_encode_load_file(WvX509::DumpMode mode)
{
    WvString tmpfile = wvtmpfilename("cert");

    WvX509Mgr cert1("cn=test.foo.com,dc=foo,dc=com", DEFAULT_KEYLEN, false);
    {
        WvFile f(tmpfile, O_WRONLY);
        WvDynBuf buf;
        cert1.encode(mode, buf);
	fprintf(stderr, "Encode to file '%s' (%ld bytes)\n",
		tmpfile.cstr(), (long)buf.used());
	size_t used = buf.used(), len;
        len = f.write(buf, buf.used());
	WVPASSEQ(len, used);
	fprintf(stderr, "Write result: %s\n", f.errstr().cstr());
    }

    WvX509Mgr cert2;
    if (mode == WvX509::CertPEM)
	cert2.decode(WvX509::CertFilePEM, tmpfile);
    else
        cert2.decode(WvX509::CertFileDER, tmpfile);
    
    unlink(tmpfile);

    WVPASSEQ(cert1.get_subject(), cert2.get_subject());
    return cert1.get_subject() == cert2.get_subject();
}


WVTEST_MAIN("encode / decode / load")
{
    WVPASS(test_encode_decode_str(WvX509::CertPEM));
    WVPASS(test_encode_decode_str(WvX509::CertHex));    

    WVPASS(test_encode_decode_buf(WvX509::CertPEM));
    WVPASS(test_encode_decode_buf(WvX509::CertHex));
    WVPASS(test_encode_decode_buf(WvX509::CertDER));
    
    WVPASS(test_encode_load_file(WvX509::CertPEM));
    WVPASS(test_encode_load_file(WvX509::CertDER));
}


WVTEST_MAIN("pkcs12")
{
    WvX509Mgr cert1("CN=test.foo.com, DC=foo, DC=com", DEFAULT_KEYLEN, true);
    WvString p12fname = wvtmpfilename("p12");
    cert1.write_p12(p12fname, "123");

    WvX509Mgr cert2;
    cert2.read_p12(p12fname, "123");
    WVPASS(cert2.isok());
    WVPASSEQ(cert1.get_ski(), cert2.get_ski());

    WvX509Mgr cert3;
    cert3.read_p12(p12fname, "321");
    WVFAIL(cert3.isok());

    ::unlink(p12fname);
}


const char random_cert[] = "-----BEGIN CERTIFICATE-----\n"
"MIIDvzCCAqegAwIBAgIEShc7yDANBgkqhkiG9w0BAQUFADBWMR0wGwYDVQQDExRj\n"
"b250aW5nZW5jeXdvcmtzLmNvbTEgMB4GCgmSJomT8ixkARkTEGNvbnRpbmdlbmN5\n"
"d29ya3MxEzARBgoJkiaJk/IsZAEZEwNjb20wHhcNMDgwNzMxMjAyMzE4WhcNMTgw\n"
"NzI5MjAyMzE4WjBWMR0wGwYDVQQDExRjb250aW5nZW5jeXdvcmtzLmNvbTEgMB4G\n"
"CgmSJomT8ixkARkTEGNvbnRpbmdlbmN5d29ya3MxEzARBgoJkiaJk/IsZAEZEwNj\n"
"b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCrFneQgdQSoVfzCDNu\n"
"IAXhFpaCJvxDzWPCCZXWPf1/OlncskrRs4FirMnrPUyvw74krPh4lVILKZxIp+Io\n"
"s7DX8oPHct1ZjQoi7eK7r6iva38ghI3YFPtZSXC3z1DllKzUTezBzNJDkhg/SqUX\n"
"ne4NNw1mZ90Q07Y8KgREyjJ8/A0+jUNoQ3TydXPWsdNevP6jcczyULbzhlZrCj5J\n"
"7c21sSWkrm7HUegvJNPPv7vX67onYCMhj0f8Fsiw7TqacmmrJksrW3ifpCf5smIZ\n"
"/W8F9RwSefnVLn1TA2UoNAFTPDxwPVWFuOEkP8ObDnZXzfWc5WHRbXeVLnTAKbab\n"
"T6slAgMBAAGjgZQwgZEwHQYDVR0OBBYEFGryLpBlzbGgYCcpX/MbiAQj8nb8MBEG\n"
"CWCGSAGG+EIBAQQEAwIGQDAjBglghkgBhvhCAQwEFhYUY29udGluZ2VuY3l3b3Jr\n"
"cy5jb20wDgYDVR0PAQH/BAQDAgOoMAkGA1UdEwQCMAAwHQYDVR0lBBYwFAYIKwYB\n"
"BQUHAwEGCCsGAQUFBwMCMA0GCSqGSIb3DQEBBQUAA4IBAQBkWsSLHPmDajwg12AT\n"
"2QW3pxJKVu46nfdgLLTo6bQekAF+kYRERjbC9E/T5Eia5+6yTUbWR8xxDZoftkSK\n"
"/m3h7/Y+mqb13XiOowTkiJWzL5kwdv3Pb4O5X7YYMcNC4Rnax806b5pplXmzDY9Z\n"
"rqq/uP6wsRFiu5S7UcfA+KjoTrjMYsSLERAEus5RaP9J6uDvdBn0PwdfTT5rr2yb\n"
"et8AuJcBLFfFuLXPKfUlbnMe29BUr9s1A01OfHDCmzVN8hAhndBs2xDI1FOy9xDt\n"
"QXj9z/yvsK0AYwxknkHKqRboW//DVk6tcsL9dCiGSlZta5ob5yt6KK+R5X+rtQ0W\n"
"InR4\n"
"-----END CERTIFICATE-----";

WVTEST_MAIN("get_fingerprint")
{
    WvX509 foo;
    foo.decode(WvX509::CertPEM, random_cert);

    WVPASSEQ(foo.get_fingerprint(WvX509::FingerSHA1),
		"CA:49:50:D4:44:51:0B:AD:46:4F:E8:6C:3B:B2:3E:3F:61:31:27:ED");
    WVPASSEQ(foo.get_fingerprint(WvX509::FingerMD5),
		"53:FD:C7:D4:8F:45:AE:1D:90:14:45:B4:0C:1B:02:BD");
}
