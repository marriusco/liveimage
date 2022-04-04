#include "webcast.h"
#include "liconfig.h"

struct __attribute__((__packed__)) JpegSep{
    uint32_t    len;
    uint32_t    confirm;
    uint32_t    magic;
    uint32_t    count;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
#define TIME_FOR_CLI    5
#define BOUNDARY        "----212367691630846570233068106084"
extern bool __alive;
/////////////////////////////////////////////////////////////////////////////////////////////////
WebCast::WebCast()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////
WebCast::~WebCast()
{
    this->stop_thread();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WebCast::stream_frame(uint8_t* pjpg, size_t length)
{
    AutoLock a(&_mut);

    if(_frame==nullptr){
        _frame = new uint8_t[length * 2];
    }
    _length = length;
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
    if(GCFG->_glb.checkcast < 5){GCFG->_glb.checkcast=5;}
    while(!this->OsThread::is_stopped() && __alive)
    {
        if(time(0)-ctime > GCFG->_glb.checkcast && _length)
        {
            _send_tcp(host, port);
            ctime = time(0);
        }
        msleep(1000);
    }
}

void WebCast::_send_tcp(const char* host , int port)
{
    tcp_cli_sock    s;
    int             by;
    uint32_t        len;
    bool            sendonreq = true;
    size_t          index = 0;
    char            options[20] = "cameraX";
    int             webms = GCFG->_glb.webms;
    bool            framed=false;

    if(s.create(_cport))
    {
        if(s.try_connect(host, port)){
            std::cout << "cam stream online \r\n";
        }
        else{
            std::cout << "cam stream offline \r\n";
            s.destroy();
        }
        if(s.isopen())
        {
            len  = sizeof(options);
            s.send((const uint8_t*)&len, (int)sizeof(len));
            by = s.send(options, len);
            std::cout << "options\r\n";

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

                        JpegSep jps;
                        jps.count=index++;
                        jps.magic=0x5a5a5a5a;
                        jps.len=_length;
                        jps.confirm = webms==1;
                        by = s.sendall((const uint8_t*)&jps, sizeof(jps));
                        by += s.sendall(_frame, _length);
            framed = by>0;
                        _length = 0;
                    }

                }while(0);
                if(by==0)
                {
                    std::cout << " stream broken";
                    s.destroy();
                }
                else if(webms==1)
                {
                    if(framed){
                        JpegSep jps;
                        if(sizeof(jps)==s.select_receive((uint8_t*)&jps, sizeof(jps), 5000))
                        {
                            std::cout << " frame confirmed";
                            sendonreq = true;
                        }
                        else{
                s.destroy();
                std::cout << "socked closed ont server\r\n";
                            sendonreq = false;
                }
                   }
                }
                msleep(16+webms);
            }
        }
    }
    s.destroy();
    std::cout << "socked close\r\n";
}



