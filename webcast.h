#ifndef WEBCAST_H
#define WEBCAST_H

#include "os.h"
#include "sock.h"
#include "sockserver.h"

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
    void stream_frame(uint8_t* pjpg, size_t length, int);
    void kill();
private:
    void _send_tcp(const char* host, const char* camname, int port);

private:

    mutex           _mut;
    uint8_t*        _frame  = nullptr;
    int             _length = 0;
    bool            _headered = false;
    std::string     _upload_page;
    time_t          _last_clicheck=0;
    int             _cport = 0;
    int             _offset = 0;
    int             _movepix = 0;
    bool            _lapse = false;
    sockserver*     _punched = nullptr;
    tcp_cli_sock    _s;
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
