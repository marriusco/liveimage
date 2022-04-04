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


#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include <limits.h>
#include "config.h"
#include "strutils.h"

//-----------------------------------------------------------------------------
Conf* PCFG;
//-----------------------------------------------------------------------------
Conf::Conf()
{
    assert(0 == PCFG);
    PCFG=this;
}

//-----------------------------------------------------------------------------
Conf::~Conf()
{
    PCFG=0;
}

//-----------------------------------------------------------------------------
bool Conf::finalize(){

    return true;
};

//-----------------------------------------------------------------------------
bool Conf::load(const char* cfgFile)
{
    FILE* pf = _ttfopen(cfgFile, "rb");
    if(pf==0)
    {
        char locations[512]={0};
        ::sprintf(locations, "/etc/%s", cfgFile);
        pf = _ttfopen(locations, "rb"); if(pf) goto done;
    }
    if(pf==0)
    {
        char locationsp[256]={0};
        char locations[400]={0};
        getcwd(locationsp, sizeof(locationsp)-1);
        ::snprintf(locations, sizeof(locations), "%s/%s", locationsp, cfgFile);
        pf = _ttfopen(locations, "rb"); if(pf) goto done;
    }

done:
    if(pf)
    {
        AutoCall<int (*)(FILE*), FILE*>    _a(::fclose, pf);
        char        pred[128];
        char        val[128];
        char        line[512];
        bool        in_comment = false;
        try
        {
            while(!::feof(pf))
            {
                if(::fgets(line, 512, pf))
                {
                    ::str_prepline(line);
                    if(*line==0)
                        continue;
                    if(in_comment || *line=='#')
                        continue;
                    const char* pnext = ::str_ccpy(pred, line, '=');
                    if(*pred=='[')
                    {
                        str_lrtim(pred);
                        _section = pred;
                        continue;
                    }
                    else   if(*pred=='}')
                    {
                        _assign("}",  " ", 0);
                    }
                    if(pnext && *pnext)
                    {
                        ::str_scpy(val, (char*)pnext+1, "#");
                        ::str_lrtim(val);
                        ::str_lrtim(pred);

                        //cheak the logs folder
                         if(pred[0]=='l')
                            _bind(pred, "logs_path", _logs_path, val);

                        _assign(pred, val, 0);
                    }
                }
                else
                    break;
                if(feof(pf))
                {
                    break;
                }
            }
        }
        catch(int& err)
        {
            ;//noting
        }
    }
    else
    {
        printf( "Cannot find configuration file in any of /etc/,  ~/. and  ./. \r\n");
        //exit(0);
    }
    return finalize();
}

//-----------------------------------------------------------------------------
#define BIND(mem_, name_)     _bind(lpred, #name_, mem_.name_, val);

//-----------------------------------------------------------------------------
// log files flush and roll-up
void    Conf::check_log_size()
{
    if(_logcntn.tellp() > 0)/* log this right away*/
    {

        char    logf[400];
        ::sprintf(logf, "%s/log%d-%d.log0", _logs_path.c_str(), getppid(), getpid());
        FILE* pf = ::fopen(logf, "ab");
        long int flen=0;

        if(pf)
        {
            ::fwrite(_logcntn.str().c_str(), 1, _logcntn.str().length(), pf);
            flen = ftell(pf);
            ::fclose(pf);
        }

        _logcntn.str("");
        _logcntn.clear();
        _logcntn.seekp(0);
        _logcntn.seekg(0);

        if(flen > MAX_ROLLUP)
        {
            ::sprintf(logf, "LOG%d-%d", getppid(), getpid());
            PCFG->rollup_logs(logf);
        }
    }
}

//-----------------------------------------------------------------------------
// rolls up the logs, when reaches the max size
void Conf::rollup_logs(const char* rootName)
{
    char oldFn[256];
    char newFn[256];

    int iold = 6;
    ::sprintf(oldFn, "%s/%s.log4", _logs_path.c_str(), rootName);
    ::unlink(oldFn);
    while(iold-- > 0)
    {
        ::sprintf(newFn, "%s/%s.log%d", _logs_path.c_str(), rootName , iold);
        ::sprintf(oldFn, "%s/%s.log%d", _logs_path.c_str(), rootName, iold-1);
        ::rename(oldFn, newFn);
    }
}

