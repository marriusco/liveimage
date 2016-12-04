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


#define LIVEIMAGE_VERSION "1.0.0"

#include <stdint.h>
#include <unistd.h>
#define  __USE_FILE_OFFSET64
#include <stdlib.h>
#include <sys/statvfs.h>
#include <iostream>
#include <string>
#include "v4ldevice.h"
#include "sockserver.h"
#include "jpeger.h"
#include "pnger.h"

/*
sudo apt-get install libpng-dev libv4l-dev libjpeg-dev
sudo apt-get install libv4l-dev
sudo apt-get install libjpeg-dev
*/


using namespace std;
static int _usage();
bool __alive = true;
bool __capture=false;

static struct {
    uint64_t sz;
    int x;
    int y;

}  _rez[]={
            {  403270  ,1024, 768},
            {  504435  ,1152, 864},
            {  614806  ,1280, 960},
            {  865866  ,1400, 1050},
            {  1082915 ,1600, 1200},
            {  1501869 ,1920, 1440},
            {  1684088 ,2048, 1536},
            {  2519088 ,2592, 1944},
            {  174204  ,640,  480},
            {  237247  ,768,  576},
            {  256298  ,800,  600},
        };



void ControlC (int i)
{
    __alive = false;
    printf("Exiting...\n");
}


void ControlP (int i)
{
}

void Capture (int i)
{
    __capture=true;
}

uint64_t _imagesz(int x, int y)
{
    for(int k=0;k<sizeof(_rez)/sizeof(_rez[0]);k++)
    {
        if(_rez[k].x==x && _rez[k].y==y)
        {
            return _rez[k].sz;
        }
    }
    return 1082915;
}

static double gtc(void)
{
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now))
    return 0;
  return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}


int main(int nargs, char* vargs[])
{
    std::cout << vargs[0] << "\n";
    if(nargs==1 )
    {
        return _usage();
    }

    signal(SIGINT,  ControlC);
    signal(SIGABRT, ControlC);
    signal(SIGKILL, ControlC);
    signal(SIGUSR2, Capture);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);


    std::string  device= "/dev/video0";
    std::string protocol="http";
    std::string format="jpg";
    int         firstf=0,lasstf=0;
    int         oneshot=0;
    int         sigcapt=0;
    int         motiona=0;
    int         motionb=0;
    int         port=0;
    int         quality=90;
    int         width=640;
    int         height=480;
    int         fps = 15;
    string      filename="";
    int         nsignal = 0;
    int         frameperiod = 0;
    for(int k=0; k<nargs; ++k)
    {
        if(vargs[k][0]=='-')
        {
            int pk=k++;
            if(pk == nargs)
                return _usage();
            switch(vargs[pk][1])
            {
            case 'd':
                device = vargs[k];
                break;
            case 'g':
                nsignal = ::atoi(vargs[k]);
                std::cout << "will signal process: " << nsignal << "\n";
                break;
            case 's':
                port = ::atoi(vargs[k]);
                break;
            case 'o':
            {
                filename = vargs[k];
                size_t fd = filename.find('.');
                if(fd != string::npos)
                    filename=filename.substr(0,fd);

            }
            break;
            case 'r':
                oneshot=::atoi(vargs[k]);
                break;
            case 'q':
                quality = ::atoi(vargs[k]);
                break;
            case 'm':
                {
                    int two = sscanf(vargs[k],"%d,%d", &motiona, &motionb);
                    if(two != 2 || motiona>=motionb)
                        return _usage();
                }
                break;
            case 'z':
            {
                int two = sscanf(vargs[k],"%dx%d", &width, &height);
                if(two != 2)
                    return _usage();
            }
            break;
            case 't':  ///  time period between captures in ms
                frameperiod = ::atoi(vargs[k]);
                break;
            case 'T':  ///  time period between captures in s
                frameperiod = ::atoi(vargs[k]) * 1000;
                break;
            case 'i':
                format = vargs[k];
                break;
            case 'c':
                sigcapt = 1; // ::atoi(vargs[k]);
                break;
            case 'v':
                std::cout << LIVEIMAGE_VERSION << "\n";
                return 0;
            default:
                return _usage();
            }
        }
    }
    outfilefmt* ffmt = 0;
    int         w,h;
    size_t      sz ;

    float ffps = 1000.0f / (float)frameperiod;
    if(ffps < 1.0)fps=1;
    else fps = int(ffps);

    if(format.find("jpg")!=string::npos)
    {
        ffmt = new jpeger(quality);
        if(!filename.empty())
            filename += ".jpg";
    }
    if(format.find("png")!=string::npos)
    {
        ffmt = new pnger(quality);
        if(!filename.empty())
            filename += ".png";
    }

    v4ldevice   dev(device.c_str(), width, height, fps, motiona, motionb);
    if(dev.open())
    {
        std::cout << device << " opened\n";
        sockserver* ps = 0;
        if(port)
        {
            ps = new sockserver(port, protocol);
            if(ps->listen()==false)
            {
                delete ps;
                return 0;
            }
        }
        time_t          lastsave = 0;
        int             delay   = 1000/fps;
        uint32_t        ct = delay-1;
        uint32_t        snap=0;
        uint8_t*        pjpg;
        const uint8_t*  video420;
        bool            capture=false;
        bool            shotsignal=false;
		char 			info[64];
		uint32_t        maximages=0;
		uint32_t        firstimage=0;
		time_t          tnow = time(0);
		int             periodexpired=0;

		lastsave = tnow;
		if(filename.find("%") != string::npos) /*saving sequencially*/
		{
            std::string path;
            size_t ls = filename.find_last_of('/');
            if(ls != string::npos)
            {
                path = filename.substr(0, ls);
            }
            else
            {
                char spath[256]={0};

                ::getcwd(spath, sizeof(spath)-1);
                path=spath;
            }
            struct statvfs64 fiData;

            if((statvfs64(path.c_str(), &fiData)) == 0 )
            {
                uint64_t bytesfree = (uint64_t)(fiData.f_bfree * fiData.f_bsize);
                uint64_t imgsz = _imagesz(width, height);
                maximages = (uint32_t)(bytesfree/imgsz)/2;

            }
            else
            {
                maximages = 1;
            }
            FILE* pff = ::fopen("./.lastimage","rb");
            if(pff)
            {
                char index[8];
                ::fgets(index, 8, pff);
                firstimage=::atoi(index);
                ::fclose(pff);
            }

            std::cout << "Current image:" << firstimage << ", Roll up at:" << maximages << "\n";
        }

        double dct =  gtc();
        while(__alive  && 0 == ::usleep(4000))
        {
            if(ps)ps->spin();

            shotsignal = sigcapt && __capture; //signal by SIGUSR1
            capture |= shotsignal | motionb | !filename.empty();
            capture |= ps && ps->has_clients();

            double dcurt =  gtc();
            double elapsed = dcurt -  dct;
            dct = dcurt;

            ct+=elapsed; // add 10 milliseconds
             periodexpired = 0;
            if(ct > frameperiod)
            {
                periodexpired=1;
                ct=0;
                capture = 1;
            }

            if(capture == 0)
            {
                continue;
            }

            video420 = dev.read(w, h, sz);
            if(video420)
            {
                int movepix = dev.movement();
				if(movepix >= motiona && movepix <= motionb){
					std::cout<<"movement pixels = " << movepix << "\n";
                }
				else{
                    movepix = 0;
				}
				tnow = time(0);
                uint32_t jpgsz = ffmt->convert420(video420, w, h, sz, quality, &pjpg);
                if(jpgsz )
                {
                    if((sigcapt || periodexpired || movepix) && !filename.empty())
                    {
                        char fname[PATH_MAX]={0};

                        if(maximages){
                            ::sprintf(fname, filename.c_str(), firstimage++);
                            if(firstimage > maximages)
                                firstimage = 0;
                        }
                        else
                            ::sprintf(fname, "%s", filename.c_str());

                        FILE* pf = ::fopen(fname,"wb");
                        if(pf)
                        {
                            ::fwrite(pjpg,1,jpgsz,pf);
                            ::fclose(pf);
                            if(nsignal){
                                ::kill(nsignal, SIGUSR2);
                                std::cout << "SIGUSR2: " << nsignal << "\n";
                            }
                            std::cout << "saving: " << fname << "\n";


                            if(tnow - lastsave > 2)//2 seconds
                            {
                                lastsave = tnow;
                                FILE* pff = ::fopen("./.lastimage","wb");
                                if(pff)
                                {
                                    ::fprintf(pff,"%d",firstimage);
                                    ::fclose(pff);
                                }
                            }
                        }
                        sigcapt=0;
                        __capture=false;
                        if(--oneshot==1) // one shot
                            break;
                    }
                    if(ps && ps->has_clients() && periodexpired)
                    {
                        ps->stream_on(pjpg, jpgsz, format=="jpg" ? "jpeg" : "png", 1);
                        int w, h;
                        size_t sz;
                        const uint8_t* mot = dev.getm(w, h, sz);
                        if(mot)
                        {
                            uint32_t jpgsz = ffmt->convertBW(mot, w, h, sz, quality, &pjpg);
                            ps->stream_on(pjpg, jpgsz, format=="jpg" ? "jpeg" : "png", 0);
                        }
                    }
                }
            }
            if(ps->socket()<0 || ps->socket()>32)
            {
                assert(0);
            }
        }
        delete ps;
        dev.close();
        delete ffmt;
    }
    std::cout << "Done\n";
    return 0;
}

static int _usage()
{
    std::cout <<
              "-d Video device '/dev/video#'. Default  /dev/video0. Add user to video group!!!\n"
              "-s Http server port.\n"
              "-g PID Proces where to send the SIGUSR2 after output is updated.\n"
              "-c signal SIGUSR1 for let go a capture.\n"
              "-o Output filename or wild*,b,e to save sequence, no extension (extension added by format [-i]). \n"
              "-i jpg|png Image format. Default jpg\n"
              "-q NN JPEG quality (0-100). Default 90%\n"
              "-z WxH Image width and height. Could be adjusted. Default 640x480\n"
              "-t NNN frame period in milliseconds\n"
              "-T NNN frame period in seconds. second option takes priority if t and T are used\n"
              "-m N,M Capture when motion pixels is in between N and M\n";
    return -1;
}


