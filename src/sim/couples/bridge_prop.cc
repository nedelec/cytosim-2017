// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "bridge_prop.h"
#include "bridge.h"

//------------------------------------------------------------------------------
Couple * BridgeProp::newCouple(Glossary * opt) const
{
    //std::cout << "ForkProp::newCouple" << std::endl;
    return new Bridge(this);
}

//------------------------------------------------------------------------------
void BridgeProp::clear()
{
    CoupleProp::clear();
    
    specificity     = BIND_ALWAYS;
    trans_activated = 0;
}

//------------------------------------------------------------------------------
void BridgeProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
    
    glos.set(specificity,     "specificity",
             KeyList<Specificity>("none",         BIND_ALWAYS,
                                  "parallel",     BIND_PARALLEL,
                                  "antiparallel", BIND_ANTIPARALLEL));
    
    glos.set(trans_activated, "trans_activated");

}

//------------------------------------------------------------------------------
void BridgeProp::complete(SimulProp const* sp, PropertyList* plist)
{
    CoupleProp::complete(sp, plist);
    
#if ( DIM > 2 )
    if ( length <= 0 )
        throw InvalidParameter("bridge:length should be defined and > 0");
#endif
    
    //Attachment is impossible if Couple:length is bigger than binding_range
    if ( length > hand_prop1->binding_range )
        Cytosim::warning("bridge:length > Hand1:binding_range\n");
    if ( length > hand_prop2->binding_range )
        Cytosim::warning("bridge:length > Hand2:binding_range\n");    
}

//------------------------------------------------------------------------------

void BridgeProp::write_data(std::ostream & os) const
{
    CoupleProp::write_data(os);
    write_param(os, "specificity", specificity);
    write_param(os, "trans_activated", trans_activated);
}

