// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "display_prop.h"
#include "glossary.h"
#include "dim.h"

//------------------------------------------------------------------------------
void DisplayProp::clear()
{
    tiled          = 1;
    fold           = 1;
    
    back_color     = 0x000000FF;
    front_color    = 0xFFFFFFFF;
    inner_color   = 0x333333FF;
    
    field_number   = 1;
    field_max      = 1;
    
    couple_select  = 7;
    single_select  = 3;
    
    point_value    = 0;
    point_size     = 4;
    line_width     = 2;
}

//------------------------------------------------------------------------------
void DisplayProp::read(Glossary& glos)
{
    glos.set(tiled,         "tile");
    glos.set(fold,          "tile", 1);
    glos.set(tiled,         "tiled");
    glos.set(fold,          "tiled", 1);
    glos.set(fold,          "fold");
    glos.set(back_color,    "background_color");
    glos.set(back_color,    "back_color");
    glos.set(front_color,   "front_color");
    glos.set(inner_color,  "inner_color");
    glos.set(field_number,  "field_number");
    glos.set(field_max,     "field_max");
    glos.set(couple_select, "couple_select");
    glos.set(single_select, "single_select");
    glos.set(point_value,   "point_value");
    glos.set(point_size,    "point_size");
    glos.set(line_width,    "line_width");
}


//------------------------------------------------------------------------------

void DisplayProp::write_data(std::ostream & os) const
{
    write_param(os, "tiled",         tiled, fold);
    write_param(os, "back_color",    back_color);
    write_param(os, "front_color",   front_color);
    write_param(os, "inner_color",  inner_color);
    write_param(os, "field_number",  field_number);
    write_param(os, "field_max",     field_max);
    write_param(os, "couple_select", couple_select);
    write_param(os, "single_select", single_select);
    write_param(os, "point_value",   point_value);
    write_param(os, "point_size",    point_size);
    write_param(os, "line_width",    line_width);
}


