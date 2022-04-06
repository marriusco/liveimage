
#include <assert.h>
#include "motion.h"
#include "liconfig.h"


mmotion::mmotion(int w, int h, int nr):_wind(w,h),_w(w),_h(h),_nr(nr)
{

	_mw = _w/4;
	_mh = _h/4;

	size_t msz = (_mw) * (_mh);

	if(GCFG->_glb.rmotionrect[2] >= _w)
		GCFG->_glb.rmotionrect[2]=_w-1;
	if(GCFG->_glb.rmotionrect[3] >= _h)
		GCFG->_glb.rmotionrect[3]=_h-1;

    _motion_rect[0] = GCFG->_glb.rmotionrect[0]/4;
    _motion_rect[1] = GCFG->_glb.rmotionrect[1]/4;
    _motion_rect[2] = GCFG->_glb.rmotionrect[2]/4;
    _motion_rect[3] = GCFG->_glb.rmotionrect[3]/4;

    if(_motion_rect[2]==0 && _motion_rect[3]==0)
    {
        _motion_rect[0]=0;	//0 x
        _motion_rect[1]=0;	//1 y
        _motion_rect[2]=_mw;	//2  X
        _motion_rect[3]=_mh;	//3  Y
    }

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
    register uint8_t*          pSeen = _motionbufs[2];
    register uint8_t*          prowprev = _motionbufs[_motionindex ? 0 : 1];
    register uint8_t*          prowcur = _motionbufs[_motionindex ? 1 : 0];
    int               dx = _w / _mw;
    int               dy = _h / _mh;
    int               pixels = 0;
    int               mdiff  = GCFG->_glb.motiondiff * 2.55;
    uint8_t           Y,YP ;

    if(mdiff<1){  mdiff=4; }
    _dark  = 0;
    _moves = 0;
    _wind.reset(_mw,_mh);
    for (int y= 0; y <_mh-dy; y++)             //height
    {
        for (int x = 0; x < _mw-dx; x++)       //width
        {
            if(x<_motion_rect[0])continue;
            if(x>_motion_rect[2])continue;
            if(y<_motion_rect[1])continue;
            if(y>_motion_rect[3])continue;

            Y  = *(base_py + ((y*dy)  * _w) + (x*dx)); /// curent pixel

            _dark += uint32_t(Y);		   //  noise and dark
            Y /= _nr; Y *= _nr;

            *(prowcur + (y * _mw) + x) = Y;       // build new video buffer
            YP = *(prowprev+(y  * _mw) + (x));   // old buffer pixel

            int diff = abs(Y - YP);

            if(diff < mdiff)
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

    //[x,y,X,Y]
    for (int y= _motion_rect[1]+1; y <_motion_rect[3]-1; y++)
    {
        *(pSeen + (y * _mw) + _motion_rect[0]) = (uint8_t)192;
        *(pSeen + (y * _mw) + _motion_rect[2]) = (uint8_t)192;
    }

    for (int x = _motion_rect[0]+1; x < _motion_rect[2]-1; x++)
    {
        *(pSeen + (_motion_rect[0] * _mw) + x) = (uint8_t)192;
        *(pSeen + (_motion_rect[2] * _mw) + x) = (uint8_t)192;
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

