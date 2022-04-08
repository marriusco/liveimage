/**

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
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "v4ldevice.h"
#include "liconfig.h"

#define VIDEO_BUFFS 2
#define MOTION_SZ   64

v4ldevice::v4ldevice(const char* device, int x, int y, int fps, int motionlow, int motionhi, int nr)
{
    _sdevice = device;
    _curbuffer = 0;
    _xy[0]=x;
    _xy[1]=y;
    _fps = fps;
    _lasttime = time(0);
    _motionlow = motionlow;
    if(_motionlow==0)_motionlow=1;
    _motionhi = motionhi;
    if(_motionhi==0)_motionhi=8000;
    _pmt = 0;
    _moved = 0;
    _nr = nr;
    _fatal = false;
    for (int i = 0; i < MAX_BUFFERS; ++i)
        _buffers[i].start=0;
}

v4ldevice::~v4ldevice()
{
    close();
}

bool v4ldevice::open()
{
    size_t diez = _sdevice.find('*');
    if(diez != std::string::npos)
    {
        std::string sdev = _sdevice.substr(0,diez);
        for(int i=0; i < 8; i++)
        {
            std::string check = sdev + std::to_string(i);
            std::cout << "opening: " << check << "\n";
            if (::access(check.c_str(),0)!=0)
            {
                std::cout << "No Device:  "<< check << ", " <<  strerror(errno)  << "\n";
                continue;
            }
            _device = v4l2_open(check.c_str(), O_RDWR | O_NONBLOCK, 0);
            if (-1 == _device)
            {
                std::cout << "Cannot open " << check << ", " <<  strerror(errno)  << "\n";
                continue;
            }
            _sdevice=check;
            break;
        }
    }
    else
    {
        if (::access(_sdevice.c_str(),0)!=0)
        {
            std::cout << "No Device: "<< _sdevice << " " <<  errno  << "\n";
            return false;
        }
        _device = v4l2_open(_sdevice.c_str(), O_RDWR | O_NONBLOCK, 0);
    }

    if (-1 == _device)
    {
        std::cout << "Cannot open " << _sdevice << " " <<  errno  << "\n";
        return false;
    }

    struct v4l2_capability  caps;// = {0};
    struct v4l2_format      frmt;// = {0};
    struct v4l2_crop        crop;// = {0};
    struct v4l2_cropcap     cropcap;// = {0};
    struct v4l2_fmtdesc     fmtdesc;


    if (-1 == _ioctl(VIDIOC_QUERYCAP, &caps))
    {
        std::cout << "_ioctl" << _sdevice << " " <<  errno  << "\n";
        return false;
    }

    if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        std::cout << "no video capture device" << _sdevice << " " <<  errno  << "\n";
        return false;
    }

    if (!(caps.capabilities & V4L2_CAP_STREAMING))
    {
        std::cout << "no video streaming device" << _sdevice << " " <<  errno  << "\n";
        return false;
    }
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == _ioctl(VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        _ioctl(VIDIOC_S_CROP, &crop);
    }

    ::memset(&fmtdesc,0,sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (_ioctl(VIDIOC_ENUM_FMT,&fmtdesc) == 0)
    {
        ::printf("%s\n", fmtdesc.description);
        fmtdesc.index++;
    }

    frmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    frmt.fmt.pix.width = _xy[0];
    frmt.fmt.pix.height = _xy[1];
    frmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    frmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
    if (-1 == _ioctl(VIDIOC_S_FMT, &frmt))
    {
        std::cout << "Unsupported format WxH" << _sdevice << " " <<  errno  << "\n";
        return false;
    }
    // realign
    _xy[0] = frmt.fmt.pix.width;
    _xy[1] = frmt.fmt.pix.height;
    if (frmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUV420)
    {
        std::cout << "libv4l cannot process YUV420 format. \n";
        return false;
    }

    if (_fps>=0)
    {
        struct v4l2_streamparm  fint;// = {0};

        fint.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fint.parm.capture.timeperframe.numerator = 1;
        fint.parm.capture.timeperframe.denominator = _fps;

        if (-1 == _ioctl(VIDIOC_S_PARM, &fint))
        {
            std::cout << "error set frame interval " << _fps << "\n";
        }
        _fps = fint.parm.capture.timeperframe.denominator;
        std::cout << "FPS: recalculated duet camera limitations at: " << _fps << "\n";
    }
    uint32_t wmin = frmt.fmt.pix.width * 2;
    if (frmt.fmt.pix.bytesperline < wmin)
        frmt.fmt.pix.bytesperline = wmin;
    wmin = frmt.fmt.pix.bytesperline * frmt.fmt.pix.height;
    if (frmt.fmt.pix.sizeimage < wmin)
        frmt.fmt.pix.sizeimage = wmin;


    uint32_t page_size  = getpagesize();
    uint32_t buffer_size = (frmt.fmt.pix.sizeimage + page_size - 1) & ~(page_size - 1);
    struct v4l2_requestbuffers req; // = {0};
    enum v4l2_buf_type type;

    _buffsize = buffer_size;
    //try mmap first
    req.count = VIDEO_BUFFS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == _ioctl(VIDIOC_REQBUFS, &req))
    {
        std::cout << "libv4l does not support VIDIOC_REQBUFS(V4L2_MEMORY_MMAP) . \n";

        req.count = VIDEO_BUFFS;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == _ioctl(VIDIOC_REQBUFS, &req))
        {
            std::cout << "libv4l does not support VIDIOC_REQBUFS(V4L2_MEMORY_USERPTR) . \n";
            return false;
        }


        for (int i = 0; i < MAX_BUFFERS; ++i)
        {
            _buffers[i].length = buffer_size;

            _buffers[i].start = ::memalign(page_size, buffer_size);
            _buffers[i].mmap = V4L2_MEMORY_USERPTR;
            if (_buffers[i].start==0)
            {
                std::cout << "out of memory: " << buffer_size << " bytes \n";
                return false;
            }

            struct v4l2_buffer buf;// = {0};
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long) _buffers[i].start;
            buf.length = _buffers[i].length;

            if (-1 == _ioctl(VIDIOC_QBUF, &buf))
            {
                std::cout << "VIDIOC_QBUF: " << buffer_size << " bytes \n";
                return false;
            }
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == _ioctl(VIDIOC_STREAMON, &type))
            return false;
        _curbuffer=0;
        return true;
    }
    // continue with mmap
    if (req.count < 2)
    {
        std::cout<< "buffer memory \n";
        return false;
    }

    for (uint32_t i = 0; i < req.count; ++i)
    {
        struct v4l2_buffer buf;// = {0};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == _ioctl(VIDIOC_QUERYBUF, &buf))
        {
            std::cout<< "VIDIOC_QUERYBUF memory \n";
            return false;
        }
        _buffers[i].length = buf.length;
        _buffsize = buf.length;
        _buffers[i].start = v4l2_mmap(NULL, buf.length,
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      _device,
                                      buf.m.offset);
        _buffers[i].mmap = V4L2_MEMORY_MMAP;

        if (MAP_FAILED == _buffers[i].start)
        {
            std::cout<< "MAP_FAILED memory \n";
            return false;
        }

        ::memset(&buf,0,sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == _ioctl(VIDIOC_QBUF, &buf))
            return false;
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == _ioctl(VIDIOC_STREAMON, &type))
        return false;

    if(_motionhi)
    {
        _pmt = new mmotion(_xy[0], _xy[1], _nr);
        if(_pmt==0)
        {
            std::cout << "motion out memeory. motion is off \n";
            _motionhi = 0;
        }
    }
    _curbuffer=0;
    return true;
}

void v4ldevice::close()
{
    if(_pmt)
    {
        delete[] _pmt;
        _pmt = 0;
    }
    if(_device>0)
    {
        for (int i = 0; i < MAX_BUFFERS; ++i)
        {
            if(_buffers[i].start)
            {
                if(_buffers[i].mmap==false)
                    ::free(_buffers[0].start);
                else
                    ::v4l2_munmap(_buffers[i].start, _buffers[i].length);
                _buffers[i].start=0;
            }
        }

        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        _ioctl(VIDIOC_STREAMOFF, &type);
        if(!_fatal)
            ::v4l2_close(_device);
    }
    _device = 0;
}

int v4ldevice::_ioctl(int request, void* argp)
{
    int r = ::v4l2_ioctl(_device, request, argp);
    while (-1 == r && EINTR == errno)
    {
        ::usleep(1000);
    }
    return r;
}

const uint8_t* v4ldevice::read(int& w, int& h, size_t& sz, bool& fatal)
{
    fd_set fds;
    struct timeval tv;
    w = _xy[0];
    h = _xy[1];
    sz = 0;

    FD_ZERO(&fds);
    FD_SET(_device, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    int r = select(_device + 1, &fds, NULL, NULL, &tv);
    if(r==-1)
    {
        fatal=true;
        _fatal=true;
        return 0; // fatal
    }
    if(r == 0 || !FD_ISSET(_device, &fds))
    {
        return 0;
    }

    struct v4l2_buffer buf; // = {0};
    buf.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = _buffers[_curbuffer].mmap;
    if(-1==_ioctl(VIDIOC_DQBUF, &buf))
    {
        if(errno==EAGAIN){
            return 0;
        }
        if(errno!=0 && errno != EIO)
        {
            fatal=true;
            _fatal=true;
            return 0;
        }
    }
    if(_buffers[_curbuffer].mmap == V4L2_MEMORY_USERPTR)
    {
        for (int i = 0; i < VIDEO_BUFFS; ++i)
        {
            if (buf.m.userptr == (unsigned long)_buffers[i].start &&
                    buf.length == _buffers[i].length)
            {
                break;
            }
        }
    }

    _curbuffer = buf.index;
    _curts  = buf.timestamp;
    sz = _buffers[_curbuffer].length;

    if (-1 == _ioctl(VIDIOC_QBUF, &buf))
    {
        sz = 0;
        return 0;
    }

    if(_motionhi )//&& cur -_lasttime > 1)
    {
        _moved = _pmt->has_moved((uint8_t*)_buffers[_curbuffer].start);
    }
    _lasttime=time(0);

    return (const uint8_t*)_buffers[_curbuffer].start;
}

const uint8_t* v4ldevice::getm(int& w, int& h, size_t& sz)
{
    if(_motionhi)
    {
        w  = _pmt->getw();
        h  = _pmt->geth();
        sz = w * h;
        return _pmt->motionbuf();
    }
    return 0;
}

