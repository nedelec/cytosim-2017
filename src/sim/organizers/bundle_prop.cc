// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "bundle_prop.h"
#include "fiber_prop.h"
#include "property_list.h"
#include "glossary.h"
#include "smath.h"


//------------------------------------------------------------------------------
void BundleProp::clear()
{
    fibers.clear();
    fiber_prop = 0;
    nb_fibers  = 0;
    stiffness  = -1;
    overlap    = -1;
    focus      = MINUS_END;
    nucleate   = true;
}


void BundleProp::read(Glossary& glos)
{
    glos.set(fibers,       "fibers");
    glos.set(nb_fibers,    "nb_fibers");
    glos.set(stiffness,    "stiffness");
    glos.set(overlap,      "overlap");
    glos.set(focus,        "focus", KeyList<FiberEnd>("plus_end", PLUS_END, "minus_end", MINUS_END));
    glos.set(nucleate,     "nucleate");
}


void BundleProp::complete(SimulProp const* sp, PropertyList* plist)
{
    if ( fibers.empty() )
        throw InvalidParameter("bundle:fibers must be specified");
    
    fiber_prop = static_cast<FiberProp*>(plist->find_or_die("fiber", fibers));
    
    if ( nb_fibers <= 0 )
        throw InvalidParameter("bundle:nb_fibers must be specified and > 0");
    
    if ( overlap < 0 )
        throw InvalidParameter("bundle:overlap must be specified and >= 0");
    
    if ( stiffness < 0 )
        throw InvalidParameter("bundle:stiffness must be specified and >= 0");
}



//------------------------------------------------------------------------------

void BundleProp::write_data(std::ostream & os) const
{
    write_param(os, "fibers",    fibers);
    write_param(os, "nb_fibers", nb_fibers);
    write_param(os, "stiffness", stiffness);
    write_param(os, "overlap",   overlap);
    write_param(os, "focus",     focus);
    write_param(os, "nucleate",  nucleate);
}

