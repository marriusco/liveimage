#ifndef WEBCAST_H
#define WEBCAST_H

#include "os.h"
#include "sock.h"

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
    void stream_frame(uint8_t* pjpg, size_t length);

private:
    void _send_tcp(const char* host);
    void _send_udp(const char* host);
    eHOW _get_mode(tcp_cli_sock&,
                   const char* host,
                   const char* page);

private:

    mutex           _mut;
    uint8_t*        _frame  = nullptr;
    size_t          _length = 0;
    bool            _headered = false;
    std::string     _upload_page;
    time_t          _last_clicheck=0;
    int             _cport = 0;
    int             _offset = 0;
};

inline int parseURL(const char* url, char* scheme, size_t
                    maxSchemeLen, char* host, size_t maxHostLen,
                    uint16_t* port, char* path, size_t maxPathLen) //Parse URL
{
  char* schemePtr = (char*) url;
  char* hostPtr = (char*) strstr(url, "://");
  if(hostPtr == NULL)
  {
    printf("Could not find host");
    return 0; //URL is invalid
  }

  if( maxSchemeLen < hostPtr - schemePtr + 1 ) //including NULL-terminating char
  {
    printf("Scheme str is too small (%lu >= %d)", maxSchemeLen,
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
    if( sscanf(portPtr, "%d", &port) != 1)
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
    printf("Host str is too small (%d >= %d)", maxHostLen, hostLen + 1);
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
    pathLen = strlen(pathPtr);
  }

  if( maxPathLen < pathLen + 1 ) //including NULL-terminating char
  {
    printf("Path str is too small (%d >= %d)", maxPathLen, pathLen + 1);
    return 0;
  }
  memcpy(path, pathPtr, pathLen);
  path[pathLen] = '\0';

  return 1;
}



#endif // WEBCAST_H
