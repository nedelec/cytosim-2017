// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "line_disp.h"


void LineDisp::clear()
{
    visible       = 1;
    color         = 0xFFFFFFFF;
    explode_shift.set(0,0,0);
    
    end_color[0]  = 0xFFFFFFFF;
    end_color[1]  = 0xFFFFFFFF;
}

