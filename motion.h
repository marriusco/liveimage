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
#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>
#include <inttypes.h>
#include <string>
#include <vector>
#include "motion.h"
#include "liconfig.h"
#include "os.h"


class wind
{
public:
    struct Rect{
        Rect(const Rect& r){_x=r._x;_y=r._y;_X=r._X;_Y=r._Y;}
        Rect(){reset(0,0);}
        Rect(int w, int h){_x=0;_y=0;_X=w;_Y=h;}
        void reset(int w, int h){
            _x=w;_y=h;_X=0;_Y=0;
        }
        bool in_rect(int x, int y)const{
            if(x>_x && x<_X && y>_y && y<_Y){
                return true;
            }
            return false;
        }
        int width()const{return _X-_x;}
        int height()const{return _Y-_y;}
        Rect& operator=(const Rect& r){_x=r._x;_y=r._y;_X=r._X;_Y=r._Y; return *this;}
        int  maxdiff(const Rect& r)const{
            int diff = std::abs(r._x-_x);
            diff = std::max(diff, std::abs(r._y-_y));
            diff = std::max(diff, std::abs(r._X-_X));
            return std::max(diff, std::abs(r._Y-_Y));
        }
        bool operator==(const Rect& r)const{return _x==r._x&&_y==r._y&&_X==r._X&&_Y==r._Y;}
        bool operator!=(const Rect& r)const{return _x!=r._x||_y!=r._y||_X!=r._X||_Y!=r._Y;}
        void re_rect(int x, int y){
            _x=std::min(_x,x);
            _X=std::max(_X,x);
            _y=std::min(_y,y);
            _Y=std::max(_Y,y);
        }
        bool empty()const{
            return _x==0&&_y==0&&_X==0&&_Y==0;
        }
        int _x;
        int _y;
        int _X;
        int _Y;
    };

    wind(int w, int h):_imgRect(w,h)
    {
        _movepix = 0;
        _accumulating =  GCFG->_glb.windaccum;
        _windy = 0;
        _checks = 0;
    }

    void reboot()
    {
        _accumulating =  GCFG->_glb.windaccum;
        _prevRect.reset(0,0);
        _rejectRect.reset(0,0);
    }

    void reset(int w, int h)
    {
        _movepix=0;
        if(--_accumulating<0)
        {
            _curRect.reset(w,h);
            _accumulating = GCFG->_glb.windaccum;
        }
    }

    bool accum_rect(int x, int y,bool mved)
    {
        if(mved)
            _curRect.re_rect(x,y);
        if(_rejectRect.in_rect(x,y))
        {
            if(mved)
                ++_movepix;
            return true;
        }
        return false;
    }
    const Rect& currect(int allmoves)
    {
        return _curRect;
    }

    const Rect& reject(int allmoves){
        if(_accumulating==0)
        {
            if(_imgRect!=_curRect)
            {
                if(allmoves > GCFG->_glb.imotion[0])
                {
                    int closeby = _prevRect.maxdiff(_curRect) * 100 / _imgRect._X;
                    if(closeby < GCFG->_glb.windcomp)
                    {
                        ++_windy;
                    }
                    ++_checks;
                    _prevRect = _curRect;
                }
                else
                {
                    _prevRect.reset(0,0);
                    _checks = 0;
                    _windy = 0;
                }
            }
            if(_checks > GCFG->_glb.windcheck)
            {
                if(_windy > (_checks/4)+1)
                {
                    _rejectRect = _prevRect;
                    _prevRect.reset(0,0);
                }
                else
                {
                    _rejectRect.reset(0,0);
                }
                _checks=0;
                _windy=0;
            }
            _curRect.reset(_imgRect._X,_imgRect._Y);
        }

        return _prevRect;
    }
    int movements()const{return _movepix;}

private:
    int       _movepix;
    int       _accumulating;
    int       _windy;
    int       _checks;
    Rect      _prevRect;
    Rect      _curRect;
    Rect      _rejectRect;
    Rect      _imgRect;

};


class mmotion
{
public:
    mmotion(int w, int h, int nr);
    ~mmotion();
    int has_moved(uint8_t* p);
    int  getw()const{return _mw;}
    int  geth()const{return _mh;}
    uint8_t*  motionbuf()const{return _motionbufs[2];}
    uint32_t darkav()const{return _dark;}

private:
    wind      _wind;
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
    int       _motion_rect[4]; //x/y/w/h
};



#endif // V4LDEVICE_H
