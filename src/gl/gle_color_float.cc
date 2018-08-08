// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "gle_color_float.h"
#include <sstream>


std::string gle_color_float::str() const
{
    std::ostringstream oss;

    oss << "(";
    oss.precision(3);
    for ( int i = 0; i < 3; ++i )
        oss << c[i] << " ";
    oss << c[3] << ")";
    return oss.str();
}

