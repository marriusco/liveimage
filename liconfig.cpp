/**
# Copyright (C) 2012-2014 Chincisan Octavian-Marius(mariuschincisan@gmail.com) - coinscode.com - N/A
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/


#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include <limits.h>
#include "strutils.h"
#include "liconfig.h"


//-----------------------------------------------------------------------------
#define BIND(mem_, name_)     _bind(lpred, #name_, mem_.name_, val);

LiConfig* GCFG;

LiConfig::LiConfig(const char* fname)
{
    GCFG = this;
    load(fname);
}

LiConfig::~LiConfig()
{
    GCFG = 0;
}

SADDR_46  fromstringip(const std::string& s)
{
    string tmp = s;
    int    nport = 80;
    if(tmp.find("http://") == 0)
        tmp = s.substr(7);
    else if(tmp.find("https://") == 0)
    {
        tmp = s.substr(8);
        nport = 443;
    }

    size_t port = tmp.find(':');
    size_t doc = tmp.find('/');
    if(port!=string::npos)
    {
        nport = ::atoi(tmp.substr(port+1, doc).c_str());
    }
    else if(doc!=string::npos)
    {
        port = doc;
    }

    char phost[256];

    ::strcpy(phost, tmp.substr(0, port).c_str());
    if(isdigit(phost[0]))
        return SADDR_46(phost, nport);
    char test[32];
    SADDR_46 r = sock::dnsgetip(phost, test, nport);
    return r;
}

//-----------------------------------------------------------------------------
void LiConfig::_assign( const char* pred, const char* val, int line)
{
    char    lpred[256];
    char    loco[256];

    ::strcpy(loco, val);
    ::strcpy(lpred, pred);
    try
    {
        if(_section == "[main]")
        {
            BIND(_glb, darklapse);
            BIND(_glb, darkmotion);
            BIND(_glb, device);
            BIND(_glb, flip);
            BIND(_glb, sigcapt);
            BIND(_glb, port);
            BIND(_glb, pathname);
            BIND(_glb, oneshot);
            BIND(_glb, quality);
            BIND(_glb, motion);
            BIND(_glb, motionnoise);
            BIND(_glb, fps);
            BIND(_glb, imagesize);
            BIND(_glb, timelapse); //ms
            BIND(_glb, format);
            BIND(_glb, signalin);
            BIND(_glb, userpid);
            BIND(_glb, motionsnap);
            BIND(_glb, motionrect);
            BIND(_glb, windcomp);
            BIND(_glb, rectacum);
            BIND(_glb, windcheck);
            BIND(_glb, windcount);
            BIND(_glb, motiondiff);
            BIND(_glb, motiontrail);
            BIND(_glb, motionw);

            BIND(_glb, savelapse);
            BIND(_glb, httpport);
            BIND(_glb, httpip);
        }
    }
    catch(int done) {}
}

bool LiConfig::finalize()
{
    ::sscanf(_glb.motion.c_str(),"%d,%d", &_glb.imotion[0], &_glb.imotion[1]);
    ::sscanf(_glb.imagesize.c_str(),"%dx%d", &_glb.w, &_glb.h);

    return Conf::finalize();
}
