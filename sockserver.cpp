/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/

    Author:  Marius O. Chincisan
    First Release: September 16 - 29 2016
*/

#include <iostream>
#include "sockserver.h"

extern bool __alive;


sockserver::sockserver(int port, const string& proto):_port(port),_proto(proto),_headered(false)
{
    //ctor
}

sockserver::~sockserver()
{
    //dtor_headered
}

bool sockserver::listen()
{
    if(_s.create(_port, SO_REUSEADDR, 0)>0)
    {
//        fcntl(_s.socket(), F_SETFD, FD_CLOEXEC);
        _s.set_blocking(0);
        if(_s.listen(4)!=0)
        {
            std::cout <<"socket canntlisten. wait abit and run again\n";
            __alive=false;
            _s.destroy();
            return false;
        }
        return true;
    }
    std::cout <<"socket canntlisten. wait abit and run again\n";
    __alive=false;
    _s.destroy();
    return false;
}

void sockserver::close()
{
    _s.destroy();
    for(auto& s : _clis)
    {
        delete s;
    }
}

bool sockserver::spin()
{
    fd_set  rd;
    int     ndfs = _s.socket();// _s.sock()+1;
    timeval tv {0,1000};


    FD_ZERO(&rd);
    FD_SET(_s.socket(), &rd);
    for(auto& s : _clis)
    {
        if(s->socket()>0)
        {
            FD_SET(s->socket(), &rd);
            ndfs = std::max(ndfs, s->socket());
        }
        else
            _dirty = true;
    }
    int is = ::select(ndfs+1, &rd, 0, 0, &tv);
    if(is ==-1) {
        __alive=false;
        return false;
    }
    if(is)
    {

        if(FD_ISSET(_s.socket(), &rd))
        {
            tcp_cli_sock* cs = new tcp_cli_sock();
            if(_s.accept(*cs)>0)
            {
                _clis.push_back(cs);
            }
        }

        for(auto& s : _clis)
        {
            if(s->socket()<=0)
                continue;
            if(FD_ISSET(s->socket(), &rd))
            {
                char req[256] = {0};

                int rt = s->receive(req,255);
                if(rt==0)//con closed
                {
                    s->destroy();
                    _dirty = true;
                }
                if(rt>0)
                    std::cout << req << "\n";
            }
        }
    }
    _clean();

    return true;
}

bool sockserver::has_clients()
{
    return _clis.size() > 0;
}

bool sockserver::snap_on( const uint8_t* jpg, uint32_t sz, const char* ifmt)
{

    static char HDR[] = "HTTP/1.0 200 OK\r\n"
                        "Connection: close\r\n"
                        "Server: v4l2net/1.0\r\n"
                        "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n"
                        "Pragma: no-cache\r\n"
                        "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n"
                        "Content-type: image/%s\r\n"
                        "Content-length: %d\r\n"
                        "X-Timestamp: %d.%06d\r\n\r\n";
    struct  timeval timestamp;
    char    hdr[512];
    struct timezone tz = {5,0};
    int szh = strlen(hdr);
    int  rv = 0;

    gettimeofday(&timestamp, &tz);
    sprintf(hdr,HDR, ifmt, sz, (int) timestamp.tv_sec, (int) timestamp.tv_usec);


    for(auto& s : _clis)
    {
        rv = s->sendall(hdr,szh,100);
        rv = s->sendall(jpg,sz,1000);

        if(rv == 0)
        {
            s->destroy();
            _dirty=true;
        }
    }
    _clean();
    return rv==(int)sz;
}

void sockserver::_clean()
{
    if(_dirty)
    {
AGAIN:

        for(std::vector<tcp_cli_sock*>::iterator s=_clis.begin();s!=_clis.end();++s)
        {
            if((*s)->socket()<=0)
            {
                delete (*s);
                _clis.erase(s);
                std::cout << " client gone \n";
                goto AGAIN;
            }
        }
    }
    _dirty=false;

}


bool sockserver::stream_on( const uint8_t* jpg, uint32_t sz, const char* ifmt)
{
    char buffer[512] = {0};
    struct timeval timestamp;
    struct timezone tz = {5,0};
    int rv = 0;

    gettimeofday(&timestamp, &tz);

    for(auto& s : _clis)
    {
        if(!_headered)
        {
            sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
            "HTTP/1.0 200 OK\r\n"
            "Connection: close\r\n"
            "Server: v4l2net/1.0\r\n"
            "Cache-Control: no-cache\r\n"
            "Content-Type: multipart/x-mixed-replace;boundary=thesupposeduniqueb\r\n" \
                    "\r\n" \
                    "--thesupposeduniqueb\r\n");

            rv = s->sendall(buffer,strlen(buffer),100);
            _headered=true;
            usleep(10000);
        }

        sprintf(buffer, "Content-Type: image/%s\r\n" \
            "Content-Length: %d\r\n" \
            "X-Timestamp: %d.%06d\r\n" \
            "\r\n", ifmt, sz, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
        rv = s->sendall(buffer,strlen(buffer),100);
        usleep(100);
        printf("< %s", buffer);
        printf("< %d bytes", sz);
        rv = s->sendall(jpg,sz,1000);
        usleep(100);
        sprintf(buffer, "\r\n--thesupposeduniqueb\r\nContent-type: image/%s\r\n", ifmt);
        rv = s->sendall(buffer,strlen(buffer),100);
        if(rv ==0)
        {
            s->destroy();
            _dirty=true;
            std::cout << "socket closed during sent \n";
        }
    }
    return rv>0;
}

