// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "dim.h"
#include "messages.h"
#include "exceptions.h"
#include "glossary.h"
#include "common.h"
#include "simul_prop.h"
#include "shackle_prop.h"
#include "shackle.h"
#include "shackle_long.h"

//------------------------------------------------------------------------------
Couple * ShackleProp::newCouple(Glossary *) const
{
    //std::cout << "ShackleProp::newCouple" << std::endl;
    if ( length > 0 )
        return new ShackleLong(this);
    else
        return new Shackle(this);
}

//------------------------------------------------------------------------------
void ShackleProp::clear()
{
    CoupleProp::clear();
}

//------------------------------------------------------------------------------
void ShackleProp::read(Glossary& glos)
{
    CoupleProp::read(glos);

    //glos.set(variable,  "variable");
}

//------------------------------------------------------------------------------
void ShackleProp::complete(SimulProp const* sp, PropertyList* plist)
{
    CoupleProp::complete(sp, plist);
    
    //Attachment is impossible if Couple:length is bigger than binding_range
    if ( length > hand_prop1->binding_range )
        Cytosim::warning("shackle:length > hand1:binding_range\n");
    if ( length > hand_prop2->binding_range )
        Cytosim::warning("shackle:length > hand2:binding_range\n");    
}

//------------------------------------------------------------------------------

void ShackleProp::write_data(std::ostream & os) const
{
    CoupleProp::write_data(os);
    //write_param(os, "variable", variable);
}

