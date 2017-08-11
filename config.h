/**
# Copyright (C) 2006-2014 Chincisan Octavian-Marius(marrius9876@gmail.com)
# Project at: https://github/comarius
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


#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "os.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include "sock.h"
#include "strutils.h"

using namespace std;


//-----------------------------------------------------------------------------
#ifdef DEBUG
#define MAX_ROLLUP 200     //200b
#else
#define MAX_ROLLUP 2000000 //2Mb
#endif
//-----------------------------------------------------------------------------

typedef int (*pfatoi) (const char *);
typedef std::map<string, string> Mapss;
typedef std::map<string, string>::iterator MapssIt;

//-----------------------------------------------------------------------------
class Conf;
extern Conf* PCFG;

/**
 * Nice logging to terminal
 */
namespace Tc
{
    enum Code {
        FG_RED      = 31, 
        FG_GREEN    = 32, 
        FG_YELLOW   = 33, 
        FG_BLUE     = 34, 
        FG_CYAN     = 36, 
        FG_DEFAULT  = 39, 

        BG_RED      = 41, 
        BG_GREEN    = 42, 
        BG_YELLOW   = 43, 
        BG_BLUE     = 44, 
        BG_CYAN     = 46, 
        BG_DEFAULT  = 49
    };
    class Md {
        Code code;
    public:
        Md(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Md& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };
};

/**
 * @brief The Conf class
 */
class Conf
{
protected:
    struct StrRule
    {
        char    s[64];
        int     val;
    };

    void _bind(const char* lpred, const char* vname, 
               size_t& val, const char* kv )
    {
        _bind(lpred, vname, (int&) val, kv );
    }
    void _bind(const char* lpred, const char* vname, 
               std::string& val, const char* kv)
    {

        if(vname)
        {
            if(!strcmp(lpred, vname))
            {
                val = kv;
                //throw(1);
                return;
            }
            return;
        }
        val = kv;
        //throw(1);
    }

    template <class T>
    void _bind(const char* lpred, const char* vname, 
               T& val, const char* kv, StrRule* r=0 )
    {
        if(!strcmp(lpred, vname))
        {
            if(r)
            {
                StrRule* pr = r;
                while(pr->s && *pr->s)
                {
                    if(!strcmp(pr->s, kv))
                    {
                        val = pr->val;
                        //throw(1);
                    }
                    pr++;
                }
            }
            else
            {
                val = ::_ttatoi(kv);
            }
        }
    }

    void _bind(const char* lpred, const char* vname, 
               std::set<string>& val, const char* kv)
    {
        if(!strcmp(lpred, vname))
        {
            std::istringstream iss(kv);
            std::string token;
            while(getline(iss, token, ','))
            {
                val.insert(token);
            }
        }
    }

    void _bind(const char* lpred, const char* vname, 
               std::vector<string>& val, const char* kv)
    {
        if(!strcmp(lpred, vname))
        {
            std::istringstream iss(kv);
            std::string token;
            while(getline(iss, token, ','))
            {
                val.push_back(token);
            }
        }
    }


    void _bind(const char* lpred, const char* vname, 
               map<SADDR_46, SADDR_46>& val, const char* kv)
    {
        if(!strcmp(lpred, vname))
        {
            std::istringstream iss(kv);
            std::string token;
            while(getline(iss, token, ','))
            {
                std::string value;
                getline(iss, value, ',');
                val[sock::sip2ip(token.c_str())] = sock::sip2ip(value.c_str());
            }
        }
    }


    void _bind(const char* lpred, const char* vname, 
               vector<SADDR_46>& val, const char* kv)
    {
        if(!strcmp(lpred, vname))
        {
            std::istringstream iss(kv);
            std::string token;
            while(getline(iss, token, ','))
            {
                val.push_back( sock::sip2ip(token.c_str()));
            }
        }
    }

    void _bind(const char* lpred, const char* vname, 
               set<SADDR_46>& val, const char* kv)
    {
        if(!strcmp(lpred, vname))
        {
            std::istringstream iss(kv);
            std::string token;
            while(getline(iss, token, ','))
            {
                val.insert( sock::sip2ip(token.c_str()));
            }
        }
    }

public:
    Conf();
    virtual ~Conf();

public:
    bool    load(const char*);
    void    check_log_size();
    void    rollup_logs(const char* rootName);
protected:

    virtual void _assign( const char* pred, const char* val, int line)=0;
    virtual bool finalize();//{return true;};
public:
    std::string         _section;
    std::string         _logs_path;

    size_t              _blog;
    std::stringstream   _logcntn;

};

//-----------------------------------------------------------------------------------
template <typename T>inline void fixval(T& val, T minv, T maxv)
{
    if(val < minv)
    {
        val = minv;
        return;
    }
    if(val > maxv)
    {
        val = maxv;
        return;
    }
}

#define LOG_PIPE   PCFG->_logcntn


#endif //_CONFIG_H_

