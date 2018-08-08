// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "fiber_disp.h"
#include "glossary.h"
#include "fiber_naked.h"
#include "sim.h"


//------------------------------------------------------------------------------

void FiberDisp::clear()
{
    visible          = 1;
    color            = 0xFFFFFFFF;
    coloring         = 0;

    line_style       = 1;
    line_width       = 2;
    
    point_style      = 0;
    point_size       = 5;

    end_style[0]     = 0;
    end_style[1]     = 0;

    end_size[0]      = 6;
    end_size[1]      = 6;
    
    end_section[0]   = 0;
    end_section[1]   = 0;
    
    end_color[0]     = 0xFFFFFFFF;  // white
    end_color[1]     = 0x00FF00FF;  // green
    end_color[2]     = 0xFFFF00FF;  // yellow
    end_color[3]     = 0xFF7538FF;  // orange
    end_color[4]     = 0xFF0000FF;  // red
    
    speckle_style    = 0;
    interval         = 1;
    
    exclude          = 0;
    right.set(1,0,0);
    
    mask             = 0;
    phase            = 0;

    rainbow          = 0.01;
    forces           = 0;
    forces_color     = 0xFF0000FF;
    
    explode          = 0;
    explode_range    = 0;
    show_average     = 0;
}


//------------------------------------------------------------------------------
void FiberDisp::read(Glossary& glos)
{
    glos.set(visible,          "visible", KeyList<int>("yes", 1, "no", 0, "transparent", -1));
    
    glos.set(color,            "color");
    glos.set(coloring,         "coloring");

    glos.set(line_width,       "line");
    glos.set(line_style,       "line", 1);
    glos.set(line_style,       "line_style");
    glos.set(line_width,       "line_width");

    glos.set(point_size,       "point");
    glos.set(point_style,      "point", 1);
    glos.set(point_style,      "point_style");
    glos.set(point_size,       "point_size");

    if ( glos.set(end_size[0], "plus_end") )
        end_style[0] = 2;
    glos.set(end_style[0],     "plus_end", 1);
    
    if ( glos.set(end_size[1], "minus_end") )
        end_style[1] = 3;
    glos.set(end_style[1],     "minus_end", 1);
    
    glos.set(end_style, 2,     "end_style");
    glos.set(end_size,  2,     "end_size");
    glos.set(end_section, 2,   "end_section");
    glos.set(end_color, 4,     "end_color");
    
    glos.set(speckle_style,    "speckle_style");
    glos.set(interval,         "interval");
    glos.set(speckle_style,    "speckles");
    glos.set(interval,         "speckles", 1);

    glos.set(exclude,          "exclude");
    glos.set(right,            "right");
    glos.set(right,            "exclude", 1);

    glos.set(mask,             "mask");
    glos.set(phase,            "phase");
    glos.set(phase,            "mask", 1);

    glos.set(rainbow,          "rainbow");
    glos.set(forces,           "forces");
    glos.set(forces_color,     "forces", 1);
    
    glos.set(explode,          "explode");
    glos.set(explode_range,    "explode", 1);
    
#ifdef BACKWARD_COMPATIBILITY
    if ( glos.set(explode_range, "display_shift") )
        explode = 2 * ( explode_range > 0 );
#endif
    
    glos.set(show_average,     "show_average");
}


//------------------------------------------------------------------------------

void FiberDisp::write_data(std::ostream & os) const
{
    write_param(os, "visible",      visible);
    write_param(os, "color",        color);
    write_param(os, "coloring",     coloring);
    
    write_param(os, "point",        point_size, point_style);
    write_param(os, "line",         line_width, line_style);
    write_param(os, "plus_end",     end_size[0], end_style[0]);
    write_param(os, "minus_end",    end_size[1], end_style[1]);
    write_param(os, "end_section",  end_section, 2);
    write_param(os, "end_color",    end_color, 5);
 
    write_param(os, "speckles",     speckle_style, interval);
    write_param(os, "exclude",      exclude, right);
    write_param(os, "mask",         mask, phase);

    write_param(os, "rainbow",      rainbow);
    write_param(os, "forces",       forces);
    write_param(os, "explode",      explode, explode_range);
    write_param(os, "show_average", show_average);
}

