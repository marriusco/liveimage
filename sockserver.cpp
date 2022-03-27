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
#include <fcntl.h>
#include "sockserver.h"
#include "outfilefmt.h"
#include "liconfig.h"

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
    int ntry = 0;
AGAIN:
    if(__alive==false)
       return false;
    if(_s.create(_port, SO_REUSEADDR, 0)>0)
    {
        fcntl(_s.socket(), F_SETFD, FD_CLOEXEC);
        _s.set_blocking(0);
        if(_s.listen(4)!=0)
        {
            std::cout <<"socket can't listen. Trying "<< (ntry+1) << " out of 10 " << DERR();

            _s.destroy();
            sleep(1);
            if(++ntry<10)
                goto AGAIN;
            return false;
        }
        std::cout << "listening port"<< _port<<"\n";
        return true;
    }
    std::cout <<"create socket. Trying "<< (ntry+1) << " out of 10 " << DERR();
    _s.destroy();
    sleep(2);
    if(++ntry<10)
        goto AGAIN;
    __alive=false;
    sleep(3);
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
    timeval tv {0,10000};

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
        std::cout << "socket select() " << DERR();
        __alive=false;
        return false;
    }
    if(is)
    {

        if(FD_ISSET(_s.socket(), &rd))
        {
            imgclient* cs = new imgclient();
            if(_s.accept(*cs)>0)
            {
                cs->set_blocking(0);
                cs->_needs=0;
                std::cout <<"new connection \n";
                _clis.push_back(cs);
            }
        }

        for(auto& s : _clis)
        {
            if(s->socket()<=0)
                continue;
            if(FD_ISSET(s->socket(), &rd))
            {
                char req[512] = {0};

                int rt = s->receive(req,511);
                if(rt==0)//con closed
                {
				    std::cout << "client closed connection \n";
                    s->destroy();
                    _dirty = true;
                }
                if(rt > 0)
                {
                    if(s->_needs == 0 )
					{
                        if( strstr(req, "/?live"))
		    			{
                            std::cout << "          ?LIVE \n";
                            s->_needs = WANTS_IMAGE;
               			}
                    	else if( strstr(req, "/?motion"))
               			{
                            std::cout << "          ?MOTION \n";
                            s->_needs = WANTS_MOTION;
               			}
                        else if( strstr(req, "/?video"))
               			{
                            std::cout << "          ?VIDEO (work in progress) \n";
                            s->_needs = WANTS_VIDEO_TODO;
               			}
                        else if( strstr(req, "/?html"))
                        {
			    char * host = strstr(req, "Host:");
			    if(host){
				char* eol = strstr(host,"\r\n");
				if(eol){
				    *eol=0;
				    _host = host+6;
	                            std::cout <<"host=["<<_host<<"\r\n";
				}
		            }
                            std::cout <<"[" <<req << "]          ?HTML \n";
                            s->_needs = WANTS_HTML;
                        }
                    }
                    // std::cout << s << "," << s->_needs  << ": " << req << "\n";
                }
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
    int szh;
    int  rv = 0;

    gettimeofday(&timestamp, &tz);
    sprintf(hdr,HDR, ifmt, sz, (int) timestamp.tv_sec, (int) timestamp.tv_usec);
    for(auto& s : _clis)
    {
        szh = strlen(hdr);
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

int  sockserver::anyone_needs()const
{
    int needs = 0;
    for(auto& s : _clis)
        needs |= s->_needs;
    return needs;
}



void sockserver::_clean()
{
    if(_dirty)
    {
AGAIN:
        size_t elems = _clis.size();
        for(std::vector<imgclient*>::iterator s=_clis.begin();s!=_clis.end();++s)
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

void sockserver::_send_page(imgclient* pc)
{
    char  image[128];
    char  html[256];

    int len = ::sprintf(image,
            "<img width='640' src='http://%s/?live' />",_host.c_str());
    len = ::sprintf(html,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-length: %d\r\n"
                    "Content-Type: text/html\r\n\r\n%s",len, image);
    pc->sendall(html, len);
}

bool sockserver::stream_on(const uint8_t* buff, uint32_t sz, const char* ifmt, int wants)
{
    bool rv;
    for(auto& s : _clis)
    {
        switch(s->_needs)
        {
            case WANTS_MOTION:
                if(wants == WANTS_MOTION)
                    rv = this->_stream_image(s, buff, sz, ifmt);
            break;
            case WANTS_IMAGE:
                if(wants == WANTS_IMAGE)
                    rv = this->_stream_image(s, buff, sz, ifmt);
                break;
            case WANTS_VIDEO_TODO:
                if(wants == WANTS_VIDEO_TODO)
                    rv = this->_stream_video(s, buff, sz);
                break;
            case WANTS_HTML:
                _send_page(s);
                break;
            default:
                break;
        }
    }
    if(_dirty)
        _clean();
    return rv;
}


bool sockserver::_stream_image(imgclient* pc, const uint8_t* buff, uint32_t sz, const char* ifmt)
{
    char buffer[256] = {0};
    struct timeval timestamp;
    struct timezone tz = {5,0};
    int rv = 0;

    gettimeofday(&timestamp, &tz);
    if(!pc->_headered)
    {
        sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
        "HTTP/1.0 200 OK\r\n"
        "Connection: close\r\n"
        "Server: v4l2net/1.0\r\n"
        "Cache-Control: no-cache\r\n"
        "Content-Type: multipart/x-mixed-replace;boundary=thesupposeduniqueb\r\n" \
                "\r\n" \
                "--thesupposeduniqueb\r\n");

        rv = pc->sendall(buffer, strlen(buffer),100);
        if(rv==0)
        {
            pc->destroy();
            _dirty=true;
            return false;
        }
        pc->_headered=true;
    }
    sprintf(buffer, "Content-Type: image/%s\r\n" \
        "Content-Length: %d\r\n" \
        "X-Timestamp: %d.%06d\r\n" \
        "\r\n", ifmt, sz, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
    rv = pc->sendall(buffer,strlen(buffer),100);
    if(rv==0)
    {
        pc->destroy();
        _dirty=true;
        return false;
    }
    rv = pc->sendall(buff,sz,1000);
    if(rv==0)
    {
        pc->destroy();
        _dirty=true;
        return false;
    }
    ::sprintf(buffer, "\r\n--thesupposeduniqueb\r\nContent-type: image/%s\r\n", ifmt);
    rv = pc->sendall(buffer,strlen(buffer),100);
    if(rv==0)
    {
        pc->destroy();
        _dirty=true;
    }
    return true;
}

bool sockserver::_stream_video(imgclient* pc, const uint8_t* buff, uint32_t sz)
{
    int rv = pc->sendall(buff,sz,1000);
    if(rv==0)
    {
        pc->destroy();
        _dirty=true;
        return false;
    }
    return true;
}

