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


Hand * SliderProp::newHand(HandMonitor* h) const
{
    return new Slider(this, h);
}


void SliderProp::clear()
{
    HandProp::clear();

    mobility = 0;
}


void SliderProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(mobility,    "mobility");
}


void SliderProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);

    if ( mobility < 0 )
        throw InvalidParameter("slider:mobility must be >= 0");

    /*
     Explicit
     */
    
    mobility_dt = sp->time_step * mobility;
    
    if ( sp->strict && mobility <= 0 )
        std::clog << "WARNING: slider `" << name() << "' will not slide because mobility=0\n";
}


void SliderProp::checkStiffness(real stiff, real len, real mul, real kT) const
{
    HandProp::checkStiffness(stiff, len, mul, kT);
    
    /*
     Estimate numerical stability from mobility and stiffness
     */
    real e = mobility_dt * stiff * mul;
    if ( e > 0.5 )
    {
        std::ostringstream oss;
        oss << "Slider `" << name() << "' may be unstable:\n";
        oss << PREF << "time_step * mobility * stiffness = " << e << "\n";
        oss << PREF << "reduce time_step\n";
        throw InvalidParameter(oss.str());
    }
}


void SliderProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "mobility",  mobility);
}

