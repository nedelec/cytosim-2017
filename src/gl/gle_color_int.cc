// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "gle_color_int.h"


char int2hex(int d)
{
    if ( d > 9 )
        return 'A' + (d-10);
    else
        return '0' + d;
}


std::string gle_color_int::str() const
{
#if ( 0 )
    std::ostringstream oss;
    oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << std::uppercase << col.rgba;
    return oss.str();
#else
    char str[16] = { 0 };
    str[0] = '0';
    str[1] = 'x';
    uint32_t n = rgba;
    for ( int c = 0; c < 8; ++c )
    {
        uint32_t d = ( n >> 28 );
        n <<= 4;
        str[c+2] = int2hex(d);
    }
    return str;
#endif
}

