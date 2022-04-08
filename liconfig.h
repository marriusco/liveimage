/*!
******************************************************************************
\file
\par Copyright (c) 2017, General Electric Co. GE Confidential and Proprietary
\par PRODUCT      GE MULTILIN SR8 SERIES SMCSDK
\par TARGET       Linux
\par AUTHOR       Octavian Marius Chincisan
******************************************************************************/



#ifndef _CONFIGPRX_H_
#define _CONFIGPRX_H_

#include "config.h"

//#############################################################################

#define MAX_GRID_COLS   3

//#############################################################################

//-----------------------------------------------------------------------------
class LiConfig: public Conf
{
public:
    LiConfig(const char* fname);
    virtual ~LiConfig();
    void    ix_path(std::string& path)
    {
        (void)path;
    }
    void refresh_domains();
protected:
    bool finalize();
    void _assign( const char* pred, const char* val, int line);
    void fix_path(std::string& path)
    {
        (void)path;
    }

public:
    struct Glb
    {
        Glb(){
            darklapse=-1;
            darkmotion=-1;
            device="/dev/video0";
            sigcapt=0;
            port=9000;
            //filename
            oneshot=0;
            quality=80;
            motion="0,0";
            motionnoise=4;
            fps=15;
            imagesize="640x480";
            timelapse=1500;
            format="jpg";
            signalin=0;
            motiontrail=3;
            userpid=0;
            motionsnap=200;
            windaccum=8;    // wind comp repetative in within 10%
            motiondiff=24;
            savelapse=200;
            motiondiff=24;
            flip=0;
            httpport=0;
            webms=30;
            windcomp=30;
            windcheck=8;
            method = "v4l";
        maxfiles=0;
        }
        int     darklapse;
        int     darkmotion;
        string  device;
        int     flip;
        int     sigcapt;
        int     motiontrail;
        int     port;
        string  pathname;
        int     motiondiff;
        int     oneshot;
        int     quality;
        string  motion;
        int     motionnoise;
        int     fps;
        string  imagesize;
        int     timelapse; //ms
        string  format;
        int     signalin;
        int     userpid;
        int     w;
        int     h;
        string  motionrect;
        int     rmotionrect[4];
        int     imotion[2];
        uint32_t     motionsnap;
        uint32_t     savelapse;
        int     httpport;
        int     windcomp;
        int     windcheck;
        int     windaccum;
        int     maxfiles;
        string  webcast;
        string  transport;
        string  kodek;
        int     checkcast = 10; // to check the server
        int     wformat = 1;
        int     webpass = 0;
        int     onenter = 0;
        int     insync = 0;
        int     record = 0;
        string  method;
        int     webms;
    }_glb;

};

//-----------------------------------------------------------------------------------
extern LiConfig* GCFG;
extern SADDR_46  fromstringip(const std::string& s);


inline time_t gtc(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
        return 0;
    return time_t(now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0);
}


#endif //_CONFIGPRX_H_
