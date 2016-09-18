// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "fork_prop.h"
#include "fork.h"

//------------------------------------------------------------------------------
Couple * ForkProp::newCouple(Glossary *) const
{
    //std::cout << "ActorProp::newHand" << std::endl;
    return new Fork(this);
}

//------------------------------------------------------------------------------
void ForkProp::clear()
{
    CoupleProp::clear();
    trans_activated = 0;
}

//------------------------------------------------------------------------------
void ForkProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
    
    glos.set(trans_activated, "trans_activated");
}

//------------------------------------------------------------------------------
void ForkProp::complete(SimulProp const* sp, PropertyList* plist)
{
    CoupleProp::complete(sp, plist);
    
}

//------------------------------------------------------------------------------

void ForkProp::write_data(std::ostream & os) const
{
    CoupleProp::write_data(os);
    write_param(os, "trans_activated", trans_activated);
}

