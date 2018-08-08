// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "sim.h"
#include "exceptions.h"
#include "glossary.h"
#include "key_list.h"
#include "common.h"
#include "property_list.h"
#include "simul_prop.h"
#include "nucleator_prop.h"
#include "nucleator.h"
#include "simul.h"

//------------------------------------------------------------------------------
Hand * NucleatorProp::newHand(HandMonitor* h) const
{
    return new Nucleator(this, h);
}

//------------------------------------------------------------------------------
void NucleatorProp::clear()
{
    HandProp::clear();

    fiber.clear();
    rate        = 0;
    spec        = "";
    specificity = NUCLEATE_ORIENTATED;
    track_end   = MINUS_END;
    addictive   = false;
}

//------------------------------------------------------------------------------
void NucleatorProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(rate,  "nucleation_rate");
    glos.set(fiber, "nucleated_fiber");
    glos.set(spec,  "nucleation_spec");
    glos.set(spec,  "nucleate_spec");
    
    
    glos.set(rate,  "nucleate", 0);
    glos.set(fiber, "nucleate", 1);
    glos.set(spec,  "nucleate", 2);
    
    glos.set(addictive,  "addictive");
    
    
    glos.set(track_end,  "track_end",
             KeyList<FiberEnd>("plus_end",    PLUS_END,
                               "minus_end",   MINUS_END,
                               "none",        NOT_END,
                               "nearest_end", NEAREST_END));
    
    glos.set(specificity, "specificity",
             KeyList<Specificity>("none",         NUCLEATE_ORIENTATED,
                                  "parallel",     NUCLEATE_PARALLEL,
                                  "antiparallel", NUCLEATE_ANTIPARALLEL));
}

//------------------------------------------------------------------------------
void NucleatorProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
    
    simul = sp->simul;
    
    if ( fiber.empty() )
        throw InvalidParameter("hand:nucleate[1] (fiber) must be specified if activity=nucleate");
    
    if ( rate < 0 )
        throw InvalidParameter("hand:nucleate (rate) must be positive");

    rate_dt = rate * sp->time_step;
}


//------------------------------------------------------------------------------

void NucleatorProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "nucleate",        rate, fiber, "("+spec+")");
    write_param(os, "track_end",       track_end);
    write_param(os, "addictive",       addictive);
}

