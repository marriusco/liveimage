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
#ifndef SOCKSERVER_H
#define SOCKSERVER_H

#include "sock.h"
#include <string>
#include <vector>


#define     WANTS_IMAGE     0x1
#define     WANTS_VIDEO_TODO     0x2
#define     WANTS_MOTION    0x4
#define     WANTS_MAX       0x8
#define     WANTS_HTML      0x10

class imgclient : public tcp_cli_sock
{
public:

    imgclient():_needs(0),_headered(false){}
    ~imgclient(){}
    int _needs;
    bool _headered;
    std::string  _message;
};


class sockserver
{
public:
    sockserver(int port, const string& proto);
    virtual ~sockserver();

    bool listen();
    void close();
    bool spin();
    int  socket() {return _s.socket();}
    bool has_clients();
    bool snap_on(  const uint8_t* jpg, uint32_t sz, const char* ifmt);
    bool stream_on(const uint8_t* buff, uint32_t sz, const char* ifmt, int wants);
    int  anyone_needs()const;

private:
    void _clean();
    bool _stream_image(imgclient* pc, const uint8_t* buff, uint32_t sz, const char* ifmt);
    bool _stream_video(imgclient* pc, const uint8_t* buff, uint32_t sz);
    void _send_page(imgclient* pc);

    tcp_srv_sock _s;
    tcp_srv_sock _h;
    int      _port;
    string   _proto;
    std::vector<imgclient*> _clis;
    bool _dirty;
    bool _headered;
};

#endif // SOCKSERVER_H
