// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "slider_prop.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "sim.h"
#include <cmath>
#include "property_list.h"
#include "simul_prop.h"
#include "slider.h"

//------------------------------------------------------------------------------
Hand * SliderProp::newHand(HandMonitor* h) const
{
    return new Slider(this, h);
}

//------------------------------------------------------------------------------
void SliderProp::clear()
{
    HandProp::clear();

    mobility = 0;
    stiffness = -1;
}

//------------------------------------------------------------------------------
void SliderProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(mobility,    "mobility");
    glos.set(stiffness,   "stiffness");
}

//------------------------------------------------------------------------------
void SliderProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
    
#if ( 1 )
    
    /*
     Explicit
     */
    if ( mobility < 0 )
        throw InvalidParameter("slider:mobility must be >= 0");
    
    mobility_dt = sp->time_step * mobility;

#else
    
    /* 
     We devise an implicit integration approach, but there are pitfalls:
     - this is done while assuming that all other elements of the simulation are static
     - this assumes that the link is Hookean of zero resting length:
     force = stiffness * offset
     However, this is true only if the Slider is part of a Single or a plain Couple.
     This does not hold in particular for any of the non-zero resting length Couple or Single.
     J. Ward found that in this case, the numerical precision is not improved compared to
     the explicit integration above.
     */
 
    mobility_dt = sp->time_step * mobility / ( 1 + sp->time_step * mobility * stiffness );
    
#endif
    
    
    if ( stiffness > 0 )
    {
        std::cerr << "WARNING: The mobility of the slider was maximized" << std::endl;
        mobility_dt = 1.0 / stiffness;
        mobility = 1.0 / ( stiffness * sp->time_step );
    }
}


//------------------------------------------------------------------------------

void SliderProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "mobility",  mobility);
    write_param(os, "stiffness", stiffness);
}

