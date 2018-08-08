// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bead_prop.h"
#include "glossary.h"
#include "property_list.h"
#include "simul_prop.h"
#include "sim.h"

void BeadProp::clear()
{
    viscosity         = -1;
    steric            = 0;
    
    confine           = CONFINE_NOT;
    confine_stiff     = 0;
    confine_space     = "first";
    confine_space_ptr = 0;
    
    display           = "";
}


void BeadProp::read(Glossary& glos)
{
    glos.set(steric,         "steric");
    glos.set(viscosity,      "viscosity");
    
    glos.set(confine,        "confine",
             KeyList<Confinement>("none",       CONFINE_NOT,
                                  "inside",     CONFINE_INSIDE,
                                  "center",     CONFINE_INSIDE,
                                  "outside",    CONFINE_OUTSIDE,
                                  "all_inside", CONFINE_ALL_INSIDE,
                                  "surface",    CONFINE_SURFACE));
    glos.set(confine_stiff,  "confine", 1);
    glos.set(confine_space,  "confine", 2);

#ifdef BACKWARD_COMPATIBILITY
    if ( confine_space == "current" )
        confine_space = "last";

    glos.set(confine,           "confined",
             KeyList<Confinement>("none",    CONFINE_NOT,
                                  "inside",  CONFINE_INSIDE,
                                  "surface", CONFINE_SURFACE));
    glos.set(confine_stiff,     "confined", 1);
#endif
    
    glos.set(display,        "display");
}


void BeadProp::complete(SimulProp const* sp, PropertyList* plist)
{
    if ( viscosity < 0 )
        viscosity = sp->viscosity;
    
    if ( viscosity < 0 )
        throw InvalidParameter("bead:viscosity or simul:viscosity should be defined");
    
    confine_space_ptr = sp->simul->findSpace(confine_space);
    
    if ( confine && confine_stiff < 0 )
        throw InvalidParameter("solid:confine[1] (stiffness value) must be specified and >= 0");
}


void BeadProp::write_data(std::ostream & os) const
{
    write_param(os, "viscosity", viscosity);
    write_param(os, "steric",    steric);
    write_param(os, "confine",   confine, confine_stiff, confine_space);
    write_param(os, "display",   "("+display+")");
}

