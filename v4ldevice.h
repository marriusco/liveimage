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
#ifndef V4LDEVICE_H
#define V4LDEVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <libv4l2.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include "os.h"

#define MAX_BUFFERS 4

struct videobuffer {
    void *  start;
    size_t  length;
    int      mmap;
};

class v4ldevice;
class mmotion : public OsThread
{
public:
    mmotion(int w, int h, int nr);
    ~mmotion();
    void thread_main();
    int has_moved(uint8_t* p);
    int  getw()const{return _mw;}
    int  geth()const{return _mh;}
    uint8_t*  motionbuf()const{return _motionbufs[2];}
    uint32_t darkav()const{return _dark;}

private:
    int       _w;
    int       _h;
    int       _mw;
    int       _mh;
    uint8_t*  _motionbufs[3];
    int       _motionindex;
    uint32_t  _motionsz;
    mutex     _m;
    int       _moves;
    uint32_t  _dark;
    int       _nr;
    int       _mmeter;
    size_t    _windtime;
    int       _pxs,_pys,_pxe,_pye;
    int       _rxs,_rys,_rxe,_rye;
    int       _checkcount;
    int       _checkpass;
};


class v4ldevice
{
public:
    v4ldevice(const char* device, int x, int y, int fps, int motionlow, int motionhi, int nr=4);
    virtual ~v4ldevice();

    bool open();
    void close();
    const uint8_t* read(int& w, int& h, size_t& sz); // ret 0 fatal, 1 aquired, -1 continue
    const uint8_t* getm(int& w, int& h, size_t& sz); // ret 0 fatal, 1 aquired, -1 continue
    int _proc_buff(const void* p, struct timeval& t);
    int movement()const{return _moved;}
    uint32_t darkaverage()const{return _pmt ? _pmt->darkav() : 255;}
private:
    int _ioctl(int request, void* argp);

private:
    char      _sdevice[64];
    int       _device;
    int       _xy[2];
    int       _fps;
    int       _curbuffer;
    timeval   _curts;
    uint32_t  _buffsize;
    struct    videobuffer  _buffers[MAX_BUFFERS];
    time_t    _lasttime;
    int       _motionlow;
    int       _motionhi;
    mmotion*  _pmt;
    int       _moved;
    int       _nr;

};

#endif // V4LDEVICE_H
