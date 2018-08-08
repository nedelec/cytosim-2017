// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "tracker_prop.h"
#include "tracker.h"

//------------------------------------------------------------------------------
Hand * TrackerProp::newHand(HandMonitor* h) const
{
    return new Tracker(this, h);
}

//------------------------------------------------------------------------------
void TrackerProp::clear()
{
    HandProp::clear();

    bind_end_range        = 0;
    bind_end              = NOT_END;
    track_end             = NOT_END;
    bind_only_growing_end = false;
    bind_only_free_end    = false;
}

//------------------------------------------------------------------------------
void TrackerProp::read(Glossary& glos)
{
    HandProp::read(glos);
    
    glos.set(bind_end,   "bind_end", 
             KeyList<FiberEnd>("plus_end", PLUS_END, "minus_end", MINUS_END, "nearest_end", NEAREST_END));
    
    glos.set(bind_end_range, "bind_end", 1);
    glos.set(bind_end_range, "bind_end_range");
    
    glos.set(track_end,  "track_end",
             KeyList<FiberEnd>("plus_end", PLUS_END, "minus_end", MINUS_END, "nearest_end", NEAREST_END));

    glos.set(bind_only_free_end, "bind_only_free_end");
    glos.set(bind_only_growing_end, "bind_only_growing_end");    
}

//------------------------------------------------------------------------------
void TrackerProp::complete(SimulProp const* sp, PropertyList* plist)
{
    HandProp::complete(sp, plist);
}


//------------------------------------------------------------------------------

void TrackerProp::write_data(std::ostream & os) const
{
    HandProp::write_data(os);
    write_param(os, "bind_end",              bind_end, bind_end_range);
    write_param(os, "track_end",             track_end);
    write_param(os, "bind_only_free_end",    bind_only_free_end);
    write_param(os, "bind_only_growing_end", bind_only_growing_end);
}

