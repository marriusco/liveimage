#include "webcast.h"
#include "liconfig.h"
#include "sockserver.h"

#define    JPEG_MAGIC       0x12345678

#define         PTRU_TOUT  30
#define          PACK_ALIGN_1   __attribute__((packed, aligned(1)))

struct  LiFrmHdr{
    enum e_format{e_mpart_jpeg, e_jpeg, e_mov};
    uint32_t    len;
    uint32_t    magic;
    uint32_t    mac;
    uint32_t    key;
    uint32_t    count;
    uint32_t    movepix;
    uint16_t    conport;
    uint8_t     lapse:1;
    uint8_t     insync:1;
    uint8_t     record:2;
    e_format    format:4;
    char        command[16];
    char        camname[16];
}PACK_ALIGN_1;

/////////////////////////////////////////////////////////////////////////////////////////////////

extern bool __alive;
/////////////////////////////////////////////////////////////////////////////////////////////////
WebCast::WebCast()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////
WebCast::~WebCast()
{
    this->stop_thread();
    delete _punched;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WebCast::stream_frame(uint8_t* pjpg, size_t length, int movepix)
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
    _length=_buffsz;
    if(_movepix==0 && movepix){
        _movepix = movepix;
    }
    _lapse = 0;
    ::memcpy(_frame, pjpg, length);
    _length = length;

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
        if((time(0)-ctime > GCFG->_glb.checkcast && _length) || (_movepix>0 && GCFG->_glb.record))
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
    LiFrmHdr        jps;
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

            ::strncpy(jps.camname, camname, sizeof(jps.camname));
            do{
                AutoLock a(&_mut);

                std::cout << "init conn with movepix " << _movepix << "\r\n";
                jps.magic = JPEG_MAGIC;
                jps.key = GCFG->_glb.webpass;
                jps.record  = GCFG->_glb.record & 0x3;
                jps.format  = (LiFrmHdr::e_format)(GCFG->_glb.wformat & 0xF);
                jps.insync  = GCFG->_glb.insync;
                jps.movepix = 0;
                jps.count   = 0;
                jps.lapse   = 0;
                jps.len     = strlen(HEADER_JPG);


            }while(0);

            uint32_t len = sizeof(jps);
            by = _s.sendall((const uint8_t*)&len, (int)sizeof(uint32_t));
            if(by!=sizeof(uint32_t))
            {
                std::cout << "SA1 ERROR \r\n";
                goto DONE;
            }
            by=_s.sendall((const uint8_t*)&jps, (int)sizeof(jps));
            if(by!=sizeof(jps))
            {
                std::cout << "SA2 ERROR \r\n";
                goto DONE;
            }
            if(jps.format == LiFrmHdr::e_mpart_jpeg)
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
                if(jps.insync)
                {
                    needsframe=false;
                    std::cout << "WAITING \r\n" ;
                    _s.set_blocking(1);
                    by = _s.receiveall((uint8_t*)&jps, sizeof(jps));
                    if(by!=sizeof(jps)){
                        std::cout << "REC ERROR " << by <<", "<< _s.error() << "\n";
                        goto DONE;
                    }
                    if(jps.len==0){
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

                    if(_length==0){
                        goto END_WILE;
                    }

                    if(jps.format == LiFrmHdr::e_mpart_jpeg){
                        gettimeofday(&timestamp, &tz);
                        lenhdr = sprintf(buffer, "Content-Type: image/%s\r\n" \
                                                 "Content-Length: %d\r\n" \
                                                 "X-Timestamp: %d.%06d\r\n" \
                                                 "\r\n", "jpg",
                                         _length,
                                         (int)timestamp.tv_sec,
                                         (int)timestamp.tv_usec);
                    }else{
                        jps.count++;
                        jps.len     = _length;
                        jps.movepix = _movepix;
                        jps.lapse   = _lapse;
                        memcpy(buffer, &jps, sizeof(jps));
                        lenhdr = sizeof jps;
                    }
                    by = _s.sendall(buffer, lenhdr);
                    if(by!=lenhdr)
                    {
                        std::cout << "SA3 ERROR " <<lenhdr <<", "<< _s.error() << "\n";
                        goto DONE;
                    }
                    by = _s.sendall(_frame, _length, 6000);
                    if(by!=_length)
                    {
                        std::cout << "SA4 ERROR sent:"<< by <<" != frame:" <<_length << ", " << _s.error() << "\n";
                        goto DONE;
                    }
                    _movepix = 0;
                    _lapse   = 0;
                    std::cout << "sent " << _length << "\n" ;

                }while(0);
                if(jps.format == LiFrmHdr::e_mpart_jpeg)
                {
                    ::sprintf(buffer, "\r\n--MY_BOUNDARY_STRING_NOONE_HAS\r\nContent-type: image/%s\r\n", "jpg");
                    by = _s.sendall(buffer,strlen(buffer),100);
                    if(by!=strlen(buffer))
                    {
                        std::cout << "SABOUND ERROR sent:"<< by <<" != frame:" <<_length << ", " << _s.error() << "\n";
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
    _length  = 0;
    _movepix = 0;
    _lapse   = 0;
    _s.destroy();
    std::cout << "socked close\r\n";
}



