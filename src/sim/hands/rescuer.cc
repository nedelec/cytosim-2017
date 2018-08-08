// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "rescuer.h"
#include "rescuer_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
extern Random RNG;

//------------------------------------------------------------------------------

Rescuer::Rescuer(RescuerProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}

//------------------------------------------------------------------------------
void Rescuer::handleOutOfRange(FiberEnd end)
{
    assert_true( attached() );
    assert_true( fbAbs < fbFiber->abscissaM() ||  fbAbs > fbFiber->abscissaP() );
    
    if ( fiber()->isShrinking(end) )
    {
        if ( RNG.test(prop->rescue_prob) )
        {
            // this is necessary to avoid an invalid position:
            moveToEnd(end);
            // induce rescue:
            fiber()->setDynamicState(end, STATE_GREEN);
        }
        else
            detach();
    }
    else
    {
        if ( prop->hold_growing_end )
            moveToEnd(end);
        else
            detach();
    }
}



//------------------------------------------------------------------------------

void Rescuer::stepUnloaded()
{
    assert_true( attached() );
    
    testDetachment();
}


void Rescuer::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    testKramersDetachment(force.norm());
}



