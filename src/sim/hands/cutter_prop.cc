// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "property_list.h"
#include "simul_prop.h"
#include "cutter_prop.h"
#include "cutter.h"


Hand * CutterProp::newHand(HandMonitor* h) const
{
    return new Cutter(this, h);
}


void CutterProp::clear()
{
    HandProp::clear();

    cutting_rate = 0;
    new_end_state[0] = STATE_WHITE;
    new_end_state[1] = STATE_WHITE;
}


void CutterProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(cutting_rate,  "cutting_rate");
    
    // possible state of the ends
    KeyList<int> keys("white",  STATE_WHITE,
                      "green",  STATE_GREEN,
                      "yellow", STATE_YELLOW,
                      "orange", STATE_ORANGE,
                      "red",    STATE_RED);
    
    glos.set(new_end_state[0], "new_end_state", keys);
    glos.set(new_end_state[1], "new_end_state", keys, 1);
}


void CutterProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
    
    if ( cutting_rate < 0 )
        throw InvalidParameter("cutter:cutting_rate must be >= 0");

    cutting_rate_prob = 1 - exp( -cutting_rate * sp->time_step );
}


void CutterProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "cutting_rate", cutting_rate);
    write_param(os, "new_end_state", new_end_state, 2);
}

