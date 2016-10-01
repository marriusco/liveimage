#ifndef OUTFILEFMT_H
#define OUTFILEFMT_H

#include <inttypes.h>
#include <stdio.h>

class outfilefmt
{
public:
    outfilefmt(){};
    virtual ~outfilefmt(){};
    virtual uint32_t convert420(const uint8_t* fmt420, int w,int h, int isize, int quality, uint8_t** ppng)=0;
protected:

private:
};


#define DERR() "Error: " << errno << "\n";

#endif // OUTFILEFMT_H
