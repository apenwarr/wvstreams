/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A fast, easy-to-use, parallelizing, pipelining HTTP/1.1 file retriever.
 * 
 * See wvhttppool.h.
 */
#include <ctype.h>
#include <time.h>
#include "wvhttppool.h"
#include "wvbufstream.h"
#include "wvtcp.h"
#include "wvsslstream.h"
#include "strutils.h"

WvFtpStream::WvFtpStream(const WvIPPortAddr &_remaddr, WvStringParm _username,
                WvStringParm _password)
    : WvUrlStream(_remaddr, _username, WvString("FTP %s", _remaddr))
{
    data = NULL;
    logged_in = false;
    password = _password;
    uses_continue_select = true;
    last_request_time = time(0);
    alarm(60000); // timeout if no connection, or something goes wrong
}


WvFtpStream::~WvFtpStream()
{
    close();
}


void WvFtpStream::doneurl()
{
    log("Done URL: %s\n", curl->url);

    curl->done();
    curl = NULL;
    if (data)
    {
        delete data;
        data = NULL;
    }
    urls.unlink_first();
    last_request_time = time(0);
    alarm(60000);
    request_next();
}


void WvFtpStream::request_next()
{
    // don't do a request if we've done too many already or we have none
    // waiting.
    if (request_count >= max_requests || waiting_urls.isempty())
        return;

    if (!urls.isempty())
        return;

    // okay then, we really do want to send a new request.
    WvUrlRequest *url = waiting_urls.first();

    waiting_urls.unlink_first();

    request_count++;
    log("Request #%s: %s\n", request_count, url->url);
    urls.append(url, false, "request_url");
    alarm(0);
}


void WvFtpStream::close()
{
    if (isok())
        log("Closing.\n");
    WvStreamClone::close();

    if (geterr())
    {
        // if there was an error, count the first URL as done.  This prevents
        // retrying indefinitely.
        if (!curl && !urls.isempty())
            curl = urls.first();
        if (!curl && !waiting_urls.isempty())
            curl = waiting_urls.first();
        if (curl)
            log("URL '%s' is FAILED\n", curl->url);
        if (curl) 
            curl->done();
    }

    if (curl)
        curl->done();
}


char *WvFtpStream::get_important_line(int timeout)
{
    char *line;
    do
    {
        line = getline(timeout);
        if (!line)
            return NULL;
    }
    while (line[3] == '-');
    return line;
}


bool WvFtpStream::pre_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;

    if (WvUrlStream::pre_select(si))
        return true;

    if (data && data->pre_select(si))
        return true;

    if (curl && curl->putstream && curl->putstream->pre_select(si))
        return true;

    si.wants = oldwant;

    return false;
}


bool WvFtpStream::post_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;

    if (WvUrlStream::post_select(si))
        return true;

    if (data && data->post_select(si))
        return true;

    if (curl && curl->putstream && curl->putstream->post_select(si))
        return true;

    si.wants = oldwant;

    return false;
}


void WvFtpStream::execute()
{
    char buf[1024], *line;
    WvStreamClone::execute();

    if (alarm_was_ticking && ((last_request_time + 60) <= time(0)))
    {
        log(WvLog::Debug4, "urls count: %s\n", urls.count());
        if (urls.isempty())
            close(); // timed out, but not really an error

        return;
    }

    if (!logged_in)
    {
        line = get_important_line(60000);
        if (!line)
            return;
        if (strncmp(line, "220", 3))
        {
            log("Server rejected connection: %s\n", line);
            seterr("server rejected connection");
            return;
        }

        log(WvLog::Debug5, "Got greeting: %s\n", line);
        write(WvString("USER %s\r\n",
                    !target.username ? "anonymous" :
                    target.username.cstr()));
        line = get_important_line(60000);
        if (!line)
            return;

        if (!strncmp(line, "230", 3))
        {
            log(WvLog::Debug5, "Server doesn't need password.\n");
            logged_in = true;        // No password needed;
        }
        else if (!strncmp(line, "33", 2))
        {
            write(WvString("PASS %s\r\n", !password ? DEFAULT_ANON_PW :
                        password));
            line = get_important_line(60000);
            if (!line)
                return;

            if (line[0] == '2')
            {
                log(WvLog::Debug5, "Authenticated.\n");
                logged_in = true;
            }
            else
            {
                log("Strange response to PASS command: %s\n", line);
                seterr("strange response to PASS command");
                return;
            }
        }
        else
        {
            log("Strange response to USER command: %s\n", line);
            seterr("strange response to USER command");
            return;
        }

        write("TYPE I\r\n");
        line = get_important_line(60000);
        if (!line)
            return;

        if (strncmp(line, "200", 3))
        {
            log("Strange response to TYPE I command: %s\n", line);
            seterr("strange response to TYPE I command");
            return;
        }
    }

    if (!curl && !urls.isempty())
    {
        curl = urls.first();

        write(WvString("CWD %s\r\n", curl->url.getfile()));
        line = get_important_line(60000);
        if (!line)
            return;

        if (!strncmp(line, "250", 3))
        {
            log(WvLog::Debug5, "This is a directory.\n");
            curl->is_dir = true;
        }

        write("PASV\r\n");
        line = get_important_line(60000);
        if (!line)
            return;

        WvIPPortAddr *dataip = parse_pasv_response(line);

        if (!dataip)
            return;

        log(WvLog::Debug4, "Data port is %s.\n", *dataip);
        // Open data connection.
        data = new WvTCPConn(*dataip);
        if (!data)
        {
            log("Can't open data connection.\n");
            seterr("can't open data connection");
            return;
        }

        if (curl->is_dir)
        {
            if (!curl->putstream)
            {
                write(WvString("LIST %s\r\n", curl->url.getfile()));
                if (curl->outstream)
                {
                    WvString url_no_pw("ftp://%s%s%s%s", curl->url.getuser(),
                            !!curl->url.getuser() ? "@" : "", 
                            curl->url.gethost(),
                            curl->url.getfile());
                    curl->outstream->write(
                            WvString(
                                "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML "
                                "4.01//EN\">\n"
                                "<html>\n<head>\n<title>%s</title>\n"
                                "<meta http-equiv=\"Content-Type\" "
                                "content=\"text/html; "
                                "charset=ISO-8859-1\">\n"
                                "<base href=\"%s\"/>\n</head>\n"
                                "<style type=\"text/css\">\n"
                                "img { border: 0; padding: 0 2px; vertical-align: "
                                "text-bottom; }\n"
                                "td  { font-family: monospace; padding: 2px 3px; "
                                "text-align: right; vertical-align: bottom; }\n"
                                "td:first-child { text-align: left; padding: "
                                "2px 10px 2px 3px; }\n"
                                "table { border: 0; }\n"
                                "a.symlink { font-style: italic; }\n"
                                "</style>\n<body>\n"
                                "<h1>Index of %s</h1>\n"
                                "<hr/><table>\n", url_no_pw, curl->url, url_no_pw 
                                ));
                }
            }
            else
            {
                log("Target is a directory.\n");
                seterr("target is a directory");
                doneurl();
                return;
            }
        }
        else if (!curl->putstream)
            write(WvString("RETR %s\r\n", curl->url.getfile()));
        else
        {
            if (curl->create_dirs)
            {
                write(WvString("CWD %s\r\n", getdirname(curl->url.getfile())));
                line = get_important_line(60000);
                if (!line)
                    return;

                if (strncmp(line, "250", 3))
                {
                    log("Path doesn't exist; creating directories...\n");
                    // create missing directories.
                    WvString current_dir("");
                    WvStringList dirs;
                    dirs.split(getdirname(curl->url.getfile()), "/");
                    WvStringList::Iter i(dirs);
                    for (i.rewind(); i.next(); )
                    {
                        current_dir.append(WvString("/%s", i()));
                        write(WvString("MKD %s\r\n", current_dir));
                        line = get_important_line(60000);
                        if (!line)
                            return;
                    }
                }
            }
            write(WvString("STOR %s\r\n", curl->url.getfile()));
        }

        log(WvLog::Debug5, "Waiting for response to STOR/RETR/LIST\n");
        line = get_important_line(60000);
        log("Response: %s\n", line);
        if (!line)
            doneurl();
        else if (strncmp(line, "150", 3))
        {
            log("Strange response to %s command: %s\n", 
                    curl->putstream ? "STOR" : "RETR", line);
            seterr(WvString("strange response to %s command",
                        curl->putstream ? "STOR" : "RETR"));
            doneurl();
        }

    }

    if (curl)
    {
        if (curl->is_dir)
        {
            line = data->getline(0);
            if (line && curl->outstream)
            {
                WvString output_line(parse_for_links(line));
                if (!!output_line)
                    curl->outstream->write(output_line);
                else
                    curl->outstream->write("Unknown format of LIST "
                            "response\n");
            }
        }
        else
        {
            if (curl->putstream)
            {
                int len = curl->putstream->read(buf, sizeof(buf));
                log(WvLog::Debug5, "Read %s bytes.\n", len);

                if (len)
                    data->write(buf, len);
            }
            else
            {
                int len = data->read(buf, sizeof(buf));
                log(WvLog::Debug5, "Read %s bytes.\n", len);

                if (len && curl->outstream)
                    curl->outstream->write(buf, len);
            }
        }

        if (!data->isok() || (curl->putstream && !curl->putstream->isok()))
        {
            if (curl->putstream && data->isok())
                data->close();
            line = get_important_line(60000);
            if (!line)
            {
                doneurl();
                return;
            }

            if (strncmp(line, "226", 3))
                log("Unexpected message: %s\n", line);

            if (curl->is_dir)
            {
                if (curl->outstream)
                    curl->outstream->write("</table><hr/></body>\n"
                            "</html>\n");
                write("CWD /\r\n");
                log(WvLog::Debug5, "Waiting for response to CWD /\n");
                line = get_important_line(60000);
                if (!line)
                    return;

                if (strncmp(line, "250", 3))
                    log("Strange resonse to \"CWD /\": %s\n", line);
                // Don't bother failing here.
            }
            doneurl();
        }
    }
}


WvString WvFtpStream::parse_for_links(char *line)
{
    WvString output_line("");
    trim_string(line);

    if (curl->is_dir && curl->outstream)
    {
        struct ftpparse fp;
        int res = ftpparse(&fp, line, strlen(line));
        if (res)
        {
            char linkname[fp.namelen+1];
            int i;
            for (i = 0; i < fp.namelen; i++)
            {
                if (fp.name[i] >= 32)
                    linkname[i] = fp.name[i];
                else
                {
                    linkname[i] = '?';
                }
            }
            linkname[i] = 0;

            WvString linkurl(curl->url);
            if (linkurl.cstr()[linkurl.len()-1] != '/')
                linkurl.append("/");
            linkurl.append(linkname);
            WvUrlLink *link = new WvUrlLink(linkname, linkurl);
            curl->outstream->links.append(link, true);

            output_line.append("<tr>\n");

            output_line.append(WvString(" <td>%s%s</td>\n", linkname,
                        fp.flagtrycwd ? "/" : ""));

            if (fp.flagtryretr)
            {
                if (!fp.sizetype)
                    output_line.append(" <td>? bytes</td>\n");
                else
                    output_line.append(WvString(" <td>%s bytes</td>\n",
                                fp.size));
                if (fp.mtimetype > 0)
                    output_line.append(WvString(" <td>%s</td>\n", (fp.mtime)));
                else
                    output_line.append(" <td>?</td>\n");
            }
            else
                output_line.append(" <td></td>\n");

            output_line.append("</tr>\n");
        }
    }
    return output_line;
}


WvIPPortAddr *WvFtpStream::parse_pasv_response(char *line)
{
    if (strncmp(line, "227 ", 4))
    {
        log("Strange response to PASV command: %s\n", line);
        seterr("strange response to PASV command");
        return NULL;
    }

    char *p = &line[3];
    while (!isdigit(*p))
    {
        if (*p == '\0' || *p == '\r' || *p == '\n')
        {
            log("Couldn't parse PASV response: %s\n", line);
            seterr("couldn't parse response to PASV command");
            return NULL;
        }
        p++;
    }
    char *ipstart = p;

    for (int i = 0; i < 4; i++)
    {
        p = strchr(p, ',');
        if (!p)
        {
            log("Couldn't parse PASV IP: %s\n", line);
            seterr("couldn't parse PASV IP");
            return NULL;
        }
        *p = '.';
    }
    *p = '\0';
    WvString pasvip(ipstart);
    p++;
    int pasvport;
    pasvport = atoi(p)*256;
    p = strchr(p, ',');
    if (!p)
    {
        log("Couldn't parse PASV IP port: %s\n", line);
        seterr("couldn't parse PASV IP port");
        return NULL;
    }
    pasvport += atoi(++p);

    WvIPPortAddr *res = new WvIPPortAddr(pasvip.cstr(), pasvport);

    return res;
}
