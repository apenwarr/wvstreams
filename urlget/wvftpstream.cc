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
    : WvUrlStream(_remaddr, _username, WvString("FTP %s", _remaddr)),
      cont(WvContCallback(this, &WvFtpStream::real_execute))
{
    data = NULL;
    logged_in = false;
    password = _password;
    last_request_time = time(0);
    alarm(60000); // timeout if no connection, or something goes wrong
}


void WvFtpStream::doneurl()
{
    log("Done URL: %s\n", curl->url);

    curl->done();
    curl = NULL;
    RELEASE(data);
    urls.unlink_first();
    last_request_time = time(0);
    alarm(60000);
    request_next();
    // We just processed the last url in the queue,
    // so go away.
    if (urls.isempty() && waiting_urls.isempty())
	close();
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


char *WvFtpStream::get_important_line()
{
    char *line;
    do
    {
        line = getline(-1);
        if (!line)
            return NULL;
    }
    while (line[3] == '-');
    log(WvLog::Debug5, ">> %s\n", line);
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


void *WvFtpStream::real_execute(void*)
{
    WvString line;
    WvStreamClone::execute();

    if (alarm_was_ticking && ((last_request_time + 60) <= time(0)))
    {
        log(WvLog::Debug4, "urls count: %s\n", urls.count());
        if (urls.isempty())
            close(); // timed out, but not really an error

        return 0;
    }

    if (!logged_in)
    {
        line = get_important_line();
        if (!line)
	{
	    seterr("Server not reachable: %s\n",strerror(errno));
            return 0;
	}
	    
        if (strncmp(line, "220", 3))
        {
            log("Server rejected connection: %s\n", line);
            seterr("server rejected connection");
            return 0;
        }
        print("USER %s\r\n", !target.username ? WvString("anonymous") :
                    target.username);
        line = get_important_line();
        if (!line)
            return 0;

        if (!strncmp(line, "230", 3))
        {
            log(WvLog::Info, "Server doesn't need password.\n");
            logged_in = true;        // No password needed;
        }
        else if (!strncmp(line, "33", 2))
        {
            print("PASS %s\r\n", !password ? DEFAULT_ANON_PW : password);
	    
            line = get_important_line();
            if (!line)
                return 0;

            if (line[0] == '2')
            {
                log(WvLog::Info, "Authenticated.\n");
                logged_in = true;
            }
            else
            {
                log("Strange response to PASS command: %s\n", line);
                seterr("strange response to PASS command");
                return 0;
            }
        }
        else
        {
            log("Strange response to USER command: %s\n", line);
            seterr("strange response to USER command");
            return 0;
        }

        print("TYPE I\r\n");
	log(WvLog::Debug5, "<< TYPE I\n");
        line = get_important_line();
        if (!line)
            return 0;
	
        if (strncmp(line, "200", 3))
        {
            log("Strange response to TYPE I command: %s\n", line);
            seterr("strange response to TYPE I command");
            return 0;
        }
    }

    if (!curl && !urls.isempty())
    {
        curl = urls.first();

        print("CWD %s\r\n", curl->url.getfile());
        line = get_important_line();
        if (!line)
            return 0;

        if (!strncmp(line, "250", 3))
        {
            log(WvLog::Debug5, "This is a directory.\n");
            curl->is_dir = true;
        }

        print("PASV\r\n");
        line = get_important_line();
        if (!line)
            return 0;
        WvIPPortAddr *dataip = parse_pasv_response(line.edit());

        if (!dataip)
            return 0;

        log(WvLog::Debug4, "Data port is %s.\n", *dataip);
        // Open data connection.
        data = new WvTCPConn(*dataip);
        if (!data)
        {
            log("Can't open data connection.\n");
            seterr("can't open data connection");
            return 0;
        }

        if (curl->is_dir)
        {
            if (!curl->putstream)
            {
                print("LIST %s\r\n", curl->url.getfile());
                if (curl->outstream)
                {
                    WvString url_no_pw("ftp://%s%s%s%s", curl->url.getuser(),
                            !!curl->url.getuser() ? "@" : "", 
                            curl->url.gethost(),
                            curl->url.getfile());
                    curl->outstream->print("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML "
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
					   );
                }
            }
            else
            {
                log("Target is a directory.\n");
                seterr("target is a directory");
                doneurl();
                return 0;
            }
        }
        else if (!curl->putstream)
            print("RETR %s\r\n", curl->url.getfile());
        else
        {
            if (curl->create_dirs)
            {
                print("CWD %s\r\n", getdirname(curl->url.getfile()));
                line = get_important_line();
                if (!line)
                    return 0;
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
                        print("MKD %s\r\n", current_dir);
                        line = get_important_line();
                        if (!line)
                            return 0;
                    }
                }
            }
            print("STOR %s\r\n", curl->url.getfile());
        }

        log(WvLog::Debug5, "Waiting for response to %s\n", curl->putstream ? "STOR" : 
	    curl->is_dir ? "LIST" : "RETR");
        line = get_important_line();

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
            line = data->getline(-1);
            if (line && curl->outstream)
            {
                WvString output_line(parse_for_links(line.edit()));
                if (!!output_line)
                    curl->outstream->write(output_line);
                else
                    curl->outstream->write("Unknown format of LIST "
                            "response\n");
            }
        }
        else
        {
	    char buf[1024];
	    
            if (curl->putstream)
            {
                while (curl->putstream->isreadable())
		{
		    int len = curl->putstream->read(buf, sizeof(buf));
		    log(WvLog::Debug5, "Read %s bytes.\n%s\n", len, hexdump_buffer(buf, len));

		    if (len)
		    {
			int wrote = data->write(buf, len);
			log(WvLog::Debug5,"Wrote %s bytes\n", wrote);
			data->flush(0);
		    }
		}
		curl->putstream->close();
            }
            else
            {
		while (data->isreadable() && curl->outstream->isok())
		{
		    int len = data->read(buf, sizeof(buf));
		    log(WvLog::Debug5, "Read %s bytes from remote.\n", len);
		    
		    if (len && curl->outstream)
		    {
			int wrote = curl->outstream->write(buf, len);
			log(WvLog::Debug5, "Wrote %s bytes to local.\n", wrote);
		    }
		}
            }
        }

        if (!data->isok() || (curl->putstream && !curl->putstream->isok()))
        {
	    log("OK, we should have finished writing!\n");
            if (curl->putstream && data->isok())
                data->close();
            line = get_important_line();
            if (!line)
            {
                doneurl();
                return 0;
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
                line = get_important_line();
                if (!line)
                    return 0;

                if (strncmp(line, "250", 3))
                    log("Strange resonse to \"CWD /\": %s\n", line);
                // Don't bother failing here.
            }
            doneurl();
        }
	else
	{
	    log("Why are we here??\n");
	}
    }

    return 0;
}


void WvFtpStream::execute()
{
    real_execute(0);
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
            char *linkname = (char *)alloca(fp.namelen+1);
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
