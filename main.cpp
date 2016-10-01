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
    int         sigcapt=0;
    int         motion=0;
    int         port=0;
    int         quality=90;
    int         width=640;
    int         height=480;
    int         fps = 30;
    string      filename="";
    int         signal = 0;
    int         delta = 30;
    for(int k=0;k<nargs;++k)
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
                    if(fps<0)fps=0;
                    break;
                case 'n':  ///  delay between frames
                    delta = ::atoi(vargs[k]);
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
    int          realfps = fps;
    if(realfps==0)realfps=1;
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

    v4ldevice   dev(device.c_str(), width, height, realfps, motion);
    if(dev.open())
    {
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

        int framelet = 1000/realfps;
        uint32_t ct = framelet-1;
        uint32_t snap=0;
        uint8_t* pjpg;
        const uint8_t* video420;

        if(fps==0)
        {
            framelet=1;
            ct = 0;
        }

        while(__alive)
        {
            bool letgo = sigcapt && __capture;

            if(ps && ps->spin());
            letgo |= motion | !filename.empty();
            letgo |= (ps && ps->has_clients()) ;
            if(letgo && (++ct % framelet == 0))
            {
                video420 = dev.acquire(w, h, sz);
                if(video420)
                {
                    uint32_t jpgsz = ffmt->convert420(video420, w, h, sz, quality, &pjpg);
                    if(jpgsz )
                    {
                        if(sigcapt)
                        {
                            --snap;
                            delta=2;
                        }
                        if( ((++snap % delta == 0)|| (sigcapt && __capture)|| fps==0) && !filename.empty())
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
                            if(fps==0)
                                break;
                        }
                        if(ps && ps->has_clients())
                        {
                            ps->stream_on(pjpg, jpgsz, format=="jpg" ? "jpeg" : "png");
                        }
                    }
                }
            }
            ::usleep(1000); // 1 milli
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
    sleep (1);
    return -1;
}


