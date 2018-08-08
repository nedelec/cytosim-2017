// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "actor.h"
#include "actor_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
extern Random RNG;

//------------------------------------------------------------------------------

Actor::Actor(ActorProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}


//------------------------------------------------------------------------------

void Actor::stepUnloaded()
{
    assert_true( attached() );

    // test for detachment
    if ( testDetachment() )
        return;
}


void Actor::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    if ( testKramersDetachment(force.norm()) )
        return;

    // do something:
}



