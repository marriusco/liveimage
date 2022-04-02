#include "webcast.h"
#include "liconfig.h"


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
    eHOW            how = e_offline;
    char            scheme[8];
    uint16_t        port = 80;
    char            host[32];
    char            path[64];
    char            url[256];

    ::strcpy(url, GCFG->_glb.webcast.c_str());
    //First we need to parse the url (http[s]://host[:port][/[path]]) -- HTTPS not supported (yet?)
    int ret = parseURL(url, scheme,
                       sizeof(scheme), host, sizeof(host),
                       &port, path, sizeof(path));

    while(!this->OsThread::is_stopped() && __alive)
    {
        if(time(0)-ctime > 5)
        {
            ctime = time(0);
            cli.destroy();
            if(cli.create(80))
            {
                std::cout << "conn to " << host << "\r\n";
                if(0==cli.try_connect(host, port))
                {
                    std::cout << "conn to " << host << " failed\r\n";
                    cli.destroy();
                }
            }
            int k=50;
            while(k>0 && !cli.is_really_connected())
            {
                k--;
                msleep(100);
            }

            how = _get_mode(cli, host, path);
            cli.destroy();
        }
        if(how==e_udp){
            std::cout << "UDPING\r\n";
            _send_udp(host);
        }
        if(how==e_tcp){
            std::cout << "TCPING\r\n";
            _send_tcp(host);
        }
        how = e_offline;
    }
}


/*
GET / HTTP/1.1
Host: localhost:8000
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:98.0) Gecko/20100101 Firefox/98.0
Accept-Language: en-CA,en-US;q=0.7,en;q=0.3
Accept-Encoding: gzip, deflate
Connection: keep-alive
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: none
Sec-Fetch-User: ?1
*/


WebCast::eHOW WebCast::_get_mode(tcp_cli_sock& s, const char* host,
                                 const char* page)
{
    eHOW ehow = e_offline;
    std::string req = "GET "; req += page; req += " HTTP/1.1\r\n";
    req += "Host: "; req += host; req += "\r\n";
    req += "Connection: close\r\n\r\n";


    if(s.send(req.c_str(), req.length()) == req.length())
    {
        char resp[256];
        int bytes = s.select_receive(resp,sizeof(resp),8000);

        std::cout << req << "\r\n";
        if(bytes>0)
        {
            resp[bytes]='\0';
            std::cout << "GOT " << resp << "\r\n";
            char* dataStart = ::strstr(resp,"\r\n\r\n");
            if(dataStart)
            {
                const char* resp = dataStart+4;
                if(::strstr(resp,"tcp")){
                    _cport = atoi(strstr(resp,":")+1);
                    ehow = e_tcp;
                }
                else if(::strstr(resp,"udp")){
                    _cport = atoi(strstr(resp,":")+1);
                    ehow = e_udp;
                }
                if(::strstr(resp,"rtp")){
                    _cport = atoi(strstr(resp,":")+1);
                    ehow = e_rtp;
                }
            }
        }else{
            std::cout << "GOT 0 \r\n";
        }
    }
    return ehow;
}


void WebCast::_send_udp(const char* host)
{
    char        ok[32];
    udp_sock    s;
    int         by;
    size_t      l = 0;
    size_t      index = 0;
    SADDR_46    saddr(host, _cport);

    if(s.create(_cport))
    {
        if(s.connect(host, _cport))
            std::cout << "stream online \r\n";
        else
            std::cout << "stream offline \r\n";
        sleep(1);
        while(s.isopen())
        {

            do{
                AutoLock a(&_mut);
                l = _length;
            }while(0);
            if(l)
            {
                AutoLock a(&_mut);
                ::sprintf((char*)_frame,"%05d%05lu", index++, l);
                by = s.send(_frame, 1024);
                if(by>0)
                {
                    by = s.receive((unsigned char*)ok,sizeof(ok), saddr);
                    if(by>0)
                    {
                        ok[by]='\0';
                        std::cout<< ok <<"<-got\r\n";
                    }

                    else if(by==0)
                    {
                        break;
                    }
                }
            }
        }
    }
    s.destroy();
    std::cout << "END \r\n";
}


void WebCast::_send_tcp(const char* host)
{
    char            ok[32];
    tcp_cli_sock    s;
    int             by;
    size_t          l = 0;
    uint32_t        len;
    size_t          index = 0;
    char            options[20] = "cameraX";

    if(s.create(_cport))
    {
        if(s.try_connect(host, _cport))
            std::cout << "stream online \r\n";
        else
            std::cout << "stream offline \r\n";
        int k = 50;
        while(!s.is_really_connected() && k>0)
        {
            k--;
            msleep(100);
        }
        if(s.is_really_connected())
        {
            len  = sizeof(options);
            s.send((const uint8_t*)&len, (int)sizeof(len));
            s.send(options, len);

            while(by && s.isopen() && __alive)
            {
                std::cout << ".";
                std::cout.flush();
                by = 1;
                do{
                    AutoLock a(&_mut);
                    if(_length)
                    {
                        by = s.sendall(_frame, _length);
                        _length = 0;
                    }

                }while(0);
                if(by==0)
                {
                    std::cout << " stream broken";
                    s.destroy();
                }
                msleep(16);
            }
        }
    }
    s.destroy();
    std::cout << "END \r\n";
}



