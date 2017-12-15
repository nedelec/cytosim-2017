// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "mighty.h"
#include "mighty_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
extern Random RNG;

//------------------------------------------------------------------------------

Mighty::Mighty(MightyProp const* p, HandMonitor* h)
: Hand(p, h), prop(p)
{
}


//------------------------------------------------------------------------------

void Mighty::stepUnloaded()
{
    assert_true( attached() );
    
    // detach or move
    if ( testDetachment() )
        return;
    
    moveBy(prop->max_speed_dt);
}



void Mighty::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    
    // the load is the projection of the force on the local direction of Fiber
    real load = force * dirFiber();
    
    // calculate load-dependent displacement:
    real dabs = prop->max_speed_dt + load * prop->var_speed_dt;
    
    // possibly limit the range of the speed:
    if ( prop->limit_speed )
    {
        if ( dabs < prop->min_dabs )
            dabs = prop->min_dabs;
        
        else if ( dabs > prop->max_dabs )
            dabs = prop->max_dabs;
    }

    /*
     The probability to detach has two contributions:
     - Kramers' theory  rate0 * exp( force / f0 )
     - movement-induced detachment
     */
    real det = prop->unbinding_rate_dt * exp(force.norm()*prop->unbinding_force_inv)
             + prop->unbinding_density * fabs(dabs);
    
    nextDetach -= det;
    if ( nextDetach < 0 )
    {
        nextDetach = RNG.exponential();
        detach();
        return;
    }
    
    moveBy(dabs);
}


