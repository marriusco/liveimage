#include "webcast.h"
#include "liconfig.h"
#include "sockserver.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

extern bool __alive;
/////////////////////////////////////////////////////////////////////////////////////////////////
WebCast::WebCast()
{
    _jps.magic   = JPEG_MAGIC;
    _jps.key     = GCFG->_glb.webpass;
    _jps.record  = GCFG->_glb.record & 0x3;
    _jps.format  = (LiFrmHdr::e_format)(GCFG->_glb.wformat & 0xF);
    _jps.insync  = GCFG->_glb.insync;

}

////////////////////////////////////////////////////////////////////////////////////////////////
WebCast::~WebCast()
{
    this->stop_thread();
    delete _punched;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WebCast::stream_frame(uint8_t* pjpg, size_t length, int movepix, int w, int h)
{
    AutoLock a(&_mut);

    if(_frame==nullptr){
        _frame = new uint8_t[length+4094];
        std::cout << "new " << length << "\n";
        _buffsz = length+4096;
    }
    if(length > _buffsz){
        delete[] _frame;
        _frame = new uint8_t[length+4096];
        std::cout << "renew " << length << "\n";
        _buffsz = length+4096;
    }
    ::memcpy(_frame, pjpg, length);
    _jps.wh[0] = w;
    _jps.wh[0] = h;
    _jps.len = length;
    _jps.movepix = movepix;
    _jps.lapse = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////
void WebCast::thread_main()
{
    time_t          ctime = 0;
    tcp_cli_sock    cli;
    char            scheme[8];
    int             port = 80;
    char            host[32];
    char            path[64];
    char            url[256];

    ::strcpy(url, GCFG->_glb.webcast.c_str());
    parseURL(url, scheme,
             sizeof(scheme), host, sizeof(host),
             &port, path, sizeof(path));

    if(GCFG->_glb.checkcast < 8){GCFG->_glb.checkcast=8;}

    while(!this->OsThread::is_stopped() && __alive)
    {
        if((time(0)-ctime > GCFG->_glb.checkcast && _jps.len) || (_jps.movepix>0 && GCFG->_glb.record))
        {
            _send_tcp(host, path+1, port);
            ctime = time(0);
        }
        msleep(128);
        if(_punched){
            _punched->spin();
        }
    }
}

void WebCast::kill()
{
    _s.destroy();
}

void WebCast::_send_tcp(const char* host, const char* camname, int port)
{
    int             by;
    int             webms = GCFG->_glb.webms;
    int             lenhdr = 0;
    int             onenter = GCFG->_glb.onenter;
    char            buffer[256];
    struct timeval  timestamp;
    struct timezone tz = {5,0};
    bool needsframe = false;

    _s.destroy();
    if(_s.create(_cport))
    {
        _s.set_blocking(1);
        if(_s.try_connect(host, port)){
            std::cout << "cam connected "<<host<< port<<"\r\n";
        }
        else{
            std::cout << "cam cannot connect "<<host<< port<<"\r\n";
            goto DONE;
        }
        if(_s.isopen())
        {
            std::cout << "cam stream really connected \r\n";

            ::strncpy(_jps.camname, camname, sizeof(_jps.camname));
            do{
                AutoLock a(&_mut);

                std::cout << "init conn with movepix " << _jps.movepix << "\r\n";
                _jps.movepix = 0;
                _jps.count   = 0;
                _jps.lapse   = 0;
                _jps.len     = strlen(HEADER_JPG);


            }while(0);

            uint32_t len = sizeof(_jps);
            by = _s.sendall((const uint8_t*)&len, (int)sizeof(uint32_t));
            if(by!=sizeof(uint32_t))
            {
                std::cout << "SA1 ERROR \r\n";
                goto DONE;
            }
            by=_s.sendall((const uint8_t*)&_jps, (int)sizeof(_jps));
            if(by!=sizeof(_jps))
            {
                std::cout << "SA2 ERROR \r\n";
                goto DONE;
            }
            if(_jps.format == LiFrmHdr::e_mpart_jpeg)
            {
                by = _s.sendall(HEADER_JPG, strlen(HEADER_JPG));
                if(by!=strlen(HEADER_JPG))
                {
                    std::cout << "SAJ ERROR \r\n";
                    goto DONE;
                }
            }
            msleep(1000);
            while(by && _s.isopen() && __alive)
            {
                if(_jps.insync)
                {
                    needsframe=false;
                    std::cout << "WAITING \r\n" ;
                    _s.set_blocking(1);
                    by = _s.receiveall((uint8_t*)&_jps, sizeof(_jps));
                    if(by!=sizeof(_jps)){
                        std::cout << "REC ERROR " << by <<", "<< _s.error() << "\n";
                        goto DONE;
                    }
                    if(_jps.len==0){
                        needsframe=true;
                    }
                    if(needsframe==false){
                        goto END_WILE;
                    }
                }

                if(onenter)
                {
                    std::string s;
                    std::cout << "press a key\n" ;
                    std::cin >> s;
                }

                do{
                    AutoLock a(&_mut);

                    if(_jps.len==0){
                        goto END_WILE;
                    }

                    if(_jps.format == LiFrmHdr::e_mpart_jpeg){
                        gettimeofday(&timestamp, &tz);
                        lenhdr = sprintf(buffer, "Content-Type: image/%s\r\n" \
                                                 "Content-Length: %d\r\n" \
                                                 "X-Timestamp: %d.%06d\r\n" \
                                                 "\r\n", "jpg",
                                         _jps.len,
                                         (int)timestamp.tv_sec,
                                         (int)timestamp.tv_usec);
                    }else{
                        _jps.count++;
                        _jps.len     = _jps.len;
                        memcpy(buffer, &_jps, sizeof(_jps));
                        lenhdr = sizeof _jps;
                    }
                    by = _s.sendall(buffer, lenhdr);
                    if(by!=lenhdr)
                    {
                        std::cout << "SA3 ERROR " <<lenhdr <<", "<< _s.error() << "\n";
                        goto DONE;
                    }
                    by = _s.sendall(_frame, _jps.len, 6000);
                    if(by!=_jps.len)
                    {
                        std::cout << "SA4 ERROR sent:"<< by <<" != frame:" <<_jps.len << ", " << _s.error() << "\n";
                        goto DONE;
                    }
                    std::cout << "sent " << _jps.len << "\n" ;

                }while(0);
                if(_jps.format == LiFrmHdr::e_mpart_jpeg)
                {
                    ::sprintf(buffer, "\r\n--MY_BOUNDARY_STRING_NOONE_HAS\r\nContent-type: image/%s\r\n", "jpg");
                    by = _s.sendall(buffer,strlen(buffer),100);
                    if(by!=strlen(buffer))
                    {
                        std::cout << "SABOUND ERROR sent:"<< by <<" != frame:" <<_jps.len << ", " << _s.error() << "\n";
                        goto DONE;
                    }
                }
END_WILE:
                msleep(16+webms);
            }
        }
    }
DONE:
    AutoLock a(&_mut);
    _jps.len  = 0;
    _s.destroy();
    std::cout << "socked close\r\n";
}



