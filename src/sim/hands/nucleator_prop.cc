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

    nucleated_fiber.clear();
    nucleated_fiber_prop = 0;
    nucleated_fiber_set = 0;
    nucleation_rate = 0;
    nucleation_spec = "";
    specificity     = NUCLEATE_ORIENTATED;
    track_end       = MINUS_END;
    addictive       = false;
}

//------------------------------------------------------------------------------
void NucleatorProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(nucleation_rate, "nucleation_rate");
    glos.set(nucleated_fiber, "nucleated_fiber");
    glos.set(nucleation_spec, "nucleation_spec");
    
    
    glos.set(nucleation_rate, "nucleate", 0);
    glos.set(nucleated_fiber, "nucleate", 1);
    glos.set(nucleation_spec, "nucleate", 2);
    
    glos.set(addictive,       "addictive");
    
    
    glos.set(track_end,  "track_end",
             KeyList<FiberEnd>("plus_end",    PLUS_END,
                               "minus_end",   MINUS_END,
                               "none",        NOT_END,
                               "nearest_end", NEAREST_END));
    
    glos.set(specificity, "specificity",
             KeyList<Specificity>("none",         NUCLEATE_ORIENTATED,
                                  "parallel",     NUCLEATE_PARALLEL,
                                  "parallel_if",  NUCLEATE_PARALLEL_IF,
#ifdef BACKWARD_COMPATIBILITY
                                  "circular",     NUCLEATE_ORIENTATED,
#endif
                                  "antiparallel", NUCLEATE_ANTIPARALLEL));
}

//------------------------------------------------------------------------------
void NucleatorProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);

    nucleated_fiber_set = &sp->simul->fibers;
    
    if ( nucleated_fiber.empty() )
        throw InvalidParameter("hand:nucleate[1] (=nucleated_fiber) must be specified if activity=nucleate");
    
    nucleated_fiber_prop = static_cast<FiberProp*>(plist->find("fiber", nucleated_fiber, true));
    
    if ( nucleation_rate < 0 )
        throw InvalidParameter("hand:nucleate (=nucleation_rate) must be positive");

    nucleation_rate_dt = nucleation_rate * sp->time_step;
}


//------------------------------------------------------------------------------

void NucleatorProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "nucleate",        nucleation_rate, nucleated_fiber);
    write_param(os, "nucleation_spec", "("+nucleation_spec+")");
    write_param(os, "track_end",       track_end);
    write_param(os, "addictive",       addictive);
}

