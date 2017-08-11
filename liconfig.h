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
    }
    void refresh_domains();
protected:
    bool finalize();
    void _assign( const char* pred, const char* val, int line);
    void fix_path(std::string& path)
    {
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
            userpid=0;
            motionsnap=200;
            savelapse=200;
            httpport=0;
        }
        int     darklapse;
        int     darkmotion;
        string  device;
        int     sigcapt;
        int     port;
        string  filename;
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
        int     imotion[2];
        uint32_t     motionsnap;
        uint32_t     savelapse;
        int     httpport;
        string  httpip;
    }_glb;

};

//-----------------------------------------------------------------------------------
extern LiConfig* GCFG;
extern SADDR_46  fromstringip(const std::string& s);

#endif //_CONFIGPRX_H_
