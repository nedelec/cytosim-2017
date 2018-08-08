// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "sphere_prop.h"
#include "glossary.h"
#include "sphere.h"
#include "space.h"
#include "sim.h"

#include "sphere.h"
#include "property_list.h"
#include "simul_prop.h"
#include "space_prop.h"


void SphereProp::clear()
{
    point_mobility    = -1;
    viscosity         = -1;
    piston_effect     = false;
    steric            = 0;
    
    confine           = CONFINE_NOT;
    confine_stiff     = -1;
    confine_space     = "first";
    confine_space_ptr = 0;
    
    display           = "";
}


void SphereProp::read(Glossary& glos)
{
    glos.set(point_mobility,  "point_mobility");
    glos.set(piston_effect,   "piston_effect");
    glos.set(viscosity,       "viscosity");
    
    glos.set(steric,          "steric");
 
    glos.set(confine,         "confine", 
             KeyList<Confinement>("none",       CONFINE_NOT,
                                  "inside",     CONFINE_INSIDE,
                                  "all_inside", CONFINE_ALL_INSIDE,
                                  "surface",    CONFINE_SURFACE));
    glos.set(confine_stiff,   "confine", 1);
    glos.set(confine_space,   "confine", 2);

#ifdef BACKWARD_COMPATIBILITY
    if ( confine_space == "current" )
        confine_space = "last";

    glos.set(confine,         "confined",
             KeyList<Confinement>("none",    CONFINE_NOT,
                                  "inside",  CONFINE_INSIDE,
                                  "surface", CONFINE_SURFACE));
    glos.set(confine_stiff,   "confined", 1);
#endif
    
    glos.set(display, "display");
}


void SphereProp::complete(SimulProp const* sp, PropertyList* plist)
{
    if ( viscosity < 0 )
        viscosity = sp->viscosity;
        
    if ( viscosity < 0 )
        throw InvalidParameter("sphere:viscosity or simul:viscosity should be defined");
    
    confine_space_ptr = sp->simul->findSpace(confine_space);
    
    if ( point_mobility < 0 )
        throw InvalidParameter("sphere:point_mobility must be specified and >= 0");
    
    if ( confine && confine_stiff < 0 )
        throw InvalidParameter("The stiffness sphere:confine[1] must be specified and >= 0");
}


void SphereProp::write_data(std::ostream & os) const
{
    write_param(os, "viscosity",      viscosity);
    write_param(os, "point_mobility", point_mobility);
    write_param(os, "piston_effect",  piston_effect);
    write_param(os, "steric",         steric);
    write_param(os, "confine",        confine, confine_stiff, confine_space);    
    write_param(os, "display",        "("+display+")");
}

