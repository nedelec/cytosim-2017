// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "couple_prop.h"
#include "couple.h"
#include "couple_long.h"
#include "hand_prop.h"
#include "glossary.h"
#include "property_list.h"
#include "simul_prop.h"
#include <cmath>

//------------------------------------------------------------------------------
/**
 This returns a new Couple if ( prop::length <= 0 ),
 or a CoupleLong if ( prop::length > 0 ).
 */
Couple * CoupleProp::newCouple(Glossary *) const
{
    //std::cout << "CoupleProp::newCouple" << std::endl;
    if ( length > 0 )
        return new CoupleLong(this);
    
    return new Couple(this);
}

//------------------------------------------------------------------------------
void CoupleProp::clear()
{
    hand1.clear();
    hand2.clear();
    hand_prop1        = 0;
    hand_prop2        = 0;
    stiffness         = -1;
    length            = 0;
    diffusion         = 0;
    fast_diffusion    = false;
    stiff             = true;
    activity          = "diffuse";
    
    confine           = CONFINE_INSIDE;
    confine_stiff     = 0;
    confine_space     = "first";
    confine_space_ptr = 0;
}


void CoupleProp::read(Glossary& glos)
{
    glos.set(hand1,          "hand1");
    glos.set(hand2,          "hand2");
    glos.set(stiffness,      "stiffness");
    glos.set(length,         "length");
    glos.set(diffusion,      "diffusion");
    glos.set(fast_diffusion, "fast_diffusion");
    glos.set(stiff,          "stiff");
    glos.set(activity,       "activity");
    
    glos.set(confine,        "confine", 
             KeyList<Confinement>("none",    CONFINE_NOT,
                                  "inside",  CONFINE_INSIDE,
                                  "surface", CONFINE_SURFACE));
    //glos.set(confine_stiff,  "confine", 1);
    glos.set(confine_space,  "confine", 2);

#ifdef BACKWARD_COMPATIBILITY
    if ( confine_space == "current" )
        confine_space = "last";
#endif
}


void CoupleProp::complete(SimulProp const* sp, PropertyList* plist)
{
    confine_space_ptr = sp->simul->findSpace(confine_space);
    
    if ( diffusion < 0 )
        throw InvalidParameter("couple:diffusion must be >= 0");
    
    if ( length < 0 )
        throw InvalidParameter("couple:length must be >= 0");

    diffusion_dt = sqrt( 6.0 * diffusion * sp->time_step );

    if ( stiffness < 0 )
        throw InvalidParameter("couple:stiffness must be specified and >= 0");
    
    if ( hand1.empty() )
        throw InvalidParameter("couple:hand1 must be defined");
    hand_prop1 = static_cast<HandProp*>(plist->find_or_die("hand", hand1));
   
    if ( hand2.empty() )
        throw InvalidParameter("couple:hand2 must be defined");
    hand_prop2 = static_cast<HandProp*>(plist->find_or_die("hand", hand2));
    
    if ( sp->strict )
    {
        hand_prop1->checkStiffness(stiffness, length, 2, sp->kT);
        if ( hand_prop2 != hand_prop1 )
            hand_prop2->checkStiffness(stiffness, length, 2, sp->kT);
    }
}


//------------------------------------------------------------------------------

void CoupleProp::write_data(std::ostream & os) const
{
    write_param(os, "hand1",          hand1);
    write_param(os, "hand2",          hand2);
    write_param(os, "stiffness",      stiffness);
    write_param(os, "length",         length);
    write_param(os, "diffusion",      diffusion);
    write_param(os, "fast_diffusion", fast_diffusion);
    write_param(os, "stiff",          stiff);
    write_param(os, "confine",        confine, confine_stiff, confine_space);
    write_param(os, "activity",       activity);
}


