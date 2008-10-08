#include "wvargs.h"
#include "wvbuf.h"
#include "wvfile.h"
#include "wvhttppool.h"
#include "wvocsp.h"


static int got_response = 0;


void load_cert(WvStringParm fname, WvX509 &cert)
{
    WvFile f(fname, O_RDONLY);
    WvDynBuf buf;
    while (f.isok())
        f.read(buf, 100);

    WvX509::DumpMode dumpmode = WvX509::CertDER;

    if (!strncmp("-----BEGIN", (const char *) buf.peek(0, 10), 10))
        dumpmode = WvX509::CertPEM;

    cert.decode(dumpmode, buf);    

    wvcon->print("Loaded certificate with name %s\n", cert.get_subject());
}


static void response_cb(WvStream &s, WvDynBuf &respbuf)
{
    char buf[1024];
    size_t numread = 0;
    size_t totalread = 0;
    while (s.isreadable() && totalread < 32768)
    {
        numread = s.read(buf, 1024);
        if (numread)
            respbuf.put(buf, numread);
        totalread += numread;
    }    
}


static void response_closed_cb()
{
    wvcon->print("Response closed!\n");
    // we just assume that we were successful
    got_response = 1;
}


int main(int argc, char *argv[])
{
    WvArgs args;
    args.add_required_arg("CLIENT_CERTIFICATE", false);
    args.add_required_arg("SIGNING_CERTIFICATE", false);
    args.add_required_arg("URL", false);

    WvStringList remaining_args;
    if (!args.process(argc, argv, &remaining_args))
        return 1;

    WvString clicertfname = remaining_args.popstr();
    WvString issuercertfname = remaining_args.popstr();
    WvString url = remaining_args.popstr();

    WvX509 clicert;
    WvX509 issuer;    
    WvX509 ocspserver;
    
    load_cert(clicertfname, clicert);
    load_cert(issuercertfname, issuer);

    wvcon->print("Sending request...\n");
    WvOCSPReq req(clicert, issuer);
    WvDynBuf reqbuf;
    req.encode(reqbuf);

    WvBufStream input_stream;
    input_stream.write(reqbuf, reqbuf.used());

    WvHttpStream::global_enable_pipelining = false;
    WvHttpPool pool;
    WvIStreamList::globallist.append(&pool, false, "http pool");

    WvStream * response_stream =  pool.addurl(
        url, "POST",
        "Content-Type: application/ocsp-request\r\n",
        &input_stream);

    WvDynBuf respbuf;
    
    response_stream->setcallback(wv::bind(&response_cb, 
                                          wv::ref(*response_stream),
                                          wv::ref(respbuf)));
    response_stream->setclosecallback(&response_closed_cb);

    WvIStreamList::globallist.append(response_stream, false, "response stream");
    wvcon->print("Beginning downloading of response...\n");
    while (!got_response)
        WvIStreamList::globallist.runonce();

    wvcon->print("Got response (length: %s), attempting to decode...\n", 
                 respbuf.used());
    
    WvOCSPResp resp;
    resp.decode(respbuf);
    if (!resp.isok())
    {
        wvcon->print("Response not ok!\n");
        exit(1);
    }
    
    if (!resp.check_nonce(req))
    {
        wvcon->print("Response nonce does not check out!\n");
        exit(1);
    }
    
    WvOCSPResp::Status status = resp.get_status(clicert, issuer);
    WvString status_str;
    switch(status)
        {
        case WvOCSPResp::ERROR:
            status_str = "ERROR";
            break;
        case WvOCSPResp::GOOD:
            status_str = "GOOD";
            break;
        case WvOCSPResp::REVOKED:
            status_str = "REVOKED";
            break;
        case WvOCSPResp::UNKNOWN:
            status_str = "UNKNOWN";
            break;
        }

    wvcon->print("Response status: %s\n", status_str);
}
