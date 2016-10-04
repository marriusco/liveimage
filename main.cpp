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


#define V4L2NET_VERSION "1.0.0"

#include <iostream>
#include <string>
#include "v4ldevice.h"
#include "sockserver.h"
#include "jpeger.h"
#include "pnger.h"

/*
sudo apt-get install libpng-dev
sudo apt-get install libv4l-dev
sudo apt-get install libjpeg-dev
*/


using namespace std;
static int _usage();
bool __alive = true;
bool __capture=false;

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
    int         oneshot=0;
    int         sigcapt=0;
    int         motion=0;
    int         port=0;
    int         quality=90;
    int         width=640;
    int         height=480;
    int         fps = 30;
    string      filename="";
    int         signal = 0;
    int         filesaveframes = 30;
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
                signal = ::atoi(vargs[k]);
                std::cout << "will signal process: " << signal << "\n";
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
                motion = ::atoi(vargs[k]);
                break;
            case 'z':
            {
                int two = sscanf(vargs[k],"%dx%d", &width, &height);
                if(two != 2)
                    return _usage();
            }
            break;
            case 'f':  ///  fps http
                fps = atoi(vargs[k]);
                if(fps>50)fps=50;
                if(fps<0)fps=1;
                break;
            case 'n':  ///  delay between frames
                filesaveframes = ::atoi(vargs[k]);
                break;
            case 'i':
                format = vargs[k];
                break;
            case 'c':
                sigcapt = 1; // ::atoi(vargs[k]);
                break;
            case 'v':
                std::cout << V4L2NET_VERSION << "\n";
                return 0;
            default:
                return _usage();
            }
        }
    }
    outfilefmt* ffmt = 0;
    int         w,h;
    size_t      sz ;

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

    v4ldevice   dev(device.c_str(), width, height, fps, motion);
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

        int             delay   = 1000/fps;
        uint32_t        ct = delay-1;
        uint32_t        snap=0;
        uint8_t*        pjpg;
        const uint8_t*  video420;
        bool            capture=false;
        bool            signal=false;

        while(__alive  && 0 == ::usleep(1000))
        {
            if(ps)ps->spin();
            signal = sigcapt && __capture; //signal by SIGUSR1
            capture |= signal | motion | !filename.empty();
            capture |= ps && ps->has_clients();
            capture |= (++ct % delay) == 0;
            if(capture == 0)
                continue;
            video420 = dev.acquire(w, h, sz);
            if(video420)
            {
                uint32_t jpgsz = ffmt->convert420(video420, w, h, sz, quality, &pjpg);
                if(jpgsz )
                {
                    if((sigcapt || ct % filesaveframes==0)&& !filename.empty())
                    {
                        FILE* pf = fopen(filename.c_str(),"wb");
                        if(pf)
                        {
                            fwrite(pjpg,1,jpgsz,pf);
                            fclose(pf);
                            if(signal)
                                kill(signal, SIGUSR1);
                            std::cout << "saving: " << filename << "\n";
                        }
                        sigcapt=0;
                        __capture=false;
                        if(--oneshot==1) // one shot
                            break;
                    }
                    if(ps && ps->has_clients())
                    {
                        ps->stream_on(pjpg, jpgsz, format=="jpg" ? "jpeg" : "png", true);
                        int w, h;
                        size_t sz;
                        const uint8_t* mot = dev.getm(w, h, sz);
                        if(mot)
                        {
                            uint32_t jpgsz = ffmt->convertBW(mot, w, h, sz, quality, &pjpg);
                            ps->stream_on(pjpg, jpgsz, format=="jpg" ? "jpeg" : "png", false);
                            /*
                            FILE* pf = fopen("motion.jpg","wb");
                            if(pf)
                            {
                                fwrite(pjpg,1,jpgsz,pf);
                                fclose(pf);
                                if(signal)
                                    kill(signal, SIGUSR1);
                                std::cout << "saving: " << filename << "\n";
                            }
                            */

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
              "-g PID Proces where to send the SIGUSR1 after output is updated.\n"
              "-c signal SIGUSR2 for let go a capture.\n"
              "-o Output filename, no extension (extension added by format [-i]). \n"
              "-i jpg|png Image format. Default jpg\n"
              "-q NN JPEG quality (0-100). Default 90%\n"
              "-z WxH Image width and height. Could be adjusted. Default 640x480\n"
              "-f FFF frames per second \n"
              "-n NNN At how many frames [-f] to save a snapshot\n"
              "-m NNN Capture when motion, motion sensitivity\n";
    return -1;
}


