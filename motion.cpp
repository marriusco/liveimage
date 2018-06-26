#include "motion.h"
#include "liconfig.h"


mmotion::mmotion(int w, int h, int nr):_wind(w,h),_w(w),_h(h),_nr(nr)
{
    _mw = GCFG->_glb.motionw;
    if(_mw>=w)
        _mw=w/2;
    else if(_mw<8)
        _mw=8;
    _mh = (_mw * _h) / _w;
    size_t msz = (_mw) * (_mh);
    _motionbufs[0] = new uint8_t[msz];
    _motionbufs[1] = new uint8_t[msz];
    _motionbufs[2] = new uint8_t[msz];
    memset(_motionbufs[0],0,msz);
    memset(_motionbufs[1],0,msz);
    memset(_motionbufs[2],0,msz);
    _motionindex = 0;
    _motionsz = msz;
    _moves=0;
    _mmeter = 0;


}

mmotion::~mmotion()
{
    delete []_motionbufs[0];
    delete []_motionbufs[1];
    delete []_motionbufs[2];
}

int mmotion::has_moved(uint8_t* fmt420)
{
    register uint8_t *base_py = fmt420;
    int               dx = _w / _mw;
    int               dy = _h / _mh;
    uint8_t*          pSeen = _motionbufs[2];
    uint8_t*          prowprev = _motionbufs[_motionindex ? 0 : 1];
    uint8_t*          prowcur = _motionbufs[_motionindex ? 1 : 0];
    int               pixels = 0;
    int               mdiff = GCFG->_glb.motiondiff * 2.55;
    uint8_t           Y,YP ;

    if(mdiff<1)
        mdiff=4;
    _dark  = 0;
    _moves = 0;
    _wind.reset(_mw,_mh);
    for (int y= 0; y <_mh; ++y)//height
    {
        for (int x = 0; x < _mw; ++x)//width
        {
            Y  = *(base_py+((y*dy)  * _w) + (x*dx)); /// curent pixel
            _dark += uint32_t(Y);
            Y /= _nr; Y *= _nr;
            *(prowcur + (y * _mw)+x) = Y;       // build new video buffer
            YP = *(prowprev+(y  * _mw) + (x));  // old buffer pixel
            int diff = Y - YP;
            if(diff<mdiff)
            {
                diff=0;                         // black no move
                if(_wind.accum_rect(x,y,false))
                {
                    diff=32;
                }
            }
            else if(diff>mdiff)
            {
                diff=255; //move
                ++_moves;
                if(_wind.accum_rect(x,y,true))
                {
                    diff=32;
                }
            }

            *(pSeen + (y * _mw)+x) = (uint8_t)diff;
            ++pixels;
        }
    }
    const wind::Rect& reject = _wind.reject(_moves);
    if(!reject.empty())
    {
        for (int y= reject._y; y <reject._Y; ++y)
        {
            *(pSeen + (y * _mw)+reject._x) = (uint8_t)255;
            *(pSeen + (y * _mw)+reject._X) = (uint8_t)255;
        }
        for (int x = reject._x; x < reject._X; ++x)
        {
            *(pSeen + (reject._y * _mw)+x) = (uint8_t)255;
            *(pSeen + (reject._Y * _mw)+x) = (uint8_t)255;
        }
    }
    _moves -= _wind.movements();

    // show movement percentage on left as bar
    int percentage = std::min(100,_moves);
    if(percentage > _mmeter)
        _mmeter = percentage;
    else if(_mmeter)
        --_mmeter;
    if(_mmeter)
    {
        int y =_mh;
        int x = 0;
        while(y)
        {
            if(_mmeter < y)
                *(pSeen + ((_mh-y) * _mw)+x) = (uint8_t)0;
            else
                *(pSeen + ((_mh-y) * _mw)+x) = (uint8_t)255;
            --y;
        }
    }
    else
    {
         _wind.reboot();
    }
    _dark /= pixels;
    _motionindex = !_motionindex;
    return _moves;
}

