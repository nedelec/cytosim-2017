// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
// Cytosim 3.0 -  Copyright Francois Nedelec et al.  EMBL 2007-2013

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "fork_prop.h"
#include "fork.h"


Couple * ForkProp::newCouple(Glossary*) const
{
    //std::clog << "ForkProp::newHand" << std::endl;
    return new Fork(this);
}


void ForkProp::clear()
{
    CoupleProp::clear();
    trans_activated = 0;
    angle   = 0;
    cosinus = 1;
    sinus   = 0;
    angular_stiffness = 0;
    flip    = true;
}


void ForkProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
    
    glos.set(trans_activated,   "trans_activated");

    // compact syntax
    glos.set(angular_stiffness, "torque");
    glos.set(angle,             "torque", 1);
    
    // alternative syntax:
    glos.set(angle,             "angle");
    glos.set(angular_stiffness, "angular_stiffness");
    
    glos.set(flip, "flip");
}


void ForkProp::complete(SimulProp const* sp, PropertyList* plist)
{
    CoupleProp::complete(sp, plist);
    
    cosinus = cos(angle);
    sinus   = sin(angle);
    
    if ( angle < 0 || sinus < 0 )
        throw InvalidParameter("The equilibrium angle should be defined in [0, pi]");
    
    if ( angular_stiffness < 0 )
        throw InvalidParameter("The angular stiffness, fork:torque[0] should be set and >= 0");
}



void ForkProp::write_data(std::ostream & os) const
{
    CoupleProp::write_data(os);
    write_param(os, "trans_activated", trans_activated);
    write_param(os, "torque", angular_stiffness, angle);
    write_param(os, "flip", flip);
}
