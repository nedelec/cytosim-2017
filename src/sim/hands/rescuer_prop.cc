// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "property_list.h"
#include "simul_prop.h"
#include "rescuer_prop.h"
#include "rescuer.h"

//------------------------------------------------------------------------------
Hand * RescuerProp::newHand(HandMonitor* h) const
{
    return new Rescuer(this, h);
}

//------------------------------------------------------------------------------
void RescuerProp::clear()
{
    HandProp::clear();

    rescue_prob = 0;
}

//------------------------------------------------------------------------------
void RescuerProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(rescue_prob,  "rescue_probability");
    glos.set(rescue_prob,  "rescue_prob");
}

//------------------------------------------------------------------------------
void RescuerProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
    
    if ( rescue_prob < 0 )
        throw InvalidParameter("rescuer:rescue_prob must be >= 0");
}



//------------------------------------------------------------------------------

void RescuerProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "rescue_prob", rescue_prob);
}

