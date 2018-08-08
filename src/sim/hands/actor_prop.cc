// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "actor_prop.h"
#include "actor.h"

//------------------------------------------------------------------------------
Hand * ActorProp::newHand(HandMonitor* h) const
{
    return new Actor(this, h);
}

//------------------------------------------------------------------------------
void ActorProp::clear()
{
    HandProp::clear();
    rate = 0;
}

//------------------------------------------------------------------------------
void ActorProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(rate,  "rate");
}

//------------------------------------------------------------------------------
void ActorProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
    
}

//------------------------------------------------------------------------------

void ActorProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "rate", rate);
}

