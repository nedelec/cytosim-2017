// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "property_list.h"
#include "fake_prop.h"
#include "aster_prop.h"
#include "glossary.h"
#include "smath.h"


//------------------------------------------------------------------------------
void FakeProp::clear()
{
    asters.clear();
    aster_prop = 0;
    stiffness  = -1;
}


void FakeProp::read(Glossary& glos)
{
    glos.set(asters,      "asters");
    glos.set(stiffness,   "stiffness");
}


void FakeProp::complete(SimulProp const* sp, PropertyList* plist)
{
    if ( asters.empty() )
        throw InvalidParameter("fake:asters must be specified");
    
    aster_prop = static_cast<AsterProp*>(plist->find_or_die("aster", asters));
    
    if ( stiffness < 0 )
        throw InvalidParameter("fake:stiffness must be specified and >= 0");
}



//------------------------------------------------------------------------------

void FakeProp::write_data(std::ostream & os) const
{
    write_param(os, "asters", asters);
    write_param(os, "stiffness", stiffness);
}

