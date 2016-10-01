/*

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/

    Author:  Marius O. Chincisan
    First Release: September 16 - 29 2016
*/

#include <iostream>
#include "pnger.h"

static void _png_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
static void _png_flush(png_structp png_ptr);

pnger::pnger(int quality)
{
    memset(&_png,0,sizeof(_png));
}

pnger::~pnger()
{
    delete[] _png.buffer;
}

uint32_t pnger::convert420(const uint8_t* fmt420, int w,int h, int isize, int quality, uint8_t** ppng)
{
    int         code=0;
    float       R,G,B;
    png_structp png_ptr = NULL;
    png_infop   info_ptr = NULL;
    register    png_bytep row = NULL;
    int         line, column;
    register uint8_t Y, U, V;
    register uint8_t *base_py = (uint8_t *)fmt420;
    register uint8_t *base_pu = (uint8_t *)fmt420+(h*w);
    register uint8_t *base_pv = (uint8_t *)fmt420+(h*w)+(h*w)/4;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        std::cerr <<  "png_create_write_struct" << DERR();
        code = 1;
        goto DONE;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        std::cerr <<  "png_create_info_struct" << DERR();
        code = 1;
        goto DONE;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        std::cerr <<  "setjmp" << DERR();
        code = 1;
        goto DONE;
    }

    if(_png.isize == 0)
    {
        _png.isize = w * (h+1) * 3;
        _png.buffer = new char [_png.isize];
        if(_png.buffer == 0)
        {
            std::cerr <<  "out of memory" << DERR();
            return 0;
        }
    }
    _png.accum = 0;

    png_set_write_fn(png_ptr, &_png, _png_write_data, _png_flush);
    png_set_IHDR(png_ptr, info_ptr, w, h,
                 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    row = (png_bytep) malloc(3 * w * sizeof(png_byte));

    /// this is yuv420 ro RGB -> png
    for (line = 0; line < h; ++line)
    {
        png_bytep prow = row;
        for (column = 0; column < w; ++column)
        {
            Y = *(base_py+(line*w)+column);
            U = *(base_pu+(line/2*w/2)+column/2);
            V = *(base_pv+(line/2*w/2)+column/2);
            B = 1.164*(Y - 16)                   + 2.018*(U - 128);
            G = 1.164*(Y - 16) - 0.813*(V - 128) - 0.391*(U - 128);
            R = 1.164*(Y - 16) + 1.596*(V - 128);
            if (R < 0){ R = 0; } if (G < 0){ G = 0; } if (B < 0){ B = 0; }
            if (R > 255 ){ R = 255; } if (G > 255) { G = 255; } if (B > 255) { B = 255; }
            *prow++ = (uint8_t)R;
            *prow++ = (uint8_t)G;
            *prow++ = (uint8_t)B;
        }
        png_write_row(png_ptr, row);
    }
    png_write_end(png_ptr, NULL);
DONE:
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL) free(row);
    if(code==0)
        *ppng = (uint8_t*)_png.buffer;
    return _png.accum;
}

void _png_flush(png_structp png_ptr)
{
}

static void _png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct pngbuffer* p=(struct pngbuffer*)png_get_io_ptr(png_ptr);

    if(p->accum + length > p->isize)
    {
        p->isize = p->accum + length + 4096;
        char* nb =  new char [p->isize];
        ::memcpy(nb, p->buffer, p->accum);
        char* ob = p->buffer;
        delete[] ob;
        p->buffer = nb;
    }
    memcpy(p->buffer + p->accum, data, length);
    p->accum += length;
}
