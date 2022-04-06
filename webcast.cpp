#include "webcast.h"
#include "liconfig.h"

#define    JPEG_MAGIC       0x12345678

#define         PTRU_TOUT  30
#define          PACK_ALIGN_1   __attribute__((packed, aligned(1)))


struct  LiFrmHdr{
    uint32_t    len;
    uint32_t    magic;
    uint32_t    mac;
    uint32_t    key;
    uint32_t    count;
    uint32_t    movepix;
    uint16_t    conport;
    uint8_t     lapse:1;
    uint8_t     confirm:1;
    uint8_t     record:6;
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
        _frame = new uint8_t[length * 2];
    }
    _length = length;
    if(_movepix==0 && movepix){
        std::cout << "move " << _movepix << "\r\n";
        _movepix = movepix;
    }
    _lapse = 0;
    ::memcpy(_frame, pjpg, length);
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
        if((time(0)-ctime > GCFG->_glb.checkcast && _length) || _movepix>0)
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

void WebCast::_send_tcp(const char* host, const char* camname, int port)
{
    tcp_cli_sock    s;
    int             by;
    bool            sendonreq = true;
    size_t          index = 0;
    int             webms = GCFG->_glb.webms;
    bool            framed=false;
    LiFrmHdr         jps;

    if(s.create(_cport))
    {
        if(s.try_connect(host, port)){
            std::cout << "cam stream online \r\n";
        }
        else{
            std::cout << "cam stream offline \r\n";
            s.destroy();
        }
        for(int k=0; k < 100 && s.is_really_connected()==false;k++)
        {
            msleep(100);
        }
        if(s.is_really_connected())
        {
            std::cout << "cam stream really connected \r\n";

            ::strncpy(jps.camname, camname, sizeof(jps.camname));
            do{
                AutoLock a(&_mut);

                std::cout << "init conn with movepix " << _movepix << "\r\n";
                jps.magic = JPEG_MAGIC;
                jps.key = GCFG->_glb.webpass;
                jps.record = GCFG->_glb.record & 0x3;
                jps.movepix = _movepix;
                jps.lapse = 0;
                _movepix = 0;
            }while(0);
            uint32_t len = sizeof(jps);
            by=s.sendall((const uint8_t*)&len, (int)sizeof(uint32_t));
            if(by!=sizeof(uint32_t))
            {
                goto DONE;
            }

            by=s.sendall((const uint8_t*)&jps, (int)sizeof(jps));
            if(by!=sizeof(jps))
            {
                goto DONE;
            }
            if(sizeof(jps) != s.receive(( uint8_t*)&jps, (int)sizeof(jps)))
            {
                goto DONE;
            }
            std::cout << "got connected port:" << jps.conport << "\r\n";
            if(GCFG->_glb.punchtru && _punched==nullptr)
            {
                s.destroy();
                std::cout << "punchtru  " << GCFG->_glb.punchtru << "\r\n";
                _punched = new sockserver(jps.conport, "http");
                if(_punched && _punched->listen()==false)
                {
                    delete _punched;
                    _punched = nullptr;
                }
            }

            if(GCFG->_glb.punchtru && _punched)
            {

                std::cout << "punching \r\n";
                time_t now = time(0);
                while(__alive && time(0)-now < PTRU_TOUT){
                    if(_punched->spin())
                    {
                        now = time(0);
                    }
                    msleep(32);
                }
                delete _punched;
                _punched = nullptr;
                std::cout << "punching done\r\n";
                return;
            }

            sendonreq = true;
            while(by && s.isopen() && __alive)
            {
                by = 1;
                do{
                    AutoLock a(&_mut);
                    if(_length && sendonreq)
                    {
                        std::cout << ".";
                        std::cout.flush();

                        jps.count = index++;
                        jps.magic = JPEG_MAGIC;
                        jps.len = _length;
                        jps.confirm = webms==1;
                        jps.movepix = _movepix;
                        jps.lapse = _lapse;
                        jps.movepix = 0;


                        by =  s.sendall((const uint8_t*)&jps, sizeof(jps));
                        if(by!=sizeof(jps)){
                            std::cout << " stream broken";
                            goto DONE;
                        }
                        by = s.sendall(_frame, _length);
                        framed = by>0;
                        _length = 0;
                        _movepix = 0;
                    }

                }while(0);
                if(by==0)
                {
                    std::cout << " stream broken";
                    goto DONE;
                }
                else if(webms==1)
                {
                    if(framed){
                        LiFrmHdr jps;
                        if(sizeof(jps)==s.select_receive((uint8_t*)&jps, sizeof(jps), 5000))
                        {
                            std::cout << " frame confirmed";
                            sendonreq = true;
                        }
                        else{
                            std::cout << "socked closed ont server\r\n";
                            sendonreq = false;
                            goto DONE;
                        }
                    }
                }
                msleep(16+webms);
            }
        }
    }
DONE:
    s.destroy();
    std::cout << "socked close\r\n";
}



