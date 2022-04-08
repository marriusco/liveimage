#ifndef WEBCAST_H
#define WEBCAST_H

#include "os.h"
#include "sock.h"
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
    uint16_t    wh[2];
    uint8_t     lapse:1;
    uint8_t     insync:1;
    uint8_t     record:2;
    e_format    format:4;
    char        command[16];
    char        camname[16];
}PACK_ALIGN_1;



class WebCast : public OsThread
{
    enum eHOW{
        e_offline,
        e_tcp,
        e_udp,
        e_rtp,
    };

public:

    WebCast();
    virtual ~WebCast();
    virtual void thread_main();
    void stream_frame(uint8_t* pjpg, size_t length, int mp, int w, int h);
    void kill();
private:
    void _send_tcp(const char* host, const char* camname, int port);

private:

    mutex           _mut;
    uint8_t*        _frame  = nullptr;
    bool            _headered = false;
    time_t          _last_clicheck=0;
    int             _cport = 0;
    sockserver*     _punched = nullptr;
    tcp_cli_sock    _s;
    LiFrmHdr        _jps;
    size_t          _buffsz=0;
};

inline int parseURL(const char* url, char* scheme, size_t
                    maxSchemeLen, char* host, size_t maxHostLen,
                    int* port, char* path, size_t maxPathLen) //Parse URL
{
    (void)maxPathLen;
    char* schemePtr = (char*) url;
    char* hostPtr = (char*) strstr(url, "://");
    if(hostPtr == NULL)
    {
        printf("Could not find host");
        return 0; //URL is invalid
    }

    if( maxSchemeLen < (size_t)(hostPtr - schemePtr + 1 )) //including NULL-terminating char
    {
        printf("Scheme str is too small (%lu >= %lu)", maxSchemeLen,
               hostPtr - schemePtr + 1);
        return 0;
    }
    memcpy(scheme, schemePtr, hostPtr - schemePtr);
    scheme[hostPtr - schemePtr] = '\0';

    hostPtr+=3;

    size_t hostLen = 0;

    char* portPtr = strchr(hostPtr, ':');
    if( portPtr != NULL )
    {
        hostLen = portPtr - hostPtr;
        portPtr++;
        if( sscanf(portPtr, "%d", port) != 1)
        {
            printf("Could not find port");
            return 0;
        }
    }
    else
    {
        *port=80;
    }
    char* pathPtr = strchr(hostPtr, '/');
    if( hostLen == 0 )
    {
        hostLen = pathPtr - hostPtr;
    }

    if( maxHostLen < hostLen + 1 ) //including NULL-terminating char
    {
        printf("Host str is too small (%lu >= %lu)", maxHostLen, hostLen + 1);
        return 0;
    }
    memcpy(host, hostPtr, hostLen);
    host[hostLen] = '\0';

    size_t pathLen;
    char* fragmentPtr = strchr(hostPtr, '#');
    if(fragmentPtr != NULL)
    {
        pathLen = fragmentPtr - pathPtr;
    }
    else
    {
        if(pathPtr)
            pathLen = strlen(pathPtr);
        else
            pathLen=0;
    }

    if(pathPtr)
    {
        memcpy(path, pathPtr, pathLen);
        path[pathLen] = '\0';
    }
    else
    {
        path[0]=0;
    }

    return 1;
}



#endif // WEBCAST_H
