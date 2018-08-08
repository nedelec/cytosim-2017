// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "crosslink_prop.h"
#include "crosslink_long.h"
#include "crosslink.h"

//------------------------------------------------------------------------------
/**
 This returns a new Crosslink if ( prop::length <= 0 ),
 or a CrosslinkLong if ( prop::length > 0 ).
 */
Couple * CrosslinkProp::newCouple(Glossary *) const
{
    //std::cout << "CrosslinkProp::newCouple" << std::endl;
    if ( length > 0 )
        return new CrosslinkLong(this);
    else
        return new Crosslink(this);
}

//------------------------------------------------------------------------------
void CrosslinkProp::clear()
{
    CoupleProp::clear();
    
    specificity     = BIND_ALWAYS;
    trans_activated = 0;
}

//------------------------------------------------------------------------------
void CrosslinkProp::read(Glossary& glos)
{
    CoupleProp::read(glos);
    
    glos.set(specificity,     "specificity",
             KeyList<Specificity>("none",         BIND_ALWAYS,
                                  "parallel",     BIND_PARALLEL,
                                  "antiparallel", BIND_ANTIPARALLEL));
    
    glos.set(trans_activated, "trans_activated");
}

//------------------------------------------------------------------------------
void CrosslinkProp::complete(SimulProp const* sp, PropertyList* plist)
{
    CoupleProp::complete(sp, plist);

    //Attachment is impossible if Couple:length is bigger than binding_range
    if ( length > hand_prop1->binding_range )
        Cytosim::warning("crosslink:length > hand1:binding_range\n");
    if ( length > hand_prop2->binding_range )
        Cytosim::warning("crosslink:length > hand2:binding_range\n");    
}

//------------------------------------------------------------------------------

void CrosslinkProp::write_data(std::ostream & os) const
{
    CoupleProp::write_data(os);
    write_param(os, "specificity",     specificity);
    write_param(os, "trans_activated", trans_activated);
}

