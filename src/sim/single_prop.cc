// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include "single_prop.h"
#include "glossary.h"
#include "messages.h"

#include "property_list.h"
#include "simul_prop.h"
#include "hand_prop.h"
#include "single.h"
#include "wrist.h"
#include "wrist_long.h"
#include "picket.h"
#include "picket_long.h"

/**
 @defgroup SingleGroup Single and Derived Activities
 @ingroup ObjectGroup
 @ingroup NewObject
 @brief A Single contains one Hand, and can thus bind to one Fiber.

 List of classes accessible by specifying single:activity:
 
 `activity`    |   Class       | Description
 --------------|---------------|--------------------------------------------
 `diffuse`     | Single        | a single Hand that is mobile (default)
 `fixed`       | Picket        | a single Hand anchored at a fixed position
 
 Another class Wrist is used automatically to anchor a Single to a Mecable.
 
 Example:
 @code
 set single grafted
 {
   hand = kinesin
   stiffness = 100
   activity = fixed
 } 
 @endcode
 */
Single * SingleProp::newSingle(Glossary * opt) const
{
    //std::cout << "SingleProp::newSingle" << std::endl;
    if ( activity == "fixed" )
    {
        if ( length > 0 )
            return new PicketLong(this);
        else
            return new Picket(this);
    }
    else if ( activity == "diffuse" )
    {
        return new Single(this);
    }
    else 
        throw InvalidParameter("unknown Single activity `"+activity+"'");
    return new Single(this);
}


/**
 The Wrist requires a anchor point to be created
 */
Wrist * SingleProp::newWrist(Mecable const* mec, const unsigned point) const
{
    //std::clog << "SingleProp::newWrist" << std::endl;
    if ( length > 0 )
        return new WristLong(this, mec, point);
    else
        return new Wrist(this, mec, point);
}


//------------------------------------------------------------------------------
#pragma mark -

void SingleProp::clear()
{
    hand.clear();
    hand_prop         = 0;
    stiffness         = 0;
    length            = 0;
    diffusion         = 0;
    activity          = "diffuse";
    
    confine           = CONFINE_INSIDE;
    confine_stiff     = 0;
    confine_space     = "first";
    confine_space_ptr = 0;
}


void SingleProp::read(Glossary& glos)
{
    glos.set(hand,      "hand");
    glos.set(stiffness, "stiffness");
    glos.set(length,    "length");
    glos.set(diffusion, "diffusion");
    glos.set(activity,  "activity");

    glos.set(confine,   "confine", 
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


void SingleProp::complete(SimulProp const* sp, PropertyList* plist)
{
    confine_space_ptr = sp->simul->findSpace(confine_space);
    
    if ( hand.empty() )
        throw InvalidParameter("single:hand must be defined");
    hand_prop = static_cast<HandProp*>(plist->find_or_die("hand", hand));
    
    if ( hand_prop == 0 )
        throw InvalidParameter("unknown single:hand '"+hand+"'");

    if ( diffusion < 0 )
        throw InvalidParameter("single:diffusion must be >= 0");

    diffusion_dt = sqrt( 6.0 * diffusion * sp->time_step );
    
    if ( stiffness < 0 )
        throw InvalidParameter("single:stiffness must be >= 0");

    if ( length < 0 )
        throw InvalidParameter("single:length must be >= 0");

    if ( stiffness > 0  &&  sp->strict )
    {
        hand_prop->checkStiffness(stiffness, length, 1, sp->kT);
    }
    
    // Attachment is impossible if Single::length is bigger than binding_range
    if ( length > hand_prop->binding_range )
        Cytosim::warning("Single:length > Hand1:binding_range\n");    
}


//------------------------------------------------------------------------------

void SingleProp::write_data(std::ostream & os) const
{
    write_param(os, "hand",      hand);
    write_param(os, "stiffness", stiffness);
    write_param(os, "length",    length);
    write_param(os, "diffusion", diffusion);
    write_param(os, "confine",   confine, confine_stiff, confine_space);
    write_param(os, "activity",  activity);
}

