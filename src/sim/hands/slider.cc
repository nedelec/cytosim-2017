// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "slider.h"
#include "slider_prop.h"
#include "glossary.h"
#include "exceptions.h"
#include "iowrapper.h"
#include "simul.h"
extern Random RNG;

//------------------------------------------------------------------------------

Slider::Slider(SliderProp const* p, HandMonitor* h)
: Hand(p,h), prop(p)
{
}


//------------------------------------------------------------------------------

void Slider::stepUnloaded()
{
    assert_true( attached() );
    
    // spontaneous detachment:
    if ( testDetachment() )
        return;
    
    /// diffusion?
}



void Slider::stepLoaded(Vector const& force)
{
    assert_true( attached() );
    
    // detachment depends on force:
    if ( testKramersDetachment(force.norm()) )
        return;

    //force-driven sliding along the fiber    
    moveBy( (force*dirFiber()) * prop->mobility_dt );
}



